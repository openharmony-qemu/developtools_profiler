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
#include "jsleak/snapshot_parser.h"
#include "shared/heapsnapshot/deobfuscator.h"
#include "shared/heapsnapshot/rawheap_converter.h"
#include "utils/platform.h"
#include "utils/file_utils.h"
#include "utils/temp_dir.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"

#include <iostream>
#include <stdexcept>

namespace mem_analyzer {

namespace {
// Step 1: rawheap → heapsnapshot（输出到 convert 子目录，父目录由转换函数创建）
std::string ConvertRawheap(const std::string& filePath, const std::string& tempBase)
{
    log::Logi("  Converting rawheap to heapsnapshot...");
    std::string heapsnapshotFile = platform::JoinPath(
        platform::JoinPath(tempBase, "convert"),
        platform::Stem(filePath) + ".heapsnapshot");
    std::string convertErr;
    if (!RawheapToHeapsnapshot(filePath, heapsnapshotFile, &convertErr)) {
        if (!convertErr.empty()) {
            log::Loge("Error: ", convertErr);
        }
        throw std::runtime_error("Failed to convert the file " + filePath +
                                 " to heapsnapshot format!");
    }
    return heapsnapshotFile;
}
} // namespace

HeapSnapshot JsleakParse(const std::string& filePath, const RunContext& ctx)
{
    std::string heapsnapshotFile = filePath;

    const bool needDeobfuscate =
        !ctx.resources.sourcemapDir.empty() || !ctx.resources.nameCacheDir.empty();
    bool fromRawheap = EndsWithIgnoreCase(filePath, "rawheap");

    // 函数级 RAII：rawheap 转换/反混淆产物须存活到下方 HeapSnapshot::from_file
    // 完成之后，故 temp_dir 不能放在 if 块内（否则块结束即清理，读取将失败）。
    TempDir tempDir;
    if (fromRawheap || needDeobfuscate) {
        // rawheap 转换和/或反混淆：中间产物放入系统 TempDir（RAII 自动清理），
        // 反混淆产物保持原始 stem 名，使下游无需感知反混淆。
        if (!tempDir.Valid()) {
            tempDir = TempDir("snapshot");
        }
        const std::string& tempBase = tempDir.Valid() ? tempDir.Path() : ctx.output.outDir;
        std::string source = filePath;

        // Step 1: rawheap → heapsnapshot
        if (fromRawheap) {
            heapsnapshotFile = ConvertRawheap(filePath, tempBase);
            source = heapsnapshotFile;
        }

        // Step 2: deobfuscate（保持原始 stem 名，失败时降级为转换后的快照；
        // deob 目录由 deobfuscate_with_indices 自行创建）
        if (needDeobfuscate) {
            log::Logi("  Deobfuscating snapshot...");
            std::string deobDir = platform::JoinPath(tempBase, "deob");
            std::string deobPath = platform::JoinPath(
                deobDir, platform::Stem(filePath) + ".heapsnapshot");
            if (DeobfuscateWithIndices(source, deobPath,
                ctx.resources.sourcemapDir,
                ctx.resources.nameCacheDir)) {
                heapsnapshotFile = deobPath;
            }
            // 反混淆失败时 heapsnapshot_file 保持为 Step 1 的转换结果（或原始输入）
        }

        std::error_code ec;
        if (!fs::is_regular_file(mem_analyzer::PathFromUtf8(heapsnapshotFile), ec)) {
            throw std::runtime_error("Heapsnapshot file " + heapsnapshotFile + " doesn't exist!");
        }
    }

    // 加载失败（文件不存在/格式非法）由 HeapSnapshot::from_file 抛异常，由模块统一打印
    return HeapSnapshot::FromFile(heapsnapshotFile);
}

} // namespace mem_analyzer
