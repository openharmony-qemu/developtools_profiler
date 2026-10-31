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
#include <iostream>
#include <string>
#include <vector>

#include "cli/cli_parser.hpp"
#include "cli/command.hpp"
#include "cli/pipeline_runner.hpp"
#include "config_mgr/preview_features_mgr.hpp"
#include "core/module_registry.hpp"
#include "utils/logger.hpp"
#include "utils/temp_dir.hpp"
#include "utils/terminal_style.hpp"
#include "utils/version.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

// ============================================================================
//  Windows: argv encoding conversion (ANSI → UTF-8)
// ============================================================================
#if defined(_WIN32)
static std::string AcpToUtf8(const std::string& acpStr)
{
    if (acpStr.empty()) {
        return {};
    }
    int wlen = MultiByteToWideChar(CP_ACP, 0, acpStr.c_str(), -1, nullptr, 0);
    if (wlen <= 0) {
        return acpStr;
    }
    std::wstring wstr(static_cast<size_t>(wlen), L'\0');
    MultiByteToWideChar(CP_ACP, 0, acpStr.c_str(), -1, &wstr[0], wlen);
    int utflen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utflen <= 0) {
        return acpStr;
    }
    std::string utf8Str(static_cast<size_t>(utflen) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], utflen, nullptr, nullptr);
    return utf8Str;
}
#endif

// ============================================================================
//  auto-detect fallback: 未指定子命令时按文件扩展名逐个处理
// ============================================================================
static int RunAutoDetect(const mem_analyzer::GlobalOptions& g)
{
    mem_analyzer::log::Logi(mem_analyzer::style::Bold("=== Auto Detect Mode ==="));
    // 未指定子命令：各域算法参数取默认值（与历史行为一致）
    const mem_analyzer::RunContext ctx = mem_analyzer::MakeBaseRunContext(g);
    bool allOk = true;
    for (auto& f : g.files) {
        auto* module = mem_analyzer::ModuleRegistry::Detect(f);
        if (!module) {
            mem_analyzer::log::Loge("Error: No parser found for file \"", f, "\"");
            allOk = false;
            continue;
        }
        mem_analyzer::log::Logd("auto-detect: ", f, " -> module '", module->Name(), "'");
        if (!mem_analyzer::ProcessFile(f, *module, ctx)) {
            allOk = false;
        }
    }
    mem_analyzer::log::Logi(mem_analyzer::style::Bold("=== Auto detect processing completed ==="));
    return allOk ? 0 : 1;
}

/// Windows: convert ANSI argv to UTF-8 in-place.
#if defined(_WIN32)
static void SetupUtf8Args(int argc, char**& argv,
    std::vector<std::string>& storage, std::vector<char*>& ptrs)
{
    SetConsoleOutputCP(CP_UTF8);
    storage.reserve(argc);
    ptrs.reserve(argc);
    for (int i = 0; i < argc; i++) {
        storage.push_back(AcpToUtf8(argv[i]));
        ptrs.push_back(&storage.back()[0]);
    }
    argv = ptrs.data();
}
#endif

/// Save the original command line for output metadata.
static void SaveCommandLine(int argc, char** argv, std::string& out)
{
    for (int i = 1; i < argc; i++) {
        if (i > 1) {
            out += " ";
        }
        out += argv[i];
    }
}

/// Register all subcommands from the module registry into the CLI parser.
static void RegisterSubcommands(mem_analyzer::CliParser& app,
    mem_analyzer::GlobalOptions& global)
{
    for (const auto& cmd : mem_analyzer::CommandRegistry::All()) {
        app.AddSubcommand(cmd->Name(), cmd->Description(), cmd->Alias(), cmd->Hidden(),
            [&cmd, &global](mem_analyzer::CliParser& sub) { cmd->Bind(sub, global); });
    }
}

/// Apply debug/verbose/keepTemp settings from global options.
static void ApplyGlobalOptions(mem_analyzer::GlobalOptions& global)
{
    if (global.debug) {
        global.verbose = true;
        global.keepTemp = true;
    }
    mem_analyzer::log::SetLevel(global.verbose ? mem_analyzer::log::Level::LOG_DEBUG :
                                                 mem_analyzer::log::Level::LOG_INFO);
    mem_analyzer::TempDir::SetKeep(global.keepTemp);
}

/// Load preview feature switches (non-fatal on failure).
static void LoadPreviewFeatures()
{
    std::string err;
    if (!mem_analyzer::PreviewFeaturesMgr::Instance().Load(err)) {
        mem_analyzer::log::Logw("Warning: preview features: ", err);
    }
}

int main(int argc, char** argv)
{
#if defined(_WIN32)
    std::vector<std::string> utf8ArgvStorage;
    std::vector<char*> utf8ArgvPtrs;
    SetupUtf8Args(argc, argv, utf8ArgvStorage, utf8ArgvPtrs);
#endif

    mem_analyzer::style::Init();

    for (const auto& issue : mem_analyzer::ModuleRegistry::SelfCheck()) {
        mem_analyzer::log::Logw("Warning: module capability inconsistency: ", issue);
    }

    mem_analyzer::CliParser app{"Memory analysis CLI tool"};
    app.SetVersion(mem_analyzer::GetVersion());

    mem_analyzer::GlobalOptions global;
    mem_analyzer::BindGlobalOptions(app, global);
    RegisterSubcommands(app, global);

    try {
        app.Parse(argc, argv);
    } catch (const mem_analyzer::CliError& e) {
        return app.Exit(e);
    }

    ApplyGlobalOptions(global);

    if (global.showVersion) {
        std::cout << "mem-analyzer version " << mem_analyzer::GetVersion() << std::endl;
        return 0;
    }

    LoadPreviewFeatures();

    SaveCommandLine(argc, argv, global.commandLine);

    int ret = 0;
    if (auto cmd = mem_analyzer::CommandRegistry::Find(app.MatchedSubcommand())) {
        ret = cmd->Run(global);
    } else {
        if (global.files.empty()) {
            app.PrintHelp();
            return 1;
        }
        ret = RunAutoDetect(global);
    }

    mem_analyzer::TempDir::CleanupRoot();
    return ret;
}
