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
#include "htrace_cpu_data_parser.h"
#include "clock_filter_ex.h"
#include "htrace_event_parser.h"
#include "process_filter.h"
#include "stat_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceCpuDataParser::HtraceCpuDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx)
{
}

HtraceCpuDataParser::~HtraceCpuDataParser()
{
    TS_LOGI("cpuData ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(GetPluginStartTime()),
            static_cast<unsigned long long>(GetPluginEndTime()));
}
void HtraceCpuDataParser::Parse(ProtoReader::BytesView tracePacket, uint64_t ts)
{
    ProtoReader::CpuData_Reader cpuData(tracePacket.data_, tracePacket.size_);
    if (!cpuData.has_cpu_usage_info() && !cpuData.has_thread_info()) {
        return;
    }
    if (cpuData.has_cpu_usage_info()) {
        auto userLoad = cpuData.user_load();
        auto sysLoad = cpuData.sys_load();
        auto process_num = cpuData.process_num();
        ts = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, ts);
        UpdatePluginTimeRange(TS_CLOCK_REALTIME, ts, ts);
        auto cpuUsage = std::make_unique<TsCpuData>();
        streamFilters_->statFilter_->IncreaseStat(TRACE_CPU_USAGE, STAT_EVENT_RECEIVED);
        cpuUsage->SetCpuUsage(ts);
        cpuUsage->SetExtInfo(cpuData.total_load(), userLoad, sysLoad, process_num);
        cpuData_.push_back(std::move(cpuUsage));
    }
}
void HtraceCpuDataParser::Finish()
{
    auto cmp = [](const std::unique_ptr<TsCpuData>& a, const std::unique_ptr<TsCpuData>& b) { return a->ts_ < b->ts_; };
#ifdef IS_WASM
    std::sort(cpuData_.begin(), cpuData_.end(), cmp);
#else
    std::stable_sort(cpuData_.begin(), cpuData_.end(), cmp);
#endif
    bool firstTime = true;
    uint64_t lastTs = 0;
    for (auto itor = cpuData_.begin(); itor != cpuData_.end(); itor++) {
        auto newTimeStamp = (*itor)->ts_;
        if (firstTime) {
            lastTs = newTimeStamp;
            firstTime = false;
            continue;
        }
        auto dur = newTimeStamp - lastTs;
        traceDataCache_->GetCpuUsageInfoData()->AppendNewData(
            newTimeStamp, dur, (*itor)->totalLoad_, (*itor)->userLoad_, (*itor)->sysLoad_, (*itor)->process_num_);
        lastTs = newTimeStamp;
    }
    cpuData_.clear();
    traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
}
} // namespace TraceStreamer
} // namespace SysTuning
