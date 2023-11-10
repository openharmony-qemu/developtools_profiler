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

#include "bytrace_hilog_parser.h"
#include "process_filter.h"
#include "stat_filter.h"

namespace SysTuning {
namespace TraceStreamer {
BytraceHilogParser::BytraceHilogParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters)
    : EventParserBase(dataCache, filters)
{
}

BytraceHilogParser::~BytraceHilogParser() = default;

bool BytraceHilogParser::HilogTimeStrToTimestamp(std::string& timeStr, uint64_t& timeStamp) const
{
    const uint64_t MS_TO_NS = 1e6;
    const uint64_t US_TO_NS = 1e3;
    const uint32_t TM_YEAR_FROM = 1900;
    const uint32_t MS_FORMAT_LEN = 3;
    const uint32_t US_FORMAT_LEN = 6;
    uint64_t sec;
    uint64_t nsec;
    std::string usecStr;
    std::smatch matcheLine;
    if (std::regex_search(timeStr, matcheLine, std::regex(R"(^\d+\.(\d+)$)"))) {
        size_t index = 0;
        usecStr = matcheLine[++index].str();
        sscanf_s(timeStr.c_str(), "%lu.%lu", &sec, &nsec);
    } else if (std::regex_search(timeStr, matcheLine,
                                 std::regex(R"(^(\d{4})?\-?(\d{2})-(\d{2}) (\d{2}):(\d{2}):(\d{2})\.(\d+)$)"))) {
        size_t index = 0;
        std::string yearStr = matcheLine[++index].str();
        std::string monthStr = matcheLine[++index].str();
        std::string dayStr = matcheLine[++index].str();
        std::string hourStr = matcheLine[++index].str();
        std::string minStr = matcheLine[++index].str();
        std::string secStr = matcheLine[++index].str();
        usecStr = matcheLine[++index].str();
        struct tm timeInfo = {0};
        std::optional<uint32_t> optionalYear = base::StrToInt<uint32_t>(yearStr);
        if (optionalYear.has_value()) {
            timeInfo.tm_year = optionalYear.value() - TM_YEAR_FROM;
        } else {
            time_t tmNow;
            tmNow = time(nullptr);
            tm* ptmNow = localtime(&tmNow);
            timeInfo.tm_year = ptmNow->tm_year;
        }
        timeInfo.tm_mon = base::StrToInt<uint32_t>(monthStr).value() - 1;
        timeInfo.tm_mday = base::StrToInt<uint32_t>(dayStr).value();
        timeInfo.tm_hour = base::StrToInt<uint32_t>(hourStr).value();
        timeInfo.tm_min = base::StrToInt<uint32_t>(minStr).value();
        timeInfo.tm_sec = base::StrToInt<uint32_t>(secStr).value();
        sec = std::mktime(&timeInfo);
        nsec = base::StrToInt<uint64_t>(usecStr).value();
    } else {
        return false;
    }

    if (usecStr.length() == MS_FORMAT_LEN) {
        nsec *= MS_TO_NS;
    } else if (usecStr.length() == US_FORMAT_LEN) {
        nsec *= US_TO_NS;
    }

    timeStamp = nsec + sec * SEC_TO_NS;
    return true;
}

void BytraceHilogParser::ParseHilogDataItem(const std::string& buffer, const uint64_t lineSeq)
{
    std::smatch matcheLine;
    if (!std::regex_search(buffer, matcheLine, hilogMatcher_)) {
        TS_LOGD("Not support this hilog format (line: %s)", buffer.c_str());
        return;
    }

    auto bufLine = std::make_unique<HilogLine>();
    bufLine->lineSeq = lineSeq;

    std::string timeStr = matcheLine[HILOG_MATCH_SEQ_TIME].str();
    HilogTimeStrToTimestamp(timeStr, bufLine->timeStamp);

    std::string pidStr = matcheLine[HILOG_MATCH_SEQ_PID].str();
    std::optional<uint32_t> optionalPid = base::StrToInt<uint32_t>(pidStr);
    if (!optionalPid.has_value()) {
        TS_LOGD("Illegal pid: %s", pidStr.c_str());
        return;
    }
    bufLine->pid = optionalPid.value();

    std::string tidStr = matcheLine[HILOG_MATCH_SEQ_TID].str();
    std::optional<uint32_t> optionalTid = base::StrToInt<uint32_t>(tidStr);
    if (!optionalTid.has_value()) {
        TS_LOGD("Illegal tid: %s", tidStr.c_str());
        return;
    }
    bufLine->tid = optionalTid.value();

    streamFilters_->processFilter_->GetOrCreateThreadWithPid(bufLine->tid, bufLine->pid);

    bufLine->level = matcheLine[HILOG_MATCH_SEQ_LEVEL].str();
    bufLine->tag = matcheLine[HILOG_MATCH_SEQ_TAG].str();
    bufLine->context = matcheLine[HILOG_MATCH_SEQ_CONTENT].str();

    FilterHilogData(std::move(bufLine));
    return;
}

void BytraceHilogParser::FilterHilogData(std::unique_ptr<HilogLine> bufLine)
{
    hilogList_.push_back(std::move(bufLine));
    return;
}

void BytraceHilogParser::FilterAllHilogData()
{
    auto cmp = [](const std::unique_ptr<HilogLine>& a, const std::unique_ptr<HilogLine>& b) {
        return a->timeStamp < b->timeStamp;
    };
#ifdef IS_WASM
    std::sort(hilogList_.begin(), hilogList_.end(), cmp);
#else
    std::stable_sort(hilogList_.begin(), hilogList_.end(), cmp);
#endif

    for (auto& item : hilogList_) {
        HilogLine* hilogData = item.get();
        BeginFilterHilogData(hilogData);
    }

    hilogList_.clear();
}

void BytraceHilogParser::BeginFilterHilogData(HilogLine* hilogData)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_HILOG, STAT_EVENT_RECEIVED);
    traceDataCache_->UpdateTraceTime(hilogData->timeStamp);
    auto curLineSeq = hilogData->lineSeq;
    auto newTimeStamp = hilogData->timeStamp;
    auto levelData = traceDataCache_->dataDict_.GetStringIndex(hilogData->level);
    auto logTag = traceDataCache_->dataDict_.GetStringIndex(hilogData->tag);
    auto logData = traceDataCache_->dataDict_.GetStringIndex(hilogData->context);
    traceDataCache_->GetHilogData()->AppendNewLogInfo(curLineSeq, newTimeStamp, hilogData->pid, hilogData->tid,
                                                      levelData, logTag, logData, hilogData->timeStamp);
    return;
}

} // namespace TraceStreamer
} // namespace SysTuning
