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
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <regex>
#include <sstream>
#include <unistd.h>

#include "common.h"
#include "file_utils.h"
#include "securec.h"
#include "string_utils.h"

#ifdef HILOG_DEBUG
#undef HILOG_DEBUG
#endif

#define HILOG_DEBUG(LOG_CORE, fmt, ...) \
    if (debugOn_) { \
        PROFILER_LOG_INFO(LOG_CORE, ":DEBUG: " fmt, ##__VA_ARGS__); \
    }

namespace {
using namespace OHOS::Developtools::Profiler;
constexpr unsigned RB_MISSED_EVENTS = (1uL << 31); // Flag when events were overwritten
constexpr unsigned RB_MISSED_STORED = (1 << 30);   // Missed count stored at end
constexpr unsigned RB_MISSED_FLAGS = (RB_MISSED_EVENTS | RB_MISSED_STORED);

constexpr unsigned COL_IDX_NAME = 0;
constexpr unsigned COL_IDX_VALUE = 1;

constexpr unsigned TS_EXT_SHIFT = 27;
constexpr uint32_t INT_MAX_LEN = 10;

inline uint64_t GetTimestampIncrements(uint64_t ext)
{
    return ext << TS_EXT_SHIFT;
}
} // namespace

FTRACE_NS_BEGIN
FtraceParser::FtraceParser()
{
    PROFILER_LOG_INFO(LOG_CORE, "FtraceParser create!");
}

bool FtraceParser::Init()
{
    fixedCharArrayRegex_ = std::regex(R"(char \w+\[\d+\])");
    flexDataLocArrayRegex_ = std::regex(R"(__data_loc [a-zA-Z_0-9 ]+\[\] \w+)");
    if (FtraceFsOps::GetInstance().IsHmKernel()) {
        return true;
    }
    std::string printkFormats = FtraceFsOps::GetInstance().GetPrintkFormats();
    CHECK_TRUE(printkFormats.size() > 0, false, "read printk_formats failed!");
    CHECK_TRUE(PrintkFormatsParser::GetInstance().Parse(printkFormats), false, "parse printk_formats failed");

    std::string formatDesc = FtraceFsOps::GetInstance().GetPageHeaderFormat();
    CHECK_TRUE(formatDesc.size() > 0, false, "read header_page failed!");
    osVersion_ = FtraceFsOps::GetInstance().GetKernelVersion();
    return ParseHeaderPageFormat(formatDesc);
}

FtraceParser::~FtraceParser()
{
    PROFILER_LOG_INFO(LOG_CORE, "FtraceParser destroy!");
}

bool FtraceParser::SetupEvent(const std::string& type, const std::string& name)
{
    if (!SubEventParser<FtraceEvent>::GetInstance().IsSupport(name)) {
        // no sub event parser found for event, so no need to parse format file
        return false;
    }

    EventFormat format;
    format.eventType = type;
    format.eventName = name;
    std::string desc = FtraceFsOps::GetInstance().GetEventDataFormat(type, name);
    if (desc != "") {
        CHECK_TRUE(ParseEventFormat(desc.data(), format), false, "parse %s/%s/format failed!",
            type.c_str(), name.c_str());
        CHECK_TRUE(SubEventParser<FtraceEvent>::GetInstance().SetupEvent(format),
            false, "setup %s/%s failed!", type.c_str(), name.c_str());
        CHECK_TRUE(SubEventParser<ProtoEncoder::FtraceEvent>::GetInstance().SetupEvent(format),
            false, "setup pbzero %s/%s failed!", type.c_str(), name.c_str());
    }
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
            if (COMMON::IsNumeric(idStr)) {
                format.eventId = static_cast<uint32_t>(atoi(idStr.c_str()));
            }
        }
    }
    CHECK_TRUE(format.fields.size() > 0, false, "ParseEventFormat failed!");
    size_t lastFiledIndex = format.fields.size() > 1 ? format.fields.size() - 1 : 0;
    format.eventSize = format.fields[lastFiledIndex].offset + format.fields[lastFiledIndex].size;
    return true;
}

static std::string SplitNameFromTypeName(const std::string& typeName)
{
    std::string name;
    if (typeName.size() > 0) { // split type and name
        auto posT0 = typeName.rfind(" ");
        std::string rightHalf = typeName.substr(posT0 + 1);
        size_t dataIndex = rightHalf.size() > 1 ? rightHalf.size() - 1 : 0;
        if (rightHalf[dataIndex] != ']') {
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
    if (COMMON::IsNumeric(offsetStr)) {
        fieldInfo.offset = atoi(offsetStr.c_str());
    }
    if (COMMON::IsNumeric(sizeStr)) {
        fieldInfo.size = atoi(sizeStr.c_str());
    }
    if (COMMON::IsNumeric(signedStr)) {
        fieldInfo.isSigned = atoi(signedStr.c_str());
    }

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
        PROFILER_LOG_WARN(LOG_CORE, "ParseSavedTgid: parsed tigds: %zu", tgidDict_.size());
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
        if (pos != std::string::npos && pos < INT_MAX_LEN) {
            auto pidStr = line.substr(0, pos);
            pid = COMMON::IsNumeric(pidStr) ? std::stoi(pidStr) : 0;
            comm = line.substr(pos + 1);
            commDict_[pid] = comm;
            retval = true;
        }
    }

    if (commDict_.size() == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "ParseSavedCmdlines: parsed cmdlines: %zu", commDict_.size());
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
    PROFILER_LOG_INFO(LOG_CORE, "ParseTimeExtend: update ts with %u to %" PRIu64, deltaExt, timestamp_);
    return true;
}

bool FtraceParser::ParseTimeStamp(const FtraceEventHeader& eventHeader)
{
    uint32_t deltaExt = 0;
    CHECK_TRUE(ReadInc(&cur_, endOfData_, &deltaExt, sizeof(deltaExt)), false, "read time delta failed!");

    // refers kernel function rb_update_write_stamp in ring_buffer.c
    timestamp_ = eventHeader.timeDelta + GetTimestampIncrements(deltaExt);
    PROFILER_LOG_INFO(LOG_CORE, "ParseTimeStamp: update ts with %u to %" PRIu64, deltaExt, timestamp_);
    return true;
}

bool FtraceParser::ReadInc(uint8_t* start[], uint8_t end[], void* outData, size_t outSize)
{
    if ((end - *start) < static_cast<ptrdiff_t>(outSize)) {
        return false;
    }
    CHECK_TRUE(memcpy_s(outData, outSize, *start, outSize) == EOK, false,
               "read %zu bytes from memory region FAILED", outSize);
    *start += outSize;
    return true;
}

bool FtraceParser::IsValidIndex(int idx)
{
    return idx != CommonFiledIndex::INVALID_IDX;
}

void FtraceParser::SetDebugOn(bool value)
{
    debugOn_ = value;
    PROFILER_LOG_INFO(LOG_CORE, "debugOption: %s", debugOn_ ? "true" : "false");
}
FTRACE_NS_END
