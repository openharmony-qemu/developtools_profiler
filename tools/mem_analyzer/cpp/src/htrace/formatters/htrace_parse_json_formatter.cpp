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
#include "htrace/formatters/htrace_parse_json_formatter.hpp"
#include "formatter/json/json_stream_writer.hpp"
#include "formatter/json/json_writer.hpp"
#include "htrace/formatters/htrace_json_formatter.hpp"
#include "htrace/htrace_category.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <cstdio>
#include <iostream>
#include <variant>

namespace mem_analyzer {
namespace {

json BuildSummary(const TraceDB& db)
{
    const auto& ts = db.meta.typeSizes;

    auto getTypeSize = [&](const char* name) -> long long {
        auto et = HtraceEventTypeFromName(name);
        if (et == HtraceEventType::UNKNOWN) {
            return 0;
        }
        auto it = ts.find(et);
        return it != ts.end() ? it->second : 0;
    };

    json summary = HtraceBuildSummaryCommon(db.meta.mode, db.meta.durationNs,
        db.meta.startTimeMs, db.meta.endTimeMs,
        db.meta.processName);

    // Parse mode: output all resource types（summary_entries 表驱动，顺序=历史字节序）
    static const char* const kParseSummaryOrder[] = {
        "nativeheap", "mmap", "ashmem", "dma", "fd", "thread"
    };
    for (const char* cat : kParseSummaryOrder) {
        for (const auto& e : HtraceCategory(cat).summaryEntries) {
            long long v = 0;
            for (const char* tn : e.typeNames) {
                v += getTypeSize(tn);
            }
            if (v > 0) {
                summary[e.jsonKey] = std::to_string(v);
            }
        }
    }

    return summary;
}

} // namespace

std::string HtraceWriteParseJson(const TraceDB& db, const std::string& filePath,
    const RunContext& ctx, const std::string& outDir)
{
    StageTimer timer("JSON write");

    auto base = platform::stem(filePath);
    if (base.empty()) {
        base = "output";
    }
    auto outPath = platform::JoinPath(outDir, base + ".json");
    platform::CreateDirectories(outDir);

    JsonStreamWriter writer(outPath);

    const auto& stacks = std::get<CompressedCallStackList>(db.stacks);
    json summary = BuildSummary(db);

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

    // callStacks — streaming, one at a time
    writer.WriteKey("callStacks");
    writer.BeginArray();
    for (const auto& cs : stacks.stacks) {
        writer.BeginObject();
        writer.WriteKeyValue("id", std::to_string(cs.id));
        writer.WriteKeyValue("type", HtraceEventTypeName(cs.type));
        if (cs.type == HtraceEventType::FILE_PAGE_MSG) {
            std::string filename = "unknown";
            auto fit = stacks.dict.find(cs.subTypeId);
            if (fit != stacks.dict.end()) {
                filename = fit->second;
            }
            filename = NormalizeFilepageFilename(filename, db.meta.mode);
            writer.WriteKeyValue("fileName", filename);
        }
        writer.WriteKeyValue("existingSize", std::to_string(cs.existingSize));
        writer.WriteKeyValue("existingCount", std::to_string(cs.existingCount));

        writer.WriteKey("frames");
        writer.BeginArray();
        for (const auto& f : cs.frames) {
            HtraceWriteCallstackFrame(writer, f, stacks.dict);
        }
        writer.EndArray();

        writer.EndObject();
    }
    writer.EndArray();

    writer.EndObject();
    writer.EndObject();

    writer.close();

    timer.Tick("total");
    log::Logok("Output written to: ", outPath);
    return outPath;
}

} // namespace mem_analyzer