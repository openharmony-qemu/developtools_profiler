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
