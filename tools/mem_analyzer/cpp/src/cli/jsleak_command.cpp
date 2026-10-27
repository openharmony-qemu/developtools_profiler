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
#include "cli/command.hpp"
#include "cli/pipeline_runner.hpp"
#include "config.hpp"
#include "core/module_registry.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"

namespace mem_analyzer {
namespace {

struct JsleakOptions {
    std::string input_file;
    std::string baseline_file;
    bool baseline_provided = false;
    int top_n = defaults::JSLEAK_TOP_N;
    std::string list_file;
    bool list_file_provided = false;
    std::string mode = defaults::JSLEAK_MODE;
    std::string sourcemap_dir;
    std::string name_cache_dir;
};

class JsleakCommand final : public ICommand {
    JsleakOptions opt_;

public:
    std::string name() const override { return "jsleak"; }
    std::string alias() const override { return "jl"; }
    std::string description() const override { return "Process heapsnapshot/rawheap files"; }
    int order() const override { return 30; }

    void bind(CliParser& sub, GlobalOptions& g) override {
        // 注意：jsleak 不注册 -p/--parse-only（保持既有 CLI 契约不变），
        // 因此这里不用 bind_common_options。
        sub.set_usage("[options] <snapshot>")
           .add_positional("files", opt_.input_file, "Path to .heapsnapshot/.rawheap file")
           .add_option("-b,--baseline", opt_.baseline_file,
                       "Baseline snapshot for comparison",
                       &opt_.baseline_provided)
           .add_option("-d,--out-dir", g.out_dir, "Output result directory")
           .add_switch("-o,--auto-open", g.auto_open, "Open result file after processing");
        // 诊断三件套（--verbose/--keep-temp/--debug）：与 trace 一致，效果由 main 统一处理。
        bind_debug_options(sub, g);
        sub.add_option("-f,--format", g.format, "Output format (json/html)")
           .add_option("-n,--top-n", opt_.top_n, "Number of top reference chains to output")
           .add_option("-l,--leaklist", opt_.list_file, "Path to .jsleaklist file",
                       &opt_.list_file_provided)
           .add_option("-m,--mode", opt_.mode,
                       "Analysis mode: all, property-tag/pt, top-chain/tc")
           .add_option("--sourcemap", opt_.sourcemap_dir,
                       "Source map file or directory (sourceMaps.map / *.map) to deobfuscate "
                       "heapsnapshot paths")
           .add_option("--namecache", opt_.name_cache_dir,
                       "nameCache file or directory (nameCache.json) to deobfuscate heapsnapshot "
                       "names");
    }

    int run(const GlobalOptions& g) override {
        // Validate mode（缩写归一化与历史 Config 行为一致）
        if (opt_.mode != "all" && opt_.mode != "property-tag" && opt_.mode != "pt" &&
            opt_.mode != "top-chain" && opt_.mode != "tc") {
            log::loge("Error: invalid mode '", opt_.mode, "'. Valid: all, property-tag/pt, top-chain/tc");
            return 1;
        }
        const std::string normalized_mode = opt_.mode == "pt"   ? "property-tag"
                                            : opt_.mode == "tc" ? "top-chain"
                                                                : opt_.mode;
        if (opt_.baseline_provided) return run_compare(g, normalized_mode);

        if (opt_.input_file.empty()) {
            log::loge("Error: jsleak requires a snapshot file");
            return 1;
        }

        log::logi(style::bold("=== Jsleak Analyzer Command ==="));
        log::logi("  input file: ", opt_.input_file);

        RunContext ctx = make_base_run_context(g);
        // 反混淆参数交给 jsleak_parse() 内部处理
        ctx.resources.sourcemap_dir = opt_.sourcemap_dir;
        ctx.resources.name_cache_dir = opt_.name_cache_dir;
        ctx.snapshot.leak_list_file = opt_.list_file;
        // 与历史 Config::set_jsleak_top_n / set_jsleak_mode 行为一致：
        // 非正 top_n 回退默认，模式缩写归一化
        ctx.snapshot.top_n = opt_.top_n > 0 ? opt_.top_n : defaults::JSLEAK_TOP_N;
        ctx.snapshot.mode = normalized_mode;

        auto* module = ModuleRegistry::find_by_extension("heapsnapshot");
        if (!module) {
            log::loge("Error: No parser found for heapsnapshot files");
            return 1;
        }
        if (!process_file(opt_.input_file, *module, ctx)) return 1;

        log::logi(style::bold("=== Jsleak analysis completed ==="));
        return 0;
    }

private:
    int run_compare(const GlobalOptions& g, const std::string& normalized_mode) {
        if (opt_.input_file.empty()) {
            log::loge("Error: jsleak -b/--baseline requires a target snapshot file");
            return 1;
        }
        if (opt_.baseline_file.empty()) {
            log::loge("Error: -b/--baseline requires a baseline snapshot file path");
            return 1;
        }
        if (opt_.list_file_provided) {
            log::loge("Error: --leaklist is not supported with -b/--baseline");
            return 1;
        }
        if (g.format == "json") {
            log::loge("Error: -f json is not supported with -b/--baseline yet");
            return 1;
        }
        if (!g.format.empty() && g.format != "html") {
            log::loge("Error: unsupported format '", g.format,
                      "' for -b/--baseline. Valid: html");
            return 1;
        }

        const std::string& baseline = opt_.baseline_file;
        const std::string& target = opt_.input_file;
        log::logi(style::bold("=== Snapshot Compare Command ==="));
        log::logi("  A (baseline): ", baseline);
        log::logi("  B (target):   ", target);
        log::logi("  mode:         ", normalized_mode);

        // 对比走与单文件一致的通用管道：process_file → module.run。
        // 输出目录嵌套与单文件相同（out/<目标文件 stem>/），
        // 输出文件名由 formatter 从 report 的 baseline/target 文件名推导。
        RunContext ctx = make_base_run_context(g);
        ctx.resources.sourcemap_dir = opt_.sourcemap_dir;
        ctx.resources.name_cache_dir = opt_.name_cache_dir;
        ctx.snapshot.top_n = opt_.top_n > 0 ? opt_.top_n : defaults::JSLEAK_TOP_N;
        ctx.snapshot.mode = normalized_mode;
        ctx.snapshot.compare.baseline_file = baseline;

        auto* module = ModuleRegistry::find_by_extension("heapsnapshot");
        if (!module) {
            log::loge("Error: No parser found for heapsnapshot files");
            return 1;
        }
        return process_file(target, *module, ctx) ? 0 : 1;
    }
};

const bool registered = []() {
    CommandRegistry::add(std::make_shared<JsleakCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
