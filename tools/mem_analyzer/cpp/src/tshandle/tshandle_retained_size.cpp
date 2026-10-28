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
void DistributeTshandleRetainedSize(
    double retainedSize,
    const std::vector<TshandleRetainedShare>& shares,
    std::unordered_map<long long, double>& totals)
{
    if (!std::isfinite(retainedSize) || retainedSize < 0.0) {
        return;
    }

    std::size_t totalAddrs = 0;
    for (const auto& share : shares) {
        totalAddrs += share.addrCount;
    }
    if (totalAddrs == 0) {
        return;
    }

    for (const auto& share : shares) {
        if (share.addrCount == 0 || share.callStackId == 0) {
            continue;
        }
        totals[share.callStackId] += retainedSize *
            static_cast<double>(share.addrCount) / static_cast<double>(totalAddrs);
    }
}

void ApplyTshandleRetainedSizes(
    std::vector<ResolvedCallStack>& CallStacks,
    const std::unordered_map<long long, double>& retainedSizes)
{
    for (auto& callStack : CallStacks) {
        double total = static_cast<double>(callStack.existingSize);
        const auto retained = retainedSizes.find(callStack.id);
        if (retained != retainedSizes.end() && std::isfinite(retained->second) &&
            retained->second >= 0.0) {
            total += retained->second;
        }
        callStack.relatedSize = total;
    }
}
} // namespace mem_analyzer
