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

json build_summary(const HtraceAnalysisOutput& full, const RunContext& ctx) {
    const auto& ts = full.type_sizes;

    auto get_type_size = [&](const char* name) -> long long {
        auto et = htrace_event_type_from_name(name);
        if (et == HtraceEventType::UNKNOWN) return 0;
        auto it = ts.find(et);
        return it != ts.end() ? it->second : 0;
    };

    json summary = htrace_build_summary_common(full.mode,
                                               full.trace_meta.duration_ns,
                                               full.trace_meta.start_time_ms,
                                               full.trace_meta.end_time_ms,
                                               full.process_name);

    // Analysis mode: output only the selected resource type（summary_entries 表驱动）
    const auto& spec = htrace_category(ctx.htrace.category);
    for (const auto& e : spec.summary_entries) {
        long long v = 0;
        for (const char* tn : e.type_names) v += get_type_size(tn);
        if (v > 0) summary[e.json_key] = std::to_string(v);
    }

    return summary;
}

} // namespace

std::string htrace_write_analysis_json(const HtraceAnalysisOutput& full,
                                       const std::string& file_path, const RunContext& ctx,
                                       const std::string& out_dir) {
    StageTimer timer("JSON write");

    auto base = platform::stem(file_path);
    if (base.empty()) base = "output";
    auto out_path = platform::join_path(out_dir, base + ".json");
    platform::create_directories(out_dir);

    JsonStreamWriter writer(out_path);

    json summary = build_summary(full, ctx);
    long long total_existing_size =
        htrace_summary_total_for_type(summary, ctx.htrace.category);
    if (total_existing_size <= 0) total_existing_size = 1;

    writer.begin_object();

    json meta = htrace_build_meta(file_path, ctx);
    writer.write_key("meta");
    writer.write_raw(meta.dump(4));

    writer.write_key("content");
    writer.begin_object();

    // summary (small, raw dump)
    writer.write_key("summary");
    writer.begin_object();
    writer.write_key("htrace");
    writer.write_raw(summary.dump(4));
    writer.end_object();

    // topSymbols — N 轨（每 scope 按轨键嵌套：mmap={mmapAnons,mmapFilepages}，非 mmap={nativeHeaps|dmas|ashmems|fds|threads}）
    const auto& top_syms = full.top_symbols;
    writer.write_key("topSymbols");
    writer.begin_object();
    writer.write_key_value("mode", top_syms.mode);

    // 单个符号（不含数组包裹，供多条流式复用）
    auto write_symbol = [&](const TopSymbolInfo& ts, int64_t& rank) {
        long long existing_size = ts.total_leak_size;
        double percentage = total_existing_size > 0
            ? std::round((double)existing_size / (double)total_existing_size * 10000.0) / 100.0
            : 0.0;

        std::string lib = ts.libs.empty() ? "" : ts.libs[0];
        std::string build_id = (!ts.build_id_null && ts.build_id != 0)
            ? full.data_dict.at(ts.build_id) : "";

        writer.begin_object();
        writer.write_key_value("rank", rank++);
        writer.write_key_value("existingSize", (int64_t)existing_size);
        writer.write_key_value("existingCount", (int64_t)ts.total_existing_count);
        writer.write_key_value("percentage", percentage);
        writer.write_key_value("type", ts.type);
        writer.write_key_value("name", ts.symbol);
        writer.write_key_value("lib", lib);
        writer.write_key_value("libOffset", ts.lib_offset);
        writer.write_key_value("buildId", build_id);

        // Always write flamegraph and icicle (even when empty, for base compatibility)
        json flame_arr = json::array();
        for (const auto& c : ts.flame_graph.children)
            flame_arr.push_back(c.to_flame_json(full.data_dict));
        writer.write_key("flamegraph");
        writer.write_raw(flame_arr.dump(4));

        json icicle_arr = json::array();
        for (const auto& c : ts.icicle_graph.children)
            icicle_arr.push_back(c.to_icicle_json(full.data_dict));
        writer.write_key("icicle");
        writer.write_raw(icicle_arr.dump(4));

        writer.end_object();
    };

    // 单个 filepage 文件（callStacks 展开，对齐 parse JSON 的调用栈结构）
    auto write_filepage = [&](const FilepageInfo& fp) {
        writer.begin_object();
        writer.write_key_value("fileName", fp.filename);
        writer.write_key_value("size", (int64_t)fp.size);
        writer.write_key_value("count", (int64_t)fp.count);
        writer.write_key_value("percentage", fp.percentage);

        writer.write_key("callStacks");
        writer.begin_array();
        for (const auto& cs : fp.call_stacks) {
            writer.begin_object();
            writer.write_key_value("id", std::to_string(cs.id));
            writer.write_key_value("existingSize", std::to_string(cs.existing_size));
            writer.write_key_value("existingCount", std::to_string(cs.existing_count));

            writer.write_key("frames");
            writer.begin_array();
            for (const auto& f : cs.frames) {
                htrace_write_callstack_frame(writer, f, full.data_dict);
            }
            writer.end_array();

            writer.end_object();
        }
        writer.end_array();

        writer.end_object();
    };

    // N 轨：每轨独立 rank（filepage 无 rank），std::get_if 按行分发
    auto write_section = [&](const HtraceSection& sec) {
        int64_t rank = 1;
        for (const auto& row : sec.rows) {
            if (const auto* ts = std::get_if<TopSymbolInfo>(&row)) {
                write_symbol(*ts, rank);
            } else if (const auto* fp = std::get_if<FilepageInfo>(&row)) {
                write_filepage(*fp);
            }
        }
    };

    // N 轨写器路由：每个 scope 恒按 json_key 嵌套成对象（单轨一个 key、多轨多个 key），
    // 且每个键恒写（该轨为空也输出空数组）。scope 全空（所有轨均无行）才跳过。
    // mmap → {mmapAnons, mmapFilepages}；非 mmap → {nativeHeaps} / {dmas} / …（键来自描述符表 tracks[].json_key）。
    auto write_scope = [&](const std::string& key, const ScopeResult& scope) {
        if (scope.empty()) return;
        writer.write_key(key);
        writer.begin_object();
        for (const auto& sec : scope.sections) {
            writer.write_key(sec.key);
            writer.begin_array();
            write_section(sec);
            writer.end_array();
        }
        writer.end_object();
    };
    write_scope("app", top_syms.app);
    write_scope("system", top_syms.system);
    write_scope("mixed", top_syms.mixed);

    writer.end_object();

    writer.end_object();

    writer.end_object();

    writer.close();

    timer.tick("total");
    log::logok("Output written to: ", out_path);
    return out_path;
}

} // namespace mem_analyzer