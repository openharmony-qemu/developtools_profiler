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
static std::string find_file_in_dir(const std::string& dir, const std::string& ext,
                                    const std::string& name_prefix = "") {
    if (!platform::directory_exists(dir)) return "";
    auto dir_path = path_from_utf8(dir);
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            auto path = path_to_utf8(entry.path());
            auto file_ext = path_to_utf8(entry.path().extension());
            // Normalize extension to lowercase for comparison
            for (auto& c : file_ext) c = (char)tolower((unsigned char)c);
            std::string target_ext = ext;
            for (auto& c : target_ext) c = (char)tolower((unsigned char)c);
            if (file_ext == target_ext) {
                if (!name_prefix.empty()) {
                    std::string stem = path_to_utf8(entry.path().stem());
                    for (auto& c : stem) c = (char)tolower((unsigned char)c);
                    std::string lower_prefix = name_prefix;
                    for (auto& c : lower_prefix) c = (char)tolower((unsigned char)c);
                    if (stem.rfind(lower_prefix, 0) != 0) continue;
                }
                return path;
            }
        }
    }
    // Also search subdirectories
    for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            auto path = path_to_utf8(entry.path());
            auto file_ext = path_to_utf8(entry.path().extension());
            for (auto& c : file_ext) c = (char)tolower((unsigned char)c);
            std::string target_ext = ext;
            for (auto& c : target_ext) c = (char)tolower((unsigned char)c);
            if (file_ext == target_ext) {
                if (!name_prefix.empty()) {
                    std::string stem = path_to_utf8(entry.path().stem());
                    for (auto& c : stem) c = (char)tolower((unsigned char)c);
                    std::string lower_prefix = name_prefix;
                    for (auto& c : lower_prefix) c = (char)tolower((unsigned char)c);
                    if (stem.rfind(lower_prefix, 0) != 0) continue;
                }
                return path;
            }
        }
    }
    return "";
}

static std::string find_htrace_file_in_dir(
    const std::string& dir, const std::string& rawheap_path, const std::string& map_path) {
    if (!platform::directory_exists(dir)) return "";
    auto dir_path = path_from_utf8(dir);
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            auto path = path_to_utf8(entry.path());
            if (path != rawheap_path && path != map_path && HtraceReader::is_profiler_file(path)) {
                return path;
            }
        }
    }
    // Also search subdirectories
    for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            auto path = path_to_utf8(entry.path());
            if (path != rawheap_path && path != map_path && HtraceReader::is_profiler_file(path)) {
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
static std::string deobfuscate_zip_snapshot(const std::string& heapsnapshot_path,
                                             const std::string& output_dir,
                                             const std::string& sourcemap_dir,
                                             const std::string& namecache_dir) {
    std::string output_path = platform::join_path(
        output_dir, platform::stem(heapsnapshot_path) + "_deobfuscated.heapsnapshot");
    if (!mem_analyzer::deobfuscate_with_indices(heapsnapshot_path, output_path,
                                                 sourcemap_dir, namecache_dir)) {
        // 未配置反混淆目录，或索引加载为空，或还原失败 → 降级为原始快照
        if (!sourcemap_dir.empty() || !namecache_dir.empty()) {
            log::logw("  Warning: Failed to deobfuscate snapshot, proceeding with the original.");
        }
        return heapsnapshot_path;
    }
    log::logok("    Deobfuscated snapshot: ", output_path);
    return output_path;
}

TshandleParseResult tshandle_parse(const std::string& file_path, const RunContext& ctx) {
    StageTimer timer("Parse zip");

    // Temp directory (in system temp dir, not output dir)
    TempDir temp_dir("tshandle");

    // ── Step 1: Extract ZIP into hiprofiler/ subdir ──────────────
    log::logi("  [1/4] Extracting ZIP: ", file_path);
    timer.stage("Extracting ZIP");
    auto unzip_dir = platform::join_path(temp_dir.path(), "hiprofiler");
    platform::create_directories(unzip_dir);
    {
        std::string zip_err;
        if (!zip_utils::extract_zip(file_path, unzip_dir, zip_err)) {
            throw std::runtime_error(zip_err.empty() ? "Failed to extract ZIP file: " + file_path
                                                     : zip_err);
        }
        log::logi("    Extracted to: ", unzip_dir);
    }
    timer.tick("extract");

    // ── Step 2: Locate .htrace, .rawheap, js_map files ────────────
    std::string rawheap_path = find_file_in_dir(unzip_dir, ".rawheap");
    std::string addr_path = find_file_in_dir(unzip_dir, ".binpb");
    if (addr_path.empty()) {
        addr_path = find_file_in_dir(unzip_dir, ".txt", "js_map");
    }
    std::string htrace_path = find_htrace_file_in_dir(unzip_dir, rawheap_path, addr_path);
    if (htrace_path.empty()) {
        throw std::runtime_error("No .htrace file found in extracted ZIP");
    }
    if (rawheap_path.empty()) {
        log::logw("  Warning: No .rawheap file found in ZIP");
    }
    if (addr_path.empty()) {
        log::logw("  Warning: No js_map file (.binpb/.txt) found in ZIP");
    }
    log::logi("    Found: ", platform::basename(htrace_path),
              (!rawheap_path.empty() ? ", " + platform::basename(rawheap_path) : ""),
              (!addr_path.empty() ? ", " + platform::basename(addr_path) : ""));

    // ── Step 3: Run trace_streamer → DB ─────────────────────────
    log::logi("  [2/4] Converting htrace to DB using trace_streamer...");
    std::string db_path;
    {
        auto htrace_name = platform::stem(htrace_path);
        db_path = platform::join_path(temp_dir.path(), htrace_name + "_trace_streamer.db");

        std::string streamer_err;
        if (!run_trace_streamer(htrace_path, db_path, ctx.resources.so_dir, &streamer_err)) {
            throw std::runtime_error(streamer_err.empty() ? "trace_streamer conversion failed"
                                                           : streamer_err);
        }
    }
    timer.tick("trace_streamer");

    // ── Step 4: Process DB → callStacks (ARK handle events) ─────
    log::logi("  [3/4] Extracting ARK handle call stacks from DB...");
    TshandleParseResult parse_result;
    parse_result.parsed_at = format_readable_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

    try {
        parse_result.db = process_tshandle_db(db_path);
        if (parse_result.db.meta.process_name.empty()) {
            std::string nm = HtraceReader::read_process_name(htrace_path);
            if (!nm.empty()) parse_result.db.meta.process_name = std::move(nm);
        }
        log::logi("    Found ", parse_result.call_stacks().size(), " call stacks.");
    } catch (const std::exception& e) {
        log::logw("    Warning: ", e.what(), " (returning empty call stacks)");
        parse_result.db.stacks = ResolvedCallStackList{};
    }

    // ── Step 5: Run rawheap_translator (optional) ────────────────
    log::logi("  [4/4] Processing rawheap (if available)...");
    if (!rawheap_path.empty()) {
        try {
            auto rawheap_name = platform::stem(rawheap_path);
            // rawheap→heapsnapshot 中间产物放入系统 TempDir（RAII 自动清理），不再落 out 目录
            auto heapsnapshot_path = platform::join_path(temp_dir.path(), rawheap_name + ".heapsnapshot");

            std::string convert_err;
            if (!rawheap_to_heapsnapshot(rawheap_path, heapsnapshot_path, &convert_err)) {
                log::logw("    Warning: ", convert_err);
            } else {
                log::logok("    Heap snapshot generated: ", heapsnapshot_path);

                // 指定了 --sourcemap 或 --namecache 时，先还原快照（源码路径/行号）再做类分析
                std::string analyze_snapshot =
                    deobfuscate_zip_snapshot(heapsnapshot_path, temp_dir.path(),
                                              ctx.resources.sourcemap_dir,
                                              ctx.resources.name_cache_dir);

                auto t_arkts_start = std::chrono::steady_clock::now();
                ArkTsAnalyzeResult arkts_result = analyze_arkts_classes(
                    analyze_snapshot, addr_path, parse_result.call_stacks());
                auto t_arkts_end = std::chrono::steady_clock::now();
                auto arkts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    t_arkts_end - t_arkts_start).count();
                log::logok("  Analyzed ArkTS classes (", arkts_ms, "ms). ", arkts_result.arkts_classes.size(), " classes.");

                parse_result.arkts_classes = std::move(arkts_result.arkts_classes);

                // Write anomaly report BEFORE moving anomaly data
                platform::create_directories(ctx.output.out_dir);
                write_anomaly_report(ctx.output.out_dir, arkts_result);

                parse_result.anomaly_counts = arkts_result.anomaly_counts;
                parse_result.anomaly_lines = std::move(arkts_result.anomaly_lines);
            }
        } catch (const std::exception& e) {
            log::logw("    Warning: rawheap processing failed: ", e.what());
        }
    } else {
        log::logi_minor("    No rawheap file, skipping.");
    }
    timer.tick("rawheap");
    timer.done();

    return parse_result;
}

} // namespace mem_analyzer
