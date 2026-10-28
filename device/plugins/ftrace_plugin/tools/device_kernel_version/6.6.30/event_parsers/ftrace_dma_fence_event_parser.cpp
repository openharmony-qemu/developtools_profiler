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
struct DmaFenceDestroyData {
    std::string driver {};
    std::string timeline {};
    uint32_t context {};
    uint32_t seqno {};
};
__attribute__((noinline)) void ParseDmaFenceDestroyData(DmaFenceDestroyData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.timeline = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.context = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseDmaFenceDestroy(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    DmaFenceDestroyData eventData {};
    ParseDmaFenceDestroyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_destroy_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_destroy, ParseDmaFenceDestroy<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_destroy, ParseDmaFenceDestroy<ProtoEncoder::FtraceEvent>);

struct DmaFenceEmitData {
    std::string driver {};
    std::string timeline {};
    uint32_t context {};
    uint32_t seqno {};
};
__attribute__((noinline)) void ParseDmaFenceEmitData(DmaFenceEmitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.timeline = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.context = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseDmaFenceEmit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    DmaFenceEmitData eventData {};
    ParseDmaFenceEmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_emit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_emit, ParseDmaFenceEmit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_emit, ParseDmaFenceEmit<ProtoEncoder::FtraceEvent>);

struct DmaFenceEnableSignalData {
    std::string driver {};
    std::string timeline {};
    uint32_t context {};
    uint32_t seqno {};
};
__attribute__((noinline)) void ParseDmaFenceEnableSignalData(
    DmaFenceEnableSignalData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.timeline = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.context = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseDmaFenceEnableSignal(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    DmaFenceEnableSignalData eventData {};
    ParseDmaFenceEnableSignalData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_enable_signal_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_enable_signal, ParseDmaFenceEnableSignal<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_enable_signal,
    ParseDmaFenceEnableSignal<ProtoEncoder::FtraceEvent>);

struct DmaFenceInitData {
    std::string driver {};
    std::string timeline {};
    uint32_t context {};
    uint32_t seqno {};
};
__attribute__((noinline)) void ParseDmaFenceInitData(DmaFenceInitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.timeline = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.context = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseDmaFenceInit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    DmaFenceInitData eventData {};
    ParseDmaFenceInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_init_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_init, ParseDmaFenceInit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_init, ParseDmaFenceInit<ProtoEncoder::FtraceEvent>);

struct DmaFenceSignaledData {
    std::string driver {};
    std::string timeline {};
    uint32_t context {};
    uint32_t seqno {};
};
__attribute__((noinline)) void ParseDmaFenceSignaledData(DmaFenceSignaledData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.timeline = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.context = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseDmaFenceSignaled(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    DmaFenceSignaledData eventData {};
    ParseDmaFenceSignaledData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_signaled_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_signaled, ParseDmaFenceSignaled<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_signaled, ParseDmaFenceSignaled<ProtoEncoder::FtraceEvent>);

struct DmaFenceWaitEndData {
    std::string driver {};
    std::string timeline {};
    uint32_t context {};
    uint32_t seqno {};
};
__attribute__((noinline)) void ParseDmaFenceWaitEndData(DmaFenceWaitEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.timeline = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.context = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseDmaFenceWaitEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    DmaFenceWaitEndData eventData {};
    ParseDmaFenceWaitEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_wait_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_wait_end, ParseDmaFenceWaitEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_wait_end, ParseDmaFenceWaitEnd<ProtoEncoder::FtraceEvent>);

struct DmaFenceWaitStartData {
    std::string driver {};
    std::string timeline {};
    uint32_t context {};
    uint32_t seqno {};
};
__attribute__((noinline)) void ParseDmaFenceWaitStartData(DmaFenceWaitStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.timeline = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.context = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseDmaFenceWaitStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    DmaFenceWaitStartData eventData {};
    ParseDmaFenceWaitStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_wait_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_wait_start, ParseDmaFenceWaitStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_wait_start, ParseDmaFenceWaitStart<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
