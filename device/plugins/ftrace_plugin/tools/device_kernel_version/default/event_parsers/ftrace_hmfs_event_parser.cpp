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

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
template <typename T> void hmfs_read_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_off(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hmfs_read_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_hmfs_read_enter_format();
        hmfs_read_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hmfs_read_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_hmfs_read_enter_format();
        hmfs_read_enter_func(msg, data, size, format);
    });

template <typename T> void hmfs_read_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_off(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_res(FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hmfs_read_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_hmfs_read_exit_format();
        hmfs_read_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hmfs_read_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_hmfs_read_exit_format();
        hmfs_read_exit_func(msg, data, size, format);
    });

template <typename T> void hmfs_write_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_off(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hmfs_write_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_hmfs_write_enter_format();
        hmfs_write_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hmfs_write_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_hmfs_write_enter_format();
        hmfs_write_enter_func(msg, data, size, format);
    });

template <typename T> void hmfs_write_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_off(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_res(FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(hmfs_write_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_hmfs_write_exit_format();
        hmfs_write_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(hmfs_write_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_hmfs_write_exit_format();
        hmfs_write_exit_func(msg, data, size, format);
    });
} // namespace
FTRACE_NS_END