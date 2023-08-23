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
#include "htrace_clock_detail_parser.h"
#include "clock_filter_ex.h"
#include "htrace_event_parser.h"
#include "measure_filter.h"
#include "process_filter.h"
#include "stat_filter.h"
#include "symbols_filter.h"
#include <cinttypes>

namespace SysTuning {
namespace TraceStreamer {
HtraceClockDetailParser::HtraceClockDetailParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters)
    : EventParserBase(dataCache, filters)
{
    for (auto i = 0; i < MEM_MAX; i++) {
        memNameDictMap_.insert(
            std::make_pair(static_cast<MemInfoType>(i),
                           traceDataCache_->GetDataIndex(config_.memNameMap_.at(static_cast<MemInfoType>(i)))));
    }
}

HtraceClockDetailParser::~HtraceClockDetailParser() = default;
void HtraceClockDetailParser::Parse(const ProtoReader::BytesView& tracePacket) const
{
    if (streamFilters_->clockFilter_->HasInitSnapShot()) {
        TS_LOGW("already has clock snapshot!!!");
        return;
    }
    ProtoReader::TracePluginResult_Reader reader((const uint8_t*)(tracePacket.data_), tracePacket.size_);
    if (!reader.has_clocks_detail()) {
        TS_LOGE("!!! no clock snapshot");
        return;
    }
    std::vector<SnapShot> snapShot;
    TS_LOGI("got clock snapshot");
    for (auto i = reader.clocks_detail(); i; i++) {
        ProtoReader::ClockDetailMsg_Reader clockInfo(i->ToBytes());
        auto id = clockInfo.FindDataArea(ProtoReader::ClockDetailMsg_Reader::kIdDataAreaNumber).ToUint32();
        ProtoReader::ClockDetailMsg_TimeSpec_Reader time(clockInfo.time());
        TS_LOGI("clockid:%d, ts:%llu", id, static_cast<unsigned long long>(time.tv_nsec() + time.tv_sec() * SEC_TO_NS));
        snapShot.push_back(SnapShot{static_cast<ClockId>(id), time.tv_nsec() + time.tv_sec() * SEC_TO_NS});
    }
    if (!snapShot.empty()) {
        streamFilters_->clockFilter_->AddClockSnapshot(snapShot);
    }
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_SYNC, STAT_EVENT_RECEIVED);
}

void HtraceClockDetailParser::Parse(const ProfilerTraceFileHeader* profilerTraceFileHeader) const
{
    if (streamFilters_->clockFilter_->HasInitSnapShot()) {
        TS_LOGW("already has clock snapshot!!!");
        return;
    }
    if (!profilerTraceFileHeader->data.boottime) {
        TS_LOGW("Profiler header has no clock snapshot!!!");
        return;
    }

    std::vector<SnapShot> snapShot;
    TS_LOGI("got clock snapshot");

    TS_LOGI("clockid: TS_CLOCK_BOOTTIME, ts:%" PRIu64 "", profilerTraceFileHeader->data.boottime);
    if (profilerTraceFileHeader->data.boottime) {
        snapShot.push_back(SnapShot{TS_CLOCK_BOOTTIME, profilerTraceFileHeader->data.boottime});
    }

    TS_LOGI("clockid: TS_CLOCK_REALTIME, ts:%" PRIu64 "", profilerTraceFileHeader->data.realtime);
    if (profilerTraceFileHeader->data.realtime) {
        snapShot.push_back(SnapShot{TS_CLOCK_REALTIME, profilerTraceFileHeader->data.realtime});
    }

    TS_LOGI("clockid: TS_CLOCK_REALTIME_COARSE, ts:%" PRIu64 "", profilerTraceFileHeader->data.realtimeCoarse);
    if (profilerTraceFileHeader->data.realtimeCoarse) {
        snapShot.push_back(SnapShot{TS_CLOCK_REALTIME_COARSE, profilerTraceFileHeader->data.realtimeCoarse});
    }

    TS_LOGI("clockid: TS_MONOTONIC, ts:%" PRIu64 "", profilerTraceFileHeader->data.monotonic);
    if (profilerTraceFileHeader->data.monotonic) {
        snapShot.push_back(SnapShot{TS_MONOTONIC, profilerTraceFileHeader->data.monotonic});
    }

    TS_LOGI("clockid: TS_MONOTONIC_COARSE, ts:%" PRIu64 "", profilerTraceFileHeader->data.monotonicCoarse);
    if (profilerTraceFileHeader->data.monotonicCoarse) {
        snapShot.push_back(SnapShot{TS_MONOTONIC_COARSE, profilerTraceFileHeader->data.monotonicCoarse});
    }

    TS_LOGI("clockid: TS_MONOTONIC_RAW, ts:%" PRIu64 "", profilerTraceFileHeader->data.monotonicRaw);
    if (profilerTraceFileHeader->data.monotonicRaw) {
        snapShot.push_back(SnapShot{TS_MONOTONIC_RAW, profilerTraceFileHeader->data.monotonicRaw});
    }

    if (!snapShot.empty()) {
        streamFilters_->clockFilter_->AddClockSnapshot(snapShot);
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_CLOCK_SYNC, STAT_EVENT_RECEIVED);
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
