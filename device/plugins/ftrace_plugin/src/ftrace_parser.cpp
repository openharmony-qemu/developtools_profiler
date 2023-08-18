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
 * Description: FtraceParser class implements
 */
#include "ftrace_parser.h"

#include <algorithm>
#include <cerrno>
#include <cinttypes>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <regex>
#include <sstream>
#include <unistd.h>

#include "file_utils.h"
#include "ftrace_fs_ops.h"
#include "logging.h"
#include "printk_formats_parser.h"
#include "securec.h"
#include "string_utils.h"
#include "sub_event_parser.h"

#ifdef HILOG_DEBUG
#undef HILOG_DEBUG
#endif

#define HILOG_DEBUG(LOG_CORE, fmt, ...) \
    if (debugOn_) { \
        HILOG_INFO(LOG_CORE, ":DEBUG: " fmt, ##__VA_ARGS__); \
    }

namespace {
constexpr unsigned RB_MISSED_EVENTS = (1uL << 31); // Flag when events were overwritten
constexpr unsigned RB_MISSED_STORED = (1 << 30);   // Missed count stored at end
constexpr unsigned RB_MISSED_FLAGS = (RB_MISSED_EVENTS | RB_MISSED_STORED);

constexpr unsigned COL_IDX_NAME = 0;
constexpr unsigned COL_IDX_VALUE = 1;

constexpr unsigned TS_EXT_SHIFT = 27;

inline uint64_t GetTimestampIncrements(uint64_t ext)
{
    return ext << TS_EXT_SHIFT;
}

bool ReadInc(uint8_t* start[], const uint8_t end[], void* outData, size_t outSize)
{
    if ((end - *start) < static_cast<ptrdiff_t>(outSize)) {
        return false;
    }
    if (memcpy_s(outData, outSize, *start, outSize) != EOK) {
        HILOG_ERROR(LOG_CORE, "read %zu bytes from memory FAILED", outSize);
        return false;
    }
    *start += outSize;
    return true;
}
} // namespace

FTRACE_NS_BEGIN
FtraceParser::FtraceParser()
{
    HILOG_INFO(LOG_CORE, "FtraceParser create!");
}

bool FtraceParser::Init()
{
    fixedCharArrayRegex_ = std::regex(R"(char \w+\[\d+\])");
    flexDataLocArrayRegex_ = std::regex(R"(__data_loc [a-zA-Z_0-9 ]+\[\] \w+)");

    std::string printkFormats = FtraceFsOps::GetInstance().GetPrintkFormats();
    CHECK_TRUE(printkFormats.size() > 0, false, "read printk_formats failed!");
    CHECK_TRUE(PrintkFormatsParser::GetInstance().Parse(printkFormats), false, "parse printk_formats failed");

    std::string formatDesc = FtraceFsOps::GetInstance().GetPageHeaderFormat();
    CHECK_TRUE(formatDesc.size() > 0, false, "read header_page failed!");
    return ParseHeaderPageFormat(formatDesc);
}

FtraceParser::~FtraceParser()
{
    HILOG_INFO(LOG_CORE, "FtraceParser destroy!");
}

bool FtraceParser::SetupEvent(const std::string& type, const std::string& name)
{
    if (!SubEventParser::GetInstance().IsSupport(name)) {
        // no sub event parser found for event, so no need to parse format file
        return false;
    }

    EventFormat format;
    format.eventType = type;
    format.eventName = name;
    std::string desc = FtraceFsOps::GetInstance().GetEventDataFormat(type, name);
    CHECK_TRUE(ParseEventFormat(desc.data(), format), false, "parse %s/%s/format failed!", type.c_str(), name.c_str());
    CHECK_TRUE(SubEventParser::GetInstance().SetupEvent(format), false, "setup %s/%s failed!", type.c_str(),
               name.c_str());

    return true;
}

bool FtraceParser::ParseHeaderPageFormat(const std::string& formatDesc)
{
    EventFormat format = {};
    CHECK_TRUE(ParseEventFormat(formatDesc, format), false, "parse events/header_page failed!");

    bool commitFound = false;
    for (auto& field : format.fields) {
        if (field.name == "timestamp") {
            pageHeaderFormat_.timestamp = field;
        } else if (field.name == "commit") {
            pageHeaderFormat_.commit = field;
            commitFound = true;
        } else if (field.name == "overwrite") {
            pageHeaderFormat_.overwrite = field;
        }
    }

    HILOG_INFO(LOG_CORE, "page header info:");
    PrintFieldInfo(pageHeaderFormat_.timestamp);
    PrintFieldInfo(pageHeaderFormat_.commit);
    PrintFieldInfo(pageHeaderFormat_.overwrite);
    CHECK_TRUE(commitFound, false, "commit field not found!");
    return true;
}

int FtraceParser::GetHeaderPageCommitSize(void)
{
    // return the size value of commit field read from events/header_page
    return pageHeaderFormat_.commit.size;
}

bool FtraceParser::ParseEventFormat(const std::string& formatDesc, EventFormat& format)
{
    std::string idLinePrefix = "ID:";
    std::string fieldLinePrefix = "field:";
    std::string printFmtLinePrefix = "print fmt:";

    std::string line;
    std::stringstream sin(formatDesc);
    while (getline(sin, line)) {
        line = StringUtils::Strip(line);
        if (line.empty()) {
            continue;
        } else if (StringUtils::StartsWith(line, fieldLinePrefix)) {
            ParseFieldFormat(line, format);
        } else if (StringUtils::StartsWith(line, idLinePrefix)) {
            auto idStr = line.substr(idLinePrefix.size() + 1);
            format.eventId = static_cast<uint32_t>(atoi(idStr.c_str()));
        }
    }

    CHECK_TRUE(format.fields.size() > 0, false, "ParseEventFormat from %s failed!", formatDesc.c_str());
    int lastFiledIndex = format.fields.size() - 1;
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

void FtraceParser::PrintFieldInfo(const FieldFormat& info)
{
    UNUSED_PARAMETER(GetProtoTypeName);
    UNUSED_PARAMETER(GetFieldTypeName);
    HILOG_DEBUG(LOG_CORE,
                "FieldFormat { offset: %u, size:%u, sign: %u fieldType: %s, protoType:%s, typeName: %s, name: %s}",
                info.offset, info.size, info.isSigned, GetFieldTypeName(info.filedType).c_str(),
                GetProtoTypeName(info.protoType).c_str(), info.typeName.c_str(), info.name.c_str());
}

static std::string SplitNameFromTypeName(const std::string& typeName)
{
    std::string name;
    if (typeName.size() > 0) { // split type and name
        auto posT0 = typeName.rfind(" ");
        std::string rightHalf = typeName.substr(posT0 + 1);
        if (rightHalf[rightHalf.size() - 1] != ']') {
            name = rightHalf;
        } else {
            std::string::size_type postT1 = rightHalf.rfind('[');
            if (postT1 == std::string::npos) {
                return "";
            }
            name = rightHalf.substr(0, postT1);
        }
    }
    return name;
}

static std::string EraseNameFromTypeName(const std::string& typeName, const std::string& name)
{
    std::string type;
    if (name.size() > 0) { // erase name part from typeName
        type = typeName;
        auto pos = type.find(name);
        type.replace(pos, name.size(), "");
        type = StringUtils::Strip(type);
    }
    return type;
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

bool FtraceParser::ParseFieldFormat(const std::string& fieldLine, EventFormat& format)
{
    FieldFormat fieldInfo;
    std::string typeName;
    std::string offsetStr;
    std::string sizeStr;
    std::string signedStr;

    for (auto& part : StringUtils::Split(fieldLine, ";")) {
        auto cols = StringUtils::Split(StringUtils::Strip(part), ":");
        if (cols.size() < COL_IDX_VALUE) {
            continue;
        }
        const auto& key = cols[COL_IDX_NAME];
        if (key == "field") {
            typeName = cols[COL_IDX_VALUE];
        } else if (key == "offset") {
            offsetStr = cols[COL_IDX_VALUE];
        } else if (key == "size") {
            sizeStr = cols[COL_IDX_VALUE];
        } else if (key == "signed") {
            signedStr = cols[COL_IDX_VALUE];
        }
    }

    std::string name = SplitNameFromTypeName(typeName);
    std::string type = EraseNameFromTypeName(typeName, name); // for field type
    fieldInfo.name = name;
    fieldInfo.typeName = typeName;
    fieldInfo.offset = atoi(offsetStr.c_str());
    fieldInfo.size = atoi(sizeStr.c_str());
    fieldInfo.isSigned = atoi(signedStr.c_str());

    ParseFieldType(type, fieldInfo);
    ParseProtoType(fieldInfo);

    if (StringUtils::StartsWith(name, "common_")) {
        ParseCommonFiledIndex(format.commonIndex, name, static_cast<int>(format.commonFields.size()));
        format.commonFields.push_back(fieldInfo);
    } else {
        format.fields.push_back(fieldInfo);
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

static bool ParseKernelAddrField(FieldFormat& field, const std::string& type)
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

bool FtraceParser::ParseFieldType(const std::string& type, FieldFormat& field)
{
    const std::string& typeName = field.typeName;
    // Fixed size C char arrary, likes "char a[LEN]"
    if (std::regex_match(typeName, fixedCharArrayRegex_)) {
        field.filedType = FIELD_TYPE_FIXEDCSTRING;
        return true;
    }

    // for flex array with __data_loc mark, likes: __data_loc char[] name; __data_loc __u8[] buf;
    if (std::regex_match(typeName, flexDataLocArrayRegex_)) {
        CHECK_TRUE(field.size == sizeof(uint32_t), false, "__data_loc %s, size: %hu", typeName.c_str(), field.size);
        field.filedType = FIELD_TYPE_DATALOC;
        return true;
    }

    if ((typeName.find("char[]") != std::string::npos) || (typeName.find("char *") != std::string::npos)) {
        field.filedType = FIELD_TYPE_STRINGPTR;
        return true;
    }

    // Variable length strings: "char foo" + size: 0 (as in 'print').
    if ((type == "char" || type == "char []") && field.size == 0) {
        field.filedType = FIELD_TYPE_CSTRING;
        return true;
    }

    // 64-bit kernel addresses
    if (ParseKernelAddrField(field, type)) {
        return true;
    }

    if (ParseSepcialIntType(field, type, typeName)) {
        return true;
    }

    // int uint:
    if (ParseCommonIntType(field, field.isSigned)) {
        return true;
    }
    return false;
}

void FtraceParser::ParseProtoType(FieldFormat& field)
{
    switch (field.filedType) {
        case FIELD_TYPE_CSTRING:
        case FIELD_TYPE_FIXEDCSTRING:
        case FIELD_TYPE_STRINGPTR:
        case FIELD_TYPE_DATALOC:
            field.protoType = PROTO_TYPE_STRING;
            break;
        case FIELD_TYPE_INT8:
        case FIELD_TYPE_INT16:
        case FIELD_TYPE_INT32:
        case FIELD_TYPE_PID32:
        case FIELD_TYPE_COMMONPID32:
            field.protoType = PROTO_TYPE_INT32;
            break;
        case FIELD_TYPE_INT64:
            field.protoType = PROTO_TYPE_INT64;
            break;
        case FIELD_TYPE_UINT8:
        case FIELD_TYPE_UINT16:
        case FIELD_TYPE_UINT32:
        case FIELD_TYPE_BOOL:
        case FIELD_TYPE_DEVID32:
        case FIELD_TYPE_SYMADDR32:
            field.protoType = PROTO_TYPE_UINT32;
            break;
        case FIELD_TYPE_DEVID64:
        case FIELD_TYPE_UINT64:
        case FIELD_TYPE_INODE32:
        case FIELD_TYPE_INODE64:
        case FIELD_TYPE_SYMADDR64:
            field.protoType = PROTO_TYPE_UINT64;
            break;
        case FIELD_TYPE_INVALID:
            field.protoType = PROTO_TYPE_UNKNOWN;
            break;
        default:
            break;
    }
}

bool FtraceParser::ParsePerCpuStatus(PerCpuStats& stats, const std::string& perCpuStats)
{
    std::string line;
    std::stringstream input(perCpuStats);

    int count = 0;
    while (getline(input, line, '\n')) {
        std::string sep = ": ";
        size_t pos = line.rfind(sep);
        if (pos == std::string::npos) {
            continue;
        }
        std::stringstream ss(line.substr(pos + sep.size()));
        std::string name = line.substr(0, pos);
        if (name == "entries") {
            ss >> stats.entries;
            count++;
        } else if (name == "overrun") {
            ss >> stats.overrun;
            count++;
        } else if (name == "commit overrun") {
            ss >> stats.commitOverrun;
            count++;
        } else if (name == "bytes") {
            ss >> stats.bytes;
            count++;
        } else if (name == "oldest event ts") {
            ss >> stats.oldestEventTs;
            count++;
        } else if (name == "now ts") {
            ss >> stats.nowTs;
            count++;
        } else if (name == "dropped events") {
            ss >> stats.droppedEvents;
            count++;
        } else if (name == "read events") {
            ss >> stats.readEvents;
            count++;
        }
    }
    return count > 0;
}

// parse kernel ring buffer page header data
bool FtraceParser::ParsePageHeader()
{
    // read time stamp
    uint64_t timestamp = 0;
    CHECK_TRUE(ReadInc(&cur_, endOfPage_, &timestamp, sizeof(timestamp)), false, "read timestamp from page failed!");
    pageHeader_.timestamp = timestamp;

    // read data size and overwriten flags
    uint64_t commit = 0;
    const int commitSize = GetHeaderPageCommitSize(); // 8B on 64bit device, 4B on 32bit device
    CHECK_TRUE(ReadInc(&cur_, endOfPage_, &commit, commitSize), false, "read commit to page header failed!");

    // refers kernel function ring_buffer_page_len:
    pageHeader_.size = (commit & ~RB_MISSED_FLAGS);
    pageHeader_.overwrite = (commit & RB_MISSED_EVENTS);

    pageHeader_.startpos = cur_;
    pageHeader_.endpos = cur_ + pageHeader_.size;
    return true;
}

// parse /sys/kernel/debug/tracing/saved_tgids
// refers kernel function saved_tgids_show
bool FtraceParser::ParseSavedTgid(const std::string& savedTgid)
{
    int32_t pid = 0;
    int32_t tgid = 0;
    std::stringstream sin(savedTgid);
    // kernel format code with: "%d %d\n"
    while (sin >> pid >> tgid) {
        tgidDict_[pid] = tgid;
    }

    if (tgidDict_.size() == 0) {
        HILOG_WARN(LOG_CORE, "ParseSavedTgid: parsed tigds: %zu", tgidDict_.size());
    }
    return true;
}

// parse /sys/kernel/debug/tracing/saved_cmdlines
// refers kernel function saved_cmdlines_show
bool FtraceParser::ParseSavedCmdlines(const std::string& savedCmdlines)
{
    bool retval = false;
    int32_t pid;
    std::string comm;
    std::string line;
    std::stringstream sin(savedCmdlines);
    while (std::getline(sin, line)) {
        // kernel format with: "%d %s\n"
        auto pos = line.find(' ');
        if (pos != std::string::npos) {
            pid = std::stoi(line.substr(0, pos));
            comm = line.substr(pos + 1);
            commDict_[pid] = comm;
            retval = true;
        }
    }

    if (commDict_.size() == 0) {
        HILOG_WARN(LOG_CORE, "ParseSavedCmdlines: parsed cmdlines: %zu", commDict_.size());
    }
    return retval;
}

bool FtraceParser::ParsePaddingData(const FtraceEventHeader& eventHeader)
{
    if (eventHeader.timeDelta == 0) {
        return false;
    }
    uint32_t paddingLength;
    CHECK_TRUE(ReadInc(&cur_, endOfData_, &paddingLength, sizeof(paddingLength)), false, "read padding len failed!");

    // skip padding data
    cur_ += paddingLength;
    return true;
}

bool FtraceParser::ParseTimeExtend(const FtraceEventHeader& eventHeader)
{
    uint32_t deltaExt = 0;
    CHECK_TRUE(ReadInc(&cur_, endOfData_, &deltaExt, sizeof(deltaExt)), false, "read time delta failed!");

    timestamp_ += GetTimestampIncrements(deltaExt);
    HILOG_INFO(LOG_CORE, "ParseTimeExtend: update ts with %u to %" PRIu64, deltaExt, timestamp_);
    return true;
}

bool FtraceParser::ParseTimeStamp(const FtraceEventHeader& eventHeader)
{
    uint32_t deltaExt = 0;
    CHECK_TRUE(ReadInc(&cur_, endOfData_, &deltaExt, sizeof(deltaExt)), false, "read time delta failed!");

    // refers kernel function rb_update_write_stamp in ring_buffer.c
    timestamp_ = eventHeader.timeDelta + GetTimestampIncrements(deltaExt);
    HILOG_INFO(LOG_CORE, "ParseTimeStamp: update ts with %u to %" PRIu64, deltaExt, timestamp_);
    return true;
}

bool FtraceParser::ParseDataRecord(const FtraceEventHeader& eventHeader, FtraceCpuDetailMsg& cpuMsg)
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
    HILOG_DEBUG(LOG_CORE, "ParseDataRecord: parse %u bytes of event data...", evtSize);

    uint8_t* evStart = cur_;
    uint8_t* evEnd = cur_ + evtSize;
    uint16_t evId = 0;
    CHECK_TRUE(ReadInc(&cur_, evEnd, &evId, sizeof(evId)), false, "read event ID failed!");

    uint32_t eventId = evId;
    SubEventParser::ParseEventCtx* parseEventCtx = SubEventParser::GetInstance().GetParseEventCtx(eventId);
    if (parseEventCtx == nullptr) {
        HILOG_DEBUG(LOG_CORE, "event %u not supported!", eventId);
    } else {
        HILOG_DEBUG(LOG_CORE, "ParseDataRecord: eventId = %u, name = %s",
                    eventId, parseEventCtx->format.eventName.c_str());
        auto ftraceEvent = cpuMsg.add_event();
        ftraceEvent->set_timestamp(timestamp_);
        ParseFtraceEvent(*ftraceEvent, evStart, evtSize, parseEventCtx);
    }
    cur_ = evEnd;
    return true;
}

bool FtraceParser::ParsePage(FtraceCpuDetailMsg& cpuMsg, uint8_t page[], size_t size)
{
    cur_ = page;
    page_ = page;
    endOfPage_ = page + size;

    ParsePageHeader();
    HILOG_DEBUG(LOG_CORE, "ParsePage: %" PRIu64 " bytes event data in page!", pageHeader_.size);
    cpuMsg.set_overwrite(pageHeader_.overwrite);

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
                retval = ParseDataRecord(eventHeader, cpuMsg);
                CHECK_TRUE(retval, false, "parse record data failed!");
                break;
        }
        HILOG_DEBUG(LOG_CORE, "parsed %ld bytes of page data.", static_cast<long>(cur_ - page_));
    }
    return true;
}

static bool IsValidIndex(int idx)
{
    return idx != CommonFiledIndex::INVALID_IDX;
}

bool FtraceParser::ParseFtraceCommonFields(FtraceEvent& ftraceEvent,
                                           uint8_t data[],
                                           size_t dataSize,
                                           const EventFormat& format)
{
    auto& index = format.commonIndex;

    CHECK_TRUE(IsValidIndex(index.pid), false, "pid index %d invalid!", index.pid);
    CHECK_TRUE(IsValidIndex(index.type), false, "type index %d invalid!", index.type);
    CHECK_TRUE(IsValidIndex(index.flags), false, "flags index %d invalid!", index.flags);
    CHECK_TRUE(IsValidIndex(index.preemt), false, "preemt index %d invalid!", index.preemt);

    auto commonFields = ftraceEvent.mutable_common_fields();
    commonFields->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.commonFields, index.pid, data, dataSize));
    commonFields->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.commonFields, index.type, data, dataSize));
    commonFields->set_flags(
        FtraceFieldParser::ParseIntField<uint32_t>(format.commonFields, index.flags, data, dataSize));
    commonFields->set_preempt_count(
        FtraceFieldParser::ParseIntField<uint32_t>(format.commonFields, index.preemt, data, dataSize));
    return true;
}

bool FtraceParser::ParseFtraceEvent(FtraceEvent& ftraceEvent,
                                    uint8_t data[],
                                    size_t dataSize,
                                    const SubEventParser::ParseEventCtx* parseEventCtx)
{
    CHECK_TRUE(dataSize >= parseEventCtx->format.eventSize, false,
               "FtraceParser::ParseFtraceEvent, dataSize not enough!");
    CHECK_TRUE(ParseFtraceCommonFields(ftraceEvent, data, dataSize, parseEventCtx->format), false,
               "parse common fields failed!");

    int pid = ftraceEvent.common_fields().pid();
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

        HILOG_DEBUG(LOG_CORE, "pid = %5d, tgid = %5d, comm = %16s, event = %s", pid, tgid, comm.c_str(),
                    parseEventCtx->format.eventName.c_str());
    }

    SubEventParser::GetInstance().ParseEvent(ftraceEvent, data, dataSize, parseEventCtx);
    return true;
}

void FtraceParser::SetDebugOn(bool value)
{
    debugOn_ = value;
    HILOG_INFO(LOG_CORE, "debugOption: %s", debugOn_ ? "true" : "false");
}
FTRACE_NS_END
