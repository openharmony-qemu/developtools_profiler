# AGENTS.md

面向 OpenCode agent 的速查。只放**不读多个文件就猜不到**的硬约束与易错点。完整架构规则见
[AGENT.md](AGENT.md)；概览/功能见 [README.md](README.md)；引擎内部结构与数据结构/配置字段见
[cpp/README.md](cpp/README.md)；CLI 用户手册见 [docs/usage.md](docs/usage.md)。

## 构建与运行

- **必须用仓库脚本编译，不要裸跑 cmake**（缺第三方依赖会运行失败）：
  - Windows：`cd cpp; .\build-win.ps1`（MinGW 静态链接；脚本自动跑 `build/setup_deps.py` 拉 sqlite/json/zlib → cmake → 打包）
  - macOS：`cd cpp; ./release-mac.sh`（可加 `--arch arm64|x86_64|all`）
  - Linux：`cd cpp; ./release-cross.sh`（原生 + MinGW Windows）
- 产物：`cpp/mem-analyzer-<version>-<platform>/`（exe + `third_party/` + `config/` + `usage.md`）及同名 ZIP。
- **分析 htrace 必须在打包目录下跑 exe**：`trace_streamer` 与 `config/` 按 exe 相对路径查找；在裸 `cpp/build/` 下跑会报 `CreateProcessW error 3`（找不到 trace_streamer）。
- **重定向构建日志**用 `Start-Process -RedirectStandardOutput/-RedirectStandardError`，不要用 PowerShell 的 `*>`：sqlite3 的 stderr 警告在 `$ErrorActionPreference=Stop` 下会把构建打断（误报 exit 1）。
- MinGW 交叉编译需 **posix 线程模型**（`g++-mingw-w64-x86_64-posix`），win32 线程模型不支持 `<thread>/<mutex>`。
- macOS 的 trace_streamer 二进制经 Git LFS 管理，需 `brew install git-lfs && git lfs install && git lfs pull`。

## 回归测试

- **测试样例目录 `test/` 被 gitignore 且不在仓库里**（`AGENT.md` 旧描述已过时）——回归前需自备 `.htrace`/`.zip`/`.heapsnapshot` 样例，通过 `--test-dir` 传给脚本。
- 跨版本输出回归用 `build/compare_commits.py`（脚本会自复制到临时目录，防 git worktree 操作影响自身）：
  ```bash
  python build/compare_commits.py <base> <new> --skip-build \
      --base-exe <base>/mem-analyzer.exe --new-exe <new>/mem-analyzer.exe
  ```
- base 用**上一个提交**的打包产物重建（`git worktree add --detach <dir> <commit>` → 跑构建脚本 → 解包）。
- **必须分开取 stdout / stderr 两路比对**：重定向后 stdout 块缓冲、stderr 无缓冲，合并流交错不稳定，会看到假差异。
- 比对前归一临时目录随机名（`mem-analyzer.xxxxxx` / `tshandle.xxxxxx`）。
- `--help`（主命令 + 各子命令）的 stdout/stderr 需逐字节一致——它是 CLI 表面契约。
- 即使只改 include/注释这类零语义改动，也应对**最终二进制**重跑一次全量回归。

## 架构铁律（违反即回归挂或编译错）

完整规则见 [AGENT.md](AGENT.md)，以下是最易踩的：

- **一个数据域 = 一个模块**，纵切 `parse → analyze → render`，全是**自由函数 + 域内强类型结构**。禁止 `std::any` 管道载体、`from` 字符串协议、`I*` + 工厂注册、全局单例（旧 `Config` 单例与 `FormatterFactory` 已删，勿再造）。
- **依赖只能单向**：`L0 cli → L1 profiler → L2 域(htrace/tshandle/jsleak) → L3 shared → L4 utils/formatter/config_mgr`，跨层契约在 `core/`。`shared/` 与 `formatter/` **禁止 include 任何域目录**。
- **`name()` 用历史 parser 名**（`htrace`/`zip`/`snapshot`），决定 `Using parser: <name>` 输出，改名属行为变更。
- **`-f` 不回落**：显式指定不支持的格式直接报错并跳过，不退回默认——有意行为，别「修」成兜底。
- **`CMakeLists.txt` 显式列举源文件**，每次增删 `.cpp` 都要同步登记。
- **搬目录与改内容分两个 commit**（先 `git mv` + CMakeLists，再改代码），让回归基线干净。
- `RunContext` 构造后只读、显式传递；各域算法只接收自己的子结构（`HtraceOptions`/`TshandleOptions`/`SnapshotOptions`），不接收整个 `RunContext`。
- 能力表 `ModuleCapability` 在启动期由 `ModuleRegistry::self_check()` 校验，正常应零告警。新增 writer 只改 `run()` 不改 `capability()` → 启动报「格式不支持」。

## 日志约定

- **运行时日志走 `log::`（`utils/logger.hpp`）落 stderr**；**结果/输出走 `std::cout`/`std::cerr`**。别用 `std::cout` 打日志（污染解析）、别用 `std::cerr` + 手染色打日志（散乱）。
- `logi`（正色）= 主线步骤，删掉用户就看不懂流程；`logi_minor`（灰）= 旁注（耗时/跳过/note）。判据：删掉这条用户还能无歧义看懂流程吗？
- `logd` 只在 `--verbose` 下显示，别拿它承载用户应默认看到的内容。
- 变参用 `<<` fold 拼接，每个调用自带换行，别写 `<< std::endl` 或结尾 `\n`。
- 流式实时输出（spinner 的 `\r` 重写、hdc transcript、轮询指标）不走 `log::`。

## 安全红线（处理不可信输入）

- **信任边界**：CLI flag/环境变量可信；**被处理文件的内容**（文件名、zip 条目名、快照内字符串）不可信——「分析第三方 dump」是真实场景。
- 执行外部进程（`trace_streamer`/`rawheap_translator`/`hdc`）用**参数数组**，不拼 shell：Windows `CreateProcessW`/`ShellExecuteW`，POSIX `fork`+`execv`（不经 `/bin/sh -c`）。禁止 `std::system`/`popen`/`_wsystem` 拼不可信内容。
- 解压 zip 用内置 minizip API（`utils/zip_utils`），不调 `unzip`/`Expand-Archive`；zip 条目路径拒绝绝对路径/盘符/`..`（防 zip-slip）。
- SQL 一律参数绑定（`sqlite3_prepare_v2` + `sqlite3_bind_*`），禁止拼接 SQL。
- `hdc shell` 是「本机 shell + 设备 shell」两层：本机层用 `proc::shell_quote`，设备层用 `proc::shell_quote_device`（始终 POSIX 单引号），不能混用。
- 内存安全：统一 `std::string`/`std::vector`，禁止 `strcpy`/`sprintf`/`gets`/`strcat`；解析不可信数据对长度/计数/索引做边界与上限校验。
- 临时目录用 `TempDir`（随机名）；构造 `fs::path` 用 `path_from_utf8()`；用户/设备进程名做路径组件前经 `as_path_segment()`。

## 平台/编码

- 内部统一 UTF-8。Windows 上构造 `fs::path` 必须经 `path_from_utf8()`、取回用 `path_to_utf8()`（`utils/file_utils.hpp`）——直接用 `std::string` 构造 `fs::path` 会按 GBK 误解释。
- argv 的 ANSI→UTF-8 转换是 `cpp/src/main.cpp` 内的 static `acp_to_utf8()`，别处不要重复实现。

## 扩展速查

新增数据域/输出格式/CLI 子命令/htrace 事件的完整步骤见 [AGENT.md](AGENT.md)。关键落点：
- 新模块注册：`cpp/src/core/module_registry.cpp` 注册块 + `cpp/CMakeLists.txt` 登记源文件 + `#include` 头。
- 新 CLI 子命令：`cpp/src/cli/<name>_command.cpp`（实现 `ICommand`，静态注册），`order()` 必须显式给值；**不改 `main.cpp`**。
- `shared/` 搬入条件：**逐函数核对真实调用点**，确认被 ≥2 个域复用才搬；按被共享对象命名，不建 `algo/common/` 大杂烩。
