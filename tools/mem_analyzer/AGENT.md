# AGENT.md

面向 AI coding agent 的项目速查。只放**架构规则、扩展步骤、高频约定与易错点**，不重复详细文档 ——
概览/构建见根 [README.md](README.md)，引擎接口/数据结构/配置字段见 [cpp/README.md](cpp/README.md)，
使用者手册见 [docs/usage.md](docs/usage.md)。

## 项目定位

C++17 命令行内存分析工具，分析 HarmonyOS 内存泄漏。

**一个数据域 = 一个模块**，模块自持 `parse → analyze → render` 整条管道，内部**强类型**（编译期确定，
不用 `std::any`、不用 `from` 字符串握手）。管道层 `process_file` 只做三件事：按扩展名选模块 →
派生 `RunContext` → 调 `module.run()`；它不认识任何具体数据域。

| 输入 | 模块 | 目录 |
|---|---|---|
| `.htrace` | `HtraceModule`（`name()` = `htrace`） | `cpp/{include,src}/htrace/` |
| `.zip` | `TshandleModule`（`name()` = `zip`） | `cpp/{include,src}/tshandle/` |
| `.heapsnapshot` | `JsleakModule`（`name()` = `snapshot`） | `cpp/{include,src}/jsleak/` |

### 分层与依赖方向（只能单向）

```
L0 App / CLI        cli/
L1 Acquisition      profiler/
L2 Modules          htrace/   tshandle/   jsleak/
L3 Shared           shared/htrace_model/   shared/heapsnapshot/
L4 Infra            utils/   formatter/html/   formatter/json/   config_mgr/
跨层契约            core/
```

依赖只能 `域 → shared → core/utils` 单向。**`shared/` 与 `formatter/` 都禁止 include 任何域目录**（当前为零，勿破坏）。

### 关键入口文件

- CLI 入口：`cpp/src/main.cpp`
- 子命令（`ICommand` / `CommandRegistry` / `GlobalOptions`）：`cpp/include/cli/command.hpp`
- 各子命令实现：`cpp/src/cli/<name>_command.cpp`（新增子命令 = 新增一个 .cpp + CMakeLists，不动 main）
- 单文件流程 `process_file`：`cpp/src/cli/pipeline_runner.cpp`
- 模块接口与注册表：`cpp/include/core/module.hpp`、`cpp/include/core/module_registry.hpp`（注册块在 `cpp/src/core/module_registry.cpp`）
- 能力表：`cpp/include/core/module_capability.hpp`
- 格式关键词路由：`cpp/include/core/format_resolver.hpp`
- 运行参数：`cpp/include/core/run_context.hpp`
- 运行时配置/分类/过滤：`HtraceConfigMgr`（`cpp/include/config_mgr/htrace_config_mgr.hpp`）
- htrace 配置：`cpp/config/htrace/trace_analyzer_config.json`

## 构建

**必须用仓库自带脚本编译，不要裸跑 cmake**（会缺第三方依赖导致运行失败）。脚本自动拉依赖并打包。

```bash
cd cpp; .\build-win.ps1        # Windows（MinGW 静态链接）
cd cpp; ./release-mac.sh       # macOS，可加 --arch arm64|x86_64|all
cd cpp; ./release-cross.sh     # Linux 原生 + MinGW Windows
```

产物：`cpp/mem-analyzer-<version>-<platform>/`（exe + `third_party/` + `config/`）及同名 ZIP。

**重定向构建日志时**用 `Start-Process -RedirectStandardOutput / -RedirectStandardError`，
不要用 PowerShell 的 `*>`——sqlite3 的 stderr 警告会在 `$ErrorActionPreference=Stop` 下把构建打断（误报 exit 1）。

## 运行与测试

- 测试样例在仓库根 `test/`（17 个 `.htrace`、3 个 `.zip`、3 个 `.heapsnapshot`）。
- 命令：`mem-analyzer trace <file> -f json`（子命令 `trace / tshandle / jsleak / profiler`，详见 `--help`）。
- 跨版本回归用 `build/compare_commits.py`（口径见 §6）。

**关键坑**：分析 htrace 必须在**打包目录**下运行 exe（`trace_streamer` 与 `config/` 按 exe 相对路径查找）。
在裸 `cpp/build/` 下跑会报 `CreateProcessW error 3`（找不到 trace_streamer）。

## 架构规则

### 1. 文件该放哪儿

| 位置 | 收纳条件 |
|---|---|
| `htrace/` `tshandle/` `jsleak/` | 只被**一个**数据域使用的东西；域内最多一层 `formatters/`，其余扁平 |
| `shared/<被共享对象名>/` | 被 **≥2 个域**复用的数据模型与算法。**只在该条件出现时才搬**，不预先造空目录 |
| `formatter/html/`、`formatter/json/` | 输出公共件，被 ≥2 条**输出路线**用到。不要与 `shared/` 混为一谈 |
| `core/` | 跨层契约：`RunContext`、模块接口与注册表、格式路由。不放实现算法 |
| `utils/` `config_mgr/` | 与数据域无关的基建 |

判定 `shared/` 搬入条件时**必须逐函数核对真实调用点**：头文件里出现某个 `#include`，不等于那个头的内容真被共享。
`shared/` 子目录按**被共享的那个东西**命名，不建 `algo/`、`common/` 这类大杂烩。

### 2. 模块纵切约定

- 一个数据域 = 一个顶层目录 + 一个 `AnalysisModule` 实现（`<域>/<域>_module.{hpp,cpp}`）+ 静态注册。
- 域内 `parse → analyze → render` 全部是**自由函数**，入参出参都是域内强类型结构：
  `htrace_parse() → TraceDB → htrace_analyze() → HtraceAnalysisOutput → htrace_write_*()`。
- 禁止再引入 `std::any` 管道载体、`from` 字符串协议、`I*` + 工厂注册。输出 writer 是普通函数，不进任何注册表。
- **`name()` 用历史 parser 名**（`htrace` / `zip` / `snapshot`）：它决定 `Using parser: <name>` 输出，改名属行为变更。
- 域内解析失败一律**抛 `std::runtime_error`**，由模块 `run()` 统一 catch 打印；不要返回半成品结构。
- 模块 `run()` 的骨架（照抄 `cpp/src/htrace/htrace_module.cpp`）：

```cpp
ModuleCapability XxxModule::capability() const { /* 见 §3 */ }

std::string XxxModule::run(const ModuleRequest& req, const RunContext& ctx) const {
    FormatKeyword kw = resolve_format_keyword(name(), capability(), req.format, req.parse_only);
    if (!kw.error.empty()) { log::loge("Error: ", kw.error); return {}; }
    const std::string& out_dir = ctx.output.out_dir;
    try {
        log::logi("Parsing...");
        auto parsed = xxx_parse(req.file_path, ctx);
        if (!req.parse_only) { /* analyze + 按 kw.keyword 选 writer */ }
        return xxx_write_parse_json(parsed, req.file_path, ctx, out_dir);
    } catch (const std::exception& e) { log::loge("Error: ", e.what()); return {}; }
}
```

### 3. 能力表约定（`ModuleRegistry::self_check()` 启动期校验，正常应零告警）

`ModuleCapability{ ModeCapability parse; ModeCapability analysis; }`，`ModeCapability{ formats, default_format }`：

- 某模式 `formats` 为空 = **明确不支持该模式**（如 snapshot 没有 parse 格式），此时 `-p` 被拒绝而不是回落到某格式；
- 声明了 `formats` 时，该模式的 `default_format` 必须非空且是 `formats` 的成员；
- `formats` 为空时 `default_format` 必须留空（否则是死数据）；
- **格式关键词直接就是路由键**：模块自己在 `run()` 里按 `(parse_only, keyword)` 选 writer。
  不存在「关键词 → formatter 名」的映射表，也**不要**再引入任何字符串注册表（P4-e 删掉的 `FormatterFactory` 那套已无消费者）。

### 4. 运行参数

一律**显式传递，不要再引入全局单例**。一次运行的参数由只读的 `RunContext` 承载，经 `process_file` →
`module.run(req, ctx)` → 域内自由函数按常引用一路下传。
**各域算法只接收自己那一份子结构**（`HtraceOptions` / `TshandleOptions` / `SnapshotOptions`），不得接收整个 `RunContext`。

### 5. 输出层（无多态）

- 管道层只决定**格式关键词**：`resolve_format_keyword(module_name, cap, explicit_format, parse_only)`。
  规则：显式 `-f` 不在该模式 `formats` 里就报「格式不支持」且**不回落默认格式**。
- JSON 的 `meta` 段统一由 `build_output_meta_base(ctx)`（`formatter/json/output_meta.hpp`）生成，勿各写一份。
- HTML 复用 `AnalysisHtmlFormatterBase`（`formatter/html/`，纯 static 工具）与 `html_analysis_common.hpp`
  （共享数据结构 / 辅助函数 / 页面模板）。**这两个头不得依赖任何域**。

### 6. 改动与验证纪律

- 每期改动**独立提交、独立全量回归**；搬目录与内容改动**分成两个 commit**（先 `git mv` + CMakeLists，再改代码），
  使回归基线干净、差异里不混搬文件噪声。`CMakeLists.txt` 是显式列举源文件，每次增删文件都要同步。
- 全量回归口径：**66 项 = JSON 40 + tshandle parse 3 + HTML 23**，须 `66/66 IDENTICAL`：

```bash
python build/compare_commits.py <base> <new> --skip-build --base-exe <base>/mem-analyzer.exe --new-exe <new>/mem-analyzer.exe
```

  base 用**上一个提交**的打包产物重建（`git worktree add --detach <dir> <commit>` → 跑 `build-win.ps1` → 解包到 `cpp/build_base/`）。
- **必须分开取 stdout / stderr 两路比对**（重定向后 stdout 块缓冲、stderr 无缓冲，合并流的交错顺序不稳定，会看到假差异）。
  临时目录随机名（`mem-analyzer.xxxxxx` / `tshandle.xxxxxx`）比对前要归一。
- `--help` 是 CLI 表面契约：主命令 + 各子命令的 stdout/stderr 需逐字节一致。
- 只改 include / 注释这类零语义改动，也应对**最终二进制**重跑一次全量回归再收尾。

### 7. 控制台输出约定（日志用 log，结果用 std::cout）

**运行时日志**（提示/进度/警告/错误/成功/调试）必须走 `log::`（`cpp/include/utils/logger.hpp`，
命名空间 `mem_analyzer::log`），不要散用 `std::cout`/`std::cerr` + `style::red/yellow/green`：

| 函数 | 用途 | 输出 |
|---|---|---|
| `loge` | 错误 | 红，stderr |
| `logw` | 警告 | 黄，stderr |
| `logok` | 成功 | 绿，stderr |
| `logi` | 常规信息 / 进度 | 正色(不染色)，stderr |
| `logi_minor` | 次要提示（跳过/忽略/note） | 灰，stderr |
| `logd` | 调试 / 命令回显（`--verbose` 才显示） | 灰，stderr |

- **`logi` vs `logi_minor` 的取舍**：`logi`（正色）承载**主线**——顺着读就能还原「现在在做什么、结果是什么」的
  主干步骤与关键状态；删掉它，用户会丢失关键脉络/结果。`logi_minor`（灰）承载**辅线/旁注**——对理解主线
  非必需，删掉后主线依旧完整。判据：**删掉这条，用户还能不能无歧义地看懂整个流程？** 能→`logi_minor`，不能→`logi`。
  典型 `logi`：阶段步骤（`Parsing...`/`Analyzing...`）、banner（`=== X Command ===`）、解析结论（`Resolved: ...`）、
  驱动运行的关键状态（`Device: ...`）。典型 `logi_minor`：耗时统计、跳过/忽略/note、临时目录清理、次级回显。

- 级别阈值：默认 `Info`，即 `logi/logi_minor/logok/logw/loge` 均显示，`logd`（`Debug`）默认被丢弃；
  顶层 `--verbose` flag 会把阈值降到 `Debug`（`main.cpp` 里 `set_level(verbose ? Debug : Info)`），`logd` 才生效。
  `logd` 只用于命令回显等调试信息，不要拿它承载用户应默认看到的内容。
- 变参用 `<<` fold 拼接，每个调用自带换行，别再写 `<< std::endl` 或结尾 `\n`。
- 正文可局部着色：`logi("...", style::bold("X"), "...")` 里 `style::bold`/`style::dim` 会透传生效
  （logger 对 ANSI 原样透传）；但**不要**再在参数里包 `style::red/yellow/green`——会与整条同色叠加/冲突。
- 耗时/阶段日志统一用 `StageTimer`（`utils/timer.hpp`），其内部已走 `logi_minor`（灰，stderr，受 `set_level` 控制）；不要为此散写 `std::cerr` + 手算 `now_ms()`。
- 多行错误正文（如错误后的提示 / `Suggested fix:`）应拼成一个带 `\n` 的字符串、一次 `loge(...)` 输出，
  tag 只挂第一行，不要散成多条 `std::cerr`。

**运行结果 / 输出**仍用 `std::cout`/`std::cerr`（是数据/结果，不是日志，走 logger 会污染解析或破坏时序）：

- 命令结果 / 结构化输出：JSON 内容、HTML 片段、`--list-apps` 列表、`mem-analyzer version`、`preview` 列表、`--help` 用法
- 流式实时输出：hdc transcript、轮询/倒计时/PSS 指标、`process_spinner` 原地重写

迁移映射：`style::red`→`loge`、`style::yellow`→`logw`、`style::green`→`logok`、普通信息→`logi`、
跳过/忽略/note→`logi_minor`、命令回显→`logd`。

### 8. 外部进程与命令注入防护（安全红线）

工具会执行外部进程（`trace_streamer` / `rawheap_translator` / `hdc`）并解压/解析不可信输入文件，
**任何把不可信字符串拼进 shell 命令的写法都视为命令注入**。约定：

- **信任边界**：CLI flag / 环境变量是可信输入；**被处理文件的内容**（文件名、zip 条目名、快照内字符串）
  是不可信输入——「分析第三方提供的 dump」是真实场景，别把文件名/文件内容当安全值。
- **能用库/API 就别调外部命令**：如解压 zip 用 `utils/zip_utils`（minizip API），不要调 `unzip`/`Expand-Archive`。
- **执行外部进程用参数数组，不拼 shell 字符串**：
  - Windows：`CreateProcessW` / `ShellExecuteW`（宽字符 + `path_from_utf8`）；
  - POSIX：`fork` + `execv`（参数数组），不经 `/bin/sh -c`。
  - 禁止用 `std::system` / `popen` / `_wsystem` 拼接文件路径或任何不可信内容。
- **多层 shell 逐层用对应转义**（`hdc shell` 是「本机 shell + 设备 shell」两层）：
  - 本机层：`proc::shell_quote`（平台自适应：Windows 用 cmd 规则 / POSIX 用 sh 规则）；
  - 设备层：`proc::shell_quote_device`（始终 POSIX 单引号，设备 shell 永远是 sh）。
  - 两者不能混用——设备侧参数在 Windows 本机上用 `shell_quote`（cmd 规则）转义会错。
- **解压/解析不可信文件要校验内容**：如 zip 条目路径拒绝绝对路径、盘符、`..`（防 zip-slip）。

### 9. 输入解析与内存/路径安全（安全基线）

工具解析的外部文件（`.htrace` / `.heapsnapshot` / `.rawheap` / `.zip`）内容均不可信，解析路径须守住以下基线：

- **SQL 一律参数绑定**：用 `sqlite3_prepare_v2` + `sqlite3_bind_*`，禁止字符串拼接 SQL；
  表名/列名只允许代码内常量（如 `native_hook` / `native_hook_statistic`）。
- **内存安全红线**：统一 `std::string` / `std::vector`；禁止 `strcpy` / `sprintf` / `gets` / `strcat`；
  `fgets(buf, sizeof(buf))` 必须用 `sizeof`；解析不可信数据（heapsnapshot 流式、zip、rawheap）对
  长度/计数/索引做边界与上限校验，防越界与整数溢出。
- **路径与临时文件**：临时目录用 `TempDir`（随机名），不要可预测固定路径；构造 `fs::path` 用
  `path_from_utf8()`；用户/设备进程名作为路径组件时先经 `as_path_segment()`。
- **敏感数据**：禁止把密钥/令牌/凭证硬编码进源码；日志不打印敏感数据（当前仅路径/进程名等非敏感值）。

## HTML 报告骨架与组件复用

新 HTML 报告统一用 `report_skeleton`（`formatter/html/report_skeleton.hpp`，12 个自由函数：
`html_report_begin/css_end/title/meta/body_open/body_close/data/data_streamed/script/core_script/end/escape_script_json`）。
jsleak 已迁；htrace/tshandle 仍走旧 `HTML_TEMPLATE`（`html_analysis_common.cpp`，待 Phase 3 删除）。
`AnalysisHtmlFormatterBase` / `html_analysis_common.hpp` 是旧骨架产物，**新代码勿再依赖**。

- **骨架迁移必然改输出**：`report_skeleton` 是统一外壳（`viewport` + `<main>` + 紧凑格式），与旧内联模板/
  HTML_TEMPLATE 的字节不兼容，「字节级 IDENTICAL」成立不了。迁移一个报告 = 接受外壳统一 + **重生成 base**，
  不要拿「迁移后字节不变」当目标。
- **组件分两层，报告按需注入**：
  - 必选核心（骨架自动注入，报告零配置）：`navigation_bar` / `help_tooltip` / `version`（footer 由 `html_report_end` 输出）。
  - 可选（报告按需注入）：`chart_module`（图）/ `tab_bar`（分类 tab）/ `tree_table`（数据树表）/ 域专属 CSS+JS。
  报告需要什么注入什么，不要把所有可选组件塞进骨架。
- **组件落点**（HTML 组件特化的 §1 判定）：只被一个域用 → 下沉该域 `formatters/`（如 `report_common`/`tree_table`
  只被 jsleak 用）；被 ≥2 域用 → 留 `formatter/html/components/`；两组件**消费者完全一致 + 语义相同** → 合并去前缀。
- **JS 复用陷阱（SSR vs client-render）**：
  - 同名 `window.Xxx` 导出**后者覆盖前者**：单快照与 compare 必须共用一套 JS（经 `report_common_js` 注入），
    不能各自导出同名函数。
  - SSR（C++ 生成 `onclick`/`id`）与 JS 侧拼接的 `id`/参数必须**逐字一致**；改了 id 生成规则必须同步改 JS，
    否则「点了没反应」。
  - 提取组件要**原字节抽出**才保回归 IDENTICAL；任何格式重排（紧凑 ↔ 多行）都是字节变更——要么接受 + 重生成 base，
    要么保持原样。CSS/HTML 片段统一「可读带空格缩进」，不写压缩单行。
## 扩展方法

### 新增数据域模块

1. 建目录 `cpp/{include,src}/<域>/`：`<域>_module.{hpp,cpp}`（入口）、`<域>_parser.{hpp,cpp}`、
   `<域>_analyzer.{hpp,cpp}`、`formatters/`（域内 writer 自由函数）。
2. 解析产物定义成该域头文件里的强类型 `struct`（对标 `TshandleParseResult`）。
3. 继承 `AnalysisModule`，实现 `name()` / `supports_extension()` / `capability()` / `run()`
   （骨架见 §2，能力表按 §3 声明）。
4. 在 `cpp/src/core/module_registry.cpp` 的注册块加一行 `ModuleRegistry::add(std::make_shared<XxxModule>());`。
5. 在 `cpp/CMakeLists.txt` 的 `SOURCES` 登记所有新 `.cpp`。
6. 在 `cpp/src/core/module_registry.cpp` 头部补 `#include "<域>/<域>_module.hpp"`。
7. 跑全量回归（§6）。若该域复用 htrace/tshandle 的模型，先按「往 shared/ 提取共享物」判定落点。

### 新增一种输出格式（同一模块）

1. 在该域 `formatters/` 加一个自由函数，形如
   `std::string <域>_write_<阶段>_<格式>(const <域>结果类型>&, const std::string& file_path, const RunContext& ctx, const std::string& out_dir);`
2. `capability()` 里把格式关键词加进对应模式的 `formats`（`parse` / `analysis`）；
   若它是该模式默认，改该模式的 `default_format`。
3. `run()` 里按 `kw.keyword` 分支调用。
4. 在 `cpp/CMakeLists.txt` 登记新 `.cpp`，跑全量回归。

### 新增 CLI 子命令

1. 新建 `cpp/src/cli/<name>_command.cpp`，实现 `ICommand`（`name/alias/description/order/bind/run`），
   文件内静态初始化器 `CommandRegistry::add(...)`。
2. `bind()` 里先调 `bind_common_options(sub, g)`，再声明本命令自己的参数。
3. `run()` 里用 `make_base_run_context(g)` 造 `RunContext`，填本命令的域参数，然后 `process_file(file, module, ctx)`。
4. `order()` 必须显式给值——help 顺序依赖它，不能依赖静态注册顺序。
5. 在 `cpp/CMakeLists.txt` 登记新 `.cpp`。**不需要改 `main.cpp`**。

### 新增 htrace 事件类型

1. `cpp/include/shared/htrace_model/htrace_data.hpp`：`HtraceEventType` 枚举末尾添加；
2. 同文件 `htrace_event_type_name()` / `htrace_event_type_from_name()` 补名称映射；
3. `cpp/include/htrace/htrace_processor.hpp`：`htrace_enabled_types()`（计不计入分析）；
4. 需要计尺寸则补 `htrace_count_size_types()`；
5. detail 模式补 `default_event_mappings()`。
6. 若该事件属于某个已有 category：在 `cpp/src/htrace/htrace_category.cpp` 的描述符表里给对应 spec 的
   `event_types` 补上（升序），让该 category 的 parse 过滤/类型聚合自动带上它（见下「新增 htrace category」）。

### 新增 htrace category / 轨道 / 行种（描述符表驱动）

htrace 的 category 语义收敛在 `cpp/include/htrace/htrace_category.hpp` 的 `HtraceCategorySpec` 描述符表里
（数据在 `cpp/src/htrace/htrace_category.cpp`），**消费方只查表、不写 `if (category == "xx")`**。三个正交维度：

1. **新增 category**（如新增 `pagecache`）：在 `htrace_category()` 的 `specs` 表加一行 `HtraceCategorySpec`：
   `name`（category 关键词，行为契约，勿改名）、`event_types`（该 category 允许的事件类型，升序）、
   `summary_total_key` / `html_type_class`（JSON 汇总键 / HTML 类型 CSS 类）、`count_mode`
   （fd/thread 这类「个数」语义置 true：同时把 analyzer 的字节 low_threshold 归零、HTML 切按个数显示）、
   `tracks`（轨道，见下）、`summary_entries`（build_summary 输出，顺序即 JSON 键顺序）。
2. **新增轨道**（一个 category 产出多段并列输出，如 mmap 的 anon+filepage 双轨）：在该 spec 的 `tracks`
   加一条 `HtraceTrackSpec{json_key, label, render}`，并让 analyzer 产出对应的 `HtraceSection`、
   让 `render` 落到已有行种。消费端（JSON/HTML writer）靠 `tracks`/`sections` 遍历，不硬编码「第几条是什么」。
3. **新增行种**（一段轨道里每行的数据种类）：`TrackRender` 加一个枚举值 + `HtraceRow`（`std::variant`）
   加一个备选 + 各 writer 的行级分派（`std::get_if`）加一个分支；这是编译期封闭类型集，改完编译器会揪出所有漏改的 writer。

易错：`label` 是 HTML 二级 tab 名的**唯一权威来源**（表里写全名，如「mmap-anon（匿名映射）」），
分析器把它抄进 `HtraceSection.label`、HTML 再读 section.label，不要在 writer 里再硬编码一份同义字符串。
### 往 shared/ 提取共享物

1. **先证明第二个使用者**：用 grep 列出真实调用点（不是「某头文件被 include」）。
2. 目标目录按被共享对象命名（`shared/<对象名>/`），不放按性质分类的大杂烩。
3. `git mv` + 脚本化重写 include 字符串 + 同步 `CMakeLists.txt`，**单独一个 commit**，跑一次全量回归。
4. 再改内容（去域语义、拆依赖），**另一个 commit**，再跑一次全量回归。
5. 确认 `shared/` 内没有任何 `#include "<域>/..."`。

## 易错点

- MinGW 交叉编译需 **posix 线程模型**（`<thread>/<mutex>` 依赖）。
- **中文/UTF-8 路径**：内部统一 UTF-8。Windows 上构造 `fs::path` 必须经 `path_from_utf8()`、取回字符串用 `path_to_utf8()`
  （声明在 `cpp/include/utils/file_utils.hpp`）；argv 的 ANSI→UTF-8 转换是 `cpp/src/main.cpp` 内的 static `acp_to_utf8()`，
  未做成公共工具，别处不要重复实现。
- **能力表自检**：新增 writer 只改 `run()` 不改 `capability()`，启动期会报「格式不支持」而非静默回落。
- **`-f` 不回落**：显式指定不支持的格式会直接报错并跳过，不会退回默认格式——这是有意行为，不要「修」成兜底。
- **`name()` 改名是行为变更**：会改变 `Using parser: <name>` 输出，回归会挂。
- **HTML/JSON 比对口径**：JSON 剥离 `meta`（时间戳/命令行），HTML 走归一化对比；不要用裸 `diff` 判产物。
- **引用链聚合 key 必须是「扩展名序列」而非「node_index 序列」**：`collect_ref_chains`
  （`jsleak/ref_chain_stage.cpp`）用 `HeapSnapshot::ref_chain_hash()` 对 target→root 的
  **扩展名（名字/类型）序列**做 128-bit FNV hash 作为分组 key。若误用 `node_index` 序列，
  会把「走同一类型路径的多个对象」拆成多条单节点链（top-chain 每条 nodeIds 变 1、retainedSize
  不再累加）。聚合粒度是「类型路径」，不是「具体对象路径」。

## 文档索引

- 概览 / 功能 / 构建：`README.md`
- 引擎接口、核心数据结构、htrace 配置字段：`cpp/README.md`
- 使用者手册（完整 CLI 参考）：`docs/usage.md`
- 跨版本输出回归脚本：`build/compare_commits.py`
