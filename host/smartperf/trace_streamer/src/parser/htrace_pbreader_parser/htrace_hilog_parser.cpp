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
#include "htrace_hilog_parser.h"
#include "clock_filter_ex.h"
#include "event_parser_base.h"
#include "hilog_plugin_result.pbreader.h"
#include "htrace_event_parser.h"
#include "process_filter.h"
#include "stat_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceHiLogParser::HtraceHiLogParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx)
{
}

HtraceHiLogParser::~HtraceHiLogParser()
{
    TS_LOGI("hilog ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(GetPluginStartTime()),
            static_cast<unsigned long long>(GetPluginEndTime()));
}
void HtraceHiLogParser::Parse(ProtoReader::BytesView tracePacket, bool& haveSplitSeg)
{
    ProtoReader::HilogInfo_Reader hilogInfo(tracePacket.data_, tracePacket.size_);
    if (!hilogInfo.has_info()) {
        return;
    }
    for (auto i = hilogInfo.info(); i; ++i) {
        ProtoReader::HilogLine_Reader hilogLine(i->ToBytes());
        uint64_t curLineSeq = hilogLine.id();
        streamFilters_->statFilter_->IncreaseStat(TRACE_HILOG, STAT_EVENT_RECEIVED);
        if (curLineSeq < lastLineSeq_ + 1) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_HILOG, STAT_EVENT_NOTMATCH);
        } else if (curLineSeq > lastLineSeq_ + 1) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_HILOG, STAT_EVENT_DATA_LOST);
        }
        lastLineSeq_ = curLineSeq;
        auto logData = traceDataCache_->GetDataIndex(hilogLine.context().ToStdString());
        ProtoReader::HilogDetails_Reader logDetails(hilogLine.detail());
        streamFilters_->processFilter_->GetOrCreateThreadWithPid(logDetails.tid(), logDetails.pid());
        auto iter = logLevelString_.find(logDetails.level());
        if (iter == logLevelString_.end()) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_HILOG, STAT_EVENT_DATA_INVALID);
            TS_LOGD("log level do not exit!!!");
            continue;
        }
        auto timeStamp = logDetails.tv_nsec() + logDetails.tv_sec() * SEC_TO_NS;
        auto newTimeStamp = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, timeStamp);
        UpdatePluginTimeRange(TS_CLOCK_REALTIME, timeStamp, newTimeStamp);
        if (traceDataCache_->isSplitFile_) {
            if (newTimeStamp >= traceDataCache_->SplitFileMinTime() &&
                newTimeStamp <= traceDataCache_->SplitFileMaxTime()) {
                haveSplitSeg = true;
                return;
            }
            continue;
        }
        DataIndex levelData = traceDataCache_->dataDict_.GetStringIndex(iter->second.c_str());
        DataIndex logTag = traceDataCache_->dataDict_.GetStringIndex(logDetails.tag().ToStdString());
        traceDataCache_->GetHilogData()->AppendNewLogInfo(curLineSeq, newTimeStamp, logDetails.pid(), logDetails.tid(),
                                                          levelData, logTag, logData, timeStamp);
    }
}
void HtraceHiLogParser::Finish()
{
    traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
}
} // namespace TraceStreamer
} // namespace SysTuning
