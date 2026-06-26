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
void ParseF2fsSyncFileEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    F2fsSyncFileEnterData eventData {};
    ParseF2fsSyncFileEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_sync_file_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pino(eventData.pino);
    msg->set_mode(eventData.mode);
    msg->set_size(eventData.size);
    msg->set_nlink(eventData.nlink);
    msg->set_blocks(eventData.blocks);
    msg->set_advise(eventData.advise);
}
void ParseF2fsSyncFileEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    F2fsSyncFileEnterData eventData {};
    ParseF2fsSyncFileEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_sync_file_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pino(eventData.pino);
    msg->set_mode(eventData.mode);
    msg->set_size(eventData.size);
    msg->set_nlink(eventData.nlink);
    msg->set_blocks(eventData.blocks);
    msg->set_advise(eventData.advise);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(f2fs_sync_file_enter, ParseF2fsSyncFileEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(f2fs_sync_file_enter, ParseF2fsSyncFileEnterEncoder);

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
void ParseF2fsSyncFileExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    F2fsSyncFileExitData eventData {};
    ParseF2fsSyncFileExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_sync_file_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_cp_reason(eventData.cp_reason);
    msg->set_datasync(eventData.datasync);
    msg->set_ret(eventData.ret);
}
void ParseF2fsSyncFileExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    F2fsSyncFileExitData eventData {};
    ParseF2fsSyncFileExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_sync_file_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_cp_reason(eventData.cp_reason);
    msg->set_datasync(eventData.datasync);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(f2fs_sync_file_exit, ParseF2fsSyncFileExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(f2fs_sync_file_exit, ParseF2fsSyncFileExitEncoder);

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
void ParseF2fsWriteBegin(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    F2fsWriteBeginData eventData {};
    ParseF2fsWriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_write_begin_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
void ParseF2fsWriteBeginEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    F2fsWriteBeginData eventData {};
    ParseF2fsWriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_write_begin_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(f2fs_write_begin, ParseF2fsWriteBegin);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(f2fs_write_begin, ParseF2fsWriteBeginEncoder);

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
void ParseF2fsWriteEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    F2fsWriteEndData eventData {};
    ParseF2fsWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_write_end_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
void ParseF2fsWriteEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    F2fsWriteEndData eventData {};
    ParseF2fsWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_f2fs_write_end_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(f2fs_write_end, ParseF2fsWriteEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(f2fs_write_end, ParseF2fsWriteEndEncoder);
} // namespace
FTRACE_NS_END
