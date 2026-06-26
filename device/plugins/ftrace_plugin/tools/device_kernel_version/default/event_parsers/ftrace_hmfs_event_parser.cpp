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
struct HmfsReadEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t off {};
    uint64_t size {};
    uint64_t i_size {};
    std::string name {};
};
__attribute__((noinline)) void ParseHmfsReadEnterData(HmfsReadEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.off = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseHmfsReadEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HmfsReadEnterData eventData {};
    ParseHmfsReadEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hmfs_read_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_off(eventData.off);
    msg->set_size(eventData.size);
    msg->set_i_size(eventData.i_size);
    msg->set_name(eventData.name);
}
void ParseHmfsReadEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HmfsReadEnterData eventData {};
    ParseHmfsReadEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hmfs_read_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_off(eventData.off);
    msg->set_size(eventData.size);
    msg->set_i_size(eventData.i_size);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hmfs_read_enter, ParseHmfsReadEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hmfs_read_enter, ParseHmfsReadEnterEncoder);

struct HmfsReadExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t off {};
    uint64_t size {};
    int64_t res {};
};
__attribute__((noinline)) void ParseHmfsReadExitData(HmfsReadExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.off = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.res = FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size);
}
void ParseHmfsReadExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HmfsReadExitData eventData {};
    ParseHmfsReadExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hmfs_read_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_off(eventData.off);
    msg->set_size(eventData.size);
    msg->set_res(eventData.res);
}
void ParseHmfsReadExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HmfsReadExitData eventData {};
    ParseHmfsReadExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hmfs_read_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_off(eventData.off);
    msg->set_size(eventData.size);
    msg->set_res(eventData.res);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hmfs_read_exit, ParseHmfsReadExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hmfs_read_exit, ParseHmfsReadExitEncoder);

struct HmfsWriteEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t off {};
    uint64_t size {};
    uint64_t i_size {};
    std::string name {};
};
__attribute__((noinline)) void ParseHmfsWriteEnterData(HmfsWriteEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.off = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseHmfsWriteEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HmfsWriteEnterData eventData {};
    ParseHmfsWriteEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hmfs_write_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_off(eventData.off);
    msg->set_size(eventData.size);
    msg->set_i_size(eventData.i_size);
    msg->set_name(eventData.name);
}
void ParseHmfsWriteEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HmfsWriteEnterData eventData {};
    ParseHmfsWriteEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hmfs_write_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_off(eventData.off);
    msg->set_size(eventData.size);
    msg->set_i_size(eventData.i_size);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hmfs_write_enter, ParseHmfsWriteEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hmfs_write_enter, ParseHmfsWriteEnterEncoder);

struct HmfsWriteExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t off {};
    uint64_t size {};
    int64_t res {};
};
__attribute__((noinline)) void ParseHmfsWriteExitData(HmfsWriteExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.off = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.res = FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size);
}
void ParseHmfsWriteExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HmfsWriteExitData eventData {};
    ParseHmfsWriteExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hmfs_write_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_off(eventData.off);
    msg->set_size(eventData.size);
    msg->set_res(eventData.res);
}
void ParseHmfsWriteExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    HmfsWriteExitData eventData {};
    ParseHmfsWriteExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_hmfs_write_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_off(eventData.off);
    msg->set_size(eventData.size);
    msg->set_res(eventData.res);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hmfs_write_exit, ParseHmfsWriteExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hmfs_write_exit, ParseHmfsWriteExitEncoder);
} // namespace
FTRACE_NS_END