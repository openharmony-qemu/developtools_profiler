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
#include <algorithm>

namespace mem_analyzer {

namespace {
std::vector<std::shared_ptr<ICommand>>& registry() {
    static std::vector<std::shared_ptr<ICommand>> reg;
    return reg;
}
} // namespace

void CommandRegistry::add(std::shared_ptr<ICommand> cmd) {
    if (!cmd) return;
    registry().push_back(std::move(cmd));
}

std::vector<std::shared_ptr<ICommand>> CommandRegistry::all() {
    auto cmds = registry();
    std::stable_sort(cmds.begin(), cmds.end(),
                     [](const std::shared_ptr<ICommand>& a,
                        const std::shared_ptr<ICommand>& b) {
                         return a->order() < b->order();
                     });
    return cmds;
}

std::shared_ptr<ICommand> CommandRegistry::find(const std::string& name_or_alias) {
    if (name_or_alias.empty()) return nullptr;
    for (const auto& cmd : registry()) {
        if (cmd->name() == name_or_alias) return cmd;
        if (!cmd->alias().empty() && cmd->alias() == name_or_alias) return cmd;
    }
    return nullptr;
}

void bind_debug_options(CliParser& sub, GlobalOptions& g) {
    sub.add_switch("--verbose", g.verbose, "Enable debug-level logging")
       .add_switch("--keep-temp", g.keep_temp, "Keep temporary directories after processing")
       .add_switch("--debug", g.debug, "Troubleshooting mode (implies --verbose and --keep-temp)");
}

void bind_global_options(CliParser& app, GlobalOptions& g) {
    app.add_option("-d,--out-dir", g.out_dir, "Output result directory")
       .add_switch("-p,--parse-only", g.parse_only, "Only parse, skip analysis")
       .add_switch("-o,--auto-open", g.auto_open, "Open result file after processing")
       .add_switch("-v,--version", g.show_version, "Print version information");
    bind_debug_options(app, g);
    app.add_option("-f,--format", g.format, "Output format (json/html)")
       .add_positional("files", g.files,
                       "Files to analyze (auto-detect: htrace, tshandle zip, heapsnapshot, rawheap)");
}

void bind_common_options(CliParser& sub, GlobalOptions& g) {
    sub.add_option("-d,--out-dir", g.out_dir, "Output result directory")
       .add_switch("-p,--parse-only", g.parse_only, "Only parse, skip analysis")
       .add_switch("-o,--auto-open", g.auto_open, "Open result file after processing");
    bind_debug_options(sub, g);
    sub.add_option("-f,--format", g.format, "Output format (json/html)");
}

RunContext make_base_run_context(const GlobalOptions& g) {
    RunContext ctx;
    ctx.output.out_dir = g.out_dir;
    ctx.output.format = g.format;
    ctx.output.parse_only = g.parse_only;
    ctx.output.auto_open = g.auto_open;
    ctx.command_line = g.command_line;
    return ctx;
}

} // namespace mem_analyzer
