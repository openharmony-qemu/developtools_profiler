/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "tshandle/zip_parser.hpp"
#include "tshandle/arkts_analyzer.hpp"
#include "utils/platform.hpp"
#include "utils/zip_utils.hpp"
#include "utils/file_utils.hpp"
#include "utils/timer.hpp"
#include "utils/temp_dir.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "shared/htrace_model/htrace_reader.hpp"
#include "shared/htrace_model/trace_streamer_runner.hpp"
#include "shared/heapsnapshot/deobfuscator.hpp"
#include "shared/heapsnapshot/rawheap_converter.hpp"
#include "formatter/json/json_writer.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace mem_analyzer {

/** Find first file with given extension in directory.
 *  When `name_prefix` is non-empty, only files whose stem (filename without
 *  extension) starts with it (case-insensitively) are considered — used so the
 *  .txt fallback only picks a js_map*.txt address file, not an arbitrary .txt.
 */
static std::string MatchFileEntry(const fs::directory_entry& entry, const std::string& ext,
    const std::string& namePrefix)
{
    if (entry.is_regular_file()) {
        auto path = PathToUtf8(entry.path());
        auto fileExt = PathToUtf8(entry.path().extension());
        // Normalize extension to lowercase for comparison
        for (auto& c : fileExt) {
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        }
        std::string targetExt = ext;
        for (auto& c : targetExt) {
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        }
        if (fileExt == targetExt) {
            if (!namePrefix.empty()) {
                std::string stem = PathToUtf8(entry.path().stem());
                for (auto& c : stem) {
                    c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
                }
                std::string lowerPrefix = namePrefix;
                for (auto& c : lowerPrefix) {
                    c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
                }
                if (stem.rfind(lowerPrefix, 0) != 0) {
                    return "";
                }
            }
            return path;
        }
    }
    return "";
}

static std::string FindFileInDir(const std::string& dir, const std::string& ext, const std::string& namePrefix = "")
{
    if (!platform::DirectoryExists(dir)) {
        return "";
    }
    auto dirPath = PathFromUtf8(dir);
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        std::string matched = MatchFileEntry(entry, ext, namePrefix);
        if (!matched.empty()) {
            return matched;
        }
    }
    // Also search subdirectories
    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        std::string matched = MatchFileEntry(entry, ext, namePrefix);
        if (!matched.empty()) {
            return matched;
        }
    }
    return "";
}

static std::string FindHtraceFileInDir(
    const std::string& dir, const std::string& rawheapPath, const std::string& mapPath)
{
    if (!platform::DirectoryExists(dir)) {
        return "";
    }
    auto dirPath = PathFromUtf8(dir);
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            auto path = PathToUtf8(entry.path());
            if (path != rawheapPath && path != mapPath && HtraceReader::IsProfilerFile(path)) {
                return path;
            }
        }
    }
    // Also search subdirectories
    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            auto path = PathToUtf8(entry.path());
            if (path != rawheapPath && path != mapPath && HtraceReader::IsProfilerFile(path)) {
                return path;
            }
        }
    }
    return "";
}

/// 若配置了反混淆索引（--sourcemap 与 --namecache），将解出的
/// heapsnapshot 还原为源码路径/行号，返回用于后续分析的快照路径；
/// 未配置或还原失败时降级返回原始快照路径。
/// 反混淆产物与中间 heapsnapshot 均写入 output_dir（系统 TempDir），由调用方清理。
static std::string DeobfuscateZipSnapshot(const std::string& heapsnapshotPath,
    const std::string& outputDir,
    const std::string& sourcemapDir,
    const std::string& namecacheDir)
{
    std::string outputPath = platform::JoinPath(
        outputDir, platform::Stem(heapsnapshotPath) + "_deobfuscated.heapsnapshot");
    if (!mem_analyzer::DeobfuscateWithIndices(heapsnapshotPath, outputPath,
        sourcemapDir, namecacheDir)) {
        // 未配置反混淆目录，或索引加载为空，或还原失败 → 降级为原始快照
        if (!sourcemapDir.empty() || !namecacheDir.empty()) {
            log::Logw("  Warning: Failed to deobfuscate snapshot, proceeding with the original.");
        }
        return heapsnapshotPath;
    }
    log::Logok("    Deobfuscated snapshot: ", outputPath);
    return outputPath;
}

TshandleParseResult TshandleParse(const std::string& filePath, const RunContext& ctx)
{
    StageTimer timer("Parse zip");

    // Temp directory (in system temp dir, not output dir)
    TempDir tempDir("tshandle");

    // ── Step 1: Extract ZIP into hiprofiler/ subdir ──────────────
    log::Logi("  [1/4] Extracting ZIP: ", filePath);
    timer.Stage("Extracting ZIP");
    auto unzipDir = platform::JoinPath(tempDir.Path(), "hiprofiler");
    platform::CreateDirectories(unzipDir);
    {
        std::string zipErr;
        if (!zipUtils::ExtractZip(filePath, unzipDir, zipErr)) {
            throw std::runtime_error(zipErr.empty() ? "Failed to extract ZIP file: " + filePath :
                                                     zipErr);
        }
        log::Logi("    Extracted to: ", unzipDir);
    }
    timer.Tick("extract");

    // ── Step 2: Locate .htrace, .rawheap, js_map files ────────────
    std::string rawheapPath = FindFileInDir(unzipDir, ".rawheap");
    std::string addrPath = FindFileInDir(unzipDir, ".binpb");
    if (addrPath.empty()) {
        addrPath = FindFileInDir(unzipDir, ".txt", "js_map");
    }
    std::string htracePath = FindHtraceFileInDir(unzipDir, rawheapPath, addrPath);
    if (htracePath.empty()) {
        throw std::runtime_error("No .htrace file found in extracted ZIP");
    }
    if (rawheapPath.empty()) {
        log::Logw("  Warning: No .rawheap file found in ZIP");
    }
    if (addrPath.empty()) {
        log::Logw("  Warning: No js_map file (.binpb/.txt) found in ZIP");
    }
    log::Logi("    Found: ", platform::Basename(htracePath),
        (!rawheapPath.empty() ? ", " + platform::Basename(rawheapPath) : ""),
        (!addrPath.empty() ? ", " + platform::Basename(addrPath) : ""));

    // ── Step 3: Run trace_streamer → DB ─────────────────────────
    log::Logi("  [2/4] Converting htrace to DB using trace_streamer...");
    std::string dbPath;
    {
        auto htraceName = platform::Stem(htracePath);
        dbPath = platform::JoinPath(tempDir.Path(), htraceName + "_trace_streamer.db");

        std::string streamerErr;
        if (!RunTraceStreamer(htracePath, dbPath, ctx.resources.soDir, &streamerErr)) {
            throw std::runtime_error(streamerErr.empty() ? "trace_streamer conversion failed" :
                                                           streamerErr);
        }
    }
    timer.Tick("trace_streamer");

    // ── Step 4: Process DB → callStacks (ARK handle events) ─────
    log::Logi("  [3/4] Extracting ARK handle call stacks from DB...");
    TshandleParseResult parseResult;
    parseResult.parsedAt = FormatReadableTimestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

    try {
        parseResult.db = ProcessTshandleDb(dbPath);
        if (parseResult.db.meta.processName.empty()) {
            std::string nm = HtraceReader::ReadProcessName(htracePath);
            if (!nm.empty()) {
                parseResult.db.meta.processName = std::move(nm);
            }
        }
        log::Logi("    Found ", parseResult.CallStacks().size(), " call stacks.");
    } catch (const std::exception& e) {
        log::Logw("    Warning: ", e.what(), " (returning empty call stacks)");
        parseResult.db.stacks = ResolvedCallStackList{};
    }

    // ── Step 5: Run rawheap_translator (optional) ────────────────
    log::Logi("  [4/4] Processing rawheap (if available)...");
    if (!rawheapPath.empty()) {
        try {
            auto rawheapName = platform::Stem(rawheapPath);
            // rawheap→heapsnapshot 中间产物放入系统 TempDir（RAII 自动清理），不再落 out 目录
            auto heapsnapshotPath = platform::JoinPath(tempDir.Path(), rawheapName + ".heapsnapshot");

            std::string convertErr;
            if (!RawheapToHeapsnapshot(rawheapPath, heapsnapshotPath, &convertErr)) {
                log::Logw("    Warning: ", convertErr);
            } else {
                log::Logok("    Heap snapshot generated: ", heapsnapshotPath);

                // 指定了 --sourcemap 或 --namecache 时，先还原快照（源码路径/行号）再做类分析
                std::string analyzeSnapshot =
                    DeobfuscateZipSnapshot(heapsnapshotPath, tempDir.Path(),
                        ctx.resources.sourcemapDir,
                        ctx.resources.nameCacheDir);

                auto tArktsStart = std::chrono::steady_clock::now();
                ArkTsAnalyzeResult arktsResult = AnalyzeArktsClasses(
                    analyzeSnapshot, addrPath, parseResult.CallStacks());
                auto tArktsEnd = std::chrono::steady_clock::now();
                auto arktsMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    tArktsEnd - tArktsStart).count();
                log::Logok("  Analyzed ArkTS classes (", arktsMs, "ms). ",
                    arktsResult.arktsClasses.size(), " classes.");

                parseResult.arktsClasses = std::move(arktsResult.arktsClasses);

                // Write anomaly report BEFORE moving anomaly data
                platform::CreateDirectories(ctx.output.outDir);
                WriteAnomalyReport(ctx.output.outDir, arktsResult);

                parseResult.anomalyCounts = arktsResult.anomalyCounts;
                parseResult.anomalyLines = std::move(arktsResult.anomalyLines);
            }
        } catch (const std::exception& e) {
            log::Logw("    Warning: rawheap processing failed: ", e.what());
        }
    } else {
        log::LogiMinor("    No rawheap file, skipping.");
    }
    timer.Tick("rawheap");
    timer.Done();

    return parseResult;
}

} // namespace mem_analyzer
