#include "htrace/leak_filter_analyzer.hpp"
#include "shared/htrace_model/htrace_data.hpp"
#include "htrace/htrace_analyzer.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

namespace mem_analyzer {

namespace {

// hex 字符串 "0x5c843f8b00" -> uint64（addr 是绝对地址）
uint64_t parse_hex_addr(const std::string& s) {
    return static_cast<uint64_t>(std::strtoull(s.c_str(), nullptr, 16));
}

// L1: 调用栈关联的分配地址（existing_addrs，hex 串）与 leaks 集合求交集，非空才留
bool has_leak_addr(const CompressedCallStack& cs,
                   const std::unordered_set<uint64_t>& leaks) {
    if (leaks.empty()) return false;
    if (!cs.existing_addrs || cs.existing_addrs->empty()) return false;
    for (const auto& a : *cs.existing_addrs) {
        if (leaks.count(parse_hex_addr(a)) > 0) return true;
    }
    return false;
}

// L3: 至少一帧 lib 含 data/storage/el 且 .so，且不含 libarkweb_engine.so
// （arkweb 例外：arkweb 虽在沙箱目录但归 system，不算应用 so 帧）
bool has_app_so_frame(const CompressedCallStack& cs,
                      const std::unordered_map<int, std::string>& dict) {
    for (const auto& f : cs.frames) {
        if (f.file_id_null) continue;
        auto it = dict.find(f.file_id);
        if (it == dict.end()) continue;
        const std::string& lib = it->second;
        if (lib.find("data/storage/el") != std::string::npos &&
            lib.find("libarkweb_engine.so") == std::string::npos &&
            lib.size() >= 3 &&
            lib.compare(lib.size() - 3, 3, ".so") == 0) {
            return true;
        }
    }
    return false;
}

// L2: 任意帧 symbol 含 "__emutls_get_address" → 该栈排除
bool has_emutls_frame(const CompressedCallStack& cs,
                      const std::unordered_map<int, std::string>& dict) {
    for (const auto& f : cs.frames) {
        auto it = dict.find(f.symbol_id);
        if (it == dict.end()) continue;
        if (it->second.find("__emutls_get_address") != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace

// L1: 按 Native Leaks 不可达内存泄漏文件地址过滤（addr 匹配）。parse 后调用，解析/分析模式均执行。
void apply_leak_addr_filter(TraceDB& db, const std::unordered_set<uint64_t>& leaks) {
    if (leaks.empty()) return;
    auto* clist = std::get_if<CompressedCallStackList>(&db.stacks);
    if (!clist) return;

    size_t total = clist->stacks.size();
    std::vector<CompressedCallStack> kept;
    kept.reserve(total);
    for (auto& cs : clist->stacks) {
        if (has_leak_addr(cs, leaks)) {
            kept.push_back(std::move(cs));
        }
    }
    size_t removed = total - kept.size();
    clist->stacks = std::move(kept);

    log::logok("  Leak L1: kept ", clist->stacks.size(),
              " / ", total, " stacks (", removed, " filtered by addr).");
}

// -l 分析模式（不带 -p）：
//   L2 __emutls_get_address 排除 → L3 栈分 app/system（arkweb 例外）→
//   按 scope 走 analyze_call_stacks(leak_mode)：
//     app-only/system-only 各跑一遍；app-and-system 跑两遍（app+system）；mixed 不分栈跑一遍。
//   每组 top N + other（非 top N 覆盖的栈合并火焰图）。
//   组装 HtraceAnalysisOutput，from=htrace-analysis。L1 已由 apply_leak_addr_filter 完成。
HtraceAnalysisOutput htrace_leak_analyze(TraceDB& db, const RunContext& ctx) {
    auto* clist = std::get_if<CompressedCallStackList>(&db.stacks);
    if (!clist) {
        throw std::runtime_error("htrace_leak_analyze: expected CompressedCallStackList in TraceDB");
    }

    Scope scope = ctx.htrace.scope;
    std::string scope_str = scope_to_string(scope);

    AnalysisConfig acfg;
    acfg.leak_mode = true;
    acfg.use_force_scope = true;
    acfg.top_n = ctx.htrace.top_n;
    acfg.scope = scope;
    acfg.category = ctx.htrace.category;

    // L2: __emutls_get_address 帧排除
    size_t total = clist->stacks.size();
    std::vector<CompressedCallStack> kept;
    kept.reserve(total);
    for (auto& cs : clist->stacks) {
        if (has_emutls_frame(cs, clist->dict)) continue;
        kept.push_back(std::move(cs));
    }
    size_t l2_removed = total - kept.size();
    clist->stacks = std::move(kept);
    log::logok("  Leak L2: kept ", clist->stacks.size(),
              " / ", total, " stacks (", l2_removed, " filtered by emutls).");

    // L3: 栈分 app/system（mixed 不分，直接用全部栈）
    std::vector<CompressedCallStack> app_stacks, system_stacks;
    if (scope != Scope::Mixed) {
        for (auto& cs : clist->stacks) {
            if (has_app_so_frame(cs, clist->dict)) {
                app_stacks.push_back(std::move(cs));
            } else {
                system_stacks.push_back(std::move(cs));
            }
        }
        log::logok("  Leak L3: app=", app_stacks.size(), ", system=", system_stacks.size());
    }

    HtraceAnalysisOutput output;
    if (scope_str == "app-only") {
        acfg.force_scope = Scope::AppOnly;
        output = analyze_call_stacks(app_stacks, clist->dict, acfg);
    } else if (scope_str == "system-only") {
        acfg.force_scope = Scope::SystemOnly;
        output = analyze_call_stacks(system_stacks, clist->dict, acfg);
    } else if (scope_str == "mixed") {
        acfg.force_scope = Scope::Mixed;
        output = analyze_call_stacks(clist->stacks, clist->dict, acfg);
    } else {  // app-and-system（默认）：走两遍
        acfg.force_scope = Scope::AppOnly;
        auto app_out = analyze_call_stacks(app_stacks, clist->dict, acfg);
        acfg.force_scope = Scope::SystemOnly;
        auto sys_out = analyze_call_stacks(system_stacks, clist->dict, acfg);
        output.top_symbols.mode = "app-and-system";
        output.top_symbols.app = std::move(app_out.top_symbols.app);
        output.top_symbols.system = std::move(sys_out.top_symbols.system);
        output.total_leak_size = app_out.total_leak_size + sys_out.total_leak_size;
        output.stats.org_set_size = app_out.stats.org_set_size + sys_out.stats.org_set_size;
        output.stats.filtered_set_size = app_out.stats.filtered_set_size + sys_out.stats.filtered_set_size;
        output.stats.final_set_size = app_out.stats.final_set_size + sys_out.stats.final_set_size;
        output.stats.equivalent_groups = app_out.stats.equivalent_groups + sys_out.stats.equivalent_groups;
        output.stats.up_approx_removed = app_out.stats.up_approx_removed + sys_out.stats.up_approx_removed;
        output.stats.down_approx_removed = app_out.stats.down_approx_removed + sys_out.stats.down_approx_removed;
        output.stats.representation_added = app_out.stats.representation_added + sys_out.stats.representation_added;
    }

    // 填 metadata（照 htrace_analyze）
    output.data_dict = std::move(clist->dict);
    output.mode = std::move(db.meta.mode);
    output.process_name = std::move(db.meta.process_name);
    output.type_sizes = std::move(db.meta.type_sizes);
    output.trace_meta = std::move(db.meta);

    int total_top = (int)(output.top_symbols.app.symbol_count()
                        + output.top_symbols.system.symbol_count()
                        + output.top_symbols.mixed.symbol_count());
    log::logok("  Leak analysis: ", total_top, " top symbols (incl other).");

    return output;
}

} // namespace mem_analyzer
