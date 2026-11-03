# mem-analyzer — C++ 核心引擎

C++17 实现的内存分析引擎，覆盖 htrace 原生内存泄漏分析、tshandle 句柄泄漏分析和快照泄漏分析三大场景。

> 项目概览和功能介绍见根目录 [README.md](../README.md)。

---

## 目录

- [快速构建](#快速构建)
- [项目结构](#项目结构)
- [流水线架构](#流水线架构)
- [接口层次结构](#接口层次结构)
- [核心数据结构](#核心数据结构)
- [如何扩展](#如何扩展)
  - [Htrace 配置文件](#htrace-配置文件)
- [Htrace 泄漏分析算法](#htrace-泄漏分析算法)
- [ArkTS 类分析流水线](#arkts-类分析流水线)
- [Heap Snapshot 解析引擎](#heap-snapshot-解析引擎)
- [事件类型系统](#事件类型系统)
- [运行参数（RunContext）](#运行参数runcontext)
- [日志系统（logger）](#日志系统logger)
- [平台适配](#平台适配)
- [依赖](#依赖)

---

## 快速构建

```bash
# 使用构建脚本（推荐）
.\build-win.ps1          # Windows
./build-mac.sh           # macOS
./release-cross.sh       # Linux 跨平台
```

### 构建系统说明

- **CMake >= 3.19**，C++17 标准
- Linux 下优先使用 Clang（clang18 → clang11 降序查找），未找到则回退 GCC
- 全静态链接：Linux `-static`，MinGW `-static-libstdc++ -static-libgcc`
- MinGW 交叉编译需要 **posix 线程模型**（`x86_64-w64-mingw32-g++-posix`），win32 线程模型不支持 `<thread>`/`<mutex>`。`release-cross.sh` 自动优先选择 posix 变体
- 旧编译器（GCC<12, Clang<9）额外链接 `stdc++fs`
- macOS 最低部署目标 10.15（`std::filesystem` 要求）
- SQLite 编译为单线程模式（`SQLITE_THREADSAFE=0`，`SQLITE_OMIT_LOAD_EXTENSION`）

---

## 项目结构

```
cpp/
├── CMakeLists.txt
├── build-win.ps1 / build-mac.sh / release-cross.sh
├── include/                      # 头文件
│   ├── cli/                      #   L0 命令层
│   │   ├── cli_parser.hpp        #     自实现参数解析器
│   │   ├── command.hpp           #     ICommand + CommandRegistry + GlobalOptions
│   │   └── pipeline_runner.hpp   #     单文件 process_file：选模块 → run
│   ├── profiler/                 #   L1 采集层：hdc 设备采集
│   ├── core/                     #   跨层契约
│   │   ├── module.hpp            #     AnalysisModule：一个数据域一条纵切管道
│   │   ├── module_capability.hpp #     ModuleCapability：能力表（模式 / 格式 / 默认格式）
│   │   ├── module_registry.hpp   #     ModuleRegistry：L0 只认这张表
│   │   ├── format_resolver.hpp   #     resolve_format_keyword()：只决定格式关键词
│   │   └── run_context.hpp       #     RunContext：一次运行的显式只读参数
│   ├── htrace/                   #   L2 数据域：htrace 原生内存
│   │   ├── htrace_module.hpp     #     模块入口（parse → analyze → render）
│   │   ├── htrace_parser.hpp     #     trace_streamer 转 DB + 调用栈解析
│   │   ├── htrace_analyzer.hpp   #     等价符号检测 / up-down approximation
│   │   ├── htrace_processor.hpp  #     事件类型表与配置
│   │   ├── flame_icicle.hpp      #     火焰图 / 冰柱图
│   │   └── formatters/           #     域内 writer（自由函数，强类型入参）
│   │       ├── htrace_parse_json_formatter.hpp / htrace_analysis_json_formatter.hpp
│   │       ├── htrace_json_formatter.hpp   # meta / summary 公共片段
│   │       ├── htrace_html_formatter.hpp
│   │       └── htrace_flame_json.hpp  #  FlameNode → 火焰/冰柱 JSON（htrace 专用）
│   ├── tshandle/                 #   L2 数据域：句柄泄漏（zip）
│   │   ├── tshandle_module.hpp   #     模块入口，name() = "zip"
│   │   ├── zip_parser.hpp        #     解压 ZIP + ArkTS 类关联（parse 阶段完成）
│   │   ├── tshandle_analyzer.hpp #     dominator / retained size / nativeRef 关联
│   │   ├── arkts_analyzer.hpp    #     ArkTS 类分析
│   │   ├── tshandle_retained_size.hpp
│   │   ├── tshandle_processor.hpp#     TshandleParseResult 等强类型载体
│   │   ├── map_parser.hpp        #     .map 文件解析
│   │   └── formatters/           #     tshandle_formatter / tshandle_html_formatter / tshandle_analysis_json_formatter / tshandle_call_graph
│   ├── jsleak/                   #   L2 数据域：快照泄漏
│   │   ├── jsleak_module.hpp     #     模块入口，name() = "snapshot"
│   │   ├── snapshot_parser.hpp   #     V8 heap snapshot 流式解析
│   │   ├── snapshot_analyzer.hpp #     属性标签 + 最短引用链
│   │   ├── snapshot_compare.hpp  #     双快照变化对象与泄漏对象对比
│   │   ├── property_tag_stage.hpp / ref_chain_stage.hpp
│   │   └── formatters/           #     snapshot_single_{json,html}_formatter / snapshot_compare_formatter
│   ├── shared/                   #   L3 被 ≥2 个域复用的模型与算法
│   │   ├── heapsnapshot/         #     HeapSnapshot、source map、name cache、反混淆
│   │   └── htrace_model/         #     TraceDB、htrace 事件类型、DB helper
│   ├── formatter/                #   L4 基建：输出公共件（不得依赖任何数据域）
│   │   ├── html/                 #     HTML 公共件
│   │   │   ├── html_analysis_common.hpp        #  共享数据结构 / 辅助函数 / 页面模板
│   │   │   ├── analysis_html_formatter_base.hpp#  公共工具（纯 static）
│   │   │   └── components/       #     chart_module / navigation_bar / help_tooltip（CSS/JS 片段）
│   │   └── json/                 #     JSON 公共件
│   │       ├── json_writer.hpp / json_stream_writer.hpp
│   │       └── output_meta.hpp   #     build_output_meta_base() 统一 meta 构造
│   ├── config_mgr/               #   L4 配置：htrace_config_mgr（分类/过滤/算法参数）、preview_features_mgr
│   ├── utils/                    #   L4 基建：平台 / 文件 / 时间 / 终端样式 / 版本
│   └── config.hpp                #   Scope 枚举 + defaults 默认值
└── src/                          # 源文件（与 include/ 对应）
```

---

## 流水线架构

所有输入文件由一个数据域模块独立跑完整条管道（parse → analyze → render）：

```
输入文件 → 按扩展名选模块 → [parse] → 域内强类型结构 → [analyze] → 域内结果 → [render] → 输出文件
```

`process_file` 只负责派生 RunContext、调用 `module.run()`、按需自动打开产物——管道层不认识任何具体数据域。

### 处理流程

1. **Parse** — 模块自己解析，产物是域内强类型结构（编译期确定，无 `std::any`）
   - `.htrace` → `HtraceModule`：调用 `trace_streamer` 外部工具将 htrace 转为 SQLite DB，解析调用栈
   - `.zip` → `TshandleModule`：解压 ZIP，提取 htrace + rawheap，parse 阶段即完成 ArkTS 类关联
   - `.heapsnapshot` → `JsleakModule`：流式解析 V8 heap snapshot JSON（`HeapSnapshot`）
2. **Analyze** — 对解析结果执行泄漏分析算法
   - htrace：等价符号检测、up/down approximation、火焰图/冰柱图构建
   - tshandle：dominator tree、retained size、nativeRef 关联
   - snapshot：属性标签检测、最短引用链分析
3. **Render & Write** — 按格式关键词走域内 writer（自由函数，强类型入参），直接写文件
   - JSON：`{ meta, content }` 结构，meta 由 `build_output_meta_base()` 统一生成
   - HTML：自包含的交互式报告，内嵌 Canvas 火焰图/冰柱图

### 格式解析

管道层只决定「用哪个格式关键词」，不碰任何 writer 实例：

```cpp
// include/core/format_resolver.hpp
FormatKeyword resolve_format_keyword(const std::string& module_name,
                                     const ModuleCapability& cap,
                                     const std::string& explicit_format,  // 显式 -f，空 = 用默认
                                     bool parse_only);
```

规则：该模式的格式列表为空 → 该模式不支持，直接报错；显式 `-f` 不在该模式格式列表里 →
报「格式不支持」**不回落默认格式**；未显式指定则用该模式默认格式（默认格式缺失时取格式列表的第一个），
最后用格式列表校验。

---

## 接口层次结构

L0 与数据域之间只有一个接口：`AnalysisModule`。模块在 `main()` 之前用静态初始化器注册进 `ModuleRegistry`。

### AnalysisModule / ModuleRegistry

```cpp
// include/core/module.hpp
struct ModuleRequest {
    std::string file_path;
    bool parse_only = false;
    std::string format;          // 显式 -f；空 = 用模块默认格式
};

class AnalysisModule {
public:
    virtual ~AnalysisModule() = default;
    virtual std::string name() const = 0;                         // htrace / zip / snapshot
    virtual bool supports_extension(const std::string& ext) const = 0;
    virtual ModuleCapability capability() const = 0;              // 调用期求值
    virtual std::string run(const ModuleRequest& req, const RunContext& ctx) const = 0;
};

// include/core/module_registry.hpp
class ModuleRegistry {
public:
    static void add(std::shared_ptr<AnalysisModule> module);
    static AnalysisModule* find(const std::string& name);
    static AnalysisModule* find_by_extension(const std::string& ext);
    static AnalysisModule* detect(const std::string& file_path);
    static std::vector<AnalysisModule*> all();
    static std::vector<std::string> self_check();   // 能力表一致性自检，正常应为空
};
```

**ModuleCapability** 声明该域支持的格式。格式关键词**直接就是路由键**——模块自己按
`(parse_only, keyword)` 选 writer，没有「关键词 → formatter 名」的中间映射：

```cpp
struct ModeCapability {
    std::vector<std::string> formats;   // 支持的格式（空 = 该模式不支持，如 snapshot 没有 parse）
    std::string default_format;         // 未给 -f 时用的格式；formats 为空时必须留空
    bool supported() const { return !formats.empty(); }
};

struct ModuleCapability {
    ModeCapability parse;
    ModeCapability analysis;
    const ModeCapability& for_mode(bool parse_only) const;
};
```

三个模块的真身与历史 parser 名的对应关系（`name()` 保持历史值，`Using parser: <name>` 输出不变）：

| 模块 | `name()` | 扩展名 | `parse.formats` | `parse.default_format` | `analysis.formats` | `analysis.default_format` |
|---|---|---|---|---|---|---|
| `HtraceModule` | `htrace` | `.htrace` | `json` | `json` | `json` / `html` | `html` |
| `TshandleModule` | `zip` | `.zip` | `json` | `json` | `json` / `html` | `html` |
| `JsleakModule` | `snapshot` | `.heapsnapshot` | —（不支持 `-p`） | — | `html` / `json` | `html` |

### 输出层（无多态）

P4-e 起不再有 `IFormatter` / `JsonFormatter` / `FormatterFactory`。各域的输出是
`<域>/formatters/*` 里的**自由函数**，入参为域内强类型结构：

```cpp
// 例：cpp/include/tshandle/formatters/tshandle_formatter.hpp
std::string tshandle_write_parse_json(const TshandleParseResult& parsed,
                                      const std::string& file_path,
                                      const RunContext& ctx, const std::string& out_dir);
```

管道只把「格式关键词」交给模块（`resolve_format_keyword()`），模块内部按关键词走对应 writer。

**JSON meta** — `build_output_meta_base(ctx)`（`include/formatter/json/output_meta.hpp`）统一生成公共字段：

```cpp
json meta = build_output_meta_base(ctx);
// → generator / generatorVersion / generateTimestamp / command
```

**HTML 公共工具** — `AnalysisHtmlFormatterBase`（`include/formatter/html/analysis_html_formatter_base.hpp`）
已退化为纯 static 工具类：Tab 栏 / Tab 内容区 / 流式模板填充 / 公共占位符替换 / 页脚版本片段，
各域 writer 直接调用。


**格式关键词 → writer**：路由不在注册表里，而在各域 `run()` 的 `(parse_only, keyword)` 分支上：

| 模块 | parse-only | analysis |
|---|---|---|
| htrace | `json` → `htrace_write_parse_json()` | `json` → `htrace_write_analysis_json()`；`html` → `htrace_write_analysis_html()` |
| zip | `json` → `tshandle_write_parse_json()` | `json` → `tshandle_write_analysis_json()`；`html` → `tshandle_write_analysis_html()` |
| snapshot | —（不支持） | `json` → `jsleak_write_analysis_json()`；`html` → `jsleak_write_analysis_html()` |

---

## 核心数据结构

### 符号字典

```cpp
using DataDict = std::unordered_map<int, std::string>;
```

整数 ID → 字符串的字典，用于栈帧中 symbol/file/build ID 的延迟解析。

### 栈帧层次结构

```cpp
// 基类：记录深度和是否已解析
struct StackFrame {
    int depth = 0;
    bool resolved = false;
};

// 已解析栈帧（字符串字段，tshandle 管道使用）
struct ResolvedStackFrame : StackFrame {
    std::string symbol;       // 函数/符号名（含偏移）
    std::string lib;          // 库/文件路径
    std::string lib_offset;   // 库内偏移（十六进制字符串，如 "0x1234"）
    std::string build_id;     // 库 build ID
};

// 压缩栈帧（整数 ID，htrace 管道使用，延迟解析以节省内存）
struct CompressedStackFrame : StackFrame {
    int symbol_id = 0;
    int file_id = 0;
    bool file_id_null = false;
    uint64_t vaddr = 0;
    bool vaddr_null = false;
    int build_id = 0;
    bool build_id_null = true;

    ResolvedStackFrame resolve(const DataDict& dict) const;  // 通过字典解析为字符串
};
```

### 调用栈层次结构

```cpp
// 基类：所有调用栈的公共字段
struct CallStack {
    long long id = 0;
    HtraceEventType type = HtraceEventType::UNKNOWN;
    long long existing_size = 0;       // 原始分配大小（字节）
    long long existing_count = 0;
    double related_size = 0.0;         // 关联 retained/shared 大小
    std::shared_ptr<std::vector<std::string>> existing_addrs;  // tshandle: 地址列表
};

// 已解析调用栈（tshandle 管道）
struct ResolvedCallStack : CallStack {
    std::vector<ResolvedStackFrame> frames;
};

// 压缩调用栈（htrace 管道，延迟解析）
struct CompressedCallStack : CallStack {
    std::vector<CompressedStackFrame> frames;

    ResolvedCallStack resolve(const DataDict& dict) const;  // 通过字典解析
};
```

### TraceDB — 解析后的追踪数据库

```cpp
// 压缩调用栈列表（含字典）
struct CompressedCallStackList {
    DataDict dict;
    std::vector<CompressedCallStack> stacks;
};

// 已解析调用栈列表（不含字典）
struct ResolvedCallStackList {
    std::vector<ResolvedCallStack> stacks;
};

// 调用栈列表的变体类型
using CallStackList = std::variant<CompressedCallStackList, ResolvedCallStackList>;

// 解析后的追踪数据库抽象 — 解析阶段的输出，下游统一消费
struct TraceDB {
    CallStackList stacks;
    TraceMeta meta;

    bool is_compressed() const;   // 是否包含压缩（int-ID）调用栈
    size_t stack_count() const;   // 总调用栈数量
    void resolve_stacks();        // 如果是压缩格式，解析为 ResolvedCallStackList
};
```

### TraceMeta — 追踪级元数据

```cpp
struct TraceMeta {
    long long start_time_ms = 0;   // 追踪开始时间（毫秒）
    long long end_time_ms = 0;     // 追踪结束时间（毫秒）
    long long duration_ns = 0;     // 追踪时长（纳秒）

    std::string process_name;

    // htrace 专用
    std::string mode;              // "statistic" 或 "detail"
    std::unordered_map<HtraceEventType, long long> type_sizes;  // 各类型累计大小
};
```

### 分析结果

```cpp
struct TopSymbolInfo {
    uint64_t uid = 0;                // (symbol_id << 32) | file_id
    std::string symbol;
    std::vector<std::string> libs;
    std::string lib_offset;
    std::string type;                // "malloc" / "ark_global_handle" 等
    long long total_leak_size = 0;
    long long total_existing_count = 0;  // 包含此符号的所有链的 existing_count 总和
    bool is_app = false;
    int build_id = 0;
    bool build_id_null = true;
    FlameNode flame_graph;           // 向上火焰图 Trie
    FlameNode icicle_graph;          // 向下冰柱图 Trie
    std::vector<CompressedCallStack> call_chains;
};

struct TopSymbols {
    std::string mode;                // "app-only" / "system-only" / "app-and-system" / "mixed"
    std::vector<TopSymbolInfo> app;
    std::vector<TopSymbolInfo> system;
    std::vector<TopSymbolInfo> mixed;
};

struct AlgorithmStats {
    int org_set_size = 0;            // 原始符号数
    int filtered_set_size = 0;       // 低阈值过滤后
    int final_set_size = 0;          // 算法输出
    int equivalent_groups = 0;       // 等价组合并数
    int up_approx_removed = 0;       // up-approximation 移除数
    int down_approx_removed = 0;     // down-approximation 移除数
    int representation_added = 0;    // 代表性恢复数
};

struct HtraceAnalysisOutput {
    long long total_leak_size = 0;
    TopSymbols top_symbols;
    AlgorithmStats stats;

    // 从解析结果携带的字段（格式化器需要）
    std::unordered_map<int, std::string> data_dict;  // 火焰图/冰柱图符号解析
    std::string mode;                                 // "statistic" 或 "detail"
    std::string process_name;
    std::unordered_map<HtraceEventType, long long> type_sizes;
    TraceMeta trace_meta;

    json to_json() const;
};

struct AnalysisConfig {
    long long low_threshold = 1024;                     // 最小泄漏过滤（字节）
    int appro_rel_ratio = 10;                            // 近似差值相对比例（%）
    long long appro_rel_cap = 10LL * 1024 * 1024;        // 近似差值相对阈值封顶（10MB）
    int appro_ratio_threshold = 30;                      // 近似贡献占比阈值（%）
    int orphan_rel_ratio = 1;                            // 孤儿栈兜底显著度相对比例（%）
    long long orphan_rel_cap = 10LL * 1024 * 1024;       // 孤儿栈兜底阈值封顶（10MB）
    int top_n = 10;                                      // Top N 输出数量
};
```

### 火焰图/冰柱图 Trie

```cpp
struct FlameKey {
    int symbol_id, file_id;
    bool file_id_null;
    int depth;
};

struct FlameNode {
    int symbol_id = 0;
    int file_id = 0;
    bool file_id_null = false;
    uint64_t vaddr = 0;
    bool vaddr_null = false;
    int build_id = 0;
    bool build_id_null = true;
    int depth = 0;                   // 火焰图为负，冰柱图为正
    long long leak_size = 0;
    long long existing_count = 0;    // 经过此节点的分配的累计 existing_count
    std::vector<FlameNode> children;
    std::unordered_map<FlameKey, size_t, FlameKeyHash> child_map;  // O(1) 子节点查找

    json to_flame_json(const DataDict& data_dict) const;
    json to_icicle_json(const DataDict& data_dict) const;
};
```

### Tshandle 数据结构

```cpp
// ArkTS 类分析
struct NativeRefEntry {
    HtraceEventType handle_type;
    long long call_stack_id;
    std::vector<std::string> addrs;
};

struct ArkTsInstance {
    uint32_t id;
    long long shallow_size;
    long long retained_size;
    std::vector<NativeRefEntry> native_ref;
};

struct ArkTsClass {
    std::string name;
    long long shallow_size;
    long long retained_size;
    std::vector<ArkTsInstance> instances;
};

struct AnomalyCounts {
    int multi = 0;
    int no_node = 0;
    int missing_node = 0;
    int invalid_addr = 0;
    int duplicate = 0;
    int addr_file_invalid = 0;
};

// tshandle 解析结果（tshandle_parse() 的返回值，直接传给 tshandle_analyze()）
struct TshandleParseResult {
    TraceDB db;
    std::string parsed_at;
    std::vector<ArkTsClass> arkts_classes;
    AnomalyCounts anomaly_counts;
    std::vector<std::string> anomaly_lines;
};

// tshandle 分析结果
struct TshandleAnalysisOutput {
    TraceMeta trace_meta;
    std::string parsed_at;
    std::vector<ResolvedCallStack> call_stacks;
    std::vector<ArkTsClass> arkts_classes;

    TshandleHandleSections global;   // GlobalHandle 按调用链拆分为 app / sys 两个 section
    TshandleTypeSection local;       // LocalHandle 不拆分

    json to_json() const;
};
```

---

## 如何扩展

### 新增数据域模块

一个数据域 = 一个顶层目录 + 一个模块类，自持 parse → analyze → render。

1. 建目录 `include/xxx/` 与 `src/xxx/`：`xxx_module.hpp/.cpp`（入口）、`xxx_parser.hpp/.cpp`、
   `xxx_analyzer.hpp/.cpp`、`formatters/`（域内 writer 自由函数）
2. 继承 `AnalysisModule`，实现 `name()`、`supports_extension()`、`capability()`、`run()`
3. 在 `src/core/module_registry.cpp` 的注册块里加一行：

```cpp
const bool REGISTERED = []() {
    ModuleRegistry::add(std::make_shared<HtraceModule>());
    ModuleRegistry::add(std::make_shared<TshandleModule>());
    ModuleRegistry::add(std::make_shared<JsleakModule>());
    ModuleRegistry::add(std::make_shared<MyModule>());   // ← 新增
    return true;
}();
```

4. 在 `CMakeLists.txt` 的 `SOURCES` 中添加 `.cpp` 文件

**能力表约定**（`ModuleRegistry::self_check()` 会在启动期校验，不一致会打印告警）：

- 某模式 `formats` 为空 = 明确不支持该模式（如 snapshot 没有 parse 格式），此时 `-p` 会被拒绝而不是回落到某格式；
- 声明了格式列表时，该模式的 `default_format` 必须非空且是 `formats` 的成员；
- `formats` 为空时 `default_format` 必须留空（否则是死数据）。

### 新增事件类型

1. 在 `include/shared/htrace_model/htrace_data.hpp` 的 `HtraceEventType` 枚举末尾添加
2. 在 `htrace_event_type_name()` 和 `htrace_event_type_from_name()` 中添加名称映射
3. 在 `include/htrace/htrace_processor.hpp` 的 `htrace_enabled_types()` 中添加
4. 如需计尺寸，在 `htrace_count_size_types()` 中添加
5. （detail 模式）在 `default_event_mappings()` 中添加事件字符串映射

### Htrace 配置文件

Htrace 分析行为可通过配置文件 `config/htrace/trace_analyzer_config.json` 自定义。配置文件缺失或字段缺失时，自动回退到内置默认值，不影响正常运行。

#### 配置文件查找路径

配置文件从**固定目录**读取：可执行文件同级目录下的 `config/htrace/trace_analyzer_config.json`，即 `<exe_dir>/config/htrace/trace_analyzer_config.json`。该文件不存在时，使用内置默认值。

#### 完整配置示例

```json
{
  "version": "1.0",

  "algorithm": {
    "low_threshold": 1024,
    "appro_rel_ratio": 10,
    "appro_rel_cap": 10485760,
    "appro_ratio_threshold": 30,
    "orphan_rel_ratio": 1,
    "orphan_rel_cap": 10485760
  },

  "symbol_classification_rules": [
    {
      "type": "system",
      "lib_contains": ["libc++.so", "libc++_shared.so", "/data/storage/el1/bundle/arkwebcore/libs/arm64/libarkweb_engine.so"]
    },
    {
      "type": "app",
      "lib_contains": [
        "/system/app/",
        "/sys_prod/app/",
        "/data/storage/el1/bundle"
      ]
    },
    {
      "type": "app",
      "require_no_vaddr": true,
      "lib_ends_with": [".hap", ".hsp", ".har"]
    },
    {
      "type": "app",
      "require_no_vaddr": true,
      "lib_ts_pattern": [".ts", ".js", ".ets"]
    }
  ],

  "symbol_filter_rules": [
    {
      "scope": "global",
      "lib": ["unknown", ""],
      "symbol_contains": ["alloc size("]
    },
    {
      "scope": "global",
      "lib": ["libc++.so", "libc++_shared.so"],
      "symbol_contains": ["operator new"]
    },
    {
      "scope": "global",
      "lib": ["stub.an", "libuv.so", "libipc_single.z.so", "ld-musl-aarch64.so.1", "stateMgmt.js"],
      "orphan_recoverable": true
    }
  ]
}
```

#### 字段说明

##### `version` (string)

配置文件版本号，当前为 `"1.0"`。预留用于未来兼容性检查。

##### `algorithm` (object)

算法阈值参数：

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `low_threshold` | int64 | 1024 | 最小泄漏大小（字节），低于此值的符号被过滤 |
| `appro_rel_ratio` | int | 10 | 近似差值相对比例（%），允许 diff 最多为被取代者总泄漏量的 10% |
| `appro_rel_cap` | int64 | 10485760 | 近似差值相对阈值封顶（字节，10MB） |
| `appro_ratio_threshold` | int | 30 | 近似贡献占比下限（%），共同链泄漏值需占对应符号总泄漏量的 30% |
| `orphan_rel_ratio` | int | 1 | 孤儿栈兜底显著度相对比例（%），门槛 = `min(总泄漏量 × 该值/100, orphan_rel_cap)` |
| `orphan_rel_cap` | int64 | 10485760 | 孤儿栈兜底阈值封顶（字节，10MB） |

> `low_threshold` 对 fd 和 thread 类型无效（这些类型的"大小"实际是计数，而非字节数），会被自动置为 0。

##### `symbol_classification_rules` (array)

符号分类规则数组。**数组顺序即优先级**，第一个匹配的规则生效，后续规则不再检查。

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `type` | string | 是 | 分类结果：`"system"` 或 `"app"` |
| `lib_contains` | string[] | 否 | 库路径包含匹配（子串匹配），任一匹配即生效 |
| `lib_ends_with` | string[] | 否 | 库路径后缀匹配，任一匹配即生效 |
| `lib_ts_pattern` | string[] | 否 | TS 源码模式匹配（如 `.ts:42:15`），任一匹配即生效 |
| `require_no_vaddr` | bool | 否 | 为 true 时，有虚拟地址的符号不匹配此规则 |

**匹配逻辑**：一个规则中的多个匹配条件（`lib_contains`、`lib_ends_with`、`lib_ts_pattern`）是 **OR 关系**——任一条件满足即匹配。

##### `symbol_filter_rules` (array)

符号过滤规则数组。每条规则独立检查，匹配即过滤（移除）。

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `scope` | string | 是 | 规则生效模式：`"global"`（所有模式）、`"app"`（app-only / app-and-system / mixed）、`"system"`（system-only / app-and-system / mixed） |
| `lib` | string[] | 否 | 库文件名精确匹配，如果指定了 lib 列表，库文件名必须至少匹配其中一项 |
| `symbol_contains` | string[] | 否 | 符号名包含匹配（子串） |
| `symbol_equals` | string[] | 否 | 符号名精确匹配 |
| `orphan_recoverable` | bool | 否 | 孤儿栈兜底时是否可回收该规则命中的符号，默认 `false`。详见下文「孤儿栈兜底」。 |

**匹配逻辑**：
1. `scope` 检查：规则的 `scope` 与当前分析模式匹配才生效，不匹配则跳过该规则
   - `"global"`：所有模式均生效
   - `"app"`：仅在 app-only / app-and-system / mixed 模式下生效
   - `"system"`：仅在 system-only / app-and-system / mixed 模式下生效
2. `lib` 检查：如果指定了 lib 列表，库文件名必须至少匹配其中一项
3. `symbol_contains` / `symbol_equals`：任一匹配即过滤

**`orphan_recoverable` 字段**：

控制「孤儿栈兜底」（见「Htrace 泄漏分析算法」的 [孤儿栈兜底](#孤儿栈兜底)）时，该规则命中的噪声符号是否可被重新拉回排行：

- 默认 `false`：命中的符号在孤儿栈兜底时**不**被拉回。`alloc size(`、`operator new` 这类「故意不参与计算」的符号应保持 `false`。
- 设为 `true`：命中的噪声符号在孤儿栈兜底时可被拉回。
- 一个符号只有在其命中的**所有**过滤规则都标记为 `orphan_recoverable: true` 时才可回收；只要命中任意一条 `false` 的规则，就不回收。

```json
{
  "scope": "global",
  "lib": ["stub.an", "libuv.so", "ld-musl-aarch64.so.1"],
  "orphan_recoverable": true
}
```

#### 默认规则

当配置文件不存在或对应数组为空时，使用以下内置默认值：

**默认分类规则**：

| 优先级 | 类型 | 条件 | 说明 |
|--------|------|------|------|
| 1 | system | lib 包含 `libc++.so` 或 `libc++_shared.so` | C++ 标准库 |
| 2 | app | lib 包含 `/system/app/`、`/sys_prod/app/`、`/data/storage/el1/bundle` | 应用安装路径 |
| 3 | app | lib 后缀为 `.hap`/`.hsp`/`.har`，且无 vaddr | 应用 bundle |
| 4 | app | lib 匹配 TS 模式（`.ts:行:列` 等），且无 vaddr | TS/JS/ETS 源码 |

**默认过滤规则**：

| 作用域 | 库 | 符号匹配 | 说明 |
|--------|-----|----------|------|
| global | `unknown` / `""` | 包含 `alloc size(` | 未知库的分配器桩符号 |
| global | `libc++.so` / `libc++_shared.so` | 包含 `operator new` | C++ 标准 new |

#### 边界情况

| 场景 | 行为 |
|------|------|
| 配置文件不存在 | 使用内置默认规则，分析正常执行 |
| JSON 格式错误 | `load()` 返回 false + 错误信息，`load_default()` 失败后抛出异常 |
| 字段缺失 | 缺失字段使用结构体默认值 |
| 空数组 | 使用内置默认规则兜底 |

#### 扩展示例

**新增分类规则**（添加到数组末尾优先级最低，添加到数组开头优先级最高）：

```json
{
  "type": "app",
  "lib_contains": ["/vendor/app/"],
  "require_no_vaddr": false
}
```

**新增过滤规则**：

```json
{
  "scope": "app",
  "lib": ["libskia.so"],
  "symbol_contains": ["SkCanvas::draw"]
}
```

**调整算法参数**：

```json
{
  "algorithm": {
    "low_threshold": 2048,
    "appro_rel_ratio": 10,
    "appro_rel_cap": 10485760,
    "appro_ratio_threshold": 50
  }
}
```

---

## Htrace 泄漏分析算法

实现在 `htrace/htrace_analyzer.cpp` 的 `analyze_call_stacks()` 中，共 9 步：

| 步骤 | 函数 | 说明 |
|:----:|------|------|
| 1 | `build_symbol_data` | 遍历调用栈，提取每个符号的特征：uid（`symbol_id << 32 \| file_id`）、lib、chain_ids、总泄漏量、最大深度。调用 `is_app_symbol()` 判定 app/system |
| 2 | `split_by_scope` | 按 app/system 分割符号集 |
| 3 | `find_equivalent_groups` | 出现在完全相同 chain ID 集合中的符号视为等价组 |
| 4 | 低阈值过滤 | 移除 < 1024 bytes 的符号，以及 `operator new` 等噪音符号 |
| 5 | 等价合并 | 等价组内保留最大深度的符号为代表，移除其余 |
| 6 | `calc_approximations` | 分析调用链内的浅层/深层符号近似关系（up-approx：浅层符号被深层近似；down-approx：深层符号被浅层近似） |
| 7 | 不动点过滤 | 反复移除被近似的符号直到不再变化 |
| 8 | 代表性检查 | 被移除的符号若无任何代表者，重新加入 |
| 9 | 排序输出 | 按泄漏量降序取 Top-N，为每个符号构建火焰图/冰柱图 Trie |

### 算法阈值

可通过 `AnalysisConfig` 调节（`include/htrace/htrace_analyzer.hpp`）：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `low_threshold` | 1024 bytes | 最小泄漏过滤 |
| `appro_rel_ratio` | 10% | 近似差值相对比例（相对被取代者总泄漏量） |
| `appro_rel_cap` | 10MB | 近似差值相对阈值封顶 |
| `appro_ratio_threshold` | 30% | 近似贡献占比下限 |
| `orphan_rel_ratio` | 1% | 孤儿栈兜底显著度相对比例（相对总泄漏量） |
| `orphan_rel_cap` | 10MB | 孤儿栈兜底阈值封顶 |
| `top_n` | 10 | Top N 输出数量 |

### 孤儿栈兜底

在符号级过滤之后存在一类特殊情况：某条调用链中**所有符号（无论 app 还是 system）都被 `symbol_filter_rules` 判定为噪声**。此时这条链的泄漏大小没有任何幸存符号承接，会从排行里凭空丢失，这类链被称为「孤儿栈」。

兜底逻辑（`build_filtered` 的 Pass 2）分三步：

1. **判定**：逐条扫描调用链，若整条链没有任何一个非噪声符号（即所有符号都命中 `symbol_filter_rules`），且链中存在属于当前 scope 的符号，则该链为孤儿栈。
2. **聚合**：对孤儿栈中标记为「可回收」的噪声符号，按符号累加其在孤儿栈内的泄漏大小（同一条链里同一个符号只计一次）。
3. **恢复**：孤儿栈内聚合泄漏量达到显著度门槛的符号，以完整副本重新加入候选集。

关键约束：

- **孤儿栈判定看整链**：只要链中存在任何一个非噪声符号（无论它属于 app 还是 system），这条链的泄漏就没有丢失，不会被当作孤儿栈。这避免了在 app-and-system 等模式下，把「泄漏已被另一侧符号承接」的链误判为孤儿栈。
- **显著度门槛**：只有符号在孤儿栈内的聚合泄漏量 ≥ `min(总泄漏量 × orphan_rel_ratio/100, orphan_rel_cap)` 时才恢复，避免把碎片化的微小孤儿栈重新带入排行、引入干扰。默认 `orphan_rel_ratio = 1`（1%）、`orphan_rel_cap = 10MB`。
- **恢复值用完整调用范围**：恢复符号沿用原始符号的完整 `chain_ids` / `total_leak_size`（而非只算孤儿栈部分），保证其报告的泄漏大小准确，不漏算非孤儿栈贡献。
- **仅回收「可回收」符号**：只有命中规则且该规则 `orphan_recoverable: true` 的符号才会被拉回；`alloc size(`、`operator new` 这类「故意不参与计算」的符号保持不可回收（默认 `false`）。

孤儿栈兜底与配置的关系见上文「Htrace 配置文件」的 `orphan_recoverable`、`orphan_rel_ratio`、`orphan_rel_cap` 字段说明。

---

## ArkTS 类分析流水线

实现在 `tshandle/arkts_analyzer.cpp` 的 `analyze_arkts_classes()` 中：

1. **解析地址映射文件** — 读取 addr 文件（`<address> <nodeId>`），标准化十六进制地址，去除 0x 前缀和重复
2. **加载 V8 Heap Snapshot** — 使用流式解析器加载，构建节点/边索引和字符串表
3. **retained size** — `HeapSnapshot::from_file` 内已由 `node_size_calculator.cpp`（Lengauer-Tarjan）计算，直接读 `heap.get_retained_size()`
4. **构建 nativeRef 关联** — 将 ARK handle 中的 existingAddrs 映射到 heapsnapshot nodeId
5. **按类名聚合** — 通过 nodeId 查询 shallow_size 和 retained_size，聚合为 `ArkTsClass`
6. **生成异常报告** — 输出 multi/noNode/missingNode/invalid/duplicate/addr_file_invalid 六类异常到 `addr_anomaly.txt`
7. **返回结果** — 包含类列表、异常计数和异常行

> 注意：retained size 统一由 `shared/heapsnapshot/node_size_calculator.cpp`（Lengauer-Tarjan，排除 weak + shortcut 边）在 `HeapSnapshot::from_file` 内一次计算；tshandle 与 jsleak 管道均复用快照上的结果，不再有独立的 Cooper 实现。

---

## Heap Snapshot 解析引擎

实现在 `shared/heapsnapshot/`（解析引擎，jsleak 与 tshandle 共用）与 `jsleak/`（jsleak 域）中，提供 V8 Heap Snapshot（`.heapsnapshot` 文件）的完整解析能力。

### 流式解析器（`heap_snapshot.cpp`）

V8 heapsnapshot 以 JSON 格式组织，包含 `nodes`、`edges`、`strings` 三个大型扁平数值数组。解析器采用 16MB 分块流式读取：

- **ChunkedUintParser** — 针对 `[n0,n1,...,nK]` 格式的扁平数值数组，跨分块边界自动拼接，手动解析数字（遇 `]` 结束），不依赖 `JSON.parse`
- **元数据快速扫描** — 64KB 分块搜索文件头部的 `snapshot`、`meta`、`node_types` 等字段，自动发现字段偏移
- **CSR 邻接表构建** — 根据 edge_count 为每个节点建立 `first_edge_pointers` 压缩稀疏行索引，O(1) 遍历出边
- **nodeId→nodeIndex 映射** — 哈希表索引，O(1) 查询

### Dominator Tree（`node_size_calculator.cpp`）

采用 **Lengauer-Tarjan 算法** 计算 dominator tree，O(N log N)：

1. DFS 生成树 — 从 GC root 出发，使用 `isEssentialEdge` 过滤边，进行 DFS 编号（dfn）
2. Semi-dominator 计算 — 按 dfn 逆序遍历，利用 DSU + 路径压缩
3. Immediate Dominator 推导 — 从 semi-dominator 推导 idom
4. Post-order 前缀和 — 自底向上累加 retained size
5. 最短路径 BFS — 分层 BFS（支持 LIFO 处理 + 低优先级边推迟队列）

关键参数：
- `exclude_weak_edges` — 排除 weak 边（默认开启，匹配 Browser DevTools）
- `exclude_shortcut_edges` — 排除 shortcut 边（默认开启）
- `exclude_hidden_edges` — 排除 hidden 边（默认开启）

---

## 事件类型系统

22 种事件类型定义在 `include/shared/htrace_model/htrace_data.hpp` 的 `HtraceEventType` 枚举中。当前启用 8 种（`include/htrace/htrace_processor.hpp` 的 `htrace_enabled_types()`）：

| 值 | 枚举名 | 启用 | 说明 |
|----|--------|:----:|------|
| 0 | `MALLOC` | ✅ | native heap 分配 |
| 1 | `MMAP_ANON` | ✅ | 匿名内存映射（MAP_ANONYMOUS） |
| 4 | `HP_FD` | ✅ | 文件描述符 |
| 5 | `HP_THREAD` | ✅ | 线程 |
| 14 | `ASHMEM` | ✅ | 匿名共享内存 |
| 15 | `HP_DMA` | ✅ | DMA 内存 |
| 17 | `ARK_GLOBAL_HANDLE` | ✅ | ArkTS 全局句柄 |
| 18 | `ARK_LOCAL_HANDLE` | ✅ | ArkTS 局部句柄 |

`htrace_count_size_types()` 控制哪些类型计入 `totalExistingSize`，当前与 `htrace_enabled_types()` 一致。

### 统计模式 vs 详情模式

| 维度 | 统计模式 | 详情模式 |
|------|---------|---------|
| 数据来源 | `native_hook_statistic` 表 | `native_hook` 表 |
| 类型来源 | `type` 列（整数） | `event_type` 列（字符串）→ `default_event_mappings()` 映射 |
| 过滤方式 | `htrace_is_enabled_type()` 检查枚举值 | `htrace_is_enabled_type()` 检查映射后的 `htrace_type` |

---

## 运行参数（RunContext）

原先的 `Config` 全局单例已移除。一次运行的全部参数由 `RunContext`（`include/core/run_context.hpp`）承载：**构造后只读**，由子命令组装，经 `process_file` 传给解析器 / 分析器 / 格式化器。

```
RunContext
├── output        输出目录、格式、parse-only、auto-open
├── resources     解析期外部资源：so_dir / sourcemap / namecache
├── command_line  原始命令行（写入输出 meta）
├── htrace        scope / top_n / category / leak_addrs
├── tshandle      top_n
└── snapshot      mode / leak_list_file / top_n
```

传递方式：由值/常引用一路显式传递——`process_file` 派生一份副本后交给 `module.run(req, ctx)`，
模块再把 `ctx`（或其转换出的域内参数）传给域内 parse / analyze / writer 自由函数。全链路无借用指针、无全局兜底。

**关键纪律**：`RunContext` 只出现在管道层（`process_file` / `AnalysisModule::run`）；各域算法只接收自己那一个子结构（`HtraceOptions` / `TshandleOptions` / `SnapshotOptions`，经 `AnalysisConfig` 等显式参数传入），不接收整个上下文。

`process_file` 会为每个输入文件派生一份副本，把输出目录再嵌一层 source stem（`out/<stem>/`），因此不再改写任何全局状态。

CLI 选项与字段的对应：

| 字段 | 默认值 | CLI 选项 |
|------|--------|----------|
| `output.out_dir` | `"out"` | `-d, --out-dir` |
| `output.parse_only` | `false` | `-p, --parse-only` |
| `output.auto_open` | `false` | `-o, --auto-open` |
| `resources.so_dir` | `""` | `-S, --so-dir` |
| `resources.sourcemap_dir` | `""` | `--sourcemap` |
| `resources.name_cache_dir` | `""` | `--namecache` |
| `htrace.scope` | `AppAndSystem` | `-s, --scope` |
| `htrace.top_n` | `10` | `-n, --top-n`（trace） |
| `htrace.category` | `"nativeheap"` | `-c, --category` |
| `htrace.leak_addrs` | `unordered_set<uint64_t>` | trace `-l, --leaklist`（Native Leaks 不可达内存泄漏文件） |
| `tshandle.top_n` | `50` | `-n, --top-n`（tshandle） |
| `snapshot.mode` | `"all"` | `-m, --mode` |
| `snapshot.leak_list_file` | `""` | `-l, --leaklist` |
| `snapshot.top_n` | `50` | `-n, --top-n`（jsleak） |

> 非正的 `top_n` 回退到默认值；`-m pt/tc` 归一化为 `property-tag` / `top-chain`（与历史 `Config` setter 行为一致）。

**Scope 枚举**：`AppOnly` / `SystemOnly` / `AppAndSystem` / `Mixed`

**默认值常量**：`include/config.hpp` 的 `defaults::` 命名空间（`OUTPUT_DIR` / `SCOPE` / `TRACE_TOP_N` / `TSHANDLE_TOP_N` / `JSLEAK_TOP_N` / `JSLEAK_MODE` / `HTRACE_TYPE` / `PROF_DURATION`）；`scope_from_string()` / `scope_to_string()` 也在该头文件中。

---

## 日志系统（logger）

统一日志模块定义在 `include/utils/logger.hpp`（命名空间 `mem_analyzer::log`）。所有运行时日志必须走 `log::`，不要散用 `std::cout`/`std::cerr` + 手写颜色。

### 六个等级入口

| 函数 | 级别 | 颜色 | 用途 |
|------|------|------|------|
| `loge` | Error | 红 | 错误 |
| `logw` | Warn | 黄 | 警告 |
| `logok` | Success | 绿 | 成功 / 关键里程碑 |
| `logi` | Info | 正色（不染色） | 常规信息 / 主线进度 |
| `logi_minor` | Info（dim） | 灰 | 次要提示（跳过/忽略/note/耗时） |
| `logd` | Debug | 灰 | 调试 / 命令回显 |

级别数值即阈值：`Debug=0 < Info=1 < Success=2 < Warn=3 < Error=4`，`set_level(min)` 丢弃所有低于 `min` 的日志（默认 `Info`，即 `logd` 默认被丢弃）。`logi` 与 `logi_minor` 的取舍判据：删掉这条，用户能否无歧义看懂流程？能 → `logi_minor`，不能 → `logi`。

### 核心实现

```cpp
template <typename... Args>
void emit(Level lv, bool dim, Args&&... args) {
    if (lv < level()) return;                 // ① 级别过滤（先于字符串构造）
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args)); // ② C++17 fold 拼接
    write_line(lv, dim, oss.str());           // ③ 前缀 + 着色 + 落盘
}
```

- 变参用 `<<` fold 拼接，每个调用自带换行，不要写 `<< std::endl` 或结尾 `\n`。
- 级别过滤发生在字符串构造之前，低级别日志不会付出拼接成本。

### 前缀与后端

```cpp
enum class Prefix : unsigned { None = 0, Timestamp = 1u << 0, ThreadId = 1u << 1 };
void set_prefix(Prefix flags);               // 默认 None

class Sink { virtual void write(Level lv, const std::string& line) = 0; };
void set_sink(std::shared_ptr<Sink> sink);   // 默认 console：stderr
```

- `Timestamp` 输出 `[HH:MM:SS.mmm]`（跨平台：Windows `localtime_s` / POSIX `localtime_r`）；`ThreadId` 输出 `[tid]`（线程 id 的短哈希）。
- `Sink` 为可替换后端；默认 console 将整行写到 **stderr**。

### 着色与样式

- **整条同色**：前缀（级别 tag，加粗级别色）与正文同色；info 正色不染色。
- **正文局部着色透传**：`logi("...", style::bold("X"), "...")` 里 `style::bold`/`style::dim` 会透传生效；但不要在参数里再包 `style::red/yellow/green`，会与整条同色叠加冲突。
- **颜色跟随 stderr**：`style::wrap()` 按 `is_stderr_tty()` 决定是否上色——非终端（管道/重定向）时自动去色，避免归档文件混入 ANSI 转义码。

### 线程安全与流约定

- `write_line()` 用 `std::mutex` 保证单条日志原子输出（多线程不交错）。
- **日志走 stderr、运行结果走 stdout**：诊断/日志全部经 `log::` 到 stderr；JSON/HTML 内容、`--list-apps` 列表、`--help` 等结果仍用 `std::cout`，两者分开，便于脚本重定向与解析。
- 流式实时输出（`process_spinner` 的 `\r` 原地重写、hdc transcript、轮询指标）不走 `log::`（会破坏原地重写与时序），仍用 `std::cout`。

### CLI 对应

| CLI 选项 | 生效点 |
|----------|--------|
| `--verbose` | `main.cpp`：`set_level(verbose ? Debug : Info)` |
| `--keep-temp` | `TempDir::set_keep(keep)`，保留中间产物目录 |
| `--debug` | 等价于 `--verbose --keep-temp`（排障组合糖） |

临时目录保留规则：非 `quiet` 的中间产物目录（`htrace`/`tshandle`/`snapshot`）在 `--keep-temp` 下保留并打印 `Kept temp: <path>`；`quiet` 的内部缓冲目录（如 `run_process_with_spinner` 的 `spinner.*`）即使保留模式也照常清理。

---

## 平台适配

通过 `_WIN32` / `__APPLE__` 宏在编译期分发平台特定代码：

| 维度 | Windows | Linux/macOS |
|------|---------|-------------|
| 路径编码 | UTF-8 → wstring（WideCharToMultiByte） | UTF-8 直接透传 |
| 命令行 argv | ACP (GBK) → UTF-8 转换 | UTF-8 原生 |
| 控制台输出 | `SetConsoleOutputCP(CP_UTF8)` | UTF-8 原生 |
| SQLite3 | `sqlite3_open16`（UTF-16） | `sqlite3_open`（UTF-8） |
| ZIP 解压 | 内置 minizip（`fill_win32_filefunc64W` + `unzOpen2_64`，无外部命令） | 内置 minizip（`unzOpen64`，无外部命令） |
| 可执行文件路径 | `GetModuleFileNameW` | `/proc/self/exe` / `_NSGetExecutablePath` |
| 外部工具 | `trace_streamer_windows.exe` | `trace_streamer_linux_static` / `trace_streamer_mac` |

### 路径编码设计

内部统一使用 UTF-8 字符串表示路径，`path_from_utf8` / `path_to_utf8` 是核心编码转换入口：

```
UTF-8 string (内部统一编码)
   ├── path_from_utf8() → fs::path
   │     Windows: MultiByteToWideChar(CP_UTF8) → fs::path(wstring)
   │     Linux/macOS: fs::path(string) 直接传递
   └── path_to_utf8() → UTF-8 string
         Windows: fs::path.wstring() → WideCharToMultiByte(CP_UTF8)
         Linux/macOS: fs::path.string() 直接返回
```

Windows 上 `fs::path` 存储 `std::wstring`（UTF-16），从 `std::string` 构造时会使用窄字符 locale（CP936/GBK），导致 UTF-8 路径被错误解释。所有 `fs::path` 构造均通过 `path_from_utf8()` 绕开此问题。

### 中文路径支持

Windows 命令行传入的 argv 使用系统 ANSI 编码（中文 Windows 为 CP936/GBK），程序入口处通过 `acp_to_utf8()` 转换为 UTF-8 后统一处理。配合 `SetConsoleOutputCP(CP_UTF8)` 确保控制台正确显示 UTF-8 路径。

---

## 依赖

所有依赖以源码/头文件形式引入，无需包管理器：

| 依赖 | 路径 | 用途 |
|------|------|------|
| nlohmann/json | `third_party/src/third_party_json/` | JSON 序列化（header-only） |
| SQLite3 | `third_party/src/third_party_sqlite/` | SQLite C 库（amalgamation 单文件） |
| trace_streamer | `third_party/bin/trace_streamer_binary/` | htrace → SQLite DB 转换（外部二进制） |
| rawheap_translator | `third_party/bin/rawheaptranslator/` | rawheap → heapsnapshot 转换（外部二进制） |

---

## 相关文档

| 文档 | 内容 |
|------|------|
| [../README.md](../README.md) | 项目概述、功能介绍、构建详情 |
| [../docs/usage.md](../docs/usage.md) | 二进制使用者手册 |
