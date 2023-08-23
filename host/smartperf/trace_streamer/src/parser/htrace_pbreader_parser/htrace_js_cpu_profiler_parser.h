/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef HTRACE_JS_CPU_PERFILER_PARSER_H
#define HTRACE_JS_CPU_PERFILER_PARSER_H
#include <cstdint>
#include <string>
#include <vector>
#include "common_types.h"
#include "event_parser_base.h"
#include "htrace_plugin_time_parser.h"
#include "json.hpp"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"
using json = nlohmann::json;

namespace SysTuning {
namespace TraceStreamer {
class HtraceJsCpuProfilerParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceJsCpuProfilerParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceJsCpuProfilerParser(){};
    void ParseJsCpuProfiler(std::string result);
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_JS_CPU_PERFILER_PARSER_H
