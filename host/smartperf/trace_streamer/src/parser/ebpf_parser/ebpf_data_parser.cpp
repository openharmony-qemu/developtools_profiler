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
#include "ebpf_data_parser.h"
#include "stat_filter.h"

namespace SysTuning {
namespace TraceStreamer {
EbpfDataParser::EbpfDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx),
      EbpfBase(dataCache, ctx),
      FileSystemDataParser(dataCache, ctx),
      PagedMemoryDataParser(dataCache, ctx),
      BioLatencyDataParser(dataCache, ctx),
      ebpfDataReader_(std::make_unique<EbpfDataReader>(dataCache, ctx))
{
}
EbpfDataParser::~EbpfDataParser()
{
    TS_LOGI("EBPF all event data ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(ebpfAllEventStartTime_),
            static_cast<unsigned long long>(ebpfAllEventEndTime_));
}

bool EbpfDataParser::Init(const std::deque<uint8_t> dequeBuffer, uint64_t size)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_EBPF, STAT_EVENT_RECEIVED);
    if (!ebpfDataReader_->InitEbpfData(dequeBuffer, size)) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_EBPF, STAT_EVENT_NOTSUPPORTED);
        TS_LOGE("InitEbpfData failed!");
        return false;
    }
    if (!InitEbpfDataParser(ebpfDataReader_.get())) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_EBPF, STAT_EVENT_NOTSUPPORTED);
        return false;
    }
    return true;
}

void EbpfDataParser::InitAndParseEbpfData(const std::deque<uint8_t>& dequeBuffer, uint64_t size)
{
    if (!Init(dequeBuffer, size)) {
        return;
    }
    if (ebpfDataReader_->GetFileSystemEventMap().size()) {
        ParseFileSystemEvent();
    }
    if (ebpfDataReader_->GetPagedMemoryMap().size()) {
        ParsePagedMemoryEvent();
    }
    if (ebpfDataReader_->GetBIOSampleMap().size()) {
        ParseBioLatencyEvent();
    }
}

void EbpfDataParser::Finish()
{
    ebpfAllEventStartTime_ = std::min(FileSystemDataParser::timeParser_->GetPluginStartTime(), ebpfAllEventStartTime_);
    ebpfAllEventStartTime_ = std::min(PagedMemoryDataParser::timeParser_->GetPluginStartTime(), ebpfAllEventStartTime_);
    ebpfAllEventStartTime_ = std::min(BioLatencyDataParser::timeParser_->GetPluginStartTime(), ebpfAllEventStartTime_);
    ebpfAllEventEndTime_ = std::max(FileSystemDataParser::timeParser_->GetPluginEndTime(), ebpfAllEventEndTime_);
    ebpfAllEventEndTime_ = std::max(PagedMemoryDataParser::timeParser_->GetPluginEndTime(), ebpfAllEventEndTime_);
    ebpfAllEventEndTime_ = std::max(BioLatencyDataParser::timeParser_->GetPluginEndTime(), ebpfAllEventEndTime_);
    // Update trace_range when there is only ebpf data in the trace file
    if (traceDataCache_->traceStartTime_ == INVALID_UINT64 || traceDataCache_->traceEndTime_ == 0) {
        traceDataCache_->MixTraceTime(ebpfAllEventStartTime_, ebpfAllEventEndTime_);
    } else {
        TS_LOGI("EBPF data time is not updated, maybe this trace file has other data");
    }
}
void EbpfDataParser::SetEbpfDataOffset(uint64_t offset)
{
    ebpfSplitter.SetEbpfDataOffset(offset);
}
void EbpfDataParser::SetSpliteTimeRange(uint64_t splitFileMinTs, uint64_t splitFileMaxTs)
{
    ebpfSplitter.SetSpliteTimeRange(splitFileMinTs, splitFileMaxTs);
}
bool EbpfDataParser::AddAndSplitEbpfData(const std::deque<uint8_t>& dequeBuffer)
{
    return ebpfSplitter.AddAndSplitEbpfData(dequeBuffer);
}
} // namespace TraceStreamer
} // namespace SysTuning
