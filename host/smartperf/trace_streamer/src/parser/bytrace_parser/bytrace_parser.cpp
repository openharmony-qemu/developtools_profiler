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

#include "bytrace_parser.h"
#include <cmath>
#include <sstream>
#include <unistd.h>
#include "app_start_filter.h"
#include "binder_filter.h"
#include "cpu_filter.h"
#include "hi_sysevent_measure_filter.h"
#include "parting_string.h"
#include "stat_filter.h"
#include "system_event_measure_filter.h"
namespace SysTuning {
namespace TraceStreamer {
BytraceParser::BytraceParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters)
    : ParserBase(filters),
      eventParser_(std::make_unique<BytraceEventParser>(dataCache, filters)),
#ifdef SUPPORTTHREAD
      dataSegArray_(std::make_unique<DataSegment[]>(MAX_SEG_ARRAY_SIZE)),
      supportThread_(true)
#else
      dataSegArray_(std::make_unique<DataSegment[]>(1))
#endif
{
}

BytraceParser::~BytraceParser() = default;

void BytraceParser::WaitForParserEnd()
{
    if (parseThreadStarted_ || filterThreadStarted_) {
        toExit_ = true;
        while (!exited_) {
            usleep(sleepDur_ * sleepDur_);
        }
    }
    eventParser_->FilterAllEvents();
    eventParser_->Clear();
    dataSegArray_.reset();
}
void BytraceParser::ParseTraceDataSegment(std::unique_ptr<uint8_t[]> bufferStr, size_t size)
{
    if (isParsingOver_) {
        return;
    }
    packagesBuffer_.insert(packagesBuffer_.end(), &bufferStr[0], &bufferStr[size]);
    auto packagesBegin = packagesBuffer_.begin();
    while (1) {
        auto packagesLine = std::find(packagesBegin, packagesBuffer_.end(), '\n');
        if (packagesLine == packagesBuffer_.end()) {
            break;
        }
        if (packagesLine == packagesBuffer_.begin()) {
            packagesLine++;
            packagesBegin = packagesLine;
            continue;
        }
        // Support parsing windows file format(ff=dos)
        auto extra = *(packagesLine - 1) == '\r' ? 1 : 0;
        std::string bufferLine(packagesBegin, packagesLine - extra);

        if (isFirstLine) {
            isFirstLine = false;
            if (IsHtmlTrace(bufferLine)) {
                isHtmlTrace_ = true;
                goto NEXT_LINE;
            }
        }
        if (isHtmlTrace_) {
            if (!isHtmlTraceContent_) {
                if (IsHtmlTraceBegin(bufferLine)) {
                    isHtmlTraceContent_ = true;
                }
                goto NEXT_LINE;
            }
            auto pos = bufferLine.find(script_.c_str());
            if (pos != std::string::npos) {
                isHtmlTraceContent_ = false;
                bufferLine = bufferLine.substr(0, pos);
                if (std::all_of(bufferLine.begin(), bufferLine.end(), isspace)) {
                    goto NEXT_LINE;
                }
            }
        }

        if (IsTraceComment(bufferLine)) {
            traceCommentLines_++;
            goto NEXT_LINE;
        }
        if (bufferLine.empty()) {
            parsedTraceInvalidLines_++;
            goto NEXT_LINE;
        }

        if (isBytrace_) {
            if (!traceBegan_) {
                traceBegan_ = true;
            }
            ParseTraceDataItem(bufferLine);
        } else {
            ParseJsonData(bufferLine);
        }

    NEXT_LINE:
        packagesBegin = packagesLine + 1;
        continue;
    }

    if (isParsingOver_) {
        packagesBuffer_.clear();
    } else {
        packagesBuffer_.erase(packagesBuffer_.begin(), packagesBegin);
    }
    return;
}

inline void BytraceParser::AppendJsonDataToHiSysEventNewValue(JsonData jData,
                                                              int32_t jIndex,
                                                              DataIndex eventSourceIndex)
{
    auto value = jData.value[jIndex];
    std::string key = jData.key[jIndex];
    streamFilters_->hiSysEventMeasureFilter_->GetOrCreateFilterId(eventSourceIndex);
    DataIndex keyIndex = eventParser_->traceDataCache_->GetDataIndex(key);
    if (value.is_string()) {
        std::string strValue = value;
        DataIndex valueIndex = eventParser_->traceDataCache_->GetDataIndex(strValue);
        streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(0, jData.timeStamp, eventSourceIndex, keyIndex, 1, 0,
                                                                 valueIndex);
    } else {
        DataIndex valueIndex = value;
        streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(0, jData.timeStamp, eventSourceIndex, keyIndex, 0,
                                                                 valueIndex, 0);
    }
}

inline void BytraceParser::NoArrayDataParse(JsonData jData,
                                            std::vector<size_t> noArrayIndex,
                                            DataIndex eventSourceIndex)
{
    for (auto itor = noArrayIndex.begin(); itor != noArrayIndex.end(); itor++) {
        AppendJsonDataToHiSysEventNewValue(jData, *itor, eventSourceIndex);
    }
}
void BytraceParser::ArrayDataParse(JsonData jData,
                                   std::vector<size_t> arrayIndex,
                                   DataIndex eventSourceIndex,
                                   size_t maxArraySize)
{
    for (int32_t j = 0; j < maxArraySize; j++) {
        for (auto itor = arrayIndex.begin(); itor != arrayIndex.end(); itor++) {
            auto value = jData.value[*itor][j];
            std::string key = jData.key[*itor];
            DataIndex keyIndex = eventParser_->traceDataCache_->GetDataIndex(key);
            streamFilters_->hiSysEventMeasureFilter_->GetOrCreateFilterId(eventSourceIndex);
            if (value.is_number()) {
                DataIndex valueIndex = value;
                streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(0, jData.timeStamp, eventSourceIndex, keyIndex,
                                                                         0, valueIndex, 0);
            } else if (value.is_string()) {
                std::string strValue = value;
                DataIndex valueIndex = eventParser_->traceDataCache_->GetDataIndex(strValue);
                streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(0, jData.timeStamp, eventSourceIndex, keyIndex,
                                                                         1, 0, valueIndex);
            }
        }
    }
}

inline void BytraceParser::CommonDataParser(JsonData jData, DataIndex eventSourceIndex)
{
    for (int32_t j = 0; j < jData.key.size(); j++) {
        AppendJsonDataToHiSysEventNewValue(jData, j, eventSourceIndex);
    }
}

void BytraceParser::ParseJsonData(const std::string& buffer)
{
    std::stringstream ss;
    json jMessage;
    ss << buffer;
    ss >> jMessage;
    JsonData jData;
    size_t maxArraySize = 0;
    std::vector<size_t> noArrayIndex = {};
    std::vector<size_t> arrayIndex = {};
    if (!streamFilters_->hiSysEventMeasureFilter_->JGetData(jMessage, jData, maxArraySize, noArrayIndex, arrayIndex)) {
        return;
    }
    DataIndex eventSourceIndex = eventParser_->traceDataCache_->GetDataIndex(jData.eventSource);
    if (maxArraySize) {
        NoArrayDataParse(jData, noArrayIndex, eventSourceIndex);
        ArrayDataParse(jData, arrayIndex, eventSourceIndex, maxArraySize);
    } else {
        CommonDataParser(jData, eventSourceIndex);
    }
    return;
}
void BytraceParser::ParseTraceDataItem(const std::string& buffer)
{
    if (!supportThread_) {
        dataSegArray_[rawDataHead_].seg = std::move(buffer);
        ParserData(dataSegArray_[rawDataHead_]);
        return;
    }
    int32_t head = rawDataHead_;
    while (!toExit_) {
        if (dataSegArray_[head].status.load() != TS_PARSE_STATUS_INIT) {
            TS_LOGD("rawDataHead_:\t%d, parseHead_:\t%d, filterHead_:\t%d status:\t%d\n", rawDataHead_, parseHead_,
                    filterHead_, dataSegArray_[head].status.load());
            usleep(sleepDur_);
            continue;
        }
        dataSegArray_[head].seg = std::move(buffer);
        dataSegArray_[head].status = TS_PARSE_STATUS_SEPRATED;
        rawDataHead_ = (rawDataHead_ + 1) % MAX_SEG_ARRAY_SIZE;
        break;
    }
    if (!parseThreadStarted_) {
        parseThreadStarted_ = true;
        int32_t tmp = maxThread_;
        while (tmp--) {
            parserThreadCount_++;
            std::thread MatchLineThread(&BytraceParser::ParseThread, this);
            MatchLineThread.detach();
            TS_LOGI("parser Thread:%d/%d start working ...\n", maxThread_ - tmp, maxThread_);
        }
    }
    return;
}
int32_t BytraceParser::GetNextSegment()
{
    int32_t head;
    dataSegMux_.lock();
    head = parseHead_;
    DataSegment& seg = dataSegArray_[head];
    if (seg.status.load() != TS_PARSE_STATUS_SEPRATED) {
        if (toExit_) {
            parserThreadCount_--;
            TS_LOGI("exiting parser, parserThread Count:%d\n", parserThreadCount_);
            dataSegMux_.unlock();
            if (!parserThreadCount_ && !filterThreadStarted_) {
                exited_ = true;
            }
            return ERROR_CODE_EXIT;
        }
        if (seg.status.load() == TS_PARSE_STATUS_PARSING) {
            dataSegMux_.unlock();
            usleep(sleepDur_);
            return ERROR_CODE_NODATA;
        }
        dataSegMux_.unlock();
        TS_LOGD("ParseThread watting:\t%d, parseHead_:\t%d, filterHead_:\t%d status:\t%d\n", rawDataHead_, parseHead_,
                filterHead_, seg.status.load());
        usleep(sleepDur_);
        return ERROR_CODE_NODATA;
    }
    parseHead_ = (parseHead_ + 1) % MAX_SEG_ARRAY_SIZE;
    seg.status = TS_PARSE_STATUS_PARSING;
    dataSegMux_.unlock();
    return head;
}

void BytraceParser::GetDataSegAttr(DataSegment& seg, const std::smatch& matcheLine) const
{
    const uint64_t S_TO_NS = 1e9;
    size_t index = 0;
    std::string pidStr = matcheLine[++index].str();
    std::optional<uint32_t> optionalPid = base::StrToInt<uint32_t>(pidStr);
    if (!optionalPid.has_value()) {
        TS_LOGD("Illegal pid: %s", pidStr.c_str());
        seg.status = TS_PARSE_STATUS_INVALID;
        return;
    }

    std::string tGidStr = matcheLine[++index].str();
    std::string cpuStr = matcheLine[++index].str();
    std::optional<uint32_t> optionalCpu = base::StrToInt<uint32_t>(cpuStr);
    if (!optionalCpu.has_value()) {
        TS_LOGD("Illegal cpu %s", cpuStr.c_str());
        seg.status = TS_PARSE_STATUS_INVALID;
        return;
    }
    std::string timeStr = matcheLine[++index].str();
    // Directly parsing double may result in accuracy loss issues
    std::optional<double> optionalTime = base::StrToDouble(timeStr);
    if (!optionalTime.has_value()) {
        TS_LOGD("Illegal ts %s", timeStr.c_str());
        seg.status = TS_PARSE_STATUS_INVALID;
        return;
    }
    std::string eventName = matcheLine[++index].str();
    seg.bufLine.task = StrTrim(matcheLine.prefix());
    if (seg.bufLine.task == "<...>") {
        seg.bufLine.task = "";
    }
    seg.bufLine.argsStr = StrTrim(matcheLine.suffix());
    seg.bufLine.pid = optionalPid.value();
    seg.bufLine.cpu = optionalCpu.value();
    seg.bufLine.ts = optionalTime.value() * S_TO_NS;
    seg.bufLine.tGidStr = tGidStr;
    seg.bufLine.eventName = eventName;
    seg.status = TS_PARSE_STATUS_PARSED;
}
void BytraceParser::ParseThread()
{
    while (1) {
        int32_t head = GetNextSegment();
        if (head < 0) {
            if (head == ERROR_CODE_NODATA) {
                continue;
            }
            if (!filterThreadStarted_) {
                exited_ = true;
            }
            return;
        }
        DataSegment& seg = dataSegArray_[head];
        ParserData(seg);
    }
}

void BytraceParser::ParserData(DataSegment& seg)
{
    std::smatch matcheLine;
    if (!std::regex_search(seg.seg, matcheLine, bytraceMatcher_)) {
        TS_LOGD("Not support this event (line: %s)", seg.seg.c_str());
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_DATA_INVALID);
        seg.status = TS_PARSE_STATUS_INVALID;
        parsedTraceInvalidLines_++;
        return;
    } else {
        parsedTraceValidLines_++;
    }
    GetDataSegAttr(seg, matcheLine);
    if (!supportThread_) {
        FilterData(seg);
        return;
    }
    if (!filterThreadStarted_) {
        filterThreadStarted_ = true;
        std::thread ParserThread(&BytraceParser::FilterThread, this);
        ParserThread.detach();
    }
}
void BytraceParser::FilterThread()
{
    while (1) {
        DataSegment& seg = dataSegArray_[filterHead_];
        if (!FilterData(seg)) {
            return;
        }
    }
}
bool BytraceParser::FilterData(DataSegment& seg)
{
    if (!supportThread_) {
        if (seg.status.load() != TS_PARSE_STATUS_INVALID) {
            eventParser_->ParseDataItem(seg.bufLine);
            seg.status = TS_PARSE_STATUS_INIT;
            return true;
        }
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_DATA_INVALID);
        return false;
    }
    if (seg.status.load() == TS_PARSE_STATUS_INVALID) {
        filterHead_ = (filterHead_ + 1) % MAX_SEG_ARRAY_SIZE;
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_OTHER, STAT_EVENT_DATA_INVALID);
        seg.status = TS_PARSE_STATUS_INIT;
        return true;
    }
    if (seg.status.load() != TS_PARSE_STATUS_PARSED) {
        if (toExit_ && !parserThreadCount_) {
            TS_LOGI("exiting FilterThread Thread\n");
            exited_ = true;
            filterThreadStarted_ = false;
            return false;
        }
        usleep(sleepDur_);
        return true;
    }
    eventParser_->ParseDataItem(seg.bufLine);
    filterHead_ = (filterHead_ + 1) % MAX_SEG_ARRAY_SIZE;
    seg.status = TS_PARSE_STATUS_INIT;
    return true;
}
// Remove space at the beginning and end of the string
std::string BytraceParser::StrTrim(const std::string& input) const
{
    std::string str = input;
    if (str.empty()) {
        return str;
    }
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
}
} // namespace TraceStreamer
} // namespace SysTuning
