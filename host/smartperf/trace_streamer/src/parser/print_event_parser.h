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
#ifndef SRC_PRINT_EVENT_PARSER_H
#define SRC_PRINT_EVENT_PARSER_H
#include <regex>
#include <string_view>
#include "common_types.h"
#include "event_parser_base.h"
#include "filter/measure_filter.h"
#include "filter/process_filter.h"
#include "filter/slice_filter.h"
#include "filter/task_pool_filter.h"
#include "string_to_numerical.h"
#include "trace_streamer_config.h"
namespace SysTuning {
namespace TraceStreamer {
class PrintEventParser : private EventParserBase {
public:
    PrintEventParser(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    bool ParsePrintEvent(const std::string& comm,
                         uint64_t ts,
                         uint32_t pid,
                         std::string_view event,
                         const BytraceLine& line);
    void
        ParseBeginEvent(const std::string& comm, uint64_t ts, uint32_t pid, TracePoint& point, const BytraceLine& line);
    void ParseEndEvent(uint64_t ts, uint32_t pid, const TracePoint& point);
    void ParseStartEvent(const std::string& comm,
                         uint64_t ts,
                         uint32_t pid,
                         const TracePoint& point,
                         const BytraceLine& line);
    void ParseFinishEvent(uint64_t ts, uint32_t pid, const TracePoint& point, const BytraceLine& line);
    void ParseCreateEvent(uint64_t ts, const TracePoint& point);
    void Finish();
    void SetTraceType(TraceFileType traceType);
    void SetTraceClockId(BuiltinClocks clock);

private:
    using FrameFuncCall = std::function<bool(const size_t callStackRow, std::string& args, const BytraceLine& line)>;
    ParseResult GetTracePoint(std::string_view pointStr, TracePoint& outPoint) const;
    ParseResult CheckTracePoint(std::string_view pointStr) const;
    uint32_t GetThreadGroupId(std::string_view pointStr, size_t& length) const;
    std::string_view GetPointNameForBegin(std::string_view pointStr, size_t tGidlength) const;
    ParseResult HandlerB(std::string_view pointStr, TracePoint& outPoint, size_t tGidlength) const;
    bool HandleFrameSliceBeginEvent(DataIndex eventName,
                                    size_t callStackRow,
                                    std::string& args,
                                    const BytraceLine& line);
    void HandleFrameSliceEndEvent(uint64_t ts, uint64_t pid, uint64_t tid, size_t callStackRow);
    void HandleFrameQueueEndEvent(uint64_t ts, uint64_t pid, uint64_t tid, size_t callStackRow);
    bool HandleAnimationBeginEvent(const TracePoint& point, size_t callStackRow, const BytraceLine& line);
    static ParseResult HandlerE(void);
    ParseResult HandlerCSF(std::string_view pointStr, TracePoint& outPoint, size_t tGidlength) const;
    static size_t GetNameLength(std::string_view pointStr, size_t nameIndex);
    size_t GetValueLength(std::string_view pointStr, size_t valueIndex) const;
    bool ReciveVsync(size_t callStackRow, std::string& args, const BytraceLine& line);
    bool RSReciveOnVsync(size_t callStackRow, std::string& args, const BytraceLine& line);
    bool OnRwTransaction(size_t callStackRow, std::string& args, const BytraceLine& line);
    bool OnMainThreadProcessCmd(size_t callStackRow, std::string& args, const BytraceLine& line);
    bool OnFrameQueueStart(uint64_t ts, size_t callStackRow, uint64_t pid);

private:
    std::map<DataIndex, FrameFuncCall> eventToFrameFunctionMap_ = {};
    const uint32_t pointLength_;
    const uint32_t maxPointLength_;
    TraceStreamerConfig config_{};
    const DataIndex recvievVsync_ = traceDataCache_->GetDataIndex("H:ReceiveVsync");
    const DataIndex rsOnVsyncEvent_ = traceDataCache_->GetDataIndex("H:RSMainThread::OnVsync");
    const std::string onFrameQueeuStartEvent_ = "H:M: Frame queued";
    const std::string onLauncherVsyncEvent_ = "launcher";
    const DataIndex marshRwTransactionData_ = traceDataCache_->GetDataIndex("H:MarshRSTransactionData");
    const DataIndex rsMainThreadProcessCmd_ = traceDataCache_->GetDataIndex("H:RSMainThread::ProcessCommandUni");
    const DataIndex onAnimationStartEvent_ = traceDataCache_->GetDataIndex("H:L:TracestartAppAnimation");
    const std::regex recvVsyncPattern_ = std::regex("(\\w+):(\\w+)");
    const std::regex transFlagPattern_ = std::regex("transactionFlag:\\[(\\d+),(\\d+)\\]");
    const std::regex mainProcessCmdPattern = std::regex("\\[(\\d+),(\\d+)\\]");
    std::vector<uint64_t> frameCallIds_ = {};
    std::vector<uint64_t> vsyncSliceIds_ = {};
    TraceFileType traceType_ = TRACE_FILETYPE_H_TRACE;
    BuiltinClocks clock_ = TS_CLOCK_BOOTTIME;
    // if convert vsync's now and expectEnd
    bool convertVsyncTs_ = true;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // SRC_PRINT_EVENT_PARSER_H
