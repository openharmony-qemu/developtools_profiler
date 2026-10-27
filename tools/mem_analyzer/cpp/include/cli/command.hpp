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
#pragma once
#include <string>
#include <vector>
#include <memory>

#include "config.hpp"
#include "cli/cli_parser.hpp"
#include "core/run_context.hpp"

namespace mem_analyzer {

/// 全局选项：真正跨子命令共享的开关。
/// 各子命令自己的参数不放这里，由命令自身持有（见 ICommand::bind）。
struct GlobalOptions {
    std::string out_dir = defaults::OUTPUT_DIR;  // -d/--out-dir
    std::string format;                          // -f/--format（空 = 用该命令的默认格式）
    bool parse_only = false;                     // -p/--parse-only
    bool auto_open = false;                      // -o/--auto-open
    bool show_version = false;                   // -v/--version（仅顶层）
    bool verbose = false;                        // --verbose（启用 debug 日志）
    bool keep_temp = false;                      // --keep-temp（保留临时目录）
    bool debug = false;                          // --debug（= --verbose --keep-temp）
    std::string command_line;                    // 原始命令行（由 main 填入，写入输出 meta）
    std::vector<std::string> files;              // 顶层位置参数（auto-detect 模式）
};

/// 顶层注册：-d/-p/-o/-v/-f + 位置参数 <files>
void bind_global_options(CliParser& app, GlobalOptions& g);

/// 子命令公共注册：-d/-p/-o/-f + 诊断三件套（不含 -v 与位置参数）
void bind_common_options(CliParser& sub, GlobalOptions& g);

/// 诊断/排障三件套：--verbose/--keep-temp/--debug。
/// 效果由 main 统一生效（--debug ⇒ --verbose --keep-temp），各子命令在 bind() 中调用即可。
void bind_debug_options(CliParser& sub, GlobalOptions& g);

/// 从全局选项构造 RunContext 的基础部分（output + command_line）。
/// 各命令随后自行填充 resources 与所属域的算法参数。
RunContext make_base_run_context(const GlobalOptions& g);

/**
 * 一个子命令 = 一个自包含单元：自己声明参数、自己执行。
 * 新增子命令只需新增一个 .cpp（在文件内静态注册）并在 CMakeLists 登记，
 * 不需要改动 main.cpp。
 *
 * 生命周期约定：命令实例由注册表的静态初始化器创建、进程内常驻；其 options
 * 成员同时是 CliParser 中 Option.target 的绑定目标，必须与 parser 同寿命。
 * 这隐含「一个进程只执行一个子命令」的假设。
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /// 子命令主名（用于分发），如 "trace"
    virtual std::string name() const = 0;
    /// 别名，如 "tr"；无别名为空串
    virtual std::string alias() const { return {}; }
    /// --help 中的一行描述
    virtual std::string description() const = 0;
    /// 隐藏子命令不出现在 --help 中（仍可被解析与分发）
    virtual bool hidden() const { return false; }
    /// help 排序权重：静态注册顺序不可控，必须显式指定以保证 --help 输出稳定
    virtual int order() const = 0;
    /// 声明本命令的参数（绑定到命令自身的 options）
    virtual void bind(CliParser& sub, GlobalOptions& g) = 0;
    /// 执行本命令，返回进程退出码
    virtual int run(const GlobalOptions& g) = 0;
};

/// 子命令注册表（与 Parser/Analyzer/Formatter 相同的静态注册模式）
class CommandRegistry {
public:
    static void add(std::shared_ptr<ICommand> cmd);
    /// 按 order() 稳定排序返回，保证 --help 输出不受链接顺序影响
    static std::vector<std::shared_ptr<ICommand>> all();
    /// 按主名或别名查找；未找到返回 nullptr
    static std::shared_ptr<ICommand> find(const std::string& name_or_alias);
};

} // namespace mem_analyzer
