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
    std::string inputFile;
    std::string baselineFile;
    bool baselineProvided = false;
    int topN = defaults::JSLEAK_TOP_N;
    std::string listFile;
    bool listFileProvided = false;
    std::string mode = defaults::JSLEAK_MODE;
    std::string sourcemapDir;
    std::string nameCacheDir;
};

class JsleakCommand final : public ICommand {
    JsleakOptions opt_;

public:
    std::string name() const override { return "jsleak"; }
    std::string alias() const override { return "jl"; }
    std::string description() const override { return "Process heapsnapshot/rawheap files"; }
    int order() const override { return 30; }

    void bind(CliParser& sub, GlobalOptions& g) override
    {
        // 注意：jsleak 不注册 -p/--parse-only（保持既有 CLI 契约不变），
        // 因此这里不用 bind_common_options。
        sub.SetUsage("[options] <snapshot>")
            .AddPositional("files", opt_.inputFile, "Path to .heapsnapshot/.rawheap file")
            .AddOption("-b,--baseline", opt_.baselineFile,
                "Baseline snapshot for comparison",
                &opt_.baselineProvided)
            .AddOption("-d,--out-dir", g.outDir, "Output result directory")
            .AddSwitch("-o,--auto-open", g.autoOpen, "Open result file after processing");
        // 诊断三件套（--verbose/--keep-temp/--debug）：与 trace 一致，效果由 main 统一处理。
        BindDebugOptions(sub, g);
        sub.AddOption("-f,--format", g.format, "Output format (json/html)")
            .AddOption("-n,--top-n", opt_.topN, "Number of top reference chains to output")
            .AddOption("-l,--leaklist", opt_.listFile, "Path to .jsleaklist file",
                &opt_.listFileProvided)
            .AddOption("-m,--mode", opt_.mode,
                "Analysis mode: all, property-tag/pt, top-chain/tc")
            .AddOption("--sourcemap", opt_.sourcemapDir,
                "Source map file or directory (sourceMaps.map / *.map) to deobfuscate "
                "heapsnapshot paths")
            .AddOption("--namecache", opt_.nameCacheDir,
                "nameCache file or directory (nameCache.json) to deobfuscate heapsnapshot "
                "names");
    }

    int run(const GlobalOptions& g) override
    {
        // Validate mode（缩写归一化与历史 Config 行为一致）
        if (opt_.mode != "all" && opt_.mode != "property-tag" && opt_.mode != "pt" &&
            opt_.mode != "top-chain" && opt_.mode != "tc") {
            log::Loge("Error: invalid mode '", opt_.mode, "'. Valid: all, property-tag/pt, top-chain/tc");
            return 1;
        }
        const std::string normalizedMode = opt_.mode == "pt"   ? "property-tag" :
                                            opt_.mode == "tc" ? "top-chain" :
                                                                opt_.mode;
        if (opt_.baselineProvided) {
            return RunCompare(g, normalizedMode);
        }

        if (opt_.inputFile.empty()) {
            log::Loge("Error: jsleak requires a snapshot file");
            return 1;
        }

        log::Logi(style::bold("=== Jsleak Analyzer Command ==="));
        log::Logi("  input file: ", opt_.inputFile);

        RunContext ctx = MakeBaseRunContext(g);
        // 反混淆参数交给 jsleak_parse() 内部处理
        ctx.resources.sourcemapDir = opt_.sourcemapDir;
        ctx.resources.nameCacheDir = opt_.nameCacheDir;
        ctx.snapshot.leakListFile = opt_.listFile;
        // 与历史 Config::set_jsleak_top_n / set_jsleak_mode 行为一致：
        // 非正 top_n 回退默认，模式缩写归一化
        ctx.snapshot.topN = opt_.topN > 0 ? opt_.topN : defaults::JSLEAK_TOP_N;
        ctx.snapshot.mode = normalizedMode;

        auto* module = ModuleRegistry::FindByExtension("heapsnapshot");
        if (!module) {
            log::Loge("Error: No parser found for heapsnapshot files");
            return 1;
        }
        if (!ProcessFile(opt_.inputFile, *module, ctx)) {
            return 1;
        }

        log::Logi(style::bold("=== Jsleak analysis completed ==="));
        return 0;
    }

private:
    int RunCompare(const GlobalOptions& g, const std::string& normalizedMode)
    {
        if (opt_.inputFile.empty()) {
            log::Loge("Error: jsleak -b/--baseline requires a target snapshot file");
            return 1;
        }
        if (opt_.baselineFile.empty()) {
            log::Loge("Error: -b/--baseline requires a baseline snapshot file path");
            return 1;
        }
        if (opt_.listFileProvided) {
            log::Loge("Error: --leaklist is not supported with -b/--baseline");
            return 1;
        }
        if (g.format == "json") {
            log::Loge("Error: -f json is not supported with -b/--baseline yet");
            return 1;
        }
        if (!g.format.empty() && g.format != "html") {
            log::Loge("Error: unsupported format '", g.format,
                "' for -b/--baseline. Valid: html");
            return 1;
        }

        const std::string& baseline = opt_.baselineFile;
        const std::string& target = opt_.inputFile;
        log::Logi(style::bold("=== Snapshot Compare Command ==="));
        log::Logi("  A (baseline): ", baseline);
        log::Logi("  B (target):   ", target);
        log::Logi("  mode:         ", normalizedMode);

        // 对比走与单文件一致的通用管道：process_file → module.run。
        // 输出目录嵌套与单文件相同（out/<目标文件 stem>/），
        // 输出文件名由 formatter 从 report 的 baseline/target 文件名推导。
        RunContext ctx = MakeBaseRunContext(g);
        ctx.resources.sourcemapDir = opt_.sourcemapDir;
        ctx.resources.nameCacheDir = opt_.nameCacheDir;
        ctx.snapshot.topN = opt_.topN > 0 ? opt_.topN : defaults::JSLEAK_TOP_N;
        ctx.snapshot.mode = normalizedMode;
        ctx.snapshot.compare.baselineFile = baseline;

        auto* module = ModuleRegistry::FindByExtension("heapsnapshot");
        if (!module) {
            log::Loge("Error: No parser found for heapsnapshot files");
            return 1;
        }
        return ProcessFile(target, *module, ctx) ? 0 : 1;
    }
};

const bool registered = []() {
    CommandRegistry::add(std::make_shared<JsleakCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
