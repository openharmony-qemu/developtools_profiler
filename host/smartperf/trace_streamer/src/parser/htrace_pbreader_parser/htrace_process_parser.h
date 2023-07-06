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
#ifndef HTRACE_PROCESS_PARSER_H
#define HTRACE_PROCESS_PARSER_H
#include <cstdint>
#include <map>
#include <string>
#include "common_types.h"
#include "event_parser_base.h"
#include "htrace_plugin_time_parser.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class HtraceProcessParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceProcessParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceProcessParser();
    void Parse(ProtoReader::BytesView tracePacket, uint64_t ts);
    void Finish();
    struct DiskioInfo {
        DiskioInfo(uint64_t rchar,
                   uint64_t wchar,
                   uint64_t syscr,
                   uint64_t syscw,
                   uint64_t rbytes,
                   uint64_t wbytes,
                   uint64_t cancelled_wbytes)
            : rchar_(rchar),
              wchar_(wchar),
              syscr_(syscr),
              syscw_(syscw),
              rbytes_(rbytes),
              wbytes_(wbytes),
              cancelled_wbytes_(cancelled_wbytes)
        {
        }
        uint64_t rchar_;
        uint64_t wchar_;
        uint64_t syscr_;
        uint64_t syscw_;
        uint64_t rbytes_;
        uint64_t wbytes_;
        uint64_t cancelled_wbytes_;
    };
    struct PssInfo {
        explicit PssInfo(int32_t pss_info) : pss_info_(pss_info) {}
        int32_t pss_info_;
    };
    struct CpuInfo {
        CpuInfo(double cpu_usage, int32_t thread_sum, uint64_t cpu_time_ms)
            : cpu_usage_(cpu_usage), thread_sum_(thread_sum), cpu_time_ms_(cpu_time_ms)
        {
        }
        double cpu_usage_;
        int32_t thread_sum_;
        uint64_t cpu_time_ms_;
    };
    struct ProcessInfo {
        ProcessInfo(int32_t pid, const std::string& name, int32_t ppid, int32_t uid)
            : pid_(pid), name_(name), ppid_(ppid), uid_(uid)
        {
        }
        int32_t pid_;
        std::string name_;
        int32_t ppid_;
        int32_t uid_;
    };
    struct TsLiveProcessData {
        void SetLiveProcess(uint64_t ts,
                            std::unique_ptr<ProcessInfo> liveProcessInfo,
                            std::unique_ptr<CpuInfo> cpuUsageData,
                            std::unique_ptr<PssInfo> pssInfo,
                            std::unique_ptr<DiskioInfo> diskio)
        {
            ts_ = ts;
            processInfo_ = std::move(liveProcessInfo);
            cpuUsageData_ = std::move(cpuUsageData);
            pssInfo_ = std::move(pssInfo);
            diskio_ = std::move(diskio);
        }
        uint64_t ts_;
        std::unique_ptr<ProcessInfo> processInfo_;
        std::unique_ptr<CpuInfo> cpuUsageData_;
        std::unique_ptr<PssInfo> pssInfo_;
        std::unique_ptr<DiskioInfo> diskio_;
    };
    std::vector<std::unique_ptr<TsLiveProcessData>> liveProcessData_;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_PROCESS_PARSER_H
