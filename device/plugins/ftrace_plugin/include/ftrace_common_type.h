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
 * Description: StreamTransporter class define
 */
#ifndef FTRACE_COMMON_TYPE_H
#define FTRACE_COMMON_TYPE_H
#include <string>
#include <vector>
#include "ftrace_namespace.h"

FTRACE_NS_BEGIN
enum PerCpuTraceStatus {
    TRACE_START = 0,
    TRACE_END = 1,
};

// refers kernel enum ring_buffer_type:
// https://github.com/torvalds/linux/blob/v4.19/include/linux/ring_buffer.h#L55
enum EventBufferType {
    BUFFER_TYPE_PADDING = 29,
    BUFFER_TYPE_TIME_EXTEND = 30,
    BUFFER_TYPE_TIME_STAMP = 31,
};

// refers kernel struct ring_buffer_event:
// https://github.com/torvalds/linux/blob/v4.19/include/linux/ring_buffer.h#L15
struct FtraceEventHeader {
    uint32_t typeLen : 5;
    uint32_t timeDelta : 27;
    uint32_t array[0];
};

// /sys/kernel/debug/tracing/events/header_page
// kernel source ring_buffer.c struct buffer_data_page
struct PageHeader {
    uint64_t timestamp = 0;
    uint64_t size = 0; // data size
    uint8_t overwrite = 0;
    uint8_t* startpos = nullptr;
    uint8_t* endpos = nullptr;
};

enum ProtoFieldType {
    PROTO_TYPE_UNKNOWN = 0,
    PROTO_TYPE_DOUBLE,
    PROTO_TYPE_FLOAT,
    PROTO_TYPE_INT64,
    PROTO_TYPE_UINT64,
    PROTO_TYPE_INT32,
    PROTO_TYPE_FIXED64,
    PROTO_TYPE_FIXED32,
    PROTO_TYPE_BOOL,
    PROTO_TYPE_STRING,
    PROTO_TYPE_GROUP, // DEPRECATED (PROTO2 ONLY)
    PROTO_TYPE_MESSAGE,
    PROTO_TYPE_BYTES,
    PROTO_TYPE_UINT32,
    PROTO_TYPE_ENUM,
    PROTO_TYPE_SFIXED32,
    PROTO_TYPE_SFIXED64,
    PROTO_TYPE_SINT32,
    PROTO_TYPE_SINT64,
    PROTO_TYPE_MAX,
};

enum EventFieldType {
    FIELD_TYPE_INVALID = 0,
    FIELD_TYPE_BOOL,
    FIELD_TYPE_INT8,
    FIELD_TYPE_UINT8,
    FIELD_TYPE_INT16,
    FIELD_TYPE_UINT16,
    FIELD_TYPE_INT32,
    FIELD_TYPE_UINT32,
    FIELD_TYPE_INT64,
    FIELD_TYPE_UINT64,
    FIELD_TYPE_FIXEDCSTRING,
    FIELD_TYPE_CSTRING,
    FIELD_TYPE_STRINGPTR,
    FIELD_TYPE_INODE32,
    FIELD_TYPE_INODE64,
    FIELD_TYPE_PID32,
    FIELD_TYPE_COMMONPID32,
    FIELD_TYPE_DEVID32,
    FIELD_TYPE_DEVID64,
    FIELD_TYPE_DATALOC,
    FIELD_TYPE_SYMADDR32,
    FIELD_TYPE_SYMADDR64,
};

// used to store content of each cpu stats data, likes /sys/kernel/debug/tracing/per_cpu/cpu0/stats.
struct PerCpuStats {
    uint64_t cpuIndex = 0;
    uint64_t entries = 0;
    uint64_t overrun = 0;
    uint64_t commitOverrun = 0;
    uint64_t bytes = 0;
    double oldestEventTs = 0.0;
    double nowTs = 0.0;
    uint64_t droppedEvents = 0;
    uint64_t readEvents = 0;
};

struct FieldFormat {
    uint16_t offset = 0;
    uint16_t size = 0;
    uint8_t isSigned = 0;
    EventFieldType filedType = FIELD_TYPE_INVALID;
    ProtoFieldType protoType = PROTO_TYPE_UNKNOWN;
    std::string name = "";
    std::string typeName = "";
};

struct CommonFiledIndex {
    static constexpr int INVALID_IDX = -1;
    int type = INVALID_IDX;
    int flags = INVALID_IDX;
    int preemt = INVALID_IDX;
    int pid = INVALID_IDX;
};

struct EventFormat {
    uint32_t eventId = 0;
    uint32_t eventSize = 0;
    std::string eventName = "";
    std::string eventType = "";
    std::vector<FieldFormat> fields = {};
    std::vector<FieldFormat> commonFields = {};
    CommonFiledIndex commonIndex = {};
};

struct PageHeaderFormat {
    FieldFormat timestamp = {};
    FieldFormat commit = {};
    FieldFormat overwrite = {};
};
FTRACE_NS_END
#endif
