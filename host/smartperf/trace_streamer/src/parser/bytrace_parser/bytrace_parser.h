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

#ifndef BYTRACE_PARSER_H
#define BYTRACE_PARSER_H

#include <condition_variable>
#include <mutex>
#include <regex>
#include <thread>

#include "bytrace_event_parser.h"
#include "json.hpp"
#include "log.h"
#include "parser_base.h"
#include "string_to_numerical.h"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class BytraceParser : public ParserBase {
public:
    BytraceParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters);
    ~BytraceParser();

    void ParseTraceDataSegment(std::unique_ptr<uint8_t[]> bufferStr, size_t size) override;
    size_t ParsedTraceValidLines() const
    {
        return parsedTraceValidLines_;
    }
    size_t ParsedTraceInvalidLines() const
    {
        return parsedTraceInvalidLines_;
    }
    size_t TraceCommentLines() const
    {
        return traceCommentLines_;
    }
    void EnableBytrace(bool enable)
    {
        isBytrace_ = enable;
    }

    void WaitForParserEnd();

private:
    enum ErrorCode { ERROR_CODE_EXIT = -2, ERROR_CODE_NODATA = -1 };
    int32_t GetNextSegment();
    void GetDataSegAttr(DataSegment& seg, const std::smatch& matcheLine) const;

    void FilterThread();
    inline static bool IsNotSpace(char c)
    {
        return !std::isspace(c);
    }
    inline static bool IsTraceComment(const std::string& buffer)
    {
        return ((buffer[0] == '#') || buffer.find("TASK-PID") != std::string::npos);
    }
    inline static bool IsHtmlTrace(const std::string& buffer)
    {
        std::string lower(buffer);
        transform(buffer.begin(), buffer.end(), lower.begin(), ::tolower);
        return ((lower.compare(0, std::string("<!doctype html>").length(), "<!doctype html>") == 0) ||
                (lower.compare(0, std::string("<html>").length(), "<html>") == 0));
    }
    inline static bool IsHtmlTraceBegin(const std::string& buffer)
    {
        return buffer.find(R"(<script class="trace-data" type="application/text">)") != std::string::npos;
    }

    void ParseTraceDataItem(const std::string& buffer) override;
    std::string StrTrim(const std::string& input) const;
    void ParseThread();
    void ParserData(DataSegment& seg);
    bool FilterData(DataSegment& seg);

private:
    using json = nlohmann::json;
    typedef struct {
        std::string eventSource;
        uint64_t timeStamp;
        std::vector<std::string> appName;
        std::vector<std::string> appVersions;
        std::vector<std::string> key;
        std::vector<json> value;
    } JsonData;
    void AppendJsonDataToHiSysEventNewValue(JsonData jData, int32_t jIndex, DataIndex eventSourceIndex);
    void NoArrayDataParse(JsonData jData, std::vector<size_t> noArrayIndex, DataIndex eventSourceIndex);
    void ArrayDataParse(JsonData jData,
                        std::vector<size_t> arrayIndex,
                        DataIndex eventSourceIndex,
                        size_t maxArraySize);
    void CommonDataParser(JsonData jData, DataIndex eventSourceIndex);
    int32_t JGetData(json& jMessage,
                     JsonData& jData,
                     size_t& maxArraySize,
                     std::vector<size_t>& noArrayIndex,
                     std::vector<size_t>& arrayIndex);
    void ParseJsonData(const std::string& buffer);

private:
    using ArgsMap = std::unordered_map<std::string, std::string>;
    bool isParsingOver_ = false;
    std::unique_ptr<BytraceEventParser> eventParser_;
    const std::regex bytraceMatcher_ = std::regex(R"(-(\d+)\s+\(?\s*(\d+|-+)?\)?\s?\[(\d+)\]\s*)"
                                                  R"([a-zA-Z0-9.]{0,5}\s+(\d+\.\d+):\s+(\S+):)");

    const std::string script_ = R"(</script>)";

    std::vector<std::string> eventsAccordingAppNames = {"POWER_IDE_BATTERY",
                                                        "POWER_IDE_CPU",
                                                        "POWER_IDE_LOCATION",
                                                        "POWER_IDE_GPU",
                                                        "POWER_IDE_DISPLAY",
                                                        "POWER_IDE_CAMERA",
                                                        "POWER_IDE_BLUETOOTH",
                                                        "POWER_IDE_FLASHLIGHT",
                                                        "POWER_IDE_AUDIO",
                                                        "POWER_IDE_WIFISCAN",
                                                        "BRIGHTNESS_NIT",
                                                        "SIGNAL_LEVEL",
                                                        "WIFI_EVENT_RECEIVED",
                                                        "AUDIO_STREAM_CHANGE",
                                                        "AUDIO_VOLUME_CHANGE",
                                                        "WIFI_STATE",
                                                        "BLUETOOTH_BR_SWITCH_STATE",
                                                        "LOCATION_SWITCH_STATE",
                                                        "ENABLE_SENSOR",
                                                        "DISABLE_SENSOR",
                                                        "WORK_REMOVE",
                                                        "WORK_START",
                                                        "WORK_STOP",
                                                        "WORK_ADD",
                                                        "POWER_RUNNINGLOCK",
                                                        "GNSS_STATE",
                                                        "ANOMALY_SCREEN_OFF_ENERGY",
                                                        "ANOMALY_ALARM_WAKEUP",
                                                        "ANOMALY_KERNEL_WAKELOCK",
                                                        "ANOMALY_RUNNINGLOCK",
                                                        "ANORMALY_APP_ENERGY",
                                                        "ANOMALY_GNSS_ENERGY",
                                                        "ANOMALY_CPU_HIGH_FREQUENCY",
                                                        "ANOMALY_CPU_ENERGY",
                                                        "ANOMALY_WAKEUP"};

    size_t parsedTraceValidLines_ = 0;
    size_t parsedTraceInvalidLines_ = 0;
    size_t traceCommentLines_ = 0;
    std::mutex dataSegMux_;
    int32_t parseHead_ = 0;
    std::atomic<bool> filterThreadStarted_{false};
    bool parseThreadStarted_ = false;
    const int32_t MAX_SEG_ARRAY_SIZE = 5000;
    const int32_t maxThread_ = 4; // 4 is the best on ubuntu 113MB/s, max 138MB/s, 6 is best on mac m1 21MB/s,
    int32_t parserThreadCount_ = 0;
    bool toExit_ = false;
    bool exited_ = false;
    std::unique_ptr<DataSegment[]> dataSegArray_;
    int32_t rawDataHead_ = 0;
    int32_t filterHead_ = 0;
    const int32_t sleepDur_ = 100;
    bool supportThread_ = false;
    bool isBytrace_ = true;
    bool traceBegan_ = false;
    bool isFirstLine = true;
    bool isHtmlTrace_ = false;
    bool isHtmlTraceContent_ = false;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // _BYTRACE_PARSER_H_
