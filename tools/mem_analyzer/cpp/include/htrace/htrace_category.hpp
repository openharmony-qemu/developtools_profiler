#pragma once
#include <string>
#include <vector>

#include "shared/htrace_model/htrace_data.hpp"

namespace mem_analyzer {

// ─────────────────────────────────────────────────────────────────────────────
// htrace「category」的单一静态事实源（描述符表）。
//
// 为什么需要它
// 旧的 category 语义散落在 6+ 处 `if (category == "xx")` 里（CLI 合法性校验 /
// allowed_event_types / analyzer 低阈值与多轨分叉 / JSON summary 键 / HTML 类型与计数语义），
// 且 mmap 的「双轨」（anon 符号 + filepage 文件）曾是一个 `bool has_mmap_split` 加
// 两套对称分支的特例。每新增一个 category、一条轨道，都要把所有分支同步改一遍，漏一处回归就挂。
//
// 这里的思路是「表驱动」：把每个 category 的全部静态事实收进一张只读描述符表
// （HtraceCategorySpec），消费方只做「category 名 → 查表 → 读字段」，不再写 category if/else。
// 新增 category 只需在 .cpp 的表里加一行，消费代码零改动。
//
// 三个正交维度（改动互不牵连）
// 1. category（有哪些种类）：    描述符表加一行 HtraceCategorySpec    —— 运行前就确定的种类，封闭的表。
// 2. 轨道（一个 category 产出几段并列输出）：spec.tracks 加一条 HtraceTrackSpec —— 开放列表（N 轨）。
// 3. 行种（一段轨道里每行的数据类型）：TrackRender 加一个值 + HtraceRow variant 加一个备选
//    + 各 writer 的行级分派（std::get_if / overloaded）加一个分支 —— 编译期封闭类型集。
//
// 完整设计（N 轨 variant 行模型、JSON/HTML 写器路由的字节契约、拆分步骤）见
// docs/htrace_refactor_design.md。本头文件只放「那张表」+ 查询接口。
// ─────────────────────────────────────────────────────────────────────────────

// 行种：描述一条轨道里每一行数据的「形状」，决定渲染时走哪套 JSON/HTML 行级 writer。
// 它与「轨数」正交：轨数是运行时数量（vector，开放），行种是编译期封闭枚举（HtraceRow variant 的 tag）。
// 当前只有两种；将来若出现第三种（如「句柄行」），走上面维度 3 的扩展路径。
enum class TrackRender {
    Symbol,   // 符号行：按符号聚合（对应 HtraceRow 里的 TopSymbolInfo），HTML 走 symbol-type-* 表格
    Filepage, // 文件行：按文件聚合（对应 HtraceRow 里的 FilepageInfo），HTML 走 file-page 表格
};

// 单条轨道声明：一段「并列输出块」的静态描述。
// 「轨道」对应旧代码里的「双轨」（mmap 的 anon 符号 + filepage 文件是两条并列输出）。
// N 轨就是这里的列表放 N 条；消费方靠 tracks/sections 遍历，不硬编码「第几条是哪一种」。
struct HtraceTrackSpec {
    // JSON 里该轨道的嵌套键。字节契约（改它 = 回归）：
    //   每个 scope（app/system/mixed）都按 json_key 嵌套成对象；单轨一个 key、多轨多个 key，
    //   且每个键恒写（该轨为空也输出空数组）。
    std::string json_key;

    // HTML 二级 tab 的显示名（全名，如 "mmap-anon（匿名映射）"）。
    // 这是 label 的唯一权威来源：分析器把它抄进 HtraceSection.label，HTML 再读 section.label；
    // 不要在 writer 里再硬编码一份同义字符串（否则改一处漏一处）。
    std::string label;

    // 该轨道每行的数据种类（决定行级 writer 走 write_symbol 还是 write_filepage 这类分派）。
    TrackRender render;
};

// build_summary 的一条输出条目：JSON 里一个「汇总键 → 若干类型之和」。
// type_names 里放的是 HtraceEventType 的 name 字符串（如 "mmap-anon"），运行时按 name 查到
// type_sizes 各自累加。表驱动取代了旧 build_summary 里的 category if/else：
//   analysis 模式只 emit 当前 category 的 summary_entries；
//   parse 模式按固定顺序 emit 全部 category 的条目（顺序由 parse formatter 自己定）。
struct HtraceSummaryEntry {
    // 输出 JSON 键，如 "existingMmapTotalSize"。
    const char* json_key;

    // 参与求和的类型名。注意是可重复列表：mmap 的 Total 条目就把 Anon/Filepage 两个名字放进来，
    // 让 writer 复用「两轨各自求和」的结果，无需对 Total 单独特判。
    std::vector<const char*> type_names;
};

// 一个 category 的全部静态事实 = 描述符表的一行。
// 字段都是「只读静态事实」，不是运行时状态；查询接口一律返回 const 引用/指针。
// 消费方拿到 spec 后按字段驱动，不再出现 category 字符串比较。
struct HtraceCategorySpec {
    // category 关键词（对应 CLI --category 与 HtraceOptions::category）。
    // 属于行为契约：它进错误文案、JSON 键、HTML 输出；改名会改输出/报错，回归会挂，勿随便动。
    std::string name;

    // allowed_event_types(name, /*filter_category=*/true) 的结果：该 category 允许的事件类型集合，升序。
    // 供 parse 阶段过滤 native_hook / 拼 SQL 的 type IN(...) 列表（见 htrace_processor）。
    std::vector<HtraceEventType> event_types;

    // 汇总总量键：htrace_summary_total_for_type(category) 返回它，用于 JSON 顶层「总泄漏量」字段。
    // 键随语义而不同——nativeheap 是字节（*HeartSize）、fd/thread 是个数（*Count），所以各 category 键名不同。
    const char* summary_total_key = "existingNativeHeapSize";

    // HTML 里 meta 卡片 / body 的类型 CSS 类（如 "htrace-mmap"），用于配色等高亮；解析不足时只是个 class 名。
    const char* html_type_class = "htrace-nativeheap";

    // 计数语义开关：泄漏单元是「个数」而非「字节」的 category（fd / thread）。
    // 一个字段同时驱动两处（都曾写成 `if (category == "fd" || category == "thread")`）：
    //   1. analyzer：把字节口径的低阈值置 0（不能用「1024 字节」去过滤「多少个 fd」）；
    //   2. HTML：把显示单位从「字节」切成「个数」。
    bool count_mode = false;

    // N 条轨道（顺序即渲染顺序）。单轨是常规形态；多轨（is_multipart()==true）是
    // mmap 这种「一个 category、多种并列输出」的情形。
    std::vector<HtraceTrackSpec> tracks;

    // build_summary 的输出条目（顺序即 JSON 键顺序）。与 tracks 是两个维度：
    // tracks 是「并列输出块」，summary_entries 是「顶部汇总数字」，两者互不一一对应。
    std::vector<HtraceSummaryEntry> summary_entries;

    // 是否多轨（tracks.size() > 1）。旧代码用 bool has_mmap_split / isMmap 判断分支，
    // 现在由这个派生布尔表达「需要按轨道展开」；真正的轨道明细看 tracks 列表。
    bool is_multipart() const { return tracks.size() > 1; }
};

// 单一数据源：按声明顺序返回全部 category 描述符。
// 顺序有意为之——它是 CLI 报错「Valid values:」与 parse summary 的展示基准（详见 .cpp 表内注释）。
const std::vector<HtraceCategorySpec>& htrace_categories();

// 找不到返回 nullptr。用于「不抛异常」的路径（如 CLI 校验要吐出统一错误文案，而不是让异常冒泡）。
const HtraceCategorySpec* htrace_find_category(const std::string& name) noexcept;

// 找不到抛 std::invalid_argument。用于「已校验、理论上必存在」的路径，做最后一层防御。
const HtraceCategorySpec& htrace_category(const std::string& name);

} // namespace mem_analyzer