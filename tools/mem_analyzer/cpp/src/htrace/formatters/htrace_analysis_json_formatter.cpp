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
#include "htrace/formatters/htrace_analysis_json_formatter.hpp"
#include "formatter/json/json_stream_writer.hpp"
#include "formatter/json/json_writer.hpp"
#include "htrace/formatters/htrace_json_formatter.hpp"
#include "htrace/htrace_category.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <variant>

namespace mem_analyzer {
namespace {

json BuildSummary(const HtraceAnalysisOutput& full, const RunContext& ctx)
{
    const auto& ts = full.typeSizes;

    auto getTypeSize = [&](const char* name) -> long long {
        auto et = HtraceEventTypeFromName(name);
        if (et == HtraceEventType::UNKNOWN) {
            return 0;
        }
        auto it = ts.find(et);
        return it != ts.end() ? it->second : 0;
    };

    json summary = HtraceBuildSummaryCommon(full.mode,
                                               full.traceMeta.durationNs,
                                               full.traceMeta.startTimeMs,
                                               full.traceMeta.endTimeMs,
                                               full.processName);

    // Analysis mode: output only the selected resource type（summary_entries 表驱动）
    const auto& spec = HtraceCategory(ctx.htrace.category);
    for (const auto& e : spec.summaryEntries) {
        long long v = 0;
        for (const char* tn : e.typeNames) {
            v += getTypeSize(tn);
        }
        if (v > 0) {
            summary[e.JsonKey] = std::to_string(v);
        }
    }

    return summary;
}

} // namespace

std::string HtraceWriteAnalysisJson(const HtraceAnalysisOutput& full,
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

    json summary = BuildSummary(full, ctx);
    long long totalExistingSize =
        HtraceSummaryTotalForType(summary, ctx.htrace.category);
    if (totalExistingSize <= 0) {
        totalExistingSize = 1;
    }

    writer.BeginObject();

    json meta = HtraceBuildMeta(filePath, ctx);
    writer.WriteKey("meta");
    writer.WriteRaw(meta.dump(4));

    writer.WriteKey("content");
    writer.BeginObject();

    // summary (small, raw dump)
    writer.WriteKey("summary");
    writer.BeginObject();
    writer.WriteKey("htrace");
    writer.WriteRaw(summary.dump(4));
    writer.EndObject();

    // topSymbols — N 轨（每 scope 按轨键嵌套：mmap={mmapAnons,mmapFilepages}，非 mmap={nativeHeaps|dmas|ashmems|fds|threads}）
    const auto& topSyms = full.topSymbols;
    writer.WriteKey("topSymbols");
    writer.BeginObject();
    writer.WriteKeyValue("mode", topSyms.mode);

    // 单个符号（不含数组包裹，供多条流式复用）
    auto writeSymbol = [&](const TopSymbolInfo& ts, int64_t& rank) {
        long long existingSize = ts.totalLeakSize;
        double percentage = totalExistingSize > 0 ?
            std::round((double)existingSize / (double)totalExistingSize * 10000.0) / 100.0 :
            0.0;

        std::string lib = ts.libs.empty() ? "" : ts.libs[0];
        std::string buildId = (!ts.buildIdNull && ts.buildId != 0) ?
            full.dataDict.at(ts.buildId) : "";

        writer.BeginObject();
        writer.WriteKeyValue("rank", rank++);
        writer.WriteKeyValue("existingSize", (int64_t)existingSize);
        writer.WriteKeyValue("existingCount", (int64_t)ts.totalExistingCount);
        writer.WriteKeyValue("percentage", percentage);
        writer.WriteKeyValue("type", ts.type);
        writer.WriteKeyValue("name", ts.symbol);
        writer.WriteKeyValue("lib", lib);
        writer.WriteKeyValue("libOffset", ts.libOffset);
        writer.WriteKeyValue("buildId", buildId);

        // Always write flamegraph and icicle (even when empty, for base compatibility)
        json flameArr = json::array();
        for (const auto& c : ts.flameGraph.children)
            flameArr.push_back(c.ToFlameJson(full.dataDict));
        writer.WriteKey("flamegraph");
        writer.WriteRaw(flameArr.dump(4));

        json icicleArr = json::array();
        for (const auto& c : ts.icicleGraph.children)
            icicleArr.push_back(c.ToIcicleJson(full.dataDict));
        writer.WriteKey("icicle");
        writer.WriteRaw(icicleArr.dump(4));

        writer.EndObject();
    };

    // 单个 filepage 文件（callStacks 展开，对齐 parse JSON 的调用栈结构）
    auto writeFilepage = [&](const FilepageInfo& fp) {
        writer.BeginObject();
        writer.WriteKeyValue("fileName", fp.filename);
        writer.WriteKeyValue("size", (int64_t)fp.size);
        writer.WriteKeyValue("count", (int64_t)fp.count);
        writer.WriteKeyValue("percentage", fp.percentage);

        writer.WriteKey("callStacks");
        writer.BeginArray();
        for (const auto& cs : fp.CallStacks) {
            writer.BeginObject();
            writer.WriteKeyValue("id", std::to_string(cs.id));
            writer.WriteKeyValue("existingSize", std::to_string(cs.existingSize));
            writer.WriteKeyValue("existingCount", std::to_string(cs.existingCount));

            writer.WriteKey("frames");
            writer.BeginArray();
            for (const auto& f : cs.frames) {
                HtraceWriteCallstackFrame(writer, f, full.dataDict);
            }
            writer.EndArray();

            writer.EndObject();
        }
        writer.EndArray();

        writer.EndObject();
    };

    // N 轨：每轨独立 rank（filepage 无 rank），std::get_if 按行分发
    auto writeSection = [&](const HtraceSection& sec) {
        int64_t rank = 1;
        for (const auto& row : sec.rows) {
            if (const auto* ts = std::get_if<TopSymbolInfo>(&row)) {
                writeSymbol(*ts, rank);
            } else if (const auto* fp = std::get_if<FilepageInfo>(&row)) {
                writeFilepage(*fp);
            }
        }
    };

    // N 轨写器路由：每个 scope 恒按 json_key 嵌套成对象（单轨一个 key、多轨多个 key），
    // 且每个键恒写（该轨为空也输出空数组）。scope 全空（所有轨均无行）才跳过。
    // mmap → {mmapAnons, mmapFilepages}；非 mmap → {nativeHeaps} / {dmas} / …（键来自描述符表 tracks[].json_key）。
    auto writeScope = [&](const std::string& key, const ScopeResult& scope) {
        if (scope.empty()) {
            return;
        }
        writer.WriteKey(key);
        writer.BeginObject();
        for (const auto& sec : scope.sections) {
            writer.WriteKey(sec.key);
            writer.BeginArray();
            writeSection(sec);
            writer.EndArray();
        }
        writer.EndObject();
    };
    writeScope("app", topSyms.app);
    writeScope("system", topSyms.system);
    writeScope("mixed", topSyms.mixed);

    writer.EndObject();

    writer.EndObject();

    writer.EndObject();

    writer.close();

    timer.tick("total");
    log::Logok("Output written to: ", outPath);
    return outPath;
}

} // namespace mem_analyzer