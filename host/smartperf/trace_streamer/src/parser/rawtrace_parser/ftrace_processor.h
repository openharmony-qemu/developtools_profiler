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
#ifndef FTRACE_PROCESSOR_H
#define FTRACE_PROCESSOR_H
#include <memory>
#include <regex>
#include <string>
#include <vector>
#include "cpu_detail_parser.h"
#include "ftrace_common_type.h"
#include "ftrace_field_processor.h"
#include "printk_formats_processor.h"
#include "ftrace_event_processor.h"

namespace SysTuning {
namespace TraceStreamer {
constexpr uint32_t FTRACE_PAGE_SIZE = 4096;
class FtraceProcessor {
public:
    FtraceProcessor();
    ~FtraceProcessor();

    bool SetupEvent(const std::string& desc);

    bool HandlePage(FtraceCpuDetailMsg& cpuDetailMsg,
                    CpuDetailParser& cpuDetailParser,
                    uint8_t page[],
                    size_t size = FTRACE_PAGE_SIZE);

    bool HandleTgids(const std::string& tgids);
    bool HandleCmdlines(const std::string& cmdlines);

public:
    bool GetEventFormatById(uint32_t id, EventFormat& format);

    int HeaderPageCommitSize(void);
    bool HandleHeaderPageFormat(const std::string& formatInfo);
    bool HandleEventFormat(const std::string& formatInfo, EventFormat& format);
    bool HandleFieldFormat(const std::string& fieldLine, EventFormat& format);
    bool HandleFieldType(const std::string& type, FieldFormat& field);
    void PrintedFieldDetails(const FieldFormat& info);
    static void HandleProtoType(FieldFormat& fieldFormat);

    bool HandlePageHeader();

    // handle different page types
    bool HandlePaddingData(const FtraceEventHeader& eventHeader);
    bool HandleTimeExtend(const FtraceEventHeader& eventHeader);
    bool HandleTimeStamp(const FtraceEventHeader& eventHeader);
    bool HandleDataRecord(const FtraceEventHeader& eventHeader,
                          FtraceCpuDetailMsg& cpuDetailMsg,
                          CpuDetailParser& cpuDetailParser);

    bool HandleFtraceEvent(FtraceEvent& ftraceEvent, uint8_t data[], size_t dataSize, const EventFormat& format);
    bool HandleFtraceCommonFields(FtraceEvent& ftraceEvent, uint8_t data[], size_t dataSize, const EventFormat& format);

private:
    std::regex fixedCharArrayRegex_;
    std::regex flexDataLocArrayRegex_;
    // first is event id, second is eventFormat
    std::unordered_map<uint32_t, EventFormat> eventFormatDict_ = {};
    PageHeaderFormat pageHeaderFormat_ = {};
    std::string savedTgidPath_ = "";
    std::string savedCmdlines_ = "";

    uint8_t* curPos_ = nullptr;
    uint8_t* curPage_ = nullptr;      // cur page start
    uint8_t* endPosOfData_ = nullptr; // end pos of event data
    uint8_t* endPosOfPage_ = nullptr; // end pos of full page
    uint64_t curTimestamp_ = 0;
    PageHeader curPageHeader_ = {};

    // first is pid, second is tgid
    std::unordered_map<int32_t, int32_t> tgidDict_ = {};
    // first is pid, second is taskName
    std::unordered_map<int32_t, std::string> taskNameDict_ = {};

    const std::string nameLinePrefix_ = "name:";
    const std::string idLinePrefix_ = "ID:";
    const std::string fieldLinePrefix_ = "field:";
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // FTRACE_PROCESSOR_H
