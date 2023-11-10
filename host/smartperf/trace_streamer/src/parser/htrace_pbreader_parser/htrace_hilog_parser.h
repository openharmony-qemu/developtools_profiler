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
#ifndef HTRACE_HILOG_PARSER_H
#define HTRACE_HILOG_PARSER_H
#include <cstdint>
#include <map>
#include <string>
#include "common_types.h"
#include "event_parser_base.h"
#include "trace_data/trace_data_cache.h"
#include "hilog_plugin_result.pb.h"
#include "htrace_plugin_time_parser.h"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class HtraceHiLogParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceHiLogParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceHiLogParser();
    void Parse(ProtoReader::BytesView tracePacket, bool& haveSplitSeg);
    std::map<uint32_t, std::string> logLevelString_ = {{TS_DEBUG, "D"},
                                                       {TS_ERROR, "E"},
                                                       {TS_INFO, "I"},
                                                       {TS_VERBOSE, "V"},
                                                       {TS_WARN, "W"}};
    void Finish();

private:
    uint64_t lastLineSeq_ = 0;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_HILOG_PARSER_H
