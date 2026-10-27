/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "tshandle/tshandle_retained_size.hpp"
#include <cmath>

namespace mem_analyzer {
void distribute_tshandle_retained_size(
    double retained_size,
    const std::vector<TshandleRetainedShare>& shares,
    std::unordered_map<long long, double>& totals) {
    if (!std::isfinite(retained_size) || retained_size < 0.0) return;

    std::size_t total_addrs = 0;
    for (const auto& share : shares) {
        total_addrs += share.addr_count;
    }
    if (total_addrs == 0) return;

    for (const auto& share : shares) {
        if (share.addr_count == 0 || share.call_stack_id == 0) continue;
        totals[share.call_stack_id] += retained_size *
            static_cast<double>(share.addr_count) / static_cast<double>(total_addrs);
    }
}

void apply_tshandle_retained_sizes(
    std::vector<ResolvedCallStack>& call_stacks,
    const std::unordered_map<long long, double>& retained_sizes) {
    for (auto& call_stack : call_stacks) {
        double total = static_cast<double>(call_stack.existing_size);
        const auto retained = retained_sizes.find(call_stack.id);
        if (retained != retained_sizes.end() && std::isfinite(retained->second) &&
            retained->second >= 0.0) {
            total += retained->second;
        }
        call_stack.related_size = total;
    }
}
} // namespace mem_analyzer
