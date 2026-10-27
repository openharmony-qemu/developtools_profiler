/* THIS FILE IS GENERATE BY ftrace_cpp_generator.py, PLEASE DON'T EDIT IT!
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
 */
#include "sub_event_parser.h"

#include <string>
#include <vector>

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
struct F2fsSyncFileEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pino {};
    uint32_t mode {};
    uint64_t size {};
    uint32_t nlink {};
    uint64_t blocks {};
    uint32_t advise {};
};
__attribute__((noinline)) void ParseF2fsSyncFileEnterData(F2fsSyncFileEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nlink = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.blocks = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.advise = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseF2fsSyncFileEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    F2fsSyncFileEnterData eventData {};
    ParseF2fsSyncFileEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_sync_file_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pino(eventData.pino);
    msg->set_mode(eventData.mode);
    msg->set_size(eventData.size);
    msg->set_nlink(eventData.nlink);
    msg->set_blocks(eventData.blocks);
    msg->set_advise(eventData.advise);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(f2fs_sync_file_enter, ParseF2fsSyncFileEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(f2fs_sync_file_enter, ParseF2fsSyncFileEnter<ProtoEncoder::FtraceEvent>);

struct F2fsSyncFileExitData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t cp_reason {};
    int32_t datasync {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseF2fsSyncFileExitData(F2fsSyncFileExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cp_reason = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.datasync = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseF2fsSyncFileExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    F2fsSyncFileExitData eventData {};
    ParseF2fsSyncFileExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_sync_file_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_cp_reason(eventData.cp_reason);
    msg->set_datasync(eventData.datasync);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(f2fs_sync_file_exit, ParseF2fsSyncFileExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(f2fs_sync_file_exit, ParseF2fsSyncFileExit<ProtoEncoder::FtraceEvent>);

struct F2fsWriteBeginData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseF2fsWriteBeginData(F2fsWriteBeginData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseF2fsWriteBegin(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    F2fsWriteBeginData eventData {};
    ParseF2fsWriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_write_begin_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(f2fs_write_begin, ParseF2fsWriteBegin<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(f2fs_write_begin, ParseF2fsWriteBegin<ProtoEncoder::FtraceEvent>);

struct F2fsWriteEndData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
    uint32_t copied {};
};
__attribute__((noinline)) void ParseF2fsWriteEndData(F2fsWriteEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.copied = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseF2fsWriteEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    F2fsWriteEndData eventData {};
    ParseF2fsWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_write_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(f2fs_write_end, ParseF2fsWriteEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(f2fs_write_end, ParseF2fsWriteEnd<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
