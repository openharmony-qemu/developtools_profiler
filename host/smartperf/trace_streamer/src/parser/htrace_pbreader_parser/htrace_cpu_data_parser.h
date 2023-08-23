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
#ifndef HTRACE_CPU_DATA_PARSER_H
#define HTRACE_CPU_DATA_PARSER_H
#include <cstdint>
#include <map>
#include <string>
#include "cpu_plugin_result.pbreader.h"
#include "event_parser_base.h"
#include "hilog_plugin_result.pbreader.h"
#include "htrace_plugin_time_parser.h"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class HtraceCpuDataParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceCpuDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceCpuDataParser();
    void Parse(ProtoReader::BytesView tracePacket, uint64_t ts);
    void Finish();
    enum TSCpuDataType { TSCpuDataType_Usage, TSCpuDataType_ThreadInfo, TSCpuDataType_Load };
    class TsCpuData {
    public:
        TsCpuData()
        {
            ts_ = 0;
            cpuDataType_ = TSCpuDataType_Usage;
        }
        void SetCpuUsage(uint64_t ts)
        {
            ts_ = ts;
            cpuDataType_ = TSCpuDataType_Usage;
        }
        void SetThreadInfo(uint64_t ts)
        {
            ts_ = ts;
            cpuDataType_ = TSCpuDataType_ThreadInfo;
        }
        void SetExtInfo(double totalLoad, double userLoad, double sysLoad, double process_num)
        {
            totalLoad_ = totalLoad;
            userLoad_ = userLoad;
            sysLoad_ = sysLoad;
            process_num_ = process_num;
            cpuDataType_ = TSCpuDataType_Load;
        }
        uint64_t ts_;
        TSCpuDataType cpuDataType_;
        double userLoad_ = 0;
        double sysLoad_ = 0;
        double process_num_ = 0;
        double totalLoad_ = 0;
    };
    std::vector<std::unique_ptr<TsCpuData>> cpuData_;

private:
    std::string threadStateDesc_[ProtoReader::THREAD_WAITING + 1] = {"undefined", "Running", "Sleep", "Sloped",
                                                                     "Watting"};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_CPU_DATA_PARSER_H
