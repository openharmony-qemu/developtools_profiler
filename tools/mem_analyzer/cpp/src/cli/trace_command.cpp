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
#include "htrace/htrace_category.hpp"
#include "utils/leak_file_reader.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"

namespace mem_analyzer {
namespace {

struct TraceOptions {
    std::string scope = defaults::SCOPE_STR;
    int topN = defaults::TRACE_TOP_N;
    std::string category = defaults::HTRACE_TYPE;
    std::string soDir;
    std::string leakListFile;   // -l: Native Leaks 不可达内存泄漏文件
    std::vector<std::string> files;
};

class TraceCommand final : public ICommand {
    TraceOptions opt_;

public:
    std::string name() const override { return "trace"; }
    std::string alias() const override { return "tr"; }
    std::string description() const override { return "Process htrace files"; }
    int order() const override { return 10; }

    void bind(CliParser& sub, GlobalOptions& g) override
    {
        BindCommonOptions(sub, g);
        sub.AddOption("-s,--scope", opt_.scope,
            "Analysis scope: app-only, system-only, app-and-system, mixed")
            .AddOption("-n,--top-n", opt_.topN, "Number of top symbols to output")
            .AddOption("-c,--category", opt_.category,
                "Resource category: nativeheap, mmap, dma, ashmem, fd, thread")
            .AddOption("-S,--so-dir", opt_.soDir,
                "Directory of native .so symbol files for offline symbolization")
            .AddOption("-l,--leaklist", opt_.leakListFile,
                "Native Leaks unreachable-memory leak file")
            .AddPositional("files", opt_.files, "Htrace files to process");
    }

    int run(const GlobalOptions& g) override
    {
        if (!HtraceFindCategory(opt_.category)) {
            std::string valid;
            for (const auto& spec : HtraceCategories()) {
                if (!valid.empty()) {
                    valid += ", ";
                }
                valid += spec.name;
            }
            log::Loge("Error: invalid type '", opt_.category, "'. Valid values: ", valid);
            return 1;
        }

        RunContext ctx = MakeBaseRunContext(g);
        ctx.resources.soDir = opt_.soDir;
        ctx.htrace.scope = ScopeFromString(opt_.scope);
        ctx.htrace.category = opt_.category;
        // 与历史 Config::set_trace_top_n 一致：非正值回退默认
        ctx.htrace.topN = opt_.topN > 0 ? opt_.topN : defaults::TRACE_TOP_N;

        // -l: 读取 Native Leaks 不可达内存泄漏文件
        if (!opt_.leakListFile.empty()) {
            try {
                auto addrs = mem_analyzer::ReadLeakFile(opt_.leakListFile);
                log::Logi("Leak file: ", opt_.leakListFile, " (", addrs.size(), " addresses)");
                ctx.htrace.leakAddrs = std::move(addrs);
            } catch (const std::exception& e)
            {
                log::Loge("Error: ", e.what());
                return 1;
            }
        }

        log::Logi(style::bold("=== Trace Command ==="));
        log::Logi("Mode: ", (g.parseOnly ? "parse-only" : "analysis"), ", category: ", opt_.category);
        if (g.parseOnly) {
            log::LogiMinor("Note: --category is ignored in parse-only mode; all resource types are "
                         "included in the parse output.");
        }

        bool allOk = true;
        for (auto& f : opt_.files) {
            auto* module = ModuleRegistry::FindByExtension("htrace");
            if (!module) {
                log::Loge("Error: No parser found for htrace files");
                allOk = false;
                continue;
            }
            if (!ProcessFile(f, *module, ctx)) {
                allOk = false;
            }
        }
        log::Logi(style::bold("=== Trace processing completed ==="));
        return allOk ? 0 : 1;
    }
};

const bool registered = []() {
    CommandRegistry::add(std::make_shared<TraceCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
