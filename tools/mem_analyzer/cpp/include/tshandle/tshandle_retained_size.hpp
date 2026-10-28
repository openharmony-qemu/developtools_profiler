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
    long long callStackId;
    std::size_t addrCount = 0;
};

void DistributeTshandleRetainedSize(
    double retainedSize,
    const std::vector<TshandleRetainedShare>& shares,
    std::unordered_map<long long, double>& totals);

void ApplyTshandleRetainedSizes(
    std::vector<ResolvedCallStack>& callStacks,
    const std::unordered_map<long long, double>& retainedSizes);

} // namespace mem_analyzer
