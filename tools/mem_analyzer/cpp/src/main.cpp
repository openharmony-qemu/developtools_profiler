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
static std::string acp_to_utf8(const std::string& acp_str) {
    if (acp_str.empty()) return {};
    int wlen = MultiByteToWideChar(CP_ACP, 0, acp_str.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return acp_str;
    std::wstring wstr(static_cast<size_t>(wlen), L'\0');
    MultiByteToWideChar(CP_ACP, 0, acp_str.c_str(), -1, &wstr[0], wlen);
    int utflen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utflen <= 0) return acp_str;
    std::string utf8_str(static_cast<size_t>(utflen) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8_str[0], utflen, nullptr, nullptr);
    return utf8_str;
}
#endif

// ============================================================================
//  auto-detect fallback: 未指定子命令时按文件扩展名逐个处理
// ============================================================================
static int run_auto_detect(const mem_analyzer::GlobalOptions& g) {
    mem_analyzer::log::logi(mem_analyzer::style::bold("=== Auto Detect Mode ==="));
    // 未指定子命令：各域算法参数取默认值（与历史行为一致）
    const mem_analyzer::RunContext ctx = mem_analyzer::make_base_run_context(g);
    bool all_ok = true;
    for (auto& f : g.files) {
        auto* module = mem_analyzer::ModuleRegistry::detect(f);
        if (!module) {
            mem_analyzer::log::loge("Error: No parser found for file \"", f, "\"");
            all_ok = false;
            continue;
        }
        mem_analyzer::log::logd("auto-detect: ", f, " -> module '", module->name(), "'");
        if (!mem_analyzer::process_file(f, *module, ctx)) all_ok = false;
    }
    mem_analyzer::log::logi(mem_analyzer::style::bold("=== Auto detect processing completed ==="));
    return all_ok ? 0 : 1;
}

// ============================================================================
//  main
// ============================================================================
int main(int argc, char** argv) {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);

    std::vector<std::string> utf8_argv_storage;
    std::vector<char*> utf8_argv_ptrs;
    utf8_argv_storage.reserve(argc);
    utf8_argv_ptrs.reserve(argc);
    for (int i = 0; i < argc; i++) {
        utf8_argv_storage.push_back(acp_to_utf8(argv[i]));
        utf8_argv_ptrs.push_back(&utf8_argv_storage.back()[0]);
    }
    argv = utf8_argv_ptrs.data();
#endif

    mem_analyzer::style::init();

    // 模块能力表一致性自检：正常构建下零输出，仅在不一致时告警（不改变退出码）。
    for (const auto& issue : mem_analyzer::ModuleRegistry::self_check()) {
        mem_analyzer::log::logw("Warning: module capability inconsistency: ", issue);
    }

    mem_analyzer::CliParser app{"Memory analysis CLI tool"};
    app.set_version(mem_analyzer::get_version());

    mem_analyzer::GlobalOptions global;
    mem_analyzer::bind_global_options(app, global);

    // 子命令全部来自注册表：新增子命令无需改动本文件。
    // bind 由 add_subcommand 同步调用，故 lambda 捕获引用是安全的。
    for (const auto& cmd : mem_analyzer::CommandRegistry::all()) {
        app.add_subcommand(cmd->name(), cmd->description(), cmd->alias(), cmd->hidden(),
                           [&](mem_analyzer::CliParser& sub) { cmd->bind(sub, global); });
    }

    try {
        app.parse(argc, argv);
    } catch (const mem_analyzer::CliError& e) {
        return app.exit(e);
    }

    if (global.debug) {
        global.verbose = true;
        global.keep_temp = true;
    }
    mem_analyzer::log::set_level(global.verbose ? mem_analyzer::log::Level::Debug
                                                : mem_analyzer::log::Level::Info);
    mem_analyzer::TempDir::set_keep(global.keep_temp);

    if (global.show_version) {
        std::cout << "mem-analyzer version " << mem_analyzer::get_version() << std::endl;
        return 0;
    }

    // Load preview feature switches (all disabled by default); failure is not fatal
    {
        std::string err;
        if (!mem_analyzer::PreviewFeaturesMgr::instance().load(err)) {
            mem_analyzer::log::logw("Warning: preview features: ", err);
        }
    }

    // Save original command line（供输出 meta 使用）
    {
        std::string cmd;
        for (int i = 1; i < argc; i++) {
            if (i > 1) cmd += " ";
            cmd += argv[i];
        }
        global.command_line = cmd;
    }

    int ret = 0;
    if (auto cmd = mem_analyzer::CommandRegistry::find(app.matched_subcommand())) {
        ret = cmd->run(global);
    } else {
        if (global.files.empty()) {
            app.print_help();
            return 1;
        }
        ret = run_auto_detect(global);
    }

    mem_analyzer::TempDir::cleanup_root();
    return ret;
}
