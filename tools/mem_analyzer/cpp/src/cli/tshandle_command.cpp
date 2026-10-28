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

struct TshandleOptions {
    int topN = defaults::TSHANDLE_TOP_N;
    std::string soDir;
    std::string sourcemapDir;
    std::string nameCacheDir;
    std::vector<std::string> files;
};

class TshandleCommand final : public ICommand {
    TshandleOptions opt_;

public:
    std::string Name() const override { return "tshandle"; }
    std::string Alias() const override { return "th"; }
    std::string Description() const override { return "Process tshandle zip files"; }
    int Order() const override { return 20; }

    void Bind(CliParser& sub, GlobalOptions& g) override
    {
        BindCommonOptions(sub, g);
        sub.AddOption("-n,--top-n", opt_.topN, "Number of top symbols to output")
            .AddOption("-S,--so-dir", opt_.soDir, "Directory of native .so symbol files for offline symbolization")
            .AddOption("--sourcemap", opt_.sourcemapDir,
                "Source map file or directory (sourceMaps.map / *.map) to deobfuscate heapsnapshot paths")
            .AddOption("--namecache", opt_.nameCacheDir,
                "nameCache file or directory (nameCache.json) to deobfuscate heapsnapshot names")
            .AddPositional("files", opt_.files, "Zip files to process");
    }

    int Run(const GlobalOptions& g) override
    {
        log::Logi(style::bold("=== Tshandle Command ==="));

        RunContext ctx = MakeBaseRunContext(g);
        ctx.resources.soDir = opt_.soDir;
        // 反混淆参数交给 tshandle_parse：对解出的 rawheap 快照先还原（源码路径/行号）再分析
        ctx.resources.sourcemapDir = opt_.sourcemapDir;
        ctx.resources.nameCacheDir = opt_.nameCacheDir;
        // 与历史 Config::set_tshandle_top_n 一致：非正值回退默认
        ctx.tshandle.topN = opt_.topN > 0 ? opt_.topN : defaults::TSHANDLE_TOP_N;

        bool allOk = true;
        for (auto& f : opt_.files) {
            auto* module = ModuleRegistry::FindByExtension("zip");
            if (!module) {
                log::Loge("Error: No parser found for zip files");
                allOk = false;
                continue;
            }
            if (!ProcessFile(f, *module, ctx)) {
                allOk = false;
            }
        }
        log::Logi(style::bold("=== Tshandle processing completed ==="));
        return allOk ? 0 : 1;
    }
};

const bool REGISTERED = []() {
    CommandRegistry::Add(std::make_shared<TshandleCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
