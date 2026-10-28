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
#include "tshandle/formatters/tshandle_analysis_json_formatter.hpp"
#include "formatter/json/json_stream_writer.hpp"
#include "formatter/json/json_writer.hpp"
#include "formatter/json/output_meta.hpp"
#include "tshandle/formatters/tshandle_call_graph.hpp"
#include "tshandle/tshandle_analyzer.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/timer.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace mem_analyzer {
namespace {

double round2(double v)
{
    return std::round(v * 100.0) / 100.0;
}

double round1(double v)
{
    return std::round(v * 10.0) / 10.0;
}

std::string FormatDuration(long long durationNs)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) <<
        (static_cast<double>(durationNs) / 1e9);
    return oss.str();
}

json TraceMetaToJson(const TshandleAnalysisOutput& analysis)
{
    struct Totals {
        long long count = 0;
        long long memory = 0;
        long long shallow = 0;
        long long retained = 0;
    };
    Totals globalTotals, localTotals;

    for (const auto& stack : analysis.callStacks) {
        if (stack.type == HtraceEventType::ARK_GLOBAL_HANDLE) {
            ++globalTotals.count;
            globalTotals.memory += stack.existingSize;
        } else if (stack.type == HtraceEventType::ARK_LOCAL_HANDLE) {
            ++localTotals.count;
            localTotals.memory += stack.existingSize;
        }
    }

    for (const auto& cls : analysis.arktsClasses) {
        for (const auto& instance : cls.instances) {
            std::unordered_set<HtraceEventType> types;
            for (const auto& ref : instance.nativeRef) {
                types.insert(ref.handleType);
            }
            for (const auto& type : types) {
                if (type == HtraceEventType::ARK_GLOBAL_HANDLE) {
                    globalTotals.shallow += instance.shallowSize;
                    globalTotals.retained += instance.retainedSize;
                } else if (type == HtraceEventType::ARK_LOCAL_HANDLE) {
                    localTotals.shallow += instance.shallowSize;
                    localTotals.retained += instance.retainedSize;
                }
            }
        }
    }

    auto makeObj = [](const Totals& t) -> json {
        return {
            {"totalCount", t.count},
            {"totalMemory", t.memory},
            {"totalShallowSize", t.shallow},
            {"totalRetainedSize", t.retained}
        };
    };

    const auto& meta = analysis.traceMeta;
    return {
        {"isStatistics", (meta.mode == "detail") ? "0" : "1"},
        {"duration", FormatDuration(meta.durationNs)},
        {"timestampFrom", meta.startTimeMs > 0 ? FormatReadableTimestamp(meta.startTimeMs) : "0"},
        {"timestampTo", meta.endTimeMs > 0 ? FormatReadableTimestamp(meta.endTimeMs) : "0"},
        {"processName", meta.processName},
        {"global", makeObj(globalTotals)},
        {"local", makeObj(localTotals)}
    };
}

json RenderNodeToJson(const TshandleRenderNode& node)
{
    json children = json::array();
    for (const auto& child : node.children) {
        children.push_back(RenderNodeToJson(child));
    }
    return {
        {"depth", node.depth},
        {"shallowSize", node.value},
        {"retainedSize", node.total},
        {"count", node.count},
        {"name", node.symbol},
        {"lib", node.lib},
        {"libOffset", node.libOffset},
        {"buildId", node.buildId},
        {"callees", std::move(children)}
    };
}

json SectionWithTreesToJson(const TshandleTypeSection& section)
{
    json objectJson = json::array();
    for (const auto& object : section.topObjects) {
        auto tree = BuildTshandleDownstreamGraph(
            object.objectName, object.callChains, object.existingSize);
        json obj = {
            {"rank", object.rank},
            {"existingSize", object.existingSize},
            {"existingCount", object.count},
            {"percentage", round2(object.percentage)},
            {"name", tree.symbol}
        };
        json icicleArr = json::array();
        for (const auto& child : tree.children) {
            icicleArr.push_back(RenderNodeToJson(child));
        }
        obj["icicle"] = std::move(icicleArr);
        objectJson.push_back(std::move(obj));
    }
    return {
        {"totalObjects", section.totalObjects},
        {"totalSize", round1(section.totalSize)},
        {"totalCount", section.totalCount},
        {"totalCallStackExistingSize", round1(section.totalCallStackExistingSize)},
        {"topArkTsObjects", objectJson}
    };
}

} // namespace

std::string TshandleWriteAnalysisJson(const TshandleAnalysisOutput& analysis,
    const std::string& filePath, const RunContext& ctx,
    const std::string& outDir)
{
    StageTimer timer("JSON write");

    auto base = platform::stem(filePath);
    if (base.empty()) {
        base = "output";
    }
    auto outPath = platform::JoinPath(outDir, base + ".json");
    platform::CreateDirectories(outDir);

    JsonStreamWriter writer(outPath);

    json meta = BuildOutputMetaBase(ctx);
    std::string filename = platform::filename(filePath);
    auto dotPos = filename.rfind('.');
    std::string fileType = (dotPos != std::string::npos) ? filename.substr(dotPos + 1) : "";
    meta["sourceFileInfo"] = {{"name", filePath}, {"fileType", fileType}};

    json content;
    content["summary"] = {{"tshandle", TraceMetaToJson(analysis)}};
    content["global"] = {
        {"app", SectionWithTreesToJson(analysis.global.app)},
        {"system", SectionWithTreesToJson(analysis.global.sys)}
    };
    content["local"] = SectionWithTreesToJson(analysis.local);

    writer.BeginObject();
    writer.WriteKey("meta");
    writer.WriteRaw(meta.dump(4));
    writer.WriteKey("content");
    writer.WriteRaw(content.dump(4));
    writer.EndObject();

    writer.close();

    timer.tick("total");
    log::Logok("Output written to: ", outPath);
    return outPath;
}

} // namespace mem_analyzer
