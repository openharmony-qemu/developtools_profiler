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
#include "core/run_context.hpp"
#include "tshandle/tshandle_processor.hpp"

namespace mem_analyzer {

struct TshandleCallChain {
    long long callStackId;
    std::vector<ResolvedStackFrame> frames;
    double existingSize = 0.0;
    long long count = 0;
    double percentage = 0.0;

    json ToJson() const;
};

struct TshandleTopObject {
    int rank = 0;
    std::string objectName;
    double existingSize = 0.0;
    long long count = 0;
    double percentage = 0.0;
    HtraceEventType type = HtraceEventType::UNKNOWN;
    std::vector<TshandleCallChain> callChains;

    json ToJson() const;
};

struct TshandleTypeSection {
    bool hasData = false;
    long long totalObjects = 0;
    double totalSize = 0.0;
    long long totalCount = 0;
    double totalCallStackExistingSize = 0.0;
    std::vector<TshandleTopObject> topObjects;

    json ToJson() const;
};

/// 一个 handle 类型按调用链符号作用域（应用 / 系统）拆分后的两个 section。
struct TshandleHandleSections {
    TshandleTypeSection app;
    TshandleTypeSection sys;
};

struct TshandleAnalysisOutput {
    // Parse data carried forward for formatters
    TraceMeta traceMeta;
    std::string parsedAt;
    std::vector<ResolvedCallStack> CallStacks;
    std::vector<ArkTsClass> arktsClasses;

    // GlobalHandle 按调用链拆分为应用 / 系统两个 section；LocalHandle 不拆分。
    TshandleHandleSections global;
    TshandleTypeSection local;

    json ToJson() const;
};

TshandleAnalysisOutput AnalyzeTshandleData(
    const std::vector<ResolvedCallStack>& CallStacks,
    const std::vector<ArkTsClass>& arktsClasses,
    int topN);

/// tshandle 分析步骤（global / local 两个 handle 类型的聚合）。
/// 强类型入参/出参：不再经过 ProcessData / std::any。
/// 会消费 `parse_result`（其 call_stacks 与 arkts_classes 被移出）。
TshandleAnalysisOutput TshandleAnalyze(TshandleParseResult& parseResult, const RunContext& ctx);

} // namespace mem_analyzer
