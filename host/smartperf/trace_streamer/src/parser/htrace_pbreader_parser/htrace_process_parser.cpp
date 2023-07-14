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
#include "htrace_process_parser.h"
#include "clock_filter_ex.h"
#include "cpu_plugin_config.pbreader.h"
#include "cpu_plugin_result.pbreader.h"
#include "diskio_plugin_config.pbreader.h"
#include "diskio_plugin_result.pbreader.h"
#include "htrace_event_parser.h"
#include "process_filter.h"
#include "process_plugin_config.pbreader.h"
#include "process_plugin_result.pbreader.h"
#include "stat_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceProcessParser::HtraceProcessParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx)
{
}

HtraceProcessParser::~HtraceProcessParser()
{
    TS_LOGI("process ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(GetPluginStartTime()),
            static_cast<unsigned long long>(GetPluginEndTime()));
    TS_LOGI("process real ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(MinTs()),
            static_cast<unsigned long long>(MaxTs()));
}
void HtraceProcessParser::Parse(ProtoReader::BytesView tracePacket, uint64_t ts)
{
    ProtoReader::ProcessData_Reader processData(tracePacket.data_, tracePacket.size_);
    for (auto i = processData.processesinfo(); i; ++i) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_PROCESS, STAT_EVENT_START);
        ProtoReader::ProcessInfo_Reader processInfoParser(i->ToBytes());
        ProtoReader::CpuInfo_Reader cpuInfoParser(processInfoParser.cpuinfo());
        ProtoReader::PssInfo_Reader pssInfoParser(processInfoParser.pssinfo());
        ProtoReader::DiskioInfo_Reader diskioInfoParser(processInfoParser.diskinfo());
        auto liveProcess = std::make_unique<TsLiveProcessData>();
        auto processInfo =
            std::make_unique<ProcessInfo>(processInfoParser.pid(), processInfoParser.name().ToStdString(),
                                          processInfoParser.ppid(), processInfoParser.uid());
        auto cpuInfo = std::make_unique<CpuInfo>(cpuInfoParser.cpu_usage(), cpuInfoParser.thread_sum(),
                                                 cpuInfoParser.cpu_time_ms());
        auto pssInfo = std::make_unique<PssInfo>(pssInfoParser.pss_info());
        auto diskioInfo = std::make_unique<DiskioInfo>(
            diskioInfoParser.rchar(), diskioInfoParser.wchar(), diskioInfoParser.syscr(), diskioInfoParser.syscw(),
            diskioInfoParser.rbytes(), diskioInfoParser.wbytes(), diskioInfoParser.cancelled_wbytes());
        liveProcess->SetLiveProcess(ts, std::move(processInfo), std::move(cpuInfo), std::move(pssInfo),
                                    std::move(diskioInfo));
        liveProcessData_.push_back(std::move(liveProcess));
    }
}
void HtraceProcessParser::Finish()
{
    if (!liveProcessData_.size()) {
        TS_LOGW("process no data");
        return;
    }
    auto cmp = [](const std::unique_ptr<TsLiveProcessData>& a, const std::unique_ptr<TsLiveProcessData>& b) {
        return a->ts_ < b->ts_;
    };
#ifdef IS_WASM
    std::sort(liveProcessData_.begin(), liveProcessData_.end(), cmp);
#else
    std::stable_sort(liveProcessData_.begin(), liveProcessData_.end(), cmp);
#endif
    bool first = true;
    uint64_t lastTs = 0;
    for (auto itor = liveProcessData_.begin(); itor != liveProcessData_.end(); itor++) {
        auto tsOld = (*itor)->ts_;
        (*itor)->ts_ = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, (*itor)->ts_);
        UpdatePluginTimeRange(TS_CLOCK_REALTIME, tsOld, (*itor)->ts_);
        if (first) {
            lastTs = (*itor)->ts_;
            first = false;
            continue;
        }
        auto dur = (*itor)->ts_ - lastTs;
        lastTs = (*itor)->ts_;
        if (!(*itor)->processInfo_->pid_) {
            continue;
        }
        traceDataCache_->GetLiveProcessData()->AppendNewData(
            (*itor)->ts_, dur, (*itor)->processInfo_->pid_, (*itor)->processInfo_->name_, (*itor)->processInfo_->ppid_,
            (*itor)->processInfo_->uid_, std::to_string((*itor)->processInfo_->uid_),
            (*itor)->cpuUsageData_->cpu_usage_, (*itor)->pssInfo_->pss_info_, (*itor)->cpuUsageData_->cpu_time_ms_,
            (*itor)->cpuUsageData_->thread_sum_, (*itor)->diskio_->wbytes_, (*itor)->diskio_->rbytes_);
    }
    liveProcessData_.clear();
    traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
}
} // namespace TraceStreamer
} // namespace SysTuning
