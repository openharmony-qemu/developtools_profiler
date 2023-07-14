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
#ifndef HTRACE_CLOCKDETAIL_PARSER_H
#define HTRACE_CLOCKDETAIL_PARSER_H
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include "event_parser_base.h"
#include "file.h"
#include "proto_reader_help.h"
#include "htrace_file_header.h"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class HtraceClockDetailParser : public EventParserBase {
public:
    HtraceClockDetailParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters);
    ~HtraceClockDetailParser();
    void Parse(const ProtoReader::BytesView& tracePacket) const;
    void Parse(const ProfilerTraceFileHeader* profilerTraceFileHeader) const;

private:
    std::map<MemInfoType, DataIndex> memNameDictMap_ = {};
    TraceStreamerConfig config_{};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_CLOCKDETAIL_PARSER_H
