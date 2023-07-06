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
#ifndef HTRACE_DISKI0_PARSER_H
#define HTRACE_DISKI0_PARSER_H
#include <cstdint>
#include <map>
#include <string>
#include "common_types.h"
#include "event_parser_base.h"
#include "htrace_plugin_time_parser.h"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class HtraceDiskIOParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceDiskIOParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceDiskIOParser();
    void Parse(ProtoReader::BytesView tracePacket, uint64_t ts);
    void Finish();

private:
    struct TsDiskIOData {
        uint64_t ts;
        int64_t rdSectorsKb;
        int64_t wrSectorsKb;
        int64_t prevRdSectorsKb;
        int64_t prevWrSectorsKb;
        double rdCountPerSec; // 每秒从设备读取的数据量 kB_read/s
        double wrCountPerSec; // 每秒向设备写入的数据量 kB_wrtn/s
        uint64_t rdCount;     // 读取的总数据量 kB_read
        uint64_t wrCount;     // 写入的总数量数据量 kB_wrtn
    };
    std::vector<TsDiskIOData> diskIOData_;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_DISKI0_PARSER_H
