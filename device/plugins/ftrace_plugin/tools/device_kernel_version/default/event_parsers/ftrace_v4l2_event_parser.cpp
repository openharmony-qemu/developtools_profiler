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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    v4l2_dqbuf,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_v4l2_dqbuf_format();
        msg->set_minor(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_index(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_bytesused(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_field(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timestamp(FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size));
        msg->set_timecode_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_frames(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_seconds(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_minutes(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_hours(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits0(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits1(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits2(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits3(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sequence(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    v4l2_qbuf,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_v4l2_qbuf_format();
        msg->set_minor(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_index(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_bytesused(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_field(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timestamp(FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size));
        msg->set_timecode_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_frames(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_seconds(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_minutes(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_hours(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits0(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits1(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits2(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits3(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sequence(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    vb2_v4l2_buf_done,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_vb2_v4l2_buf_done_format();
        msg->set_minor(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_field(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timestamp(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_timecode_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_frames(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_seconds(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_minutes(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_hours(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits0(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits1(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits2(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits3(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sequence(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    vb2_v4l2_buf_queue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_vb2_v4l2_buf_queue_format();
        msg->set_minor(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_field(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timestamp(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_timecode_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_frames(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_seconds(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_minutes(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_hours(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits0(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits1(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits2(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits3(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sequence(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    vb2_v4l2_dqbuf,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_vb2_v4l2_dqbuf_format();
        msg->set_minor(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_field(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timestamp(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_timecode_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_frames(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_seconds(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_minutes(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_hours(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits0(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits1(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits2(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits3(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sequence(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    vb2_v4l2_qbuf,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_vb2_v4l2_qbuf_format();
        msg->set_minor(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_field(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timestamp(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_timecode_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_frames(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_seconds(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_minutes(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_hours(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits0(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits1(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits2(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timecode_userbits3(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sequence(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
