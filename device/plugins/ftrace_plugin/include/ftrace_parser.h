/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
 *
 * Description: FtraceParser class define
 */
#ifndef FTRACE_EVENT_CONTAINER_H
#define FTRACE_EVENT_CONTAINER_H
#include <memory>
#include <regex>
#include <string>
#include <vector>
#include "ftrace_common_type.h"
#include "ftrace_field_parser.h"
#include "printk_formats_parser.h"
#include "sub_event_parser.h"

FTRACE_NS_BEGIN
class FtraceParser {
public:
    FtraceParser();
    ~FtraceParser();

    bool Init();
    bool SetupEvent(const std::string& type, const std::string& name);

    bool ParsePerCpuStatus(PerCpuStats& stats, const std::string& perCpuStats);
    bool ParsePage(FtraceCpuDetailMsg& cpuDetailMsg, uint8_t page[], size_t size);

    bool ParseSavedTgid(const std::string& savedTgid);
    bool ParseSavedCmdlines(const std::string& savedCmdlines);

    void SetDebugOn(bool value);

private:
    int GetHeaderPageCommitSize(void);
    bool ParseHeaderPageFormat(const std::string& formatDesc);
    bool ParseEventFormat(const std::string& formatDesc, EventFormat& format);
    bool ParseFieldFormat(const std::string& fieldLine, EventFormat& format);
    bool ParseFieldType(const std::string& type, FieldFormat& field);
    void PrintFieldInfo(const FieldFormat& info);
    static void ParseProtoType(FieldFormat& field);

    bool ParsePageHeader();

    // parse different page types
    bool ParsePaddingData(const FtraceEventHeader& eventHeader);
    bool ParseTimeExtend(const FtraceEventHeader& eventHeader);
    bool ParseTimeStamp(const FtraceEventHeader& eventHeader);
    bool ParseDataRecord(const FtraceEventHeader& eventHeader, FtraceCpuDetailMsg& cpuDetailMsg);

    bool ParseFtraceEvent(FtraceEvent& ftraceEvent, uint8_t data[],
                          size_t dataSize, const SubEventParser::ParseEventCtx* parseEventCtx);
    bool ParseFtraceCommonFields(FtraceEvent& ftraceEvent, uint8_t data[], size_t dataSize, const EventFormat& format);

private:
    DISALLOW_COPY_AND_MOVE(FtraceParser);
    bool debugOn_ = false;
    std::regex fixedCharArrayRegex_;
    std::regex flexDataLocArrayRegex_;
    PageHeaderFormat pageHeaderFormat_ = {};
    std::string savedTgidPath_ = "";
    std::string savedCmdlines_ = "";

    uint8_t* cur_ = nullptr;
    uint8_t* page_ = nullptr;      // page start
    uint8_t* endOfData_ = nullptr; // end of event data
    uint8_t* endOfPage_ = nullptr; // end of full page
    uint64_t timestamp_ = 0;
    PageHeader pageHeader_ = {};

    std::unordered_map<int32_t, int32_t> tgidDict_ = {};
    std::unordered_map<int32_t, std::string> commDict_ = {};
};
FTRACE_NS_END
#endif
