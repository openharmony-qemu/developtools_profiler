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

json htrace_build_meta(const std::string& file_path, const RunContext& ctx) {
    json meta = build_output_meta_base(ctx);
    meta["sourceFileInfo"] = {
        {"name", file_path},
        {"fileType", "htrace"}
    };
    return meta;
}

json htrace_build_summary_common(const std::string& mode, long long duration_ns, long long start_ms,
                                 long long end_ms, const std::string& process_name) {
    json summary;
    summary["isStatistics"] = (mode == "detail") ? "0" : "1";

    double duration_seconds = std::round(duration_ns / 1e6) / 1e3;
    std::ostringstream dur_ss;
    dur_ss << std::fixed << std::setprecision(3) << duration_seconds;
    summary["duration"] = dur_ss.str();
    summary["timestampFrom"] = start_ms > 0 ? format_readable_timestamp(start_ms) : "0";
    summary["timestampTo"] = end_ms > 0 ? format_readable_timestamp(end_ms) : "0";
    summary["processName"] = process_name;
    return summary;
}

long long htrace_summary_total_for_type(const json& htrace_summary, const std::string& category) {
    const char* key = htrace_category(category).summary_total_key;
    if (htrace_summary.contains(key)) {
        const auto& v = htrace_summary[key];
        if (v.is_string()) return std::atoll(v.get<std::string>().c_str());
        return v.get<long long>();
    }
    return 0;
}

void htrace_write_callstack_frame(JsonStreamWriter& writer,
                                  const CompressedStackFrame& frame,
                                  const DataDict& dict) {
    writer.begin_object();

    writer.write_key_value("depth", std::to_string(frame.depth));

    std::string symbol = "unknown";
    auto sym_it = dict.find(frame.symbol_id);
    if (sym_it != dict.end()) symbol = sym_it->second;
    writer.write_key_value("symbol", symbol);

    std::string lib = "unknown";
    if (!frame.file_id_null) {
        auto lib_it = dict.find(frame.file_id);
        if (lib_it != dict.end()) lib = lib_it->second;
    }
    writer.write_key_value("lib", lib);

    std::string lib_offset;
    if (!frame.vaddr_null) {
        char buf[32];
        snprintf(buf, sizeof(buf), "0x%llx", (unsigned long long)frame.vaddr);
        lib_offset = buf;
    }
    writer.write_key_value("libOffset", lib_offset);

    std::string build_id;
    if (!frame.build_id_null && frame.build_id != 0) {
        auto bd_it = dict.find(frame.build_id);
        if (bd_it != dict.end()) build_id = bd_it->second;
    }
    writer.write_key_value("buildId", build_id);

    writer.end_object();
}

} // namespace mem_analyzer
