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
#include "tshandle/tshandle_analyzer.hpp"
#include "config_mgr/htrace_config_mgr.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace mem_analyzer {
namespace {

constexpr double PERCENTAGE_SCALE = 100.0;

bool CallStackIsApp(const ResolvedCallStack& stack)
{
    for (const auto& frame : stack.frames) {
        if (HtraceConfigMgr::Instance().IsAppSymbol(frame.lib, true)) {
            return true;
        }
    }
    return false;
}

/// 构建一个 section，只纳入 id 在 valid_stack_ids 中的调用栈。
/// object.existing_size = 该对象这些调用链分摊到的 retained size（链 size 之和），
/// total_size 为各对象 size 之和，percentage 相对该 total_size 计算。
/// 对同一对象，应用作用域链 size + 系统作用域链 size = 该对象全部链 size 之和（拆分前后守恒）。
TshandleTypeSection buildSectionWithStacks(
    HtraceEventType handleType,
    const std::vector<ArkTsClass>& arktsClasses,
    const std::unordered_map<long long, const ResolvedCallStack*>& stacksById,
    const std::unordered_set<long long>& validStackIds,
    int topN)
{
    TshandleTypeSection section;
    for (long long id : validStackIds) {
        const auto it = stacksById.find(id);
        if (it != stacksById.end()) {
            section.hasData = true;
            section.totalCallStackExistingSize += it->second->relatedSize;
        }
    }

    std::vector<TshandleTopObject> objects;
    for (const auto& cls : arktsClasses) {
        if (cls.name.empty()) {
            continue;
        }
        TshandleTopObject object;
        object.objectName = cls.name;
        object.type = handleType;
        std::vector<long long> callStackRefs;
        std::unordered_set<long long> seenCallStackIds;
        std::unordered_map<long long, double> chainSizeByCallStack;
        std::unordered_map<long long, long long> chainCountByCallStack;

        for (const auto& instance : cls.instances) {
            int totalAddrs = 0;
            for (const auto& ref : instance.nativeRef) {
                if (ref.handleType == handleType) {
                    totalAddrs += static_cast<int>(ref.addrs.size());
                }
            }
            if (totalAddrs == 0) {
                continue;
            }
            bool instanceHasRef = false;
            for (const auto& ref : instance.nativeRef) {
                if (ref.handleType != handleType||
                    ref.callStackId == 0 ||
                    validStackIds.find(ref.callStackId) == validStackIds.end()) {
                    continue;
                }
                long long id = ref.callStackId;
                instanceHasRef = true;
                if (seenCallStackIds.insert(id).second) {
                    callStackRefs.push_back(id);
                }
                if (instance.retainedSize > 0) {
                    chainSizeByCallStack[id] +=
                        (static_cast<double>(instance.retainedSize) * ref.addrs.size() / totalAddrs);
                }
                chainCountByCallStack[id] += static_cast<long long>(ref.addrs.size());
            }
            if (instanceHasRef) {
                ++object.count;
            }
        }

        double objectScopeSize = 0.0;
        for (const auto& [id, sz] : chainSizeByCallStack) {
            objectScopeSize += sz;
        }

        std::stable_sort(callStackRefs.begin(), callStackRefs.end());
        for (const auto& id : callStackRefs) {
            const auto stackIt = stacksById.find(id);
            if (stackIt == stacksById.end()) {
                continue;
            }
            const ResolvedCallStack* stack = stackIt->second;
            if (stack->type != handleType || stack->frames.empty()) {
                continue;
            }
            const auto csIt = chainSizeByCallStack.find(id);
            const double csSize = (csIt != chainSizeByCallStack.end()) ? csIt->second : 0.0;
            const auto countIt = chainCountByCallStack.find(id);
            const long long csCount = (countIt != chainCountByCallStack.end()) ? countIt->second : 0LL;
            if (csSize <= 0.0 && csCount == 0) {
                continue;
            }
            object.callChains.push_back({stack->id, stack->frames, csSize, csCount, 0.0});
        }
        if (object.callChains.empty()) {
            continue;
        }
        object.existingSize = objectScopeSize;
        objects.push_back(std::move(object));
    }

    section.totalObjects = static_cast<long long>(objects.size());
    std::stable_sort(objects.begin(), objects.end(), [](const auto& left, const auto& right) {
        return left.existingSize > right.existingSize;
    });

    double totalSize = 0.0;
    for (const auto& object : objects) {
        totalSize += object.existingSize;
        section.totalCount += object.count;
    }
    section.totalSize = totalSize;

    if (topN > 0 && objects.size() > static_cast<std::size_t>(topN)) {
        objects.resize(static_cast<std::size_t>(topN));
    }

    for (std::size_t index = 0; index < objects.size(); ++index) {
        auto& object = objects[index];
        object.rank = static_cast<int>(index + 1);
        std::stable_sort(object.callChains.begin(), object.callChains.end(),
            [](const auto& left, const auto& right) {
                return left.existingSize > right.existingSize;
            });
    }
    section.topObjects = std::move(objects);
    return section;
}

void ComputeSectionPercentages(TshandleTypeSection& section, double denominator)
{
    for (auto& object : section.topObjects) {
        object.percentage = (denominator > 0.0) ? (object.existingSize / denominator * PERCENTAGE_SCALE) : 0.0;
        for (auto& chain : object.callChains) {
            chain.percentage = (denominator > 0.0) ? (chain.existingSize / denominator * PERCENTAGE_SCALE) : 0.0;
        }
    }
}

json FrameToJson(const ResolvedStackFrame& frame)
{
    return {{"depth", frame.depth}, {"symbol", frame.symbol}, {"buildId", frame.buildId},
            {"lib", frame.lib}, {"libOffset", frame.libOffset}};
}

} // namespace

json TshandleCallChain::ToJson() const
{
    json frameJson = json::array();
    for (const auto& frame : frames) {
        frameJson.push_back(FrameToJson(frame));
    }
    return {{"callStackId", callStackId},
            {"existingSize", existingSize},
            {"count", count},
            {"percentage", percentage},
            {"frames", frameJson}};
}

json TshandleTopObject::ToJson() const
{
    json ChainJson = json::array();
    for (const auto& chain : callChains) {
        ChainJson.push_back(chain.ToJson());
    }
    return {{"rank", rank},
            {"objectName", objectName},
            {"existingSize", existingSize},
            {"count", count},
            {"percentage", percentage},
            {"type", HtraceEventTypeName(type)},
            {"callChains", ChainJson}};
}

json TshandleTypeSection::ToJson() const
{
    json objectJson = json::array();
    for (const auto& object : topObjects) {
        objectJson.push_back(object.ToJson());
    }
    return {{"hasData", hasData},
            {"totalObjects", totalObjects},
            {"totalSize", totalSize},
            {"totalCount", totalCount},
            {"totalCallStackExistingSize", totalCallStackExistingSize},
            {"topObjects", objectJson}};
}

json TshandleAnalysisOutput::ToJson() const
{
    return {
        {"global", {{"app", global.app.ToJson()}, {"sys", global.sys.ToJson()}}},
        {"local", local.ToJson()}
    };
}

TshandleAnalysisOutput AnalyzeTshandleData(
    const std::vector<ResolvedCallStack>& callStacks,
    const std::vector<ArkTsClass>& arktsClasses,
    int topN)
{
    auto& cfg = HtraceConfigMgr::Instance();
    if (!cfg.IsLoaded()) {
        std::string err;
        cfg.LoadDefault(err);
    }

    std::unordered_map<long long, const ResolvedCallStack*> stacksById;
    for (const auto& stack : callStacks) {
        if (stack.id != 0) {
            stacksById[stack.id] = &stack;
        }
    }

    TshandleAnalysisOutput output;

    // GlobalHandle：按调用链符号作用域拆分为应用 / 系统两个 section。
    std::unordered_set<long long> globalAppIds;
    std::unordered_set<long long> globalSysIds;
    for (const auto& [id, stack] : stacksById) {
        if (stack->type != HtraceEventType::ARK_GLOBAL_HANDLE || stack->frames.empty()) {
            continue;
        }
        if (CallStackIsApp(*stack)) {
            globalAppIds.insert(id);
        } else {
            globalSysIds.insert(id);
        }
    }
    output.global.app = buildSectionWithStacks(
        HtraceEventType::ARK_GLOBAL_HANDLE, arktsClasses, stacksById, globalAppIds, topN);
    output.global.sys = buildSectionWithStacks(
        HtraceEventType::ARK_GLOBAL_HANDLE, arktsClasses, stacksById, globalSysIds, topN);
    const double globalTotal = output.global.app.totalSize + output.global.sys.totalSize;
    ComputeSectionPercentages(output.global.app, globalTotal);
    ComputeSectionPercentages(output.global.sys, globalTotal);

    // LocalHandle：不拆分，所有调用链合成单个 section。
    std::unordered_set<long long> localIds;
    for (const auto& [id, stack] : stacksById) {
        if (stack->type == HtraceEventType::ARK_LOCAL_HANDLE && !stack->frames.empty()) {
            localIds.insert(id);
        }
    }
    output.local = buildSectionWithStacks(
        HtraceEventType::ARK_LOCAL_HANDLE, arktsClasses, stacksById, localIds, topN);
    ComputeSectionPercentages(output.local, output.local.totalSize);

    return output;
}

TshandleAnalysisOutput TshandleAnalyze(TshandleParseResult& parseResult, const RunContext& ctx)
{
    auto analysis = AnalyzeTshandleData(parseResult.CallStacks(), parseResult.arktsClasses,
        ctx.tshandle.topN);

    // Carry parse-level data needed by formatters
    analysis.traceMeta = std::move(parseResult.db.meta);
    analysis.parsedAt = std::move(parseResult.parsedAt);
    analysis.callStacks = std::move(parseResult.CallStacks());
    analysis.arktsClasses = std::move(parseResult.arktsClasses);

    log::Logok("  Analysis completed. Global(app): ", analysis.global.app.totalObjects, " objects, ",
        analysis.global.app.totalCount, " instances | Global(sys): ",
        analysis.global.sys.totalObjects, " objects, ",
        analysis.global.sys.totalCount, " instances | Local: ",
        analysis.local.totalObjects, " objects, ",
        analysis.local.totalCount, " instances.");

    return analysis;
}

} // namespace mem_analyzer
