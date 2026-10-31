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
#include "cli/command.h"
#include <algorithm>

namespace mem_analyzer {

namespace {
std::vector<std::shared_ptr<ICommand>>& registry()
{
    static std::vector<std::shared_ptr<ICommand>> reg;
    return reg;
}
} // namespace

void CommandRegistry::Add(std::shared_ptr<ICommand> cmd)
{
    if (!cmd) {
        return;
    }
    registry().push_back(std::move(cmd));
}

std::vector<std::shared_ptr<ICommand>> CommandRegistry::All()
{
    auto cmds = registry();
    std::stable_sort(cmds.begin(), cmds.end(),
        [](const std::shared_ptr<ICommand>& a, const std::shared_ptr<ICommand>& b) { return a->Order() < b->Order(); });
    return cmds;
}

std::shared_ptr<ICommand> CommandRegistry::Find(const std::string& nameOrAlias)
{
    if (nameOrAlias.empty()) {
        return nullptr;
    }
    for (const auto& cmd : registry()) {
        if (cmd->Name() == nameOrAlias) {
            return cmd;
        }
        if (!cmd->Alias().empty() && cmd->Alias() == nameOrAlias) {
            return cmd;
        }
    }
    return nullptr;
}

void BindDebugOptions(CliParser& sub, GlobalOptions& g)
{
    sub.AddSwitch("--verbose", g.verbose, "Enable debug-level logging")
        .AddSwitch("--keep-temp", g.keepTemp, "Keep temporary directories after processing")
        .AddSwitch("--debug", g.debug, "Troubleshooting mode (implies --verbose and --keep-temp)");
}

void BindGlobalOptions(CliParser& app, GlobalOptions& g)
{
    app.AddOption("-d,--out-dir", g.outDir, "Output result directory")
        .AddSwitch("-p,--parse-only", g.parseOnly, "Only parse, skip analysis")
        .AddSwitch("-o,--auto-open", g.autoOpen, "Open result file after processing")
        .AddSwitch("-v,--version", g.showVersion, "Print version information");
    BindDebugOptions(app, g);
    app.AddOption("-f,--format", g.format, "Output format (json/html)")
        .AddPositional("files", g.files, "Files to analyze (auto-detect: htrace, tshandle zip, heapsnapshot, rawheap)");
}

void BindCommonOptions(CliParser& sub, GlobalOptions& g)
{
    sub.AddOption("-d,--out-dir", g.outDir, "Output result directory")
        .AddSwitch("-p,--parse-only", g.parseOnly, "Only parse, skip analysis")
        .AddSwitch("-o,--auto-open", g.autoOpen, "Open result file after processing");
    BindDebugOptions(sub, g);
    sub.AddOption("-f,--format", g.format, "Output format (json/html)");
}

RunContext MakeBaseRunContext(const GlobalOptions& g)
{
    RunContext ctx;
    ctx.output.outDir = g.outDir;
    ctx.output.format = g.format;
    ctx.output.parseOnly = g.parseOnly;
    ctx.output.autoOpen = g.autoOpen;
    ctx.commandLine = g.commandLine;
    return ctx;
}

} // namespace mem_analyzer
