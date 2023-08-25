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
#include "bio_latency_data_parser.h"
#include "process_filter.h"
#include "stat_filter.h"
#include "string_to_numerical.h"
#include <cinttypes>

namespace SysTuning {
namespace TraceStreamer {
BioLatencyDataParser::BioLatencyDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx), EbpfBase(dataCache, ctx), timeParser_(std::make_unique<HtracePluginTimeParser>())
{
}
BioLatencyDataParser::~BioLatencyDataParser()
{
    TS_LOGI("EBPF Bio data ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(timeParser_->GetPluginStartTime()),
            static_cast<unsigned long long>(timeParser_->GetPluginEndTime()));
}
void BioLatencyDataParser::ParseBioLatencyEvent()
{
    if (!reader_->GetBIOSampleMap().size()) {
        return;
    }
    for (auto mapItor = reader_->GetBIOSampleMap().begin(); mapItor != reader_->GetBIOSampleMap().end(); mapItor++) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_EBPF_BIO_LATENCY, STAT_EVENT_RECEIVED);
        auto bioFixedHeadrAddr = mapItor->second;
        bool callIdExistFlag = false;

        auto userIpsAddr = reinterpret_cast<const uint64_t*>(bioFixedHeadrAddr + 1);
        if (bioFixedHeadrAddr->nips) {
            std::string ipsToStr(reinterpret_cast<const char*>(userIpsAddr), bioFixedHeadrAddr->nips * SINGLE_IP_SIZE);
            auto ipsHashValue = hashFun_(ipsToStr);
            auto value = pidAndipsToCallId_.Find(bioFixedHeadrAddr->pid, ipsHashValue);
            if (value != INVALID_UINT64) {
                callIdExistFlag = true;
                currentCallId_ = value;
            } else {
                pidAndipsToCallId_.Insert(bioFixedHeadrAddr->pid, ipsHashValue, callChainId_);
                currentCallId_ = callChainId_++;
            }
        } else {
            currentCallId_ = INVALID_UINT64;
        }

        uint32_t type = bioFixedHeadrAddr->type;
        // Init process name data
        const char* processName = reinterpret_cast<const char*>(bioFixedHeadrAddr->processName);
        uint32_t ipid =
            streamFilters_->processFilter_->UpdateOrCreateProcessWithName(bioFixedHeadrAddr->pid, processName);
        uint32_t itid =
            streamFilters_->processFilter_->GetOrCreateThreadWithPid(bioFixedHeadrAddr->tid, bioFixedHeadrAddr->pid);
        uint64_t startTs = bioFixedHeadrAddr->startTime;
        uint64_t endTs = bioFixedHeadrAddr->endTime;
        auto newStartTs = streamFilters_->clockFilter_->ToPrimaryTraceTime(clockId_, startTs);
        timeParser_->UpdatePluginTimeRange(clockId_, startTs, newStartTs);
        auto newEndTs = streamFilters_->clockFilter_->ToPrimaryTraceTime(clockId_, endTs);
        timeParser_->UpdatePluginTimeRange(clockId_, endTs, newEndTs);
        if (newStartTs > newEndTs) {
            TS_LOGE("startTs = %" PRIu64 ", endTs = %" PRIu64 ", newStartTs = %" PRIu64 ", newEndTs = %" PRIu64 "",
                    startTs, endTs, newStartTs, newEndTs);
            streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_EBPF_PAGED_MEMORY, STAT_EVENT_DATA_INVALID);
            return;
        }
        uint64_t duration = newEndTs - newStartTs;
        auto prio = bioFixedHeadrAddr->prio;
        auto size = bioFixedHeadrAddr->size;
        const int32_t FOUR_KB = 1024 * 4;
        uint32_t durPer4K = INVALID_UINT32;
        if (size) {
            durPer4K = duration / (size / FOUR_KB);
        }

        auto blkCount = ConvertToHexTextIndex(bioFixedHeadrAddr->blkcnt);

        auto tracerEventToStrIndexMap = reader_->GetTracerEventToStrIndexMap();
        auto pathId = tracerEventToStrIndexMap.Find(ITEM_EVENT_BIO, type, itid, startTs);
        if (pathId != INVALID_UINT64) {
            tracerEventToStrIndexMap.Erase(ITEM_EVENT_FS, type, itid, startTs);
        }
        traceDataCache_->GetBioLatencySampleData()->AppendNewData(
            currentCallId_, type, ipid, itid, newStartTs, newEndTs, duration, prio, size, blkCount, pathId, durPer4K);
        if (!callIdExistFlag) {
            ParseCallStackData(userIpsAddr, bioFixedHeadrAddr->nips, bioFixedHeadrAddr->pid, currentCallId_);
        }
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
