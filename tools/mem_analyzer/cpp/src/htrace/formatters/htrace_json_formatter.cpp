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
#include "htrace/formatters/htrace_json_formatter.hpp"
#include "htrace/htrace_category.hpp"
#include "formatter/json/json_writer.hpp"
#include "formatter/json/output_meta.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace mem_analyzer {

constexpr int DURATION_PRECISION = 3;

json HtraceBuildMeta(const std::string& filePath, const RunContext& ctx)
{
    json meta = BuildOutputMetaBase(ctx);
    meta["sourceFileInfo"] = {
        {"name", filePath},
        {"fileType", "htrace"}
    };
    return meta;
}

json HtraceBuildSummaryCommon(const std::string& mode, long long durationNs, long long startMs,
    long long endMs, const std::string& processName)
{
    json summary;
    summary["isStatistics"] = (mode == "detail") ? "0" : "1";

    double durationSeconds = std::round(durationNs / 1e6) / 1e3;
    std::ostringstream durSs;
    durSs << std::fixed << std::setprecision(DURATION_PRECISION) << durationSeconds;
    summary["duration"] = durSs.str();
    summary["timestampFrom"] = startMs > 0 ? FormatReadableTimestamp(startMs) : "0";
    summary["timestampTo"] = endMs > 0 ? FormatReadableTimestamp(endMs) : "0";
    summary["processName"] = processName;
    return summary;
}

long long HtraceSummaryTotalForType(const json& htraceSummary, const std::string& category)
{
    const char* key = HtraceCategory(category).summaryTotalKey;
    if (htraceSummary.contains(key)) {
        const auto& v = htraceSummary[key];
        if (v.is_string()) {
            return std::atoll(v.get<std::string>().c_str());
        }
        return v.get<long long>();
    }
    return 0;
}

void HtraceWriteCallstackFrame(JsonStreamWriter& writer, const CompressedStackFrame& frame, const DataDict& dict)
{
    writer.BeginObject();

    writer.WriteKeyValue("depth", std::to_string(frame.depth));

    std::string symbol = "unknown";
    auto symIt = dict.find(frame.symbolId);
    if (symIt != dict.end()) {
        symbol = symIt->second;
    }
    writer.WriteKeyValue("symbol", symbol);

    std::string lib = "unknown";
    if (!frame.fileIdNull) {
        auto libIt = dict.find(frame.fileId);
        if (libIt != dict.end()) {
            lib = libIt->second;
        }
    }
    writer.WriteKeyValue("lib", lib);

    std::string libOffset;
    if (!frame.vaddrNull) {
        char buf[32];
        snprintf(buf, sizeof(buf), "0x%llx", (unsigned long long)frame.vaddr);
        libOffset = buf;
    }
    writer.WriteKeyValue("libOffset", libOffset);

    std::string buildId;
    if (!frame.buildIdNull && frame.buildId != 0) {
        auto bdIt = dict.find(frame.buildId);
        if (bdIt != dict.end()) {
            buildId = bdIt->second;
        }
    }
    writer.WriteKeyValue("buildId", buildId);

    writer.EndObject();
}

} // namespace mem_analyzer
