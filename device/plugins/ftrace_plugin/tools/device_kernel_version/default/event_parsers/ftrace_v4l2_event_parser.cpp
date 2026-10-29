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
struct V4l2DqbufData {
    int32_t minor {};
    uint32_t index {};
    uint32_t type {};
    uint32_t bytesused {};
    uint32_t flags {};
    uint32_t field {};
    int64_t timestamp {};
    uint32_t timecode_type {};
    uint32_t timecode_flags {};
    uint32_t timecode_frames {};
    uint32_t timecode_seconds {};
    uint32_t timecode_minutes {};
    uint32_t timecode_hours {};
    uint32_t timecode_userbits0 {};
    uint32_t timecode_userbits1 {};
    uint32_t timecode_userbits2 {};
    uint32_t timecode_userbits3 {};
    uint32_t sequence {};
};
__attribute__((noinline)) void ParseV4l2DqbufData(V4l2DqbufData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.minor = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.bytesused = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.field = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timestamp = FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size);
    eventData.timecode_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_frames = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_seconds = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_minutes = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_hours = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits0 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits1 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits2 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits3 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sequence = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseV4l2Dqbuf(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    V4l2DqbufData eventData {};
    ParseV4l2DqbufData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_v4l2_dqbuf_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_minor(eventData.minor);
    msg->set_index(eventData.index);
    msg->set_type(eventData.type);
    msg->set_bytesused(eventData.bytesused);
    msg->set_flags(eventData.flags);
    msg->set_field(eventData.field);
    msg->set_timestamp(eventData.timestamp);
    msg->set_timecode_type(eventData.timecode_type);
    msg->set_timecode_flags(eventData.timecode_flags);
    msg->set_timecode_frames(eventData.timecode_frames);
    msg->set_timecode_seconds(eventData.timecode_seconds);
    msg->set_timecode_minutes(eventData.timecode_minutes);
    msg->set_timecode_hours(eventData.timecode_hours);
    msg->set_timecode_userbits0(eventData.timecode_userbits0);
    msg->set_timecode_userbits1(eventData.timecode_userbits1);
    msg->set_timecode_userbits2(eventData.timecode_userbits2);
    msg->set_timecode_userbits3(eventData.timecode_userbits3);
    msg->set_sequence(eventData.sequence);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(v4l2_dqbuf, ParseV4l2Dqbuf<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(v4l2_dqbuf, ParseV4l2Dqbuf<ProtoEncoder::FtraceEvent>);

struct V4l2QbufData {
    int32_t minor {};
    uint32_t index {};
    uint32_t type {};
    uint32_t bytesused {};
    uint32_t flags {};
    uint32_t field {};
    int64_t timestamp {};
    uint32_t timecode_type {};
    uint32_t timecode_flags {};
    uint32_t timecode_frames {};
    uint32_t timecode_seconds {};
    uint32_t timecode_minutes {};
    uint32_t timecode_hours {};
    uint32_t timecode_userbits0 {};
    uint32_t timecode_userbits1 {};
    uint32_t timecode_userbits2 {};
    uint32_t timecode_userbits3 {};
    uint32_t sequence {};
};
__attribute__((noinline)) void ParseV4l2QbufData(V4l2QbufData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.minor = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.bytesused = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.field = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timestamp = FtraceFieldParser::ParseIntField<int64_t>(format.fields, i++, data, size);
    eventData.timecode_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_frames = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_seconds = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_minutes = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_hours = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits0 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits1 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits2 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits3 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sequence = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseV4l2Qbuf(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    V4l2QbufData eventData {};
    ParseV4l2QbufData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_v4l2_qbuf_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_minor(eventData.minor);
    msg->set_index(eventData.index);
    msg->set_type(eventData.type);
    msg->set_bytesused(eventData.bytesused);
    msg->set_flags(eventData.flags);
    msg->set_field(eventData.field);
    msg->set_timestamp(eventData.timestamp);
    msg->set_timecode_type(eventData.timecode_type);
    msg->set_timecode_flags(eventData.timecode_flags);
    msg->set_timecode_frames(eventData.timecode_frames);
    msg->set_timecode_seconds(eventData.timecode_seconds);
    msg->set_timecode_minutes(eventData.timecode_minutes);
    msg->set_timecode_hours(eventData.timecode_hours);
    msg->set_timecode_userbits0(eventData.timecode_userbits0);
    msg->set_timecode_userbits1(eventData.timecode_userbits1);
    msg->set_timecode_userbits2(eventData.timecode_userbits2);
    msg->set_timecode_userbits3(eventData.timecode_userbits3);
    msg->set_sequence(eventData.sequence);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(v4l2_qbuf, ParseV4l2Qbuf<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(v4l2_qbuf, ParseV4l2Qbuf<ProtoEncoder::FtraceEvent>);

struct Vb2V4l2BufDoneData {
    int32_t minor {};
    uint32_t flags {};
    uint32_t field {};
    uint64_t timestamp {};
    uint32_t timecode_type {};
    uint32_t timecode_flags {};
    uint32_t timecode_frames {};
    uint32_t timecode_seconds {};
    uint32_t timecode_minutes {};
    uint32_t timecode_hours {};
    uint32_t timecode_userbits0 {};
    uint32_t timecode_userbits1 {};
    uint32_t timecode_userbits2 {};
    uint32_t timecode_userbits3 {};
    uint32_t sequence {};
};
__attribute__((noinline)) void ParseVb2V4l2BufDoneData(Vb2V4l2BufDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.minor = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.field = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timestamp = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.timecode_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_frames = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_seconds = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_minutes = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_hours = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits0 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits1 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits2 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits3 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sequence = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseVb2V4l2BufDone(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Vb2V4l2BufDoneData eventData {};
    ParseVb2V4l2BufDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_vb2_v4l2_buf_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_minor(eventData.minor);
    msg->set_flags(eventData.flags);
    msg->set_field(eventData.field);
    msg->set_timestamp(eventData.timestamp);
    msg->set_timecode_type(eventData.timecode_type);
    msg->set_timecode_flags(eventData.timecode_flags);
    msg->set_timecode_frames(eventData.timecode_frames);
    msg->set_timecode_seconds(eventData.timecode_seconds);
    msg->set_timecode_minutes(eventData.timecode_minutes);
    msg->set_timecode_hours(eventData.timecode_hours);
    msg->set_timecode_userbits0(eventData.timecode_userbits0);
    msg->set_timecode_userbits1(eventData.timecode_userbits1);
    msg->set_timecode_userbits2(eventData.timecode_userbits2);
    msg->set_timecode_userbits3(eventData.timecode_userbits3);
    msg->set_sequence(eventData.sequence);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(vb2_v4l2_buf_done, ParseVb2V4l2BufDone<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(vb2_v4l2_buf_done, ParseVb2V4l2BufDone<ProtoEncoder::FtraceEvent>);

struct Vb2V4l2BufQueueData {
    int32_t minor {};
    uint32_t flags {};
    uint32_t field {};
    uint64_t timestamp {};
    uint32_t timecode_type {};
    uint32_t timecode_flags {};
    uint32_t timecode_frames {};
    uint32_t timecode_seconds {};
    uint32_t timecode_minutes {};
    uint32_t timecode_hours {};
    uint32_t timecode_userbits0 {};
    uint32_t timecode_userbits1 {};
    uint32_t timecode_userbits2 {};
    uint32_t timecode_userbits3 {};
    uint32_t sequence {};
};
__attribute__((noinline)) void ParseVb2V4l2BufQueueData(Vb2V4l2BufQueueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.minor = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.field = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timestamp = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.timecode_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_frames = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_seconds = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_minutes = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_hours = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits0 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits1 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits2 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits3 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sequence = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseVb2V4l2BufQueue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Vb2V4l2BufQueueData eventData {};
    ParseVb2V4l2BufQueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_vb2_v4l2_buf_queue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_minor(eventData.minor);
    msg->set_flags(eventData.flags);
    msg->set_field(eventData.field);
    msg->set_timestamp(eventData.timestamp);
    msg->set_timecode_type(eventData.timecode_type);
    msg->set_timecode_flags(eventData.timecode_flags);
    msg->set_timecode_frames(eventData.timecode_frames);
    msg->set_timecode_seconds(eventData.timecode_seconds);
    msg->set_timecode_minutes(eventData.timecode_minutes);
    msg->set_timecode_hours(eventData.timecode_hours);
    msg->set_timecode_userbits0(eventData.timecode_userbits0);
    msg->set_timecode_userbits1(eventData.timecode_userbits1);
    msg->set_timecode_userbits2(eventData.timecode_userbits2);
    msg->set_timecode_userbits3(eventData.timecode_userbits3);
    msg->set_sequence(eventData.sequence);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(vb2_v4l2_buf_queue, ParseVb2V4l2BufQueue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(vb2_v4l2_buf_queue, ParseVb2V4l2BufQueue<ProtoEncoder::FtraceEvent>);

struct Vb2V4l2DqbufData {
    int32_t minor {};
    uint32_t flags {};
    uint32_t field {};
    uint64_t timestamp {};
    uint32_t timecode_type {};
    uint32_t timecode_flags {};
    uint32_t timecode_frames {};
    uint32_t timecode_seconds {};
    uint32_t timecode_minutes {};
    uint32_t timecode_hours {};
    uint32_t timecode_userbits0 {};
    uint32_t timecode_userbits1 {};
    uint32_t timecode_userbits2 {};
    uint32_t timecode_userbits3 {};
    uint32_t sequence {};
};
__attribute__((noinline)) void ParseVb2V4l2DqbufData(Vb2V4l2DqbufData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.minor = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.field = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timestamp = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.timecode_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_frames = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_seconds = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_minutes = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_hours = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits0 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits1 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits2 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits3 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sequence = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseVb2V4l2Dqbuf(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Vb2V4l2DqbufData eventData {};
    ParseVb2V4l2DqbufData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_vb2_v4l2_dqbuf_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_minor(eventData.minor);
    msg->set_flags(eventData.flags);
    msg->set_field(eventData.field);
    msg->set_timestamp(eventData.timestamp);
    msg->set_timecode_type(eventData.timecode_type);
    msg->set_timecode_flags(eventData.timecode_flags);
    msg->set_timecode_frames(eventData.timecode_frames);
    msg->set_timecode_seconds(eventData.timecode_seconds);
    msg->set_timecode_minutes(eventData.timecode_minutes);
    msg->set_timecode_hours(eventData.timecode_hours);
    msg->set_timecode_userbits0(eventData.timecode_userbits0);
    msg->set_timecode_userbits1(eventData.timecode_userbits1);
    msg->set_timecode_userbits2(eventData.timecode_userbits2);
    msg->set_timecode_userbits3(eventData.timecode_userbits3);
    msg->set_sequence(eventData.sequence);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(vb2_v4l2_dqbuf, ParseVb2V4l2Dqbuf<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(vb2_v4l2_dqbuf, ParseVb2V4l2Dqbuf<ProtoEncoder::FtraceEvent>);

struct Vb2V4l2QbufData {
    int32_t minor {};
    uint32_t flags {};
    uint32_t field {};
    uint64_t timestamp {};
    uint32_t timecode_type {};
    uint32_t timecode_flags {};
    uint32_t timecode_frames {};
    uint32_t timecode_seconds {};
    uint32_t timecode_minutes {};
    uint32_t timecode_hours {};
    uint32_t timecode_userbits0 {};
    uint32_t timecode_userbits1 {};
    uint32_t timecode_userbits2 {};
    uint32_t timecode_userbits3 {};
    uint32_t sequence {};
};
__attribute__((noinline)) void ParseVb2V4l2QbufData(Vb2V4l2QbufData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.minor = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.field = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timestamp = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.timecode_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_frames = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_seconds = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_minutes = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_hours = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits0 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits1 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits2 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timecode_userbits3 = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sequence = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseVb2V4l2Qbuf(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Vb2V4l2QbufData eventData {};
    ParseVb2V4l2QbufData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_vb2_v4l2_qbuf_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_minor(eventData.minor);
    msg->set_flags(eventData.flags);
    msg->set_field(eventData.field);
    msg->set_timestamp(eventData.timestamp);
    msg->set_timecode_type(eventData.timecode_type);
    msg->set_timecode_flags(eventData.timecode_flags);
    msg->set_timecode_frames(eventData.timecode_frames);
    msg->set_timecode_seconds(eventData.timecode_seconds);
    msg->set_timecode_minutes(eventData.timecode_minutes);
    msg->set_timecode_hours(eventData.timecode_hours);
    msg->set_timecode_userbits0(eventData.timecode_userbits0);
    msg->set_timecode_userbits1(eventData.timecode_userbits1);
    msg->set_timecode_userbits2(eventData.timecode_userbits2);
    msg->set_timecode_userbits3(eventData.timecode_userbits3);
    msg->set_sequence(eventData.sequence);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(vb2_v4l2_qbuf, ParseVb2V4l2Qbuf<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(vb2_v4l2_qbuf, ParseVb2V4l2Qbuf<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
