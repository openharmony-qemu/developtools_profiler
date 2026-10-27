#include "jsleak/snapshot_parser.hpp"
#include "shared/heapsnapshot/deobfuscator.hpp"
#include "shared/heapsnapshot/rawheap_converter.hpp"
#include "utils/platform.hpp"
#include "utils/file_utils.hpp"
#include "utils/temp_dir.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"

#include <iostream>
#include <stdexcept>

namespace mem_analyzer {

HeapSnapshot jsleak_parse(const std::string& file_path, const RunContext& ctx) {
    std::string heapsnapshot_file = file_path;

    const bool need_deobfuscate =
        !ctx.resources.sourcemap_dir.empty() || !ctx.resources.name_cache_dir.empty();
    bool from_rawheap = ends_with_ignore_case(file_path, "rawheap");

    // 函数级 RAII：rawheap 转换/反混淆产物须存活到下方 HeapSnapshot::from_file
    // 完成之后，故 temp_dir 不能放在 if 块内（否则块结束即清理，读取将失败）。
    TempDir temp_dir;
    if (from_rawheap || need_deobfuscate) {
        // rawheap 转换和/或反混淆：中间产物放入系统 TempDir（RAII 自动清理），
        // 反混淆产物保持原始 stem 名，使下游无需感知反混淆。
        if (!temp_dir.valid()) {
            temp_dir = TempDir("snapshot");
        }
        const std::string& temp_base = temp_dir.valid() ? temp_dir.path() : ctx.output.out_dir;
        std::string source = file_path;

        // Step 1: rawheap → heapsnapshot（输出到 convert 子目录，父目录由转换函数创建）
        if (from_rawheap) {
            log::logi("  Converting rawheap to heapsnapshot...");
            heapsnapshot_file = platform::join_path(
                platform::join_path(temp_base, "convert"),
                platform::stem(file_path) + ".heapsnapshot");
            std::string convert_err;
            if (!rawheap_to_heapsnapshot(file_path, heapsnapshot_file, &convert_err)) {
                if (!convert_err.empty()) {
                    log::loge("Error: ", convert_err);
                }
                throw std::runtime_error("Failed to convert the file " + file_path +
                                         " to heapsnapshot format!");
            }
            source = heapsnapshot_file;
        }

        // Step 2: deobfuscate（保持原始 stem 名，失败时降级为转换后的快照；
        // deob 目录由 deobfuscate_with_indices 自行创建）
        if (need_deobfuscate) {
            log::logi("  Deobfuscating snapshot...");
            std::string deob_dir = platform::join_path(temp_base, "deob");
            std::string deob_path = platform::join_path(
                deob_dir, platform::stem(file_path) + ".heapsnapshot");
            if (deobfuscate_with_indices(source, deob_path,
                                          ctx.resources.sourcemap_dir,
                                          ctx.resources.name_cache_dir)) {
                heapsnapshot_file = deob_path;
            }
            // 反混淆失败时 heapsnapshot_file 保持为 Step 1 的转换结果（或原始输入）
        }

        std::error_code ec;
        if (!fs::is_regular_file(mem_analyzer::path_from_utf8(heapsnapshot_file), ec)) {
            throw std::runtime_error("Heapsnapshot file " + heapsnapshot_file + " doesn't exist!");
        }
    }

    // 加载失败（文件不存在/格式非法）由 HeapSnapshot::from_file 抛异常，由模块统一打印
    return HeapSnapshot::from_file(heapsnapshot_file);
}

} // namespace mem_analyzer
