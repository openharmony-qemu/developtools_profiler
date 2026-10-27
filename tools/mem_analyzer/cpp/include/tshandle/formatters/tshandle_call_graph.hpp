#pragma once
#include <string>
#include <vector>
#include "tshandle/tshandle_analyzer.hpp"

namespace mem_analyzer {

struct TshandleRenderNode {
    std::string name;
    std::string display_name;
    std::string symbol;
    std::string lib;
    std::string lib_offset;
    int depth = 0;
    double value = 0.0;
    double total = 0.0;
    long long count = 0;
    std::vector<TshandleRenderNode> children;
    bool ts = false;
    std::string build_id;

    json to_json() const;
};

TshandleRenderNode build_tshandle_downstream_graph(
    const std::string& object_name,
    const std::vector<TshandleCallChain>& call_chains,
    double object_existing_size);

} // namespace mem_analyzer
