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
#include "tshandle/formatters/tshandle_formatter.hpp"
#include "formatter/json/json_stream_writer.hpp"
#include "formatter/json/output_meta.hpp"
#include "tshandle/tshandle_processor.hpp"
#include "tshandle/tshandle_analyzer.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace mem_analyzer {
namespace {

constexpr double PERCENTAGE_SCALE = 100.0;
constexpr int JSON_INDENT_SPACES = 4;

// Round double to 2 decimal places for JSON output
static double Round2(double v)
{
    return std::round(v * PERCENTAGE_SCALE) / PERCENTAGE_SCALE;
}

json BuildSummary(const std::vector<ResolvedCallStack>& callStacks, const std::vector<ArkTsClass>& arktsClasses)
{
    struct Totals {
        long long count = 0;
        long long memory = 0;
        long long shallow = 0;
        long long retained = 0;
    };
    std::unordered_map<HtraceEventType, Totals> byType = {
        {HtraceEventType::ARK_GLOBAL_HANDLE, {}},
        {HtraceEventType::ARK_LOCAL_HANDLE, {}}
    };

    for (const auto& stack : callStacks) {
        const auto found = byType.find(stack.type);
        if (found == byType.end()) {
            continue;
        }
        ++found->second.count;
        found->second.memory += stack.existingSize;
    }

    for (const auto& cls : arktsClasses) {
        for (const auto& instance : cls.instances) {
            std::unordered_set<HtraceEventType> types;
            for (const auto& ref : instance.nativeRef) {
                types.insert(ref.handleType);
            }
            for (const auto& type : types) {
                const auto found = byType.find(type);
                if (found == byType.end()) {
                    continue;
                }
                found->second.shallow += instance.shallowSize;
                found->second.retained += instance.retainedSize;
            }
        }
    }

    json handles = json::array();
    for (auto htype : {HtraceEventType::ARK_GLOBAL_HANDLE, HtraceEventType::ARK_LOCAL_HANDLE}) {
        const auto& total = byType.at(htype);
        if (total.count == 0 && total.memory == 0 &&
            total.shallow == 0 && total.retained == 0) continue;
        handles.push_back({
            {"handleType", HtraceEventTypeName(htype)},
            {"totalCount", total.count},
            {"totalMemory", total.memory},
            {"totalShallowSize", total.shallow},
            {"totalRetainedSize", total.retained}
        });
    }
    return {{"arkTsHandle", handles}};
}

// Convert ResolvedCallStack to JSON for output
static json CallStackToJson(const ResolvedCallStack& stack)
{
    json j;
    j["id"] = stack.id;
    j["type"] = HtraceEventTypeName(stack.type);
    j["existingSize"] = Round2(stack.relatedSize);
    j["nativeRefObjSize"] = stack.existingSize;
    json addrs = json::array();
    if (stack.existingAddrs) {
        for (const auto& addr : *stack.existingAddrs) {
            addrs.push_back(addr);
        }
    }
    j["existingAddrs"] = addrs;
    json frames = json::array();
    for (const auto& f : stack.frames) {
        frames.push_back({
            {"depth", f.depth},
            {"symbol", f.symbol},
            {"buildId", f.buildId},
            {"lib", f.lib},
            {"libOffset", f.libOffset}
        });
    }
    j["frames"] = frames;
    return j;
}

// Convert TraceMeta to JSON
static json TraceMetaToJson(const TraceMeta& meta)
{
    return {
        {"startTimeMs", meta.startTimeMs},
        {"endTimeMs", meta.endTimeMs},
        {"duration", meta.durationNs},
        {"processName", meta.processName}
    };
}

static void MergeArktsClasses(const std::vector<ArkTsClass>& classes,
    std::unordered_map<std::string, ArkTsClass>& mergedMap, std::vector<std::string>& order)
{
    // 1. Merge buckets by clean JSON name
    for (const auto& cls : classes) {
        std::string jsonName = cls.name;
        if (jsonName.find("JSSharedObject(") == 0) {
            jsonName = "JSSharedObject";
        } else if (jsonName.find("JSObject(") == 0) {
            jsonName = "JSObject";
        } else if (jsonName.find("string(size:") == 0) {
            jsonName = "(unknown)";
        }

        auto it = mergedMap.find(jsonName);
        if (it == mergedMap.end()) {
            ArkTsClass newCls;
            newCls.name = jsonName;
            newCls.shallowSize = cls.shallowSize;
            newCls.retainedSize = cls.retainedSize;
            newCls.instances = cls.instances;
            mergedMap[jsonName] = std::move(newCls);
            order.push_back(jsonName);
        } else {
            it->second.shallowSize += cls.shallowSize;
            it->second.retainedSize += cls.retainedSize;
            it->second.instances.insert(it->second.instances.end(), cls.instances.begin(), cls.instances.end());
        }
    }
}

// Convert ArkTsClass vector to JSON
static json ArktsClassesToJson(const std::vector<ArkTsClass>& classes)
{
    // 1. Merge buckets by clean JSON name
    std::unordered_map<std::string, ArkTsClass> mergedMap;
    std::vector<std::string> order; // Preserve insertion order
    MergeArktsClasses(classes, mergedMap, order);

    // Sort the keys alphabetically to perfectly match the legacy std::map output order
    std::sort(order.begin(), order.end());

    // 2. Output to JSON
    json arr = json::array();
    for (const auto& key : order) {
        const auto& cls = mergedMap[key];
        json cj;
        cj["name"] = cls.name;
        cj["shallowSize"] = cls.shallowSize;
        cj["retainedSize"] = cls.retainedSize;
        json instances = json::array();
        for (const auto& inst : cls.instances) {
            json ij;
            ij["id"] = inst.id;
            ij["shallowSize"] = inst.shallowSize;
            ij["retainedSize"] = inst.retainedSize;
            json refs = json::array();
            // 排序：先按 handleType（global handle 优先），再按 callStackId 从大到小
            auto sortedRefs = inst.nativeRef;
            std::sort(sortedRefs.begin(), sortedRefs.end(),
                [](const NativeRefEntry& a, const NativeRefEntry& b) {
                    if (a.handleType != b.handleType) {
                        return a.handleType < b.handleType;
                    }
                    return a.callStackId > b.callStackId;
                });
            for (const auto& ref : sortedRefs) {
                json rj;
                rj["handleType"] = HtraceEventTypeName(ref.handleType);
                rj["callStackId"] = ref.callStackId;
                json raddrs = json::array();
                for (const auto& addr : ref.addrs) {
                    raddrs.push_back(addr);
                }
                rj["addrs"] = raddrs;
                refs.push_back(rj);
            }
            ij["nativeRef"] = refs;
            instances.push_back(ij);
        }
        cj["instances"] = instances;
        arr.push_back(cj);
    }
    return arr;
}

} // namespace

std::string TshandleWriteParseJson(const TshandleParseResult& parsed, const std::string& filePath,
    const RunContext& ctx, const std::string& outDir)
{
    StageTimer timer("JSON write");

    auto base = platform::Stem(filePath);
    if (base.empty()) {
        base = "output";
    }
    auto outPath = platform::JoinPath(outDir, base + ".json");
    platform::CreateDirectories(outDir);

    JsonStreamWriter writer(outPath);

    json meta = BuildOutputMetaBase(ctx);
    meta["sourceFileInfo"] = {{"zipFile", filePath}};

    json content;
    content["summary"] = BuildSummary(parsed.CallStacks(), parsed.arktsClasses);

    if (!parsed.arktsClasses.empty()) {
        content["arkTsClasses"] = ArktsClassesToJson(parsed.arktsClasses);
    } else {
        content["arkTsClasses"] = json::array();
    }

    // memoryTraceInfo
    json callStacksJson = json::array();
    for (const auto& cs : parsed.CallStacks()) {
        callStacksJson.push_back(CallStackToJson(cs));
    }
    content["memoryTraceInfo"] = json::object();
    content["memoryTraceInfo"]["callStacks"] = callStacksJson;

    writer.BeginObject();
    writer.WriteKey("meta");
    writer.WriteRaw(meta.dump(JSON_INDENT_SPACES));
    writer.WriteKey("content");
    writer.WriteRaw(content.dump(JSON_INDENT_SPACES));
    writer.EndObject();

    writer.Close();

    timer.Tick("total");
    log::Logok("Output written to: ", outPath);
    return outPath;
}

} // namespace mem_analyzer
