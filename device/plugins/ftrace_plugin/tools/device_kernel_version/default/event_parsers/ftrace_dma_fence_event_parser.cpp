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
template <typename T> void dma_fence_destroy_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_timeline(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_context(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_seqno(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_destroy,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_destroy_format();
        dma_fence_destroy_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_destroy,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_destroy_format();
        dma_fence_destroy_func(msg, data, size, format);
    });

template <typename T> void dma_fence_emit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_timeline(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_context(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_seqno(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_emit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_emit_format();
        dma_fence_emit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_emit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_emit_format();
        dma_fence_emit_func(msg, data, size, format);
    });

template <typename T> void dma_fence_enable_signal_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_timeline(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_context(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_seqno(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_enable_signal,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_enable_signal_format();
        dma_fence_enable_signal_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_enable_signal,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_enable_signal_format();
        dma_fence_enable_signal_func(msg, data, size, format);
    });

template <typename T> void dma_fence_init_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_timeline(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_context(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_seqno(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_init,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_init_format();
        dma_fence_init_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_init,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_init_format();
        dma_fence_init_func(msg, data, size, format);
    });

template <typename T> void dma_fence_signaled_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_timeline(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_context(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_seqno(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_signaled,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_signaled_format();
        dma_fence_signaled_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_signaled,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_signaled_format();
        dma_fence_signaled_func(msg, data, size, format);
    });

template <typename T> void dma_fence_wait_end_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_timeline(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_context(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_seqno(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_wait_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_wait_end_format();
        dma_fence_wait_end_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_wait_end,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_wait_end_format();
        dma_fence_wait_end_func(msg, data, size, format);
    });

template <typename T> void dma_fence_wait_start_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_driver(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_timeline(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_context(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_seqno(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_wait_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_wait_start_format();
        dma_fence_wait_start_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_wait_start,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_dma_fence_wait_start_format();
        dma_fence_wait_start_func(msg, data, size, format);
    });
} // namespace
FTRACE_NS_END
