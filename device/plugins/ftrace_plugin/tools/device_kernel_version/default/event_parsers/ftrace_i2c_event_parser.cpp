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
struct I2cReadData {
    int32_t adapter_nr {};
    uint32_t msg_nr {};
    uint32_t addr {};
    uint32_t flags {};
    uint32_t len {};
};
__attribute__((noinline)) void ParseI2cReadData(I2cReadData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.adapter_nr = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.msg_nr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseI2cRead(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    I2cReadData eventData {};
    ParseI2cReadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_i2c_read_format();
    msg->set_adapter_nr(eventData.adapter_nr);
    msg->set_msg_nr(eventData.msg_nr);
    msg->set_addr(eventData.addr);
    msg->set_flags(eventData.flags);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(i2c_read, ParseI2cRead<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(i2c_read, ParseI2cRead<ProtoEncoder::FtraceEvent>);

struct I2cReplyData {
    int32_t adapter_nr {};
    uint32_t msg_nr {};
    uint32_t addr {};
    uint32_t flags {};
    uint32_t len {};
    uint32_t buf {};
};
__attribute__((noinline)) void ParseI2cReplyData(I2cReplyData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.adapter_nr = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.msg_nr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.buf = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseI2cReply(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    I2cReplyData eventData {};
    ParseI2cReplyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_i2c_reply_format();
    msg->set_adapter_nr(eventData.adapter_nr);
    msg->set_msg_nr(eventData.msg_nr);
    msg->set_addr(eventData.addr);
    msg->set_flags(eventData.flags);
    msg->set_len(eventData.len);
    msg->set_buf(eventData.buf);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(i2c_reply, ParseI2cReply<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(i2c_reply, ParseI2cReply<ProtoEncoder::FtraceEvent>);

struct I2cResultData {
    int32_t adapter_nr {};
    uint32_t nr_msgs {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseI2cResultData(I2cResultData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.adapter_nr = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.nr_msgs = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseI2cResult(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    I2cResultData eventData {};
    ParseI2cResultData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_i2c_result_format();
    msg->set_adapter_nr(eventData.adapter_nr);
    msg->set_nr_msgs(eventData.nr_msgs);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(i2c_result, ParseI2cResult<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(i2c_result, ParseI2cResult<ProtoEncoder::FtraceEvent>);

struct I2cWriteData {
    int32_t adapter_nr {};
    uint32_t msg_nr {};
    uint32_t addr {};
    uint32_t flags {};
    uint32_t len {};
    uint32_t buf {};
};
__attribute__((noinline)) void ParseI2cWriteData(I2cWriteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.adapter_nr = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.msg_nr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.buf = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseI2cWrite(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    I2cWriteData eventData {};
    ParseI2cWriteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_i2c_write_format();
    msg->set_adapter_nr(eventData.adapter_nr);
    msg->set_msg_nr(eventData.msg_nr);
    msg->set_addr(eventData.addr);
    msg->set_flags(eventData.flags);
    msg->set_len(eventData.len);
    msg->set_buf(eventData.buf);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(i2c_write, ParseI2cWrite<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(i2c_write, ParseI2cWrite<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
