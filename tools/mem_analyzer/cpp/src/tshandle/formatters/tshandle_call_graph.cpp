#include "tshandle/formatters/tshandle_call_graph.hpp"
#include "config_mgr/htrace_config_mgr.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>

namespace mem_analyzer {
namespace {

std::string key(const ResolvedStackFrame& f) {
    return f.symbol + '\0' + f.lib + '\0' + f.lib_offset;
}

TshandleRenderNode make_node(const ResolvedStackFrame& f, int depth) {
    const auto location = !f.lib.empty() ? (!f.lib_offset.empty() ? f.lib + "+" + f.lib_offset : f.lib) : "";
    const bool unresolved = f.symbol.empty() || f.symbol == location || f.symbol.find(".so") != std::string::npos;
    const auto slash = f.lib.find_last_of("/\\");
    const auto libname = slash == std::string::npos ? f.lib : f.lib.substr(slash + 1);
    const auto display = unresolved ? libname + (f.lib_offset.empty() ? "" : "+" + f.lib_offset) : f.symbol;
    const auto name = location.empty() ? (f.symbol.empty() ? "unknown" : f.symbol) : (unresolved ? location : location + " (" + f.symbol + ")");
    const bool is_ts = HtraceConfigMgr::instance().is_ts_symbol(f.lib, f.lib_offset.empty());
    return {name, display, f.symbol, f.lib, f.lib_offset, depth, 0, 0, 0, {}, is_ts, f.build_id};
}

void compute_value(TshandleRenderNode& node) {
    double child_total = 0;
    for (auto& child : node.children) {
        compute_value(child);
        child_total += child.total;
    }
    node.value = node.total > child_total ? node.total - child_total : 0;
}

} // namespace

json TshandleRenderNode::to_json() const {
    json result = {
        {"n", name},
        {"displayName", display_name},
        {"symbol", symbol},
        {"lib", lib},
        {"libOffset", lib_offset},
        {"d", depth},
        {"v", value},
        {"t", total},
        {"cnt", count},
        {"ts", ts}
    };
    result["c"] = json::array();
    for (const auto& child : children) {
        result["c"].push_back(child.to_json());
    }
    return result;
}

TshandleRenderNode build_tshandle_downstream_graph(const std::string& object_name,
                                                   const std::vector<TshandleCallChain>& chains,
                                                   double object_existing_size) {
    TshandleRenderNode root{object_name, object_name, object_name, "", "", -1, 0, 0, 0, {}};
    for (const auto& chain : chains) {
        auto frames = chain.frames;
        std::stable_sort(frames.begin(), frames.end(), [](const ResolvedStackFrame& a, const ResolvedStackFrame& b) {
            return a.depth < b.depth;
        });

        const double size = chain.existing_size;
        const long long count = chain.count;
        root.count += count;
        auto* current = &root;

        for (std::size_t i = 0; i < frames.size(); ++i) {
            const auto id = key(frames[i]);
            auto child = std::find_if(current->children.begin(), current->children.end(), [&](const TshandleRenderNode& n) {
                return n.symbol == frames[i].symbol && n.lib == frames[i].lib && n.lib_offset == frames[i].lib_offset;
            });

            if (child == current->children.end()) {
                current->children.push_back(make_node(frames[i], static_cast<int>(i)));
                child = std::prev(current->children.end());
            }

            child->total += size;
            child->count += count;
            current = &*child;
        }
    }

    root.total = object_existing_size;
    compute_value(root);
    return root;
}

} // namespace mem_analyzer
