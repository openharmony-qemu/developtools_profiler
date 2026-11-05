# AGENTS.md

## 1. 代码地图

本 AGENTS.md 适用于 `host/smartperf/client/client_ui/` —— SmartPerf UI 版本（HAP 应用），基于 ArkTS/eTS Stage 模型开发。CLI 版本（SP_daemon）位于 `../client_command/`，不在本文件范围内；两者通过 UDP socket 协议协作。

SmartPerf UI 是 OpenHarmony 性能测试应用，采集 CPU、GPU、FPS、DDR、RAM、Power、Temperature、Network 等指标，通过悬浮窗实时展示，并将结果写入 rdb 数据库与 CSV 报告文件。带屏幕设备使用此 UI 版本，无屏幕/低性能设备使用 CLI 版本。

最重要的架构边界有三条：(1) **工厂 + 单例采集器模式** —— 每个采集器继承 `BaseProfiler`，由 `ProfilerFactory` 按 `CollectorType` 分发；(2) **`globalThis` 共享状态总线** —— 采集配置、定时器、数据队列、设备能力等运行时状态全部挂在 `globalThis` 上跨线程/页面共享；(3) **UDP socket 与 SP_daemon 进程通信** —— FPS/RAM/截图/trace 数据由 worker 线程经 UDP 向 SP_daemon 进程请求获取。

关键区域：
- `BUILD.gn`：HAP 打包配置；`ohos_hap("SmartPerf")`，bundleName `com.ohos.smartperf`，安装目录 `app/com.ohos.gameperceptio`，subsystem `developtools`/part `hiprofiler`，`js_build_mode = "debug"`，`ets2abc = true`
- `AppScope/app.json`：应用元数据；minAPIVersion 8，targetAPIVersion 9，bundleName `com.ohos.smartperf`
- `entry/src/main/module.json`：模块配置；入口 `./ets/Application/AbilityStage.ts`，mainElement `MainAbility`，声明 9 项权限（含 `SYSTEM_FLOAT_WINDOW`、`GET_INSTALLED_BUNDLE_LIST`、`READ/WRITE_USER_STORAGE`、`KEEP_BACKGROUND_RUNNING`），deviceTypes `default`+`tablet`，uiSyntax `ets`
- `entry/build-profile.json`：`apiType: stageMode`、`arkEnable: true`，targets `default` + `ohosTest`（ohosTest 当前无实际源码）
- `entry/hvigorfile.js`：使用 `@ohos/hvigor-ohos-plugin` 的 `hapTasks`（HVigor 构建工具，非标准 npm/tsc）
- `entry/src/main/resources/base/profile/main_pages.json`：**页面路由注册表** —— 新增页面必须在此登记
- `entry/src/main/ets/Application/AbilityStage.ts`：应用级入口；设置 `globalThis.hapModuleName = 'entry'`
- `entry/src/main/ets/MainAbility/MainAbility.ts`：UIAbility 入口；创建 worker 线程（`entry/ets/workers/worker.js`）、绑定 `MainWorker.onmessage -> WorkerHandler.socketHandler`、加载 `pages/LoginPage` 为初始页、前台时调用 `initDb()` + `FloatWindowFun.initAllFun()`、按屏宽计算 `globalThis.coefficient`（>1400→1，800-1400→1.8，其余→1，用于悬浮窗尺寸缩放）
- `entry/src/main/ets/workers/worker.js`：worker 线程；UDP 绑定 `127.0.0.1:8282`，向 `127.0.0.1:8283`（SP_daemon 监听端口）发送消息；协议字符串：`set_pkgName::<pkg>`、`get_fps_and_jitters`、`get_ram_info::<pkg>`、`get_capture`、`catch_trace_start`/`catch_trace_end`、`set_dubai_db`；回传消息格式 `RAM$<data>`、`FPS$<fps>$<jitters>`、`UdpStatus$<n>`
- `entry/src/main/ets/common/profiler/base/BaseProfiler.ets`（采集器抽象基类）：抽象基类；`init()`/`isSupport()`/`readData()` 是数据契约
- `entry/src/main/ets/common/profiler/base/ProfilerFactory.ets`（采集器工厂）：`CollectorType` → 单例采集器映射；**新增采集器在此注册**
- `entry/src/main/ets/common/profiler/base/ProfilerConstant.ts`（采集器常量定义）：`CollectorType` 枚举（CPU/GPU/DDR/FPS/COUNTER/POWER/TEMP/RAM/NETWORK/UNKNOWN）、`TaskStatus`、`CatchTraceStatus`
- `entry/src/main/ets/common/profiler/base/SocketProfiler.ets`（采集器 socket 接口）：需从 worker 消息队列读取数据的采集器接口（`readMessageQueue()`）
- `entry/src/main/ets/common/profiler/item/*.ets`（采集器实现目录）：具体采集器（CPU/GPU/FPS/Power/RAM/Thermal/DDR/NetWork）—— 全部单例，继承 `BaseProfiler`；FPS/RAM 实现 `SocketProfiler`（数据来自 SP_daemon）
- `entry/src/main/ets/common/profiler/ProfilerTask.ets`（采集任务编排器）：单例编排器；`initModule()`/`taskInit()`/`taskStart()`（每 tick 读取循环）/`taskSingleItemStart()`/`taskStop()`（落库 + 生成 CSV）/`taskGetDubai()`（查询 dubai.db 功耗归因）/`taskDestroy()`
- `entry/src/main/ets/common/profiler/WorkerHandler.ts`（采集器 worker 消息处理）：解析 worker 消息（按 `$` 切分 RAM/FPS/UdpStatus），写入 `globalThis` 队列
- `entry/src/main/ets/common/database/DatabaseUtils.ts`：rdb（`@ohos.data.relationalStore`）操作；建表 `t_general_info`/`t_index_info`/`task_powersensor_info`/`task_powerapp_info`；查询外部 `dubai.db` 做功耗归因
- `entry/src/main/ets/common/database/LocalRepository.ts`：`initDb()`（gp.db + t_general_info）、`initDbIndex()`（按会话时间戳建 `<dbTime>.db` + t_index_info）、`getReportListDb()`
- `entry/src/main/ets/common/constant/ConstantSQL.ts`：全部建表 DDL + `dbName = 'gp.db'`；**`t_index_info` 列名是持久化输出契约**
- `entry/src/main/ets/common/constant/ConstantsPath.ts`：`AppFileRealDir = '/data/app/el2/100/base/com.ohos.smartperf/files/'`
- `entry/src/main/ets/common/entity/DatabaseEntity.ets`：`GPData`/`TGeneralInfo`/`TIndexInfo`/`TPowerSensorInfo`/`TPowerAppInfo`
- `entry/src/main/ets/common/utils/SPLogger.ts`：日志门面（DEBUG/INFO/WARN/ERROR，tag 前缀 `SmartPerf:`）
- `entry/src/main/ets/common/ui/floatwindow/FloatWindowFun.ts`：向 `globalThis` 注册窗口管理函数（CreateFloatingWindow/MoveFloatingWindow/CreateTitleWindow/Show/Hide/Destroy 等），使用 `@ohos.window` `TYPE_FLOAT`
- `entry/src/main/ets/common/ui/detail/`：报告详情页组件（Summary/Power/PowerDubai/Temperature/Performance/Load + chart/data/utils）
- `entry/src/main/ets/common/ui/main/`：主页子组件（Home/Report/Mine/TopComponent/HomeBottomPage）
- `entry/src/main/ets/pages/*.ets`：19 个页面（含 MainPage、StartTestPage、AppSelectPage、ReportDetail、各折线图页、FloatBall、TitleWindowPage 等）

任务导航：
- 新增/修改采集器 -> `profiler/base/ProfilerFactory.ets`（采集器工厂）、`profiler/item/<Name>.ets`、`profiler/base/ProfilerConstant.ts`（采集器常量，如有新类型）、`profiler/ProfilerTask.ets`（采集任务编排逻辑）、`database/DatabaseUtils.ts`（`gpArray2Index` switch 与入库列）、`constant/ConstantSQL.ts`（`t_index_info` 列）
- 修改 UDP 消息协议 -> `workers/worker.js`（发送/接收）、`profiler/WorkerHandler.ts`（采集器消息解析）、对应 `item/` 采集器；**必须与 SP_daemon 端（`../client_command/`）保持一致**
- 修改页面路由 -> `pages/<Name>.ets`（新增 `@Entry` 页面）、`resources/base/profile/main_pages.json`（登记 src）
- 修改权限 -> `entry/src/main/module.json` 的 `requestPermissions`
- 修改 DB schema -> `constant/ConstantSQL.ts`（DDL）、`database/DatabaseUtils.ts`（插入/查询列）、`entity/DatabaseEntity.ets`（实体）
- 修改悬浮窗 -> `common/ui/floatwindow/FloatWindowFun.ts` + `FloatWindowConstant.ets`
- 修改构建/打包 -> `BUILD.gn`、`entry/build-profile.json`、`build/config.gni`（采集模块根）

## 2. 知识路由

在规划或编辑前，先对任务分类并读取匹配的文档。通过 `skill` 工具加载技能。

### 基于任务的路由
- 新增采集器或修改采集逻辑 -> 先阅读 `profiler/ProfilerTask.ets`（采集任务生命周期）与 `profiler/base/BaseProfiler.ets`（采集器契约），参考 `profiler/item/CPU.ets`（sysfs 直读型）或 `profiler/item/FPS.ets`（socket 型）
- 修改与 SP_daemon 的 UDP 通信 -> 必须对照 `../client_command/` 的 socket 协议实现（端口 8282/8283、消息字符串）
- 修改持久化 schema 或 CSV 输出 -> 阅读 `constant/ConstantSQL.ts` 与 `database/DatabaseUtils.ts`，确认列名与 `gpArray2Index` 的 `moduleType` switch 一致
- 新增页面 -> 阅读 `resources/base/profile/main_pages.json` 路由注册表与现有 `pages/` 页面的 `@Entry @Component struct` 模式
- 修改功耗归因 -> 阅读 `database/DatabaseUtils.ts` 的 `query_applications_*` 系列（查询外部 `dubai.db`）
- C++ 侧（SP_daemon）变更 -> 使用 skill `sp-daemon-collector-comprehensive`；参见 `../client_command/AGENTS.md`

### 基于路径的路由
- `entry/src/main/module.json` -> 权限与 Ability 配置的唯一事实来源；新增权限或 Ability 必须在此编辑
- `entry/src/main/resources/base/profile/main_pages.json` -> 页面路由注册表；新增 `@Entry` 页面必须在此登记，否则路由找不到
- `common/profiler/base/ProfilerFactory.ets`（采集器工厂） -> 新采集器必须在 `getProfilerByConfig` 的 if-else 链中注册
- `common/profiler/ProfilerTask.ets`（采集任务编排） -> 采集生命周期编排；`taskStart` 的 `gpDataArr` 装配顺序与 `gpArray2Index` 的 switch 一一对应
- `common/profiler/base/BaseProfilerUtils.ets`（采集器工具基类） -> `fileOpen`/`travelFile`/`isAccess` 是 sysfs 节点读取的统一入口；采集器不应绕过它直接用 `fileio`
- `workers/worker.js` -> UDP 协议字符串的 HAP 端实现；与 `../client_command/include/common.h` 的 `MESSAGE_MAP` 是契约对端
- `BUILD.gn` -> 任何新增源文件路径变化通过 `ohos_js_assets`（source_dir = `entry/src/main/ets`，整目录打包）隐式包含；但 `signature/openharmony_smartperf.p7b` 签名文件路径若移动需同步更新

### 基于术语的路由

| 术语 | 风险提示 | 阅读 |
|---|---|---|
| globalThis | 运行时共享状态总线；`dbTime`/`startTime`/`endTime`/`collectConfigs`/`collectPkg`/`fpsArr`/`ramArr`/`useDaemon`/`coefficient`/`abilityContext`/`MainWorker` 等散落各处，跨页面跨线程读写 —— 修改变量前需搜索全部引用 | 全仓库 |
| CollectorType | 采集器类型枚举；字符串值（如 `'POWER'`、`'TEMP'`、`'NETWORK'`）是 `globalThis.collectConfigs` 的键和工厂分发依据，不得随意改名 | `profiler/base/ProfilerConstant.ts`（采集器常量定义） |
| GPData.moduleType | `gpArray2Index` switch 的 case 值（`CPU`/`CPULoad`/`GPU`/`DDR`/`Temp`/`Power`/`RAM`/`FPS`/`NetWork`）；与采集器 `createGPData` 第一参数必须一致 | `database/DatabaseUtils.ts:471`、各 `item/*.ets` |
| SP_daemon | HAP 依赖的后台采集进程（CLI 变体）；若未运行，`useDaemon=false`，FPS/RAM 等采集器返回空数据 | `../client_command/`、`README_zh.md` |
| dubai.db | 外部功耗归因数据库；HAP 通过 relationalStore 查询 `detailed_applications_*` 表获取 display/cpu/gpu/audio/ddr/dss/system_idle/wifi_data/sensor/rom 维度功耗 | `database/DatabaseUtils.ts` |
| dbTime | 会话级时间戳主键；每次 `taskInit` 生成 `<dbTime>.db` 与 `<dbTime>/` CSV 目录，贯穿采集/入库/报告全流程 | `profiler/ProfilerTask.ets`（采集任务编排） |
| coefficient | 屏宽驱动的 UI 缩放系数（1 或 1.8）；悬浮窗/标题窗尺寸均乘以此值 | `MainAbility/MainAbility.ts`、`floatwindow/FloatWindowFun.ts` |
| CatchTraceStatus | trace 自动抓取状态机；帧 jitter > 100ms 触发 `catch_trace_start`，60 tick 冷却（`catch_trace_times = 60`） | `profiler/base/ProfilerConstant.ts`（采集器常量定义）、`profiler/WorkerHandler.ts`（采集器消息处理） |
| HVigor | OpenHarmony 应用构建工具（非 npm/tsc/webpack）；`entry/hvigorfile.js` 使用 `hapTasks`，`ets2abc=true` 编译 ArkTS 字节码 | `entry/hvigorfile.js`、`entry/build-profile.json` |
| AppFileRealDir | 沙箱存储真实路径 `/data/app/el2/100/base/com.ohos.smartperf/files/`；对应沙箱路径 `abilityContext.filesDir` | `constant/ConstantsPath.ts` |

在计划中声明：
- 任务分类（新增采集器 / 协议变更 / schema 变更 / 页面变更 / 权限变更）
- 已读文档 / 已调用技能
- 发现的约束
- 是否应使用某个 skill/工作流

## 3. 约束与边界

### 架构/领域不变量
- 所有采集器必须继承 `BaseProfiler` 并实现 `init(): CollectorType`、`isSupport(): Boolean`、`readData(): GPData`。`GPData.moduleType` 与 `createGPData` 第一参数必须与 `DatabaseUtils.gpArray2Index` 的 switch case 一致 —— 这是数据装配的唯一纽带。
- 采集器必须是单例（`private static instance` + `static getInstance()`），由 `ProfilerFactory.getProfilerByConfig` 返回；禁止 `new` 采集器。
- `globalThis` 是跨 Ability/页面/worker 的共享状态总线；`taskInit`/`taskStart`/`taskStop`/`taskDestroy` 依赖一批约定变量（`dbTime`/`startTime`/`endTime`/`collectIntervalNum`/`tTndexs`/`tTndex`/各队列等）的初始化与重置顺序，不得跳过。
- UDP socket 协议（端口 8282 绑定 / 8283 发送、消息字符串 `set_pkgName::`/`get_fps_and_jitters`/`get_ram_info::`/`get_capture`/`catch_trace_start`/`catch_trace_end`/`set_dubai_db`）由 HAP 与 SP_daemon 双端解析 —— 修改一端必须同步另一端。
- `t_index_info` 的列名（如 `cpu0Frequency`/`gpuFrequency`/`currentNow`/`pss`/`fps`/`fpsJitters` 等）是 rdb 入库与 CSV 输出的公共契约，被报告页消费 —— 不得重命名已有列。
- 每个文件使用 Apache 2.0 许可证头。`.ets` 用于 UI/装饰器（`@Entry`/`@Component`/`@State`），`.ts` 用于纯逻辑；新增文件按此区分。
- 设备约束：仅支持带屏幕设备（deviceTypes `default`+`tablet`）；自 OH 3.2 起预制。

### 禁止事项
- 禁止重命名 `CollectorType` 枚举字符串值或 `GPData.moduleType` —— 它们是 `collectConfigs` 键和 `gpArray2Index` switch 的依据。
- 禁止重命名 `t_index_info` 已有列 —— 报告详情页（`common/ui/detail/`）与 CSV 导出依赖它们。
- 禁止修改 UDP 消息字符串或端口而不同步 `../client_command/`（SP_daemon）端 —— 会破坏 HAP 与 daemon 互操作。
- 禁止绕过 `ProfilerFactory` 直接实例化采集器。
- 禁止绕过 `BaseProfilerUtils.fileOpen`/`isAccess` 直接使用 `@ohos.fileio` 读 sysfs —— 统一的错误处理与 `null` 返回约定依赖于此。
- 禁止为通过测试而删除 `SPLogger.DEBUG/INFO/WARN/ERROR` 调用 —— 日志是 DFX 的一部分。
- 禁止在不理解 `coefficient` 缩放逻辑的情况下修改悬浮窗尺寸常量 —— 多分辨率适配依赖它。
- 禁止在 `module.json` 中随意增删权限而不评估影响 —— 每项权限对应采集能力（如 `SYSTEM_FLOAT_WINDOW`→悬浮窗、`READ/WRITE_USER_STORAGE`→报告读写）。

### 必须先询问
- 在 `module.json` 中新增或删除权限声明。
- 修改 UDP 协议字符串或端口（需与 SP_daemon 团队协调）。
- 修改 `t_index_info`/`t_general_info` 列名或 CSV 列顺序。
- 修改 `build/config.gni`（影响整个 SmartPerf 采集子系统）。
- 新增 `external_deps` 或第三方依赖（当前 `entry/package.json` 的 dependencies 为空，HAP 仅依赖 OHOS 系统能力）。
- 在已连接设备上执行破坏性 hdc 操作（`hdc_std shell` kill 进程、`rm` 等）。

## 4. 验证

### 最小检查
SmartPerf UI 作为 OpenHarmony 全源码树的一部分通过 GN + Ninja + HVigor 构建，无独立构建脚本。典型命令从 OH 源码根目录执行：

- 构建 HAP：`./build.sh --product-name <product> --gn-args 'hiprofiler_SmartPerf=true' --build-target SmartPerf`
- 安装到设备：构建产物为 HAP，通过 `hdc install` 推送（需连接带屏幕的 OpenHarmony 设备）
- 前置依赖：确认设备上 `SP_daemon` 进程已启动（`hdc_std shell` → `ps -ef | grep SP_daemon`，不存在则执行 `SP_daemon` 拉起），否则 FPS/RAM/截图/trace 等能力不可用
- 编译器/语法约束：ArkTS 严格模式（`ets2abc`）；`.ets` 使用 `@Entry`/`@Component`/`@State` 装饰器；混合 `.ts`/`.ets`，注意 import 路径与扩展名
- 无独立 lint/typecheck 脚本；语法错误会在 HVigor 编译阶段暴露

### 按任务类型的检查
- 新增采集器 -> 在 `ProfilerFactory` 注册；在 `ConstantSQL.ts` 的 `t_index_info` 增列；在 `DatabaseUtils.gpArray2Index` 增 case；在 `ProfilerTask.taskStart` 增读取；验证 `readData()` 返回的 `GPData` 键与入库列一致
- UDP 协议变更 -> 同步 `../client_command/` 端；在设备上启动 SP_daemon 后验证 HAP 能收到 `UdpStatus$1` 并置 `useDaemon=true`
- DB schema 变更 -> 同步 DDL（`ConstantSQL.ts`）、插入列（`DatabaseUtils.ts`）、查询列、实体（`DatabaseEntity.ets`）、报告页消费处
- 新增页面 -> 在 `main_pages.json` 登记；确认 `@Entry struct` 命名与路由路径一致
- 权限变更 -> 确认对应能力在 `MainAbility`/采集器中实际使用；验证签名配置（`signature/openharmony_smartperf.p7b`）仍匹配
- 悬浮窗/窗口变更 -> 在真机上验证多分辨率（>1400 / 800-1400 / 其它）下尺寸与拖拽边界
- 仅 UI 变更 -> 至少验证初始页 `LoginPage` → `MainPage` → `StartTestPage` 流程

### 完成定义
仅当满足以下条件时，任务才算完成：
- 请求的行为已实现。
- 若新增了页面，`main_pages.json` 已登记；若新增了采集器，工厂、常量、schema、编排、入库链路均已同步。
- 已运行相关构建检查，或已说明无法运行的原因（如无可用带屏设备）。
- `CollectorType` 枚举值、`GPData.moduleType`、`t_index_info` 列名、UDP 协议字符串保持不变，除非任务明确要求修改且已与对端协调。
- 最终回复包含变更文件、验证结果和剩余风险。
- 不包含无关的格式化、重构或附带变更。
