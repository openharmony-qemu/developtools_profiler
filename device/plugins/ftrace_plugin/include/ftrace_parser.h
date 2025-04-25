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
#include <cinttypes>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "logging.h"
#include "ftrace_common_type.h"
#include "ftrace_field_parser.h"
#include "ftrace_fs_ops.h"
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

    template <typename T, typename E> bool ParsePage(T& ftraceCpuDetailMsg, uint8_t page[], size_t size, E* ftraceEvent)
    {
        cur_ = page;
        page_ = page;
        endOfPage_ = page + size;

        CHECK_TRUE(ParsePageHeader(), false, "parse page header fail!");
        ftraceCpuDetailMsg.set_overwrite(pageHeader_.overwrite);

        timestamp_ = pageHeader_.timestamp;
        endOfData_ = pageHeader_.endpos;
        while (cur_ < pageHeader_.endpos) {
            FtraceEventHeader eventHeader = {};
            CHECK_TRUE(ReadInc(&cur_, endOfData_, &eventHeader, sizeof(FtraceEventHeader)), false,
                       "read EventHeader fail!");

            timestamp_ += eventHeader.timeDelta;

            bool retval = false;
            switch (eventHeader.typeLen) {
                case BUFFER_TYPE_PADDING:
                    retval = ParsePaddingData(eventHeader);
                    CHECK_TRUE(retval, false, "parse PADDING data failed!");
                    break;
                case BUFFER_TYPE_TIME_EXTEND:
                    retval = ParseTimeExtend(eventHeader);
                    CHECK_TRUE(retval, false, "parse TIME_EXTEND failed!");
                    break;
                case BUFFER_TYPE_TIME_STAMP:
                    retval = ParseTimeStamp(eventHeader);
                    CHECK_TRUE(retval, false, "parse TIME_STAMP failed!");
                    break;
                default:
                    retval = ParseDataRecord(eventHeader, ftraceCpuDetailMsg, ftraceEvent);
                    CHECK_TRUE(retval, false, "parse record data failed!");
                    break;
            }
        }
        return true;
    }

    bool ParseSavedTgid(const std::string& savedTgid);
    bool ParseSavedCmdlines(const std::string& savedCmdlines);

    void SetDebugOn(bool value);

    template <typename T, typename P> bool HmParseFtraceEvent(T& ftraceEvent, uint8_t data[],
            size_t dataSize, P& parseEventCtx)
    {
        return ParseFtraceEvent(ftraceEvent, data, dataSize, parseEventCtx);
    }

private:
    int GetHeaderPageCommitSize(void);
    bool ParseHeaderPageFormat(const std::string& formatDesc);
    bool ParseEventFormat(const std::string& formatDesc, EventFormat& format);
    bool ParseFieldFormat(const std::string& fieldLine, EventFormat& format);
    bool ParseFieldType(const std::string& type, FieldFormat& field);
    static void ParseProtoType(FieldFormat& field);

    bool ParsePageHeader();

    // parse different page types
    bool ParsePaddingData(const FtraceEventHeader& eventHeader);
    bool ParseTimeExtend(const FtraceEventHeader& eventHeader);
    bool ParseTimeStamp(const FtraceEventHeader& eventHeader);

    template <typename T, typename E>
    bool ParseDataRecord(const FtraceEventHeader& eventHeader, T& ftraceCpuDetailMsg, E* event)
    {
        uint32_t evtSize = 0;
        // refers comments of kernel function rb_event_data_length:
        if (eventHeader.typeLen) {
            evtSize = sizeof(eventHeader.array[0]) * eventHeader.typeLen;
        } else {
            CHECK_TRUE(ReadInc(&cur_, endOfData_, &evtSize, sizeof(evtSize)), false, "read event size failed!");
            if (evtSize < sizeof(uint32_t)) {
                return false;
            }
            evtSize -= sizeof(uint32_t); // array[0] is length, array[1...array[0]] is event data
        }

        uint8_t* evStart = cur_;
        uint8_t* evEnd = cur_ + evtSize;
        uint16_t evId = 0;
        CHECK_TRUE(ReadInc(&cur_, evEnd, &evId, sizeof(evId)), false, "read event ID failed!");

        uint32_t eventId = evId;
        auto* parseEventCtx = SubEventParser<E>::GetInstance().GetParseEventCtx(eventId);
        if (parseEventCtx != nullptr) {
            auto* ftraceEvent = ftraceCpuDetailMsg.add_event();
            ftraceEvent->set_timestamp(timestamp_);
            parseEventCtx->format.osVersion = osVersion_;
            ParseFtraceEvent(*ftraceEvent, evStart, evtSize, parseEventCtx);
        }

        cur_ = evEnd;
        return true;
    }

    template <typename T, typename P>  // P: SubEventParser<FtraceEvent>::ParseEventCtx
    bool ParseFtraceEvent(T& ftraceEvent, uint8_t data[], size_t dataSize, P& parseEventCtx)
    {
        CHECK_TRUE(
            dataSize >= parseEventCtx->format.eventSize, false,
            "FtraceParser::ParseFtraceEvent, dataSize not enough! event name is %s,eventSize is %u, dataSize is %zd",
            parseEventCtx->format.eventName.c_str(), parseEventCtx->format.eventSize, dataSize);

        int pid = 0;
        CHECK_TRUE(ParseFtraceCommonFields(ftraceEvent, data, dataSize, parseEventCtx->format, pid),
                   false, "parse common fields failed!");
        if (pid != 0) {
            int tgid = 0;
            if (auto it = tgidDict_.find(pid); it != tgidDict_.end()) {
                tgid = it->second;
                ftraceEvent.set_tgid(tgid);
            } else {
                ParseSavedTgid(FtraceFsOps::GetInstance().GetSavedTgids());
                if (auto itm = tgidDict_.find(pid); itm != tgidDict_.end()) {
                    tgid = itm->second;
                    ftraceEvent.set_tgid(tgid);
                }
            }

            std::string comm;
            if (auto it = commDict_.find(pid); it != commDict_.end()) {
                comm = it->second;
            } else {
                if (tgid != 0) {
                    comm = FtraceFsOps::GetInstance().GetThreadComm(tgid, pid);
                } else {
                    comm = FtraceFsOps::GetInstance().GetProcessComm(pid);
                }
                if (comm.size() > 0) {
                    comm.pop_back(); // /proc/xxx/comm end with `\n`
                    commDict_.insert(std::pair<int32_t, std::string>(pid, comm));
                }
            }
            if (comm.size() > 0) {
                ftraceEvent.set_comm(comm);
            }
        }

        SubEventParser<T>::GetInstance().ParseEvent(ftraceEvent, data, dataSize, parseEventCtx);
        return true;
    }

    template <typename T>
    bool ParseFtraceCommonFields(T& ftraceEvent, uint8_t data[], size_t dataSize, const EventFormat& format, int& pid)
    {
        auto& index = format.commonIndex;

        CHECK_TRUE(IsValidIndex(index.pid), false, "pid index %d invalid!", index.pid);
        CHECK_TRUE(IsValidIndex(index.type), false, "type index %d invalid!", index.type);
        CHECK_TRUE(IsValidIndex(index.flags), false, "flags index %d invalid!", index.flags);
        CHECK_TRUE(IsValidIndex(index.preemt), false, "preemt index %d invalid!", index.preemt);

        auto& fields = format.commonFields;
        auto commonFields = ftraceEvent.mutable_common_fields();
        pid = FtraceFieldParser::ParseIntField<int32_t>(fields, index.pid, data, dataSize);
        commonFields->set_pid(pid);
        commonFields->set_type(FtraceFieldParser::ParseIntField<uint32_t>(fields, index.type, data, dataSize));
        commonFields->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(fields, index.flags, data, dataSize));
        commonFields->set_preempt_count(FtraceFieldParser::ParseIntField<uint32_t>(fields, index.preemt,
                                                                                   data, dataSize));
        return true;
    }

    bool ReadInc(uint8_t* start[], uint8_t end[], void* outData, size_t outSize);
    bool IsValidIndex(int idx);

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
    std::string osVersion_ = "";
    PageHeader pageHeader_ = {};

    std::unordered_map<int32_t, int32_t> tgidDict_ = {};
    std::unordered_map<int32_t, std::string> commDict_ = {};
};
FTRACE_NS_END
#endif
