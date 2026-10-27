#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include "tshandle/tshandle_processor.hpp"

namespace mem_analyzer {

struct TshandleRetainedShare {
    long long call_stack_id;
    std::size_t addr_count = 0;
};

void distribute_tshandle_retained_size(
    double retained_size,
    const std::vector<TshandleRetainedShare>& shares,
    std::unordered_map<long long, double>& totals);

void apply_tshandle_retained_sizes(
    std::vector<ResolvedCallStack>& call_stacks,
    const std::unordered_map<long long, double>& retained_sizes);

} // namespace mem_analyzer
