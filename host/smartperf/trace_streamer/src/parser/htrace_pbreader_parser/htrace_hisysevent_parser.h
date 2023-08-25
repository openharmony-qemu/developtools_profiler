
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

#ifndef HTRACE_HISYSEVENT_PARSER_H
#define HTRACE_HISYSEVENT_PARSER_H

#include "clock_filter_ex.h"
#include "event_parser_base.h"
#include "hi_sysevent_measure_filter.h"
#include "hisysevent_plugin_config.pbreader.h"
#include "hisysevent_plugin_result.pbreader.h"
#include "htrace_plugin_time_parser.h"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_filters.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
class HtraceHisyseventParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceHisyseventParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceHisyseventParser();
    void Finish();
    void Parse(ProtoReader::HisyseventInfo_Reader* tracePacket, uint64_t ts);
    void Parse(ProtoReader::HisyseventConfig_Reader* tracePacket, uint64_t ts);

private:
    void NoArrayDataParse(JsonData jData,
                          std::vector<size_t> noArrayIndex,
                          DataIndex eventSourceIndex,
                          uint64_t serial);
    void ArrayDataParse(JsonData jData,
                        std::vector<size_t> arrayIndex,
                        DataIndex eventSourceIndex,
                        size_t maxArraySize,
                        uint64_t serial);
    void CommonDataParser(JsonData jData, DataIndex eventSourceIndex, uint64_t serial);
    void AppendStringValue(nlohmann::json& value,
                           uint64_t serial,
                           DataIndex eventSourceIndex,
                           DataIndex keyIndex,
                           uint64_t timeStamp);
    const uint64_t MSEC_TO_NS = 1000 * 1000;
    std::vector<uint64_t> hisyseventTS_;
    bool isDeviceState = true;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // HTRACE_HISYSEVENT_PARSER_H
