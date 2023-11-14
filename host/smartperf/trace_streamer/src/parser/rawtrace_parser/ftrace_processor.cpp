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
#include "ftrace_processor.h"

#include <algorithm>
#include <cerrno>
#include <cinttypes>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <regex>
#include <sstream>
#include <unistd.h>

#include "log.h"
#include "printk_formats_processor.h"
#include "securec.h"
#include "string_help.h"
#include "string_to_numerical.h"

namespace {
constexpr unsigned RB_MISSED_EVENTS = (1uL << 31); // Flag when events were overwritten
constexpr unsigned RB_MISSED_STORED = (1 << 30);   // Missed count stored at end
constexpr unsigned RB_MISSED_FLAGS = (RB_MISSED_EVENTS | RB_MISSED_STORED);

constexpr unsigned COL_IDX_NAME = 0;
constexpr unsigned COL_IDX_VALUE = 1;

constexpr unsigned TS_EXT_SHIFT = 27;

inline uint64_t TimestampIncrements(uint64_t ext)
{
    return ext << TS_EXT_SHIFT;
}

bool ReadInfo(uint8_t* startPtr[], uint8_t* endPtr, void* outData, size_t outSize)
{
    if ((endPtr - *startPtr) < static_cast<ptrdiff_t>(outSize)) {
        return false;
    }
    if (memcpy_s(outData, outSize, *startPtr, outSize) != EOK) {
        TS_LOGE("read %zu bytes from memory region [%p, %p) FAILED", outSize, *startPtr, endPtr);
        return false;
    }
    *startPtr += outSize;
    return true;
}
} // namespace

namespace SysTuning {
namespace TraceStreamer {
FtraceProcessor::FtraceProcessor()
{
    fixedCharArrayRegex_ = std::regex(R"(char \w+\[\d+\])");
    flexDataLocArrayRegex_ = std::regex(R"(__data_loc [a-zA-Z_0-9 ]+\[\] \w+)");
}

FtraceProcessor::~FtraceProcessor()
{
    TS_LOGI("FtraceProcessor destroy!");
}

bool FtraceProcessor::SetupEvent(const std::string& desc)
{
    EventFormat format;
    TS_CHECK_TRUE(HandleEventFormat(desc.data(), format), false, "HandleEventFormat failed!");
    TS_CHECK_TRUE(FtraceEventProcessor::GetInstance().SetupEvent(format), false, "setup %s failed!",
                  format.eventName.c_str());
    eventFormatDict_[format.eventId] = format;
    return true;
}

bool FtraceProcessor::GetEventFormatById(uint32_t id, EventFormat& format)
{
    auto iter = eventFormatDict_.find(id);
    if (iter != eventFormatDict_.end()) {
        format = iter->second;
        return true;
    }
    return false;
}

bool FtraceProcessor::HandleHeaderPageFormat(const std::string& formatInfo)
{
    EventFormat format = {};
    TS_CHECK_TRUE(HandleEventFormat(formatInfo, format), false, "handle events/header_page failed!");

    bool commitExist = false;
    for (auto& curField : format.fields) {
        if (curField.name == "timestamp") {
            pageHeaderFormat_.timestamp = curField;
        } else if (curField.name == "commit") {
            pageHeaderFormat_.commit = curField;
            commitExist = true;
        } else if (curField.name == "overwrite") {
            pageHeaderFormat_.overwrite = curField;
        }
    }

    TS_LOGD("page header details:");
    PrintedFieldDetails(pageHeaderFormat_.timestamp);
    PrintedFieldDetails(pageHeaderFormat_.commit);
    PrintedFieldDetails(pageHeaderFormat_.overwrite);
    TS_CHECK_TRUE(commitExist, false, "commit field not exist!");
    return true;
}

int FtraceProcessor::HeaderPageCommitSize(void)
{
    // return the size value (8B on 64bit device, 4B on 32bit device) of commit field read from events/header_page
    return pageHeaderFormat_.commit.size;
}

bool FtraceProcessor::HandleEventFormat(const std::string& formatInfo, EventFormat& format)
{
    std::string curLine;
    std::stringstream formatStream(formatInfo);
    while (getline(formatStream, curLine)) {
        curLine = Strip(curLine);
        if (curLine.empty()) {
            continue;
        } else if (StartWith(curLine, fieldLinePrefix_)) {
            HandleFieldFormat(curLine, format);
        } else if (StartWith(curLine, idLinePrefix_)) {
            auto idStr = curLine.substr(idLinePrefix_.size() + 1);
            format.eventId = static_cast<uint32_t>(atoi(idStr.c_str()));
        } else if (StartWith(curLine, nameLinePrefix_)) {
            format.eventName = curLine.substr(nameLinePrefix_.size() + 1);
            TS_CHECK_TRUE(FtraceEventProcessor::GetInstance().IsSupported(format.eventName), false,
                          "Isn't Supported %s event!", format.eventName.data());
        }
    }

    TS_CHECK_TRUE(format.fields.size() > 0, false, "HandleEventFormat from %s failed!", formatInfo.c_str());
    auto lastFiledIndex = format.fields.size() - 1;
    format.eventSize = format.fields[lastFiledIndex].offset + format.fields[lastFiledIndex].size;
    return true;
}

static std::string GetName(const std::map<int, std::string>& nameMap, int type)
{
    auto it = nameMap.find(type);
    if (it != nameMap.end()) {
        return it->second;
    }
    return "";
}

static std::string GetFieldTypeName(EventFieldType type)
{
    static std::map<int, std::string> toNames = {
#define VALUE_NAME(x) {x, #x}
        VALUE_NAME(FIELD_TYPE_INVALID),   VALUE_NAME(FIELD_TYPE_BOOL),         VALUE_NAME(FIELD_TYPE_INT8),
        VALUE_NAME(FIELD_TYPE_UINT8),     VALUE_NAME(FIELD_TYPE_INT16),        VALUE_NAME(FIELD_TYPE_UINT16),
        VALUE_NAME(FIELD_TYPE_INT32),     VALUE_NAME(FIELD_TYPE_UINT32),       VALUE_NAME(FIELD_TYPE_INT64),
        VALUE_NAME(FIELD_TYPE_UINT64),    VALUE_NAME(FIELD_TYPE_FIXEDCSTRING), VALUE_NAME(FIELD_TYPE_CSTRING),
        VALUE_NAME(FIELD_TYPE_STRINGPTR), VALUE_NAME(FIELD_TYPE_INODE32),      VALUE_NAME(FIELD_TYPE_INODE64),
        VALUE_NAME(FIELD_TYPE_PID32),     VALUE_NAME(FIELD_TYPE_COMMONPID32),  VALUE_NAME(FIELD_TYPE_DEVID32),
        VALUE_NAME(FIELD_TYPE_DEVID64),   VALUE_NAME(FIELD_TYPE_DATALOC),      VALUE_NAME(FIELD_TYPE_SYMADDR32),
        VALUE_NAME(FIELD_TYPE_SYMADDR64),
#undef VALUE_NAME
    };
    return GetName(toNames, type);
}

static std::string GetProtoTypeName(ProtoFieldType type)
{
    static std::map<int, std::string> toNames = {
#define VALUE_NAME(x) {x, #x}
        VALUE_NAME(PROTO_TYPE_UNKNOWN),  VALUE_NAME(PROTO_TYPE_DOUBLE),   VALUE_NAME(PROTO_TYPE_FLOAT),
        VALUE_NAME(PROTO_TYPE_INT64),    VALUE_NAME(PROTO_TYPE_UINT64),   VALUE_NAME(PROTO_TYPE_INT32),
        VALUE_NAME(PROTO_TYPE_FIXED64),  VALUE_NAME(PROTO_TYPE_FIXED32),  VALUE_NAME(PROTO_TYPE_BOOL),
        VALUE_NAME(PROTO_TYPE_STRING),   VALUE_NAME(PROTO_TYPE_GROUP),    VALUE_NAME(PROTO_TYPE_MESSAGE),
        VALUE_NAME(PROTO_TYPE_BYTES),    VALUE_NAME(PROTO_TYPE_UINT32),   VALUE_NAME(PROTO_TYPE_ENUM),
        VALUE_NAME(PROTO_TYPE_SFIXED32), VALUE_NAME(PROTO_TYPE_SFIXED64), VALUE_NAME(PROTO_TYPE_SINT32),
        VALUE_NAME(PROTO_TYPE_SINT64),   VALUE_NAME(PROTO_TYPE_MAX),
#undef VALUE_NAME
    };
    return GetName(toNames, type);
}

void FtraceProcessor::PrintedFieldDetails(const FieldFormat& info)
{
    TS_LOGD(
        "FieldFormat { offset: %u, size:%u, sign: %u fieldType: %s, "
        "protoType:%s, typeName: %s, name: %s}",
        info.offset, info.size, info.isSigned, GetFieldTypeName(info.filedType).c_str(),
        GetProtoTypeName(info.protoType).c_str(), info.typeName.c_str(), info.name.c_str());
}

static std::string GetNameFromTypeName(const std::string& typeName)
{
    if (typeName.empty()) {
        return "";
    }
    std::string curName;
    auto posT0 = typeName.rfind(" ");
    std::string rightPart = typeName.substr(posT0 + 1);
    if (rightPart[rightPart.size() - 1] != ']') {
        curName = rightPart;
    } else {
        auto posT1 = rightPart.rfind('[');
        TS_CHECK_TRUE(posT1 != std::string::npos, "", "GetNameFromTypeName Failed!");
        curName = rightPart.substr(0, posT1);
    }
    return curName;
}

static std::string GetTypeFromTypeName(const std::string& typeName, const std::string& name)
{
    std::string curType;
    if (!name.empty()) {
        curType = typeName;
        auto pos = curType.find(name);
        curType.replace(pos, name.size(), "");
        curType = Strip(curType);
    }
    return curType;
}

static void ParseCommonFiledIndex(CommonFiledIndex& commonIndex, const std::string& name, int index)
{
    if (name == "common_type") {
        commonIndex.type = index;
    } else if (name == "common_flags") {
        commonIndex.flags = index;
    } else if (name == "common_preempt_count") {
        commonIndex.preemt = index;
    } else if (name == "common_pid") {
        commonIndex.pid = index;
    }
}

bool FtraceProcessor::HandleFieldFormat(const std::string& fieldLine, EventFormat& format)
{
    std::string typeName;
    std::string offsetInfo;
    std::string sizeInfo;
    std::string signedInfo;
    for (auto& partInfo : SplitStringToVec(fieldLine, ";")) {
        auto fieldMap = SplitStringToVec(Strip(partInfo), ":");
        if (fieldMap.size() < COL_IDX_VALUE) {
            continue;
        }
        const auto& fieldName = fieldMap[COL_IDX_NAME];
        if (fieldName == "field") {
            typeName = fieldMap[COL_IDX_VALUE];
        } else if (fieldName == "offset") {
            offsetInfo = fieldMap[COL_IDX_VALUE];
        } else if (fieldName == "size") {
            sizeInfo = fieldMap[COL_IDX_VALUE];
        } else if (fieldName == "signed") {
            signedInfo = fieldMap[COL_IDX_VALUE];
        }
    }
    std::string filedName = GetNameFromTypeName(typeName);
    std::string filedType = GetTypeFromTypeName(typeName, filedName);
    FieldFormat fieldFormat;
    fieldFormat.name = filedName;
    fieldFormat.typeName = typeName;
    fieldFormat.offset = atoi(offsetInfo.c_str());
    fieldFormat.size = atoi(sizeInfo.c_str());
    fieldFormat.isSigned = atoi(signedInfo.c_str());

    HandleFieldType(filedType, fieldFormat);
    HandleProtoType(fieldFormat);

    if (StartWith(filedName, "common_")) {
        ParseCommonFiledIndex(format.commonIndex, filedName, static_cast<int>(format.commonFields.size()));
        format.commonFields.push_back(fieldFormat);
    } else {
        format.fields.push_back(fieldFormat);
    }
    return true;
}

static bool ParseSepcialIntType(FieldFormat& field, const std::string& type, const std::string& typeName)
{
    if (type == "bool") {
        field.filedType = FIELD_TYPE_BOOL;
        return true;
    }

    if (type == "ino_t" || type == "i_ino") {
        if (field.size == sizeof(uint32_t)) {
            field.filedType = FIELD_TYPE_INODE32;
            return true;
        } else if (field.size == sizeof(uint64_t)) {
            field.filedType = FIELD_TYPE_INODE64;
            return true;
        }
    }

    if (type == "dev_t") {
        if (field.size == sizeof(uint32_t)) {
            field.filedType = FIELD_TYPE_DEVID32;
            return true;
        } else if (field.size == sizeof(uint64_t)) {
            field.filedType = FIELD_TYPE_DEVID64;
            return true;
        }
    }

    // Pids (as in 'sched_switch').
    if (type == "pid_t") {
        field.filedType = FIELD_TYPE_PID32;
        return true;
    }

    if ((typeName.find("common_pid") != std::string::npos)) {
        field.filedType = FIELD_TYPE_COMMONPID32;
        return true;
    }
    return false;
}

static bool ParseCommonIntType(FieldFormat& field, bool sign)
{
    switch (field.size) {
        case sizeof(int8_t):
            field.filedType = sign ? FIELD_TYPE_INT8 : FIELD_TYPE_UINT8;
            return true;
        case sizeof(int16_t):
            field.filedType = sign ? FIELD_TYPE_INT16 : FIELD_TYPE_UINT16;
            return true;
        case sizeof(int32_t):
            field.filedType = sign ? FIELD_TYPE_INT32 : FIELD_TYPE_UINT32;
            return true;
        case sizeof(int64_t):
            field.filedType = sign ? FIELD_TYPE_INT64 : FIELD_TYPE_UINT64;
            return true;
        default:
            break;
    }
    return false;
}

bool ParseKernelAddrField(FieldFormat& field, const std::string& type)
{
    if (type == "void*" || type == "void *") {
        if (field.size == sizeof(uint64_t)) { // 64-bit kernel addresses
            field.filedType = FIELD_TYPE_SYMADDR64;
            return true;
        } else if (field.size == sizeof(uint32_t)) { // 32-bit kernel addresses
            field.filedType = FIELD_TYPE_SYMADDR32;
            return true;
        }
    }
    return false;
}

bool FtraceProcessor::HandleFieldType(const std::string& type, FieldFormat& field)
{
    const std::string& curTypeName = field.typeName;
    // for fixed size C char arrary, likes "char a[LEN]"
    if (std::regex_match(curTypeName, fixedCharArrayRegex_)) {
        field.filedType = FIELD_TYPE_FIXEDCSTRING;
        return true;
    }

    // for flex array with __data_loc mark, likes: __data_loc char[] name;
    if (std::regex_match(curTypeName, flexDataLocArrayRegex_)) {
        if (field.size != sizeof(uint32_t)) {
            TS_LOGW("__data_loc: %s, size: %hu", curTypeName.c_str(), field.size);
            return false;
        }
        field.filedType = FIELD_TYPE_DATALOC;
        return true;
    }

    if ((curTypeName.find("char[]") != std::string::npos) || (curTypeName.find("char *") != std::string::npos)) {
        field.filedType = FIELD_TYPE_STRINGPTR;
        return true;
    }

    // for variable length strings: "char foo" + size: 0 (as in 'print').
    if ((type == "char" || type == "char []") && field.size == 0) {
        field.filedType = FIELD_TYPE_CSTRING;
        return true;
    }

    // for 64-bit kernel addresses
    TS_CHECK_TRUE_RET(!ParseKernelAddrField(field, type), true);
    TS_CHECK_TRUE_RET(!ParseSepcialIntType(field, type, curTypeName), true);
    // for int uint:
    TS_CHECK_TRUE_RET(!ParseCommonIntType(field, field.isSigned), true);
    return false;
}

void FtraceProcessor::HandleProtoType(FieldFormat& fieldFormat)
{
    switch (fieldFormat.filedType) {
        case FIELD_TYPE_INT8:
        case FIELD_TYPE_INT16:
        case FIELD_TYPE_INT32:
        case FIELD_TYPE_PID32:
        case FIELD_TYPE_COMMONPID32:
            fieldFormat.protoType = PROTO_TYPE_INT32;
            break;
        case FIELD_TYPE_INT64:
            fieldFormat.protoType = PROTO_TYPE_INT64;
            break;
        case FIELD_TYPE_UINT8:
        case FIELD_TYPE_UINT16:
        case FIELD_TYPE_UINT32:
        case FIELD_TYPE_BOOL:
        case FIELD_TYPE_DEVID32:
        case FIELD_TYPE_SYMADDR32:
            fieldFormat.protoType = PROTO_TYPE_UINT32;
            break;
        case FIELD_TYPE_DEVID64:
        case FIELD_TYPE_UINT64:
        case FIELD_TYPE_INODE32:
        case FIELD_TYPE_INODE64:
        case FIELD_TYPE_SYMADDR64:
            fieldFormat.protoType = PROTO_TYPE_UINT64;
            break;
        case FIELD_TYPE_CSTRING:
        case FIELD_TYPE_FIXEDCSTRING:
        case FIELD_TYPE_STRINGPTR:
        case FIELD_TYPE_DATALOC:
            fieldFormat.protoType = PROTO_TYPE_STRING;
            break;
        case FIELD_TYPE_INVALID:
            fieldFormat.protoType = PROTO_TYPE_UNKNOWN;
            break;
        default:
            break;
    }
}

bool FtraceProcessor::HandlePageHeader()
{
    (void)memset_s(&curPageHeader_, sizeof(curPageHeader_), 0, sizeof(PageHeader));

    uint64_t curTimestamp = 0;
    TS_CHECK_TRUE(ReadInfo(&curPos_, endPosOfPage_, &curTimestamp, sizeof(curTimestamp)), false,
                  "read timestamp from page failed!");
    curPageHeader_.timestamp = curTimestamp;

    uint64_t curCommit = 0;
    TS_CHECK_TRUE(ReadInfo(&curPos_, endPosOfPage_, &curCommit, HeaderPageCommitSize()), false,
                  "read commit to page header failed!");

    // for refers kernel function ring_buffer_page_len:
    curPageHeader_.size = (curCommit & ~RB_MISSED_FLAGS);
    curPageHeader_.overwrite = (curCommit & RB_MISSED_EVENTS);

    curPageHeader_.startpos = curPos_;
    curPageHeader_.endpos = curPos_ + curPageHeader_.size;
    return true;
}

bool FtraceProcessor::HandleTgids(const std::string& tgids)
{
    int32_t pid = 0;
    int32_t tgid = 0;
    bool state = false;
    std::stringstream tgidsStream(tgids);
    // format info for: "%d %d\n"
    while (tgidsStream >> pid >> tgid) {
        tgidDict_[pid] = tgid;
        state = true;
    }

    if (tgidDict_.size() == 0) {
        TS_LOGW("tgidDict_ is zero!");
    }
    return state;
}

bool FtraceProcessor::HandleCmdlines(const std::string& cmdlines)
{
    bool state = false;
    int32_t pid;
    std::string taskName;
    std::string curLine;
    std::stringstream cmdlinesStream(cmdlines);
    // format info for: "%d %s\n"
    while (std::getline(cmdlinesStream, curLine)) {
        auto pos = curLine.find(' ');
        if (pos != std::string::npos) {
            pid = std::stoi(curLine.substr(0, pos));
            taskName = curLine.substr(pos + 1);
            taskNameDict_[pid] = taskName;
            state = true;
        }
    }

    if (taskNameDict_.size() == 0) {
        TS_LOGW("taskNameDict_ is zero!");
    }
    return state;
}

bool FtraceProcessor::HandlePaddingData(const FtraceEventHeader& eventHeader)
{
    TS_CHECK_TRUE_RET(eventHeader.timeDelta != 0, false);
    uint32_t paddingLength;
    TS_CHECK_TRUE(ReadInfo(&curPos_, endPosOfData_, &paddingLength, sizeof(paddingLength)), false,
                  "read padding len failed!");
    // for skip padding data
    curPos_ += paddingLength;
    return true;
}

bool FtraceProcessor::HandleTimeExtend(const FtraceEventHeader& eventHeader)
{
    uint32_t deltaExt = 0;
    TS_CHECK_TRUE(ReadInfo(&curPos_, endPosOfData_, &deltaExt, sizeof(deltaExt)), false, "read time delta failed!");

    curTimestamp_ += TimestampIncrements(deltaExt);
    TS_LOGD("HandleTimeExtend: update ts with %u to %" PRIu64, deltaExt, curTimestamp_);
    return true;
}

bool FtraceProcessor::HandleTimeStamp(const FtraceEventHeader& eventHeader)
{
    uint32_t deltaExt = 0;
    TS_CHECK_TRUE(ReadInfo(&curPos_, endPosOfData_, &deltaExt, sizeof(deltaExt)), false, "read time delta failed!");

    // refers kernel function rb_update_write_stamp in ring_buffer.c
    curTimestamp_ = eventHeader.timeDelta + TimestampIncrements(deltaExt);
    TS_LOGD("update ts with %u to %" PRIu64, deltaExt, curTimestamp_);
    return true;
}

bool FtraceProcessor::HandleDataRecord(const FtraceEventHeader& eventHeader,
                                       FtraceCpuDetailMsg& cpuMsg,
                                       CpuDetailParser& cpuDetailParser)
{
    uint32_t eventSize = 0;
    // refers comments of kernel function rb_event_data_length:
    if (eventHeader.typeLen) {
        eventSize = sizeof(eventHeader.array[0]) * eventHeader.typeLen;
    } else {
        TS_CHECK_TRUE(ReadInfo(&curPos_, endPosOfData_, &eventSize, sizeof(eventSize)), false,
                      "read event size failed!");
        if (eventSize < sizeof(uint32_t)) {
            return false;
        }
        eventSize -= sizeof(uint32_t); // array[0] is length, array[1...array[0]] is event data
    }
    TS_LOGD("HandleDataRecord: parse %u bytes of event data...", eventSize);

    uint8_t* eventStart = curPos_;
    uint8_t* eventEnd = curPos_ + eventSize;
    uint16_t eventId = 0;
    TS_CHECK_TRUE(ReadInfo(&curPos_, eventEnd, &eventId, sizeof(eventId)), false, "read event ID failed!");

    EventFormat format = {};
    if (!GetEventFormatById(eventId, format)) {
        TS_LOGD("event with id %u we not interested!", eventId);
        curPos_ = eventEnd;
        return true;
    }
    TS_LOGD("HandleDataRecord: eventId = %u, name = %s", eventId, format.eventName.c_str());

    if (FtraceEventProcessor::GetInstance().IsSupported(format.eventId)) {
        std::unique_ptr<FtraceEvent> ftraceEvent = std::make_unique<FtraceEvent>();
        ftraceEvent->set_timestamp(curTimestamp_);
        HandleFtraceEvent(*ftraceEvent, eventStart, eventSize, format);
        std::unique_ptr<RawTraceEventInfo> event = std::make_unique<RawTraceEventInfo>();
        event->cpuId = cpuMsg.cpu();
        event->eventId = eventId;
        event->msgPtr = std::move(ftraceEvent);
        cpuDetailParser.EventAppend(std::move(event));
    } else {
        TS_LOGD("event %u %s not supported!", format.eventId, format.eventName.c_str());
    }
    curPos_ = eventEnd;
    return true;
}

bool FtraceProcessor::HandlePage(FtraceCpuDetailMsg& cpuMsg,
                                 CpuDetailParser& cpuDetailParser,
                                 uint8_t page[],
                                 size_t size)
{
    curPos_ = page;
    curPage_ = page;
    endPosOfPage_ = page + size;

    HandlePageHeader();
    TS_LOGD("HandlePage: %" PRIu64 " bytes event data in page!", curPageHeader_.size);
    cpuMsg.set_overwrite(curPageHeader_.overwrite);

    curTimestamp_ = curPageHeader_.timestamp;
    endPosOfData_ = curPageHeader_.endpos;
    while (curPos_ < curPageHeader_.endpos) {
        FtraceEventHeader eventHeader = {};
        TS_CHECK_TRUE(ReadInfo(&curPos_, endPosOfData_, &eventHeader, sizeof(FtraceEventHeader)), false,
                      "read EventHeader fail!");

        curTimestamp_ += eventHeader.timeDelta;

        bool retval = false;
        switch (eventHeader.typeLen) {
            case BUFFER_TYPE_PADDING:
                retval = HandlePaddingData(eventHeader);
                TS_CHECK_TRUE(retval, false, "parse PADDING data failed!");
                break;
            case BUFFER_TYPE_TIME_EXTEND:
                retval = HandleTimeExtend(eventHeader);
                TS_CHECK_TRUE(retval, false, "parse TIME_EXTEND failed!");
                break;
            case BUFFER_TYPE_TIME_STAMP:
                retval = HandleTimeStamp(eventHeader);
                TS_CHECK_TRUE(retval, false, "parse TIME_STAMP failed!");
                break;
            default:
                retval = HandleDataRecord(eventHeader, cpuMsg, cpuDetailParser);
                TS_CHECK_TRUE(retval, false, "parse record data failed!");
                break;
        }
        TS_LOGD("parsed %ld bytes of page data.", static_cast<long>(curPos_ - curPage_));
    }
    return true;
}

static bool IsValidIndex(int index)
{
    return index != CommonFiledIndex::INVALID_IDX;
}

bool FtraceProcessor::HandleFtraceCommonFields(FtraceEvent& ftraceEvent,
                                               uint8_t data[],
                                               size_t dataSize,
                                               const EventFormat& format)
{
    auto curIndex = format.commonIndex;

    TS_CHECK_TRUE(IsValidIndex(curIndex.pid), false, "pid curIndex %d invalid!", curIndex.pid);
    TS_CHECK_TRUE(IsValidIndex(curIndex.type), false, "type curIndex %d invalid!", curIndex.type);
    TS_CHECK_TRUE(IsValidIndex(curIndex.flags), false, "flags curIndex %d invalid!", curIndex.flags);
    TS_CHECK_TRUE(IsValidIndex(curIndex.preemt), false, "preemt curIndex %d invalid!", curIndex.preemt);

    auto fields = format.commonFields;
    auto eventCommonFields = ftraceEvent.mutable_common_fields();
    eventCommonFields->set_pid(FtraceFieldProcessor::HandleIntField<int32_t>(fields, curIndex.pid, data, dataSize));
    eventCommonFields->set_type(FtraceFieldProcessor::HandleIntField<uint32_t>(fields, curIndex.type, data, dataSize));
    eventCommonFields->set_flags(
        FtraceFieldProcessor::HandleIntField<uint32_t>(fields, curIndex.flags, data, dataSize));
    eventCommonFields->set_preempt_count(
        FtraceFieldProcessor::HandleIntField<uint32_t>(fields, curIndex.preemt, data, dataSize));
    return true;
}

bool FtraceProcessor::HandleFtraceEvent(FtraceEvent& ftraceEvent,
                                        uint8_t data[],
                                        size_t dataSize,
                                        const EventFormat& format)
{
    TS_CHECK_TRUE(dataSize >= format.eventSize, false, "dataSize not enough!");
    TS_CHECK_TRUE(HandleFtraceCommonFields(ftraceEvent, data, dataSize, format), false, "parse common fields failed!");

    int pid = ftraceEvent.common_fields().pid();
    if (pid != 0) {
        auto tgidIter = tgidDict_.find(pid);
        if (tgidIter != tgidDict_.end()) {
            ftraceEvent.set_tgid(tgidIter->second);
        } else {
            TS_LOGD("pid = %d, tgid can't find.", pid);
        }
        auto commIter = taskNameDict_.find(pid);
        if (commIter != taskNameDict_.end()) {
            ftraceEvent.set_comm(commIter->second);
        } else {
            TS_LOGD("pid = %d, taskName can't find.taskName = %s", pid, std::to_string(pid).data());
        }
        TS_LOGD("pid = %5d, tgid = %5d, event = %s", pid, ftraceEvent.tgid(), format.eventName.c_str());
    }
    FtraceEventProcessor::GetInstance().HandleEvent(ftraceEvent, data, dataSize, format);
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning
