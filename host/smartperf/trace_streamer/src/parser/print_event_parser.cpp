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
#include "print_event_parser.h"
#include "animation_filter.h"
#include "clock_filter_ex.h"
#include "frame_filter.h"
#include "stat_filter.h"
#include "string_to_numerical.h"
#include <cinttypes>
namespace SysTuning {
namespace TraceStreamer {
PrintEventParser::PrintEventParser(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : EventParserBase(dataCache, filter), pointLength_(1), maxPointLength_(2)
{
    eventToFrameFunctionMap_ = {
        {recvievVsync_, bind(&PrintEventParser::ReciveVsync, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3)},
        {rsOnVsyncEvent_, bind(&PrintEventParser::RSReciveOnVsync, this, std::placeholders::_1, std::placeholders::_2,
                               std::placeholders::_3)},
        {marshRwTransactionData_, bind(&PrintEventParser::OnRwTransaction, this, std::placeholders::_1,
                                       std::placeholders::_2, std::placeholders::_3)},
        {rsMainThreadProcessCmd_, bind(&PrintEventParser::OnMainThreadProcessCmd, this, std::placeholders::_1,
                                       std::placeholders::_2, std::placeholders::_3)}};
}

bool PrintEventParser::ParsePrintEvent(const std::string& comm,
                                       uint64_t ts,
                                       uint32_t pid,
                                       std::string_view event,
                                       const BytraceLine& line)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TRACING_MARK_WRITE, STAT_EVENT_RECEIVED);
    TracePoint point;
    if (GetTracePoint(event, point) != PARSE_SUCCESS) {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TRACING_MARK_WRITE, STAT_EVENT_DATA_INVALID);
        return false;
    }
    if (point.tgid_) {
        // tgid use 'B|' after with 'TGID', the '(TGID)' maybe wrong, eg: xxx-21675 ( 1264) ...: print: B|21675|...
        const_cast<BytraceLine&>(line).tgid = point.tgid_;
        streamFilters_->processFilter_->GetOrCreateInternalPid(ts, point.tgid_);
    }
    switch (point.phase_) {
        case 'B': {
            ParseBeginEvent(comm, ts, pid, point, line);
            break;
        }
        case 'E': {
            ParseEndEvent(ts, pid, point);
            break;
        }
        case 'S': {
            ParseStartEvent(comm, ts, pid, point, line);
            break;
        }
        case 'F': {
            ParseFinishEvent(ts, pid, point, line);
            break;
        }
        case 'C': {
            ParseCreateEvent(ts, point);
            break;
        }
        default:
            TS_LOGD("point missing!");
            return false;
    }
    return true;
}
void PrintEventParser::ParseBeginEvent(const std::string& comm,
                                       uint64_t ts,
                                       uint32_t pid,
                                       TracePoint& point,
                                       const BytraceLine& line)
{
    uint32_t index = streamFilters_->sliceFilter_->BeginSlice(comm, ts, pid, point.tgid_, INVALID_DATAINDEX,
                                                              traceDataCache_->GetDataIndex(point.name_));
    if (index != INVALID_UINT32) {
        // add distributed data
        traceDataCache_->GetInternalSlicesData()->SetDistributeInfo(index, point.chainId_, point.spanId_,
                                                                    point.parentSpanId_, point.flag_, point.args_);
        if (pid == point.tgid_) {
            if (HandleFrameSliceBeginEvent(point.funcPrefixId_, index, point.funcArgs_, line)) {
                return;
            }
        }
        bool isDiscontinued = false;
        if (traceDataCache_->TaskPoolTraceEnabled()) {
            isDiscontinued = streamFilters_->taskPoolFilter_->TaskPoolEvent(point.name_, index);
        }
        if (traceDataCache_->AnimationTraceEnabled() && !isDiscontinued) {
            isDiscontinued = HandleAnimationBeginEvent(point, index, line);
        }
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TRACING_MARK_WRITE, STAT_EVENT_DATA_LOST);
    }
}
void PrintEventParser::ParseEndEvent(uint64_t ts, uint32_t pid, const TracePoint& point)
{
    uint32_t index = streamFilters_->sliceFilter_->EndSlice(ts, pid, point.tgid_);
    if (pid == point.tgid_) {
        HandleFrameSliceEndEvent(ts, point.tgid_, pid, index);
    }
}
void PrintEventParser::ParseStartEvent(const std::string& comm,
                                       uint64_t ts,
                                       uint32_t pid,
                                       const TracePoint& point,
                                       const BytraceLine& line)
{
    auto cookie = static_cast<uint64_t>(point.value_);
    auto index = streamFilters_->sliceFilter_->StartAsyncSlice(ts, pid, point.tgid_, cookie,
                                                               traceDataCache_->GetDataIndex(point.name_));
    if (point.name_ == onFrameQueeuStartEvent_ && index != INVALID_UINT64) {
        OnFrameQueueStart(ts, index, point.tgid_);
    } else if (traceDataCache_->AnimationTraceEnabled() && index != INVALID_UINT64 &&
               EndWith(comm, onLauncherVsyncEvent_) && // the comm is taskName
               onAnimationStartEvent_ == traceDataCache_->GetDataIndex(point.name_)) {
        streamFilters_->animationFilter_->StartAnimationEvent(line, index);
    }
}
void PrintEventParser::ParseFinishEvent(uint64_t ts, uint32_t pid, const TracePoint& point, const BytraceLine& line)
{
    auto cookie = static_cast<uint64_t>(point.value_);
    auto index = streamFilters_->sliceFilter_->FinishAsyncSlice(ts, pid, point.tgid_, cookie,
                                                                traceDataCache_->GetDataIndex(point.name_));
    HandleFrameQueueEndEvent(ts, point.tgid_, point.tgid_, index);
    if (traceDataCache_->AnimationTraceEnabled()) {
        streamFilters_->animationFilter_->FinishAnimationEvent(line, index);
    }
}
void PrintEventParser::ParseCreateEvent(uint64_t ts, const TracePoint& point)
{
    DataIndex nameIndex = traceDataCache_->GetDataIndex(point.name_);
    uint32_t internalPid = streamFilters_->processFilter_->GetInternalPid(point.tgid_);
    if (internalPid != INVALID_ID) {
        streamFilters_->processMeasureFilter_->AppendNewMeasureData(internalPid, nameIndex, ts, point.value_);
        streamFilters_->processFilter_->AddProcessMemory(internalPid);
    } else {
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_TRACING_MARK_WRITE, STAT_EVENT_DATA_INVALID);
    }
}
bool PrintEventParser::HandleAnimationBeginEvent(const TracePoint& point, size_t callStackRow, const BytraceLine& line)
{
    if (!streamFilters_->animationFilter_->UpdateDeviceInfoEvent(point, line)) {
        return streamFilters_->animationFilter_->BeginDynamicFrameEvent(point, callStackRow);
    }
    return true;
}
void PrintEventParser::SetTraceType(TraceFileType traceType)
{
    traceType_ = traceType;
}
void PrintEventParser::SetTraceClockId(BuiltinClocks clock)
{
    if (clock != clock_) {
        clock_ = clock;
    }
}
void PrintEventParser::Finish()
{
    eventToFrameFunctionMap_.clear();
    frameCallIds_.clear();
    vsyncSliceIds_.clear();
    streamFilters_->animationFilter_->Clear();
    streamFilters_->frameFilter_->Finish();
}
ParseResult PrintEventParser::CheckTracePoint(std::string_view pointStr) const
{
    if (pointStr.size() == 0) {
        TS_LOGD("get trace point data size is 0!");
        return PARSE_ERROR;
    }

    std::string clockSyncSts = "trace_event_clock_sync";
    if (pointStr.compare(0, clockSyncSts.length(), clockSyncSts.c_str()) == 0) {
        TS_LOGD("skip trace point :%s!", clockSyncSts.c_str());
        return PARSE_ERROR;
    }

    if (pointStr.find_first_of('B') != 0 && pointStr.find_first_of('E') != 0 && pointStr.find_first_of('C') != 0 &&
        pointStr.find_first_of('S') != 0 && pointStr.find_first_of('F') != 0) {
        TS_LOGD("trace point not supported : [%c] !", pointStr[0]);
        return PARSE_ERROR;
    }

    if (pointStr.find_first_of('E') != 0 && pointStr.size() == 1) {
        TS_LOGD("point string size error!");
        return PARSE_ERROR;
    }

    if (pointStr.size() >= maxPointLength_) {
        if ((pointStr[1] != '|') && (pointStr[1] != '\n')) {
            TS_LOGD("not support data formart!");
            return PARSE_ERROR;
        }
    }

    return PARSE_SUCCESS;
}

std::string_view PrintEventParser::GetPointNameForBegin(std::string_view pointStr, size_t tGidlength) const
{
    size_t index = maxPointLength_ + tGidlength + pointLength_;

    size_t length = pointStr.size() - index - ((pointStr.back() == '\n') ? 1 : 0);
    std::string_view name = std::string_view(pointStr.data() + index, length);
    // remove space at the end
    name = std::string_view(name.data(), name.find_last_not_of(" ") + 1);
    return name;
}

ParseResult PrintEventParser::HandlerB(std::string_view pointStr, TracePoint& outPoint, size_t tGidlength) const
{
    outPoint.name_ = GetPointNameForBegin(pointStr, tGidlength);
    if (outPoint.name_.empty()) {
        TS_LOGD("point name is empty!");
        return PARSE_ERROR;
    }
    // Use $# to differentiate distributed data
    if (outPoint.name_.find("$#") == std::string::npos) {
        auto space = outPoint.name_.find(' ');
        if (space != std::string::npos) {
            outPoint.funcPrefix_ = outPoint.name_.substr(0, space);
            outPoint.funcPrefixId_ = traceDataCache_->GetDataIndex(outPoint.funcPrefix_);
            outPoint.funcArgs_ = outPoint.name_.substr(space + 1);
        } else {
            outPoint.funcPrefixId_ = traceDataCache_->GetDataIndex(outPoint.name_);
        }
        return PARSE_SUCCESS;
    }
    // Resolve distributed calls
    // the normal data mybe like:
    // system-1298 ( 1298) [001] ...1 174330.287420: tracing_mark_write: B|1298|[8b00e96b2,2,1]:C$#decodeFrame$#"
    //    "{\"Process\":\"DecodeVideoFrame\",\"frameTimestamp\":37313484466} \
    //        system - 1298(1298)[001]... 1 174330.287622 : tracing_mark_write : E | 1298 \n
    const std::regex distributeMatcher = std::regex(R"((?:^\[([a-z0-9]+),(\d+),(\d+)\]:?([CS]?)\$#)?(.*)\$#(.*)$)");
    std::smatch matcheLine;
    bool matched = std::regex_match(outPoint.name_, matcheLine, distributeMatcher);
    if (matched) {
        size_t index = 0;
        outPoint.chainId_ = matcheLine[++index].str();
        outPoint.spanId_ = matcheLine[++index].str();
        outPoint.parentSpanId_ = matcheLine[++index].str();
        outPoint.flag_ = matcheLine[++index].str();
        outPoint.name_ = matcheLine[++index].str();
        outPoint.args_ = matcheLine[++index].str();
    }
    return PARSE_SUCCESS;
}

bool PrintEventParser::HandleFrameSliceBeginEvent(DataIndex eventName,
                                                  size_t callStackRow,
                                                  std::string& args,
                                                  const BytraceLine& line)
{
    auto it = eventToFrameFunctionMap_.find(eventName);
    if (it != eventToFrameFunctionMap_.end()) {
        it->second(callStackRow, args, line);
        return true;
    }
    return false;
}
bool PrintEventParser::ReciveVsync(size_t callStackRow, std::string& args, const BytraceLine& line)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_VSYNC, STAT_EVENT_RECEIVED);
    // args is like "dataCount:24bytes now:211306766162 expectedEnd:211323423844 vsyncId:3179"
    TS_LOGD("ts:%lu tid:%d, %s callStackRow:%lu", line.ts, line.pid, args.c_str(), callStackRow);
    std::sregex_iterator it(args.begin(), args.end(), recvVsyncPattern_);
    std::sregex_iterator end;
    uint64_t now = INVALID_UINT64;
    uint64_t expectEnd = INVALID_UINT64;
    uint32_t vsyncId = INVALID_UINT32;
    while (it != end) {
        std::smatch match = *it;
        std::string key = match.str(1);
        std::string value = match.str(2);
        if (key == "now") {
            now = base::StrToInt<uint64_t>(value).value();
        } else if (key == "expectedEnd") {
            expectEnd = base::StrToInt<uint64_t>(value).value();
        } else if (key == "vsyncId") {
            vsyncId = base::StrToInt<uint64_t>(value).value();
        }
        ++it;
    }
    if (convertVsyncTs_ && traceType_ == TRACE_FILETYPE_H_TRACE) {
        if (now != INVALID_UINT64) {
            now = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_MONOTONIC, now);
        }
        if (expectEnd != INVALID_UINT64) {
            expectEnd = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_MONOTONIC, expectEnd);
        }
    }
    auto iTid = streamFilters_->processFilter_->GetInternalTid(line.pid);
    auto iPid = streamFilters_->processFilter_->GetInternalPid(line.tgid);
    streamFilters_->frameFilter_->BeginVsyncEvent(line.ts, iPid, iTid, now, expectEnd, vsyncId, callStackRow);
    vsyncSliceIds_.push_back(callStackRow);
    return true;
}
bool PrintEventParser::RSReciveOnVsync(size_t callStackRow, std::string& args, const BytraceLine& line)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_ONVSYNC, STAT_EVENT_RECEIVED);
    TS_LOGD("ts:%lu tid:%d, %s callStackRow:%lu", line.ts, line.pid, args.c_str(), callStackRow);
    auto iTid = streamFilters_->processFilter_->GetInternalTid(line.pid);
    (void)streamFilters_->frameFilter_->MarkRSOnvsyncEvent(line.ts, iTid);
    return true;
}
bool PrintEventParser::OnRwTransaction(size_t callStackRow, std::string& args, const BytraceLine& line)
{
    // H:MarshRSTransactionData cmdCount:20 transactionFlag:[3799,8] isUni:1
    TS_LOGD("ts:%lu tid:%d, %s callStackRow:%lu", line.ts, line.pid, args.c_str(), callStackRow);
    std::smatch match;
    if (std::regex_search(args, match, transFlagPattern_)) {
        std::string flag2 = match.str(2);
        auto iTid = streamFilters_->processFilter_->GetInternalTid(line.pid);
        return streamFilters_->frameFilter_->BeginRSTransactionData(line.ts, iTid,
                                                                    base::StrToInt<uint32_t>(flag2).value());
    }
    return true;
}
bool PrintEventParser::OnMainThreadProcessCmd(size_t callStackRow, std::string& args, const BytraceLine& line)
{
    TS_LOGD("ts:%lu tid:%d, %s callStackRow:%lu", line.ts, line.pid, args.c_str(), callStackRow);
    std::sregex_iterator it(args.begin(), args.end(), mainProcessCmdPattern);
    std::sregex_iterator end;
    std::vector<FrameFilter::FrameMap> frames;
    while (it != end) {
        std::smatch match = *it;
        std::string value1 = match.str(1);
        std::string value2 = match.str(2);
        frames.push_back({streamFilters_->processFilter_->GetInternalTid(base::StrToInt<uint32_t>(value1).value()),
                          base::StrToInt<uint32_t>(value2).value()});
        ++it;
    }
    auto iTid = streamFilters_->processFilter_->GetInternalTid(line.pid);
    return streamFilters_->frameFilter_->BeginProcessCommandUni(line.ts, iTid, frames, callStackRow);
}
bool PrintEventParser::OnFrameQueueStart(uint64_t ts, size_t callStackRow, uint64_t pid)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_FRAMEQUEUE, STAT_EVENT_RECEIVED);
    TS_LOGD("ts:%llu tid:%llu, callStackRow:%zu", ts, pid, callStackRow);
    auto iTid = streamFilters_->processFilter_->GetInternalTid(pid);
    if (streamFilters_->frameFilter_->StartFrameQueue(ts, iTid)) {
        frameCallIds_.push_back(callStackRow);
    }
    return true;
}
void PrintEventParser::HandleFrameSliceEndEvent(uint64_t ts, uint64_t pid, uint64_t tid, size_t callStackRow)
{
    // it can be frame or slice
    auto iTid = streamFilters_->processFilter_->GetInternalTid(tid);
    auto pos = std::find(vsyncSliceIds_.begin(), vsyncSliceIds_.end(), callStackRow);
    if (pos != vsyncSliceIds_.end()) {
        TS_LOGD("ts:%" PRIu64 ", RenderSliceEnd:%" PRIu64 ", callStackRow:%zu", ts, tid, callStackRow);
        if (!streamFilters_->frameFilter_->EndVsyncEvent(ts, iTid)) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_VSYNC, STAT_EVENT_NOTMATCH);
            TS_LOGW("ts:%" PRIu64 ", RenderSliceEnd:%" PRIu64 ", callStackRow:%zu failed", ts, tid, callStackRow);
        }
        vsyncSliceIds_.erase(pos);
    }
    return;
}

void PrintEventParser::HandleFrameQueueEndEvent(uint64_t ts, uint64_t pid, uint64_t tid, size_t callStackRow)
{
    // it can be frame or slice
    auto iTid = streamFilters_->processFilter_->GetInternalTid(tid);
    auto pos = std::find(frameCallIds_.begin(), frameCallIds_.end(), callStackRow);
    if (pos != frameCallIds_.end()) {
        TS_LOGD("ts:%" PRIu64 ", frameSliceEnd:%" PRIu64 "", ts, tid);
        if (!streamFilters_->frameFilter_->EndFrameQueue(ts, iTid)) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_FRAMEQUEUE, STAT_EVENT_NOTMATCH);
            TS_LOGW("ts:%" PRIu64 ", frameSliceEnd:%" PRIu64 " failed", ts, tid);
        }
        frameCallIds_.erase(pos);
    }
    return;
}
ParseResult PrintEventParser::HandlerE(void)
{
    return PARSE_SUCCESS;
}

size_t PrintEventParser::GetNameLength(std::string_view pointStr, size_t nameIndex)
{
    size_t namelength = 0;
    for (size_t i = nameIndex; i < pointStr.size(); i++) {
        if (pointStr[i] == ' ') {
            namelength = i - nameIndex;
        }
        if (pointStr[i] == '|') {
            namelength = i - nameIndex;
            break;
        }
    }
    return namelength;
}

size_t PrintEventParser::GetValueLength(std::string_view pointStr, size_t valueIndex) const
{
    size_t valuePipe = pointStr.find('|', valueIndex);
    size_t valueLen = pointStr.size() - valueIndex;
    if (valuePipe != std::string_view::npos) {
        valueLen = valuePipe - valueIndex;
    }

    if (valueLen == 0) {
        return 0;
    }

    if (pointStr[valueIndex + valueLen - pointLength_] == '\n') {
        valueLen--;
    }

    return valueLen;
}

ParseResult PrintEventParser::HandlerCSF(std::string_view pointStr, TracePoint& outPoint, size_t tGidlength) const
{
    // point name
    size_t nameIndex = maxPointLength_ + tGidlength + pointLength_;
    size_t namelength = GetNameLength(pointStr, nameIndex);
    if (namelength == 0) {
        TS_LOGD("point name length is error!");
        return PARSE_ERROR;
    }
    outPoint.name_ = std::string_view(pointStr.data() + nameIndex, namelength);

    // point value
    size_t valueIndex = nameIndex + namelength + pointLength_;
    size_t valueLen = GetValueLength(pointStr, valueIndex);
    if (valueLen == 0) {
        TS_LOGD("point value length is error!");
        return PARSE_ERROR;
    }

    std::string valueStr(pointStr.data() + valueIndex, valueLen);
    if (!base::StrToInt<uint64_t>(valueStr).has_value()) {
        TS_LOGD("point value is error!");
        return PARSE_ERROR;
    }
    outPoint.value_ = base::StrToInt<uint64_t>(valueStr).value();

    size_t valuePipe = pointStr.find('|', valueIndex);
    if (valuePipe != std::string_view::npos) {
        size_t groupLen = pointStr.size() - valuePipe - pointLength_;
        if (groupLen == 0) {
            return PARSE_ERROR;
        }

        if (pointStr[pointStr.size() - pointLength_] == '\n') {
            groupLen--;
        }

        outPoint.categoryGroup_ = std::string_view(pointStr.data() + valuePipe + 1, groupLen);
    }

    return PARSE_SUCCESS;
}

ParseResult PrintEventParser::GetTracePoint(std::string_view pointStr, TracePoint& outPoint) const
{
    if (CheckTracePoint(pointStr) != PARSE_SUCCESS) {
        return PARSE_ERROR;
    }

    size_t tGidlength = 0;
    // we may get wrong format data like tracing_mark_write: E
    // while the format data must be E|call-tid
    // please use a regular-format to get all the data
    outPoint.phase_ = pointStr.front();
    outPoint.tgid_ = GetThreadGroupId(pointStr, tGidlength);

    ParseResult ret = PARSE_ERROR;
    switch (outPoint.phase_) {
        case 'B': {
            ret = HandlerB(pointStr, outPoint, tGidlength);
            break;
        }
        case 'E': {
            ret = HandlerE();
            break;
        }
        case 'S':
        case 'F':
        case 'C': {
            ret = HandlerCSF(pointStr, outPoint, tGidlength);
            break;
        }
        default:
            return PARSE_ERROR;
    }
    return ret;
}

uint32_t PrintEventParser::GetThreadGroupId(std::string_view pointStr, size_t& length) const
{
    for (size_t i = maxPointLength_; i < pointStr.size(); i++) {
        if (pointStr[i] == '|' || pointStr[i] == '\n') {
            break;
        }

        if (pointStr[i] < '0' || pointStr[i] > '9') {
            return PARSE_ERROR;
        }

        length++;
    }

    std::string str(pointStr.data() + maxPointLength_, length);
    return base::StrToInt<uint32_t>(str).value_or(0);
}
} // namespace TraceStreamer
} // namespace SysTuning
