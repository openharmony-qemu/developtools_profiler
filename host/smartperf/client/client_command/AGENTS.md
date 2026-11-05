# AGENTS.md

## 1. 代码结构

本 AGENTS.md 适用于 `host/smartperf/client/client_command/` —— SP_daemon 命令行工具，即 OpenHarmony SmartPerf 的 CLI 变体。UI 变体位于 `../client_ui/`，不在本文件范围内。

SP_daemon 是一个单二进制性能采集器，从 OpenHarmony 设备采集 CPU、GPU、FPS、DDR、RAM、Power、Temperature、Network 等指标，将结果打印到 stdout 并写入 CSV 文件 `/data/local/tmp/data.csv`。

最重要的架构边界是**工厂 + 单例采集器模式**：每个指标采集器都继承自 `SpProfiler`，通过 `ItemData()` 返回 `map<string,string>`，并由 `SpProfilerFactory` 根据 `CommandType`（CLI）或 `MessageType`（socket/daemon）进行分发。

关键区域：
- `smartperf_main.cpp`：程序入口；参数校验、开发者模式门控，分发到 `SmartPerfCommand` 或特定命令（`-editor`、`-start/-stop`、`-profilerfps` 等）
- `smartperf_command.cpp/.h`：CLI 命令解析与执行；启动 `-server`/`-editorServer`/`-deviceServer` socket 监听
- `include/common.h`：**所有命令/消息枚举与字符串映射表** —— `COMMAND_MAP`、`COMMAND_SHELL_MAP`、`MESSAGE_MAP`、`CMD_COMMAND_MAP`、`HIDUMPER_CMD_MAP`、`HISYSEVENT_CMD_MAP`。修改 CLI 标志或 socket 消息字符串必须在此编辑。
- `include/sp_profiler.h`（采集器基类）：采集器抽象基类；`ItemData()` 是数据契约
- `sp_profiler_factory.cpp`（采集器工厂）：将 `CommandType`/`MessageType` 映射到具体采集器单例；**在此注册新采集器**
- `sp_thread_socket.cpp`：editor 和 device server 模式的 TCP/UDP socket 消息处理；实现 `init::`/`start::`/`stop::` 协议
- `sp_server_socket.h`：底层 socket 封装（UDP 8283、TCP 8284、UDP-EX 8285）
- `sp_task.cpp/.h`：editor 模式的任务生命周期（`InitTask`/`StartTask`/`StopTask`）
- `client_control.cpp`：`-start`/`-stop` 后台采集控制
- `editor_command.cpp` → `control_call_cmd.cpp`：基于场景的 editor 命令（responseTime、completeTime、fpsohtest、frameLoss 等）
- `include/`：所有头文件（每个 .cpp 对应一个同名 .h）
- `interface/`：游戏引擎 / GPU counter 插件的回调接口（`GameServicePlugin.h`、`GpuCounterCallback.h` 等）
- `etc/`：系统参数文件（`spdaemon.para`、`spdaemon.para.dac`）
- `test/unittest/`：HWTEST_F 单元测试；`test/fuzztest/`：libFuzzer 模糊测试

任务导航：
- 修改 CLI 标志或 socket 消息 -> `include/common.h`（映射表是唯一事实来源）
- 修改 editor/device TCP 协议 -> `sp_thread_socket.cpp`、`client_control.cpp`
- 修改 start/stop daemon 流程 -> `client_control.cpp`、`smartperf_main.cpp`
- 修改构建依赖或编译器标志 -> `BUILD.gn`、`build/config.gni`
- 新增/修改单元测试 -> `test/unittest/`、`test/BUILD.gn`

## 2. 知识路由

在规划或编辑前，先对任务分类并读取匹配的文档。通过 `skill` 工具加载技能。

### 基于任务的路由
- 新增 SpProfiler 采集器（CPU/GPU/FPS 等）-> 使用 skill `sp-daemon-collector-comprehensive`
- 新增基于插件的采集器（外部 .so、游戏引擎回调）-> 使用 skill `sp-daemon-plugin`
- 修改 VisualEffectAnalysis（RS/FE trace 指标）-> 使用 skill `sp-daemon-visual-effect-analysis`
- 生成或更新 HWTEST_F 单元测试 -> 使用 skill `oh-ut-generator`
- C++ 变更的安全审查 -> 使用 skill `ohos-cpp-security-review-enhanced`

### 基于路径的路由
- `include/common.h` -> 此文件定义所有 CLI 标志、socket 消息和 shell 命令字符串；任何标志/消息变更必须在 `COMMAND_MAP`、`COMMAND_SHELL_MAP`、`MESSAGE_MAP` 和 `CMD_COMMAND_MAP` 之间保持一致
- `BUILD.gn` -> 任何新增 `.cpp` 源文件必须**同时**添加到 `BUILD.gn`（约第 43 行 sources）和 `test/BUILD.gn`（约第 24 行 sources）
- `interface/` -> 插件契约；变更影响外部 .so 插件，必须保持 ABI

### 基于术语的路由

| 术语 | 风险提示 | 阅读 |
|---|---|---|
| SpProfiler / sp_profiler_factory | 采集器抽象基类与工厂；`ItemData()` 返回 `map<string,string>` —— 这是数据契约，非随意命名 | `include/sp_profiler.h`、`sp_profiler_factory.cpp` |
| MESSAGE_MAP | daemon <=1.0.6 UDP 通信的 socket 消息字符串；修改某个值会破坏 HAP 兼容性 | `include/common.h:66` |
| COMMAND_MAP / COMMAND_SHELL_MAP | CLI 标志 → CommandType 映射；`COMMAND_MAP` 使用大写标志（-CPU），`COMMAND_SHELL_MAP` 使用小写（-c） | `include/common.h:158,224` |
| CMD_COMMAND_MAP | shell 命令字符串（hitrace、kill、pidof 等）；通过 `SPUtils::LoadCmd` 在设备上执行 | `include/common.h:290` |
| init::/start::/stop:: | editor/device TCP 协议命令；`init:::`（带额外冒号）跳过 token 校验 | `sp_thread_socket.cpp` |
| RK flag | RK3568 与 Hi3516 设备差异；许多采集器根据 `isRk` 分支选择 sysfs 路径 | `sp_utils.cpp`（`SetRkFlag`） |
| Dubai | 通过 `hidumper -s 1213` 获取功耗数据；DB 文件位于 `/data/service/el2/100/xpower/dubai.db` | `Dubai.cpp` |
| GpuCounter | 通过插件回调采集 GPU 性能计数器；单独注册 | `GpuCounter.cpp`、`interface/GpuCounterCallback.h` |
| arkxtest_able | 条件构建标志；当 `testfwk_arkxtest` part 存在时，添加 arkxtest 依赖 | `build/config.gni` |
| HWTEST_F | OpenHarmony GTest 宏；测试使用 `TestSize.Level1` 和 `testing::ext` 命名空间 | `test/unittest/` |

在计划中声明：
- 任务分类（新增采集器 / 标志变更 / 协议变更 / 测试）
- 已读文档 / 已调用技能
- 发现的约束
- 是否应使用某个 skill/工作流

## 3. 约束与边界

### 架构不变量
- 所有采集器必须继承 `SpProfiler` 并实现 `ItemData()` 返回 `std::map<std::string, std::string>`。map 的键（如 `cpu0Frequency`、`gpuload`、`fps`）是 **公共输出契约**，被 SmartPerf Editor 和 CSV 导出消费 —— 不得重命名已有键。
- 采集器必须是单例（私有构造函数 + `static Instance` 模式）。工厂返回单例实例的指针；禁止 `new`/`delete` 采集器。
- `include/common.h` 中的映射表是定义在头文件中的 `const` 全局变量，被多个编译单元包含 —— 它们是 CLI 标志和 socket 消息的唯一事实来源。禁止在其它位置复制标志字符串。
- socket 协议响应（`init::True`/`init::False`/`start::True`/`stop::True`）由 SmartPerf Editor 客户端解析 —— 修改格式会破坏互操作。
- 整个代码库命名空间为 `OHOS::SmartPerf`。每个文件使用 Apache 2.0 许可证头。

### 禁止事项
- 禁止未经明确批准引入新的生产依赖。`BUILD.gn` 的 `external_deps` 是受控列表。
- 禁止修改已有 CLI 标志字符串（`-c`、`-g`、`-f`、`-N`、`-PKG` 等）—— 它们是面向用户的契约，已在 `--help` 中文档化。
- 禁止重命名出现在 CSV 输出中的 `ItemData()` map 键 —— 外部工具和 Editor 依赖它们。
- 禁止绕过工厂模式；采集器访问必须通过 `SpProfilerFactory::GetCmdProfilerItem*` 路由。
- 禁止为通过测试而删除 `LOGD`/`LOGE`/`WLOGI` 日志调用。日志是 DFX 的一部分。
- 禁止在不理解命令的情况下执行破坏性设备操作（`rm -rf`、`kill -9`、`taskset`）—— 许多命令在 `CMD_COMMAND_MAP` 中，会在真实设备上运行。
- 禁止未经与 Editor/HAP 客户端协调修改 socket 端口（UDP 8283、TCP 8284、UDP-EX 8285）。
- 禁止在不理解系统参数权限的情况下修改 `etc/spdaemon.para` 或 `spdaemon.para.dac`。

### 必须先询问
- 在 `BUILD.gn` 中新增 `external_deps` 条目。
- 修改 socket 协议格式或端口。
- 修改 `ItemData()` 键名或输出 CSV 列顺序。
- 修改 `build/config.gni`（影响整个 SmartPerf 采集子系统）。
- 新增设备特定分支（RK vs Hi3516）—— 需确认目标硬件上 sysfs 路径存在。
- 在已连接的设备上运行任何 `CMD_COMMAND_MAP` 命令。

## 4. 验证

### 最小检查
SP_daemon 在 OpenHarmony 全源码树中使用 GN + Ninja 构建。无独立构建。典型命令从 OH 源码根目录执行：

- 构建 daemon：`./build.sh --product-name <product> --gn-args 'hiprofiler_SmartPerf=true' --build-target SP_daemon`
- 构建单元测试：`./build.sh --product-name <product> --build-target sp_daemon_ut`
- 在设备上运行单元测试：推送 `sp_daemon_ut` 二进制文件，运行 `./sp_daemon_ut`（需通过 `hdc` 连接 OpenHarmony 设备）
- 模糊测试：构建 `fuzztest` 组，运行 `spdaemon_fuzzer` harness

`BUILD.gn` 中强制执行的编译器标志：`-Wall -Werror -g3 -Wunused-variable -Wno-unused-but-set-variable -fexceptions -O2 -flto -fvisibility=hidden`。触发警告的代码将导致构建失败。

### 按任务类型的检查
- 新增采集器 -> 添加到 `BUILD.gn` sources **和** `test/BUILD.gn` sources；在 `test/unittest/` 中创建 `<name>_test.cpp`；在工厂中注册；验证 `ItemData()` 返回预期的键
- CLI 标志变更 -> 更新 `common.h` 中的 `COMMAND_MAP` 和/或 `COMMAND_SHELL_MAP`；更新 `smartperf_command.h` 中的 `--help` 文本；验证 `smartperf_main.cpp` 中 `g_checkCmdParam` 白名单
- socket 协议变更 -> 验证 `init::`/`start::`/`stop::` 响应格式未变或已与 Editor 协调；用 `SP_daemon -editorServer` + Editor 客户端测试
- Start/Stop 流程变更 -> 在设备上测试 `SP_daemon -start -c` 然后 `SP_daemon -stop`；验证 CSV 生成
- 仅测试变更 -> 运行修改的测试及至少一个相邻相关测试
- 原生 C++ 变更 -> 构建受影响模块并运行附近单元测试；考虑运行安全审查 skill

### 完成定义
仅当满足以下条件时，任务才算完成：
- 请求的行为已实现。
- 如果新增了文件，`BUILD.gn` 和 `test/BUILD.gn` 源文件列表已更新。
- 已运行相关的构建/测试/lint 检查，或已说明无法运行的原因（如无可用设备）。
- `ItemData()` map 键和 CLI 标志字符串保持不变，除非任务明确要求修改。
- 最终回复包含变更文件、验证结果和剩余风险。
- 不包含无关的格式化、重构或附带变更。
