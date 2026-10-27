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
    int top_n = defaults::TRACE_TOP_N;
    std::string category = defaults::HTRACE_TYPE;
    std::string so_dir;
    std::string leak_list_file;   // -l: Native Leaks 不可达内存泄漏文件
    std::vector<std::string> files;
};

class TraceCommand final : public ICommand {
    TraceOptions opt_;

public:
    std::string name() const override { return "trace"; }
    std::string alias() const override { return "tr"; }
    std::string description() const override { return "Process htrace files"; }
    int order() const override { return 10; }

    void bind(CliParser& sub, GlobalOptions& g) override {
        bind_common_options(sub, g);
        sub.add_option("-s,--scope", opt_.scope,
                       "Analysis scope: app-only, system-only, app-and-system, mixed")
           .add_option("-n,--top-n", opt_.top_n, "Number of top symbols to output")
           .add_option("-c,--category", opt_.category,
                       "Resource category: nativeheap, mmap, dma, ashmem, fd, thread")
           .add_option("-S,--so-dir", opt_.so_dir,
                       "Directory of native .so symbol files for offline symbolization")
           .add_option("-l,--leaklist", opt_.leak_list_file,
                       "Native Leaks unreachable-memory leak file")
           .add_positional("files", opt_.files, "Htrace files to process");
    }

    int run(const GlobalOptions& g) override {
        if (!htrace_find_category(opt_.category)) {
            std::string valid;
            for (const auto& spec : htrace_categories()) {
                if (!valid.empty()) valid += ", ";
                valid += spec.name;
            }
            log::loge("Error: invalid type '", opt_.category, "'. Valid values: ", valid);
            return 1;
        }

        RunContext ctx = make_base_run_context(g);
        ctx.resources.so_dir = opt_.so_dir;
        ctx.htrace.scope = scope_from_string(opt_.scope);
        ctx.htrace.category = opt_.category;
        // 与历史 Config::set_trace_top_n 一致：非正值回退默认
        ctx.htrace.top_n = opt_.top_n > 0 ? opt_.top_n : defaults::TRACE_TOP_N;

        // -l: 读取 Native Leaks 不可达内存泄漏文件
        if (!opt_.leak_list_file.empty()) {
            try {
                auto addrs = mem_analyzer::read_leak_file(opt_.leak_list_file);
                log::logi("Leak file: ", opt_.leak_list_file, " (", addrs.size(), " addresses)");
                ctx.htrace.leak_addrs = std::move(addrs);
            } catch (const std::exception& e) {
                log::loge("Error: ", e.what());
                return 1;
            }
        }

        log::logi(style::bold("=== Trace Command ==="));
        log::logi("Mode: ", (g.parse_only ? "parse-only" : "analysis"), ", category: ", opt_.category);
        if (g.parse_only) {
            log::logi_minor("Note: --category is ignored in parse-only mode; all resource types are "
                         "included in the parse output.");
        }

        bool all_ok = true;
        for (auto& f : opt_.files) {
            auto* module = ModuleRegistry::find_by_extension("htrace");
            if (!module) {
                log::loge("Error: No parser found for htrace files");
                all_ok = false;
                continue;
            }
            if (!process_file(f, *module, ctx)) all_ok = false;
        }
        log::logi(style::bold("=== Trace processing completed ==="));
        return all_ok ? 0 : 1;
    }
};

const bool registered = []() {
    CommandRegistry::add(std::make_shared<TraceCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
