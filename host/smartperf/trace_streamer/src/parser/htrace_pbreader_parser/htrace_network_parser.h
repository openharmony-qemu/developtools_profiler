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
#ifndef HTRACE_NETWORK_PARSER_H
#define HTRACE_NETWORK_PARSER_H
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "common_types.h"
#include "event_parser_base.h"
#include "htrace_plugin_time_parser.h"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"
namespace SysTuning {
namespace TraceStreamer {
class HtraceNetworkParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceNetworkParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceNetworkParser();
    void Parse(ProtoReader::BytesView tracePacket, uint64_t ts);
    void Finish();
    struct TsNetworkData {
        uint64_t ts;
        uint64_t tv_sec;
        uint64_t tv_nsec;
        uint64_t rx_bytes;
        uint64_t rx_packets;
        uint64_t tx_bytes;
        uint64_t tx_packets;
    };
    std::vector<TsNetworkData> networkData_;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_NETWORK_PARSER_H
