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
#include <string>
#include <vector>
#include "tshandle/tshandle_analyzer.h"

namespace mem_analyzer {

struct TshandleRenderNode {
    std::string name;
    std::string displayName;
    std::string symbol;
    std::string lib;
    std::string libOffset;
    int depth = 0;
    double value = 0.0;
    double total = 0.0;
    long long count = 0;
    std::vector<TshandleRenderNode> children;
    bool ts = false;
    std::string buildId;

    json ToJson() const;
};

TshandleRenderNode BuildTshandleDownstreamGraph(
    const std::string& objectName,
    const std::vector<TshandleCallChain>& callChains,
    double objectExistingSize);

} // namespace mem_analyzer
