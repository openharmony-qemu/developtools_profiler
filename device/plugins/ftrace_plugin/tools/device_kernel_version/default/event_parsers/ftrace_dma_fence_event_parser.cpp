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
void ParseDmaFenceDestroy(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceDestroyData eventData {};
    ParseDmaFenceDestroyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_destroy_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
void ParseDmaFenceDestroyEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceDestroyData eventData {};
    ParseDmaFenceDestroyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_destroy_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_destroy, ParseDmaFenceDestroy);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_destroy, ParseDmaFenceDestroyEncoder);

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
void ParseDmaFenceEmit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceEmitData eventData {};
    ParseDmaFenceEmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_emit_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
void ParseDmaFenceEmitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceEmitData eventData {};
    ParseDmaFenceEmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_emit_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_emit, ParseDmaFenceEmit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_emit, ParseDmaFenceEmitEncoder);

struct DmaFenceEnableSignalData {
    std::string driver {};
    std::string timeline {};
    uint32_t context {};
    uint32_t seqno {};
};
__attribute__((noinline)) void ParseDmaFenceEnableSignalData(DmaFenceEnableSignalData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.driver = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.timeline = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.context = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseDmaFenceEnableSignal(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceEnableSignalData eventData {};
    ParseDmaFenceEnableSignalData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_enable_signal_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
void ParseDmaFenceEnableSignalEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceEnableSignalData eventData {};
    ParseDmaFenceEnableSignalData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_enable_signal_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_enable_signal, ParseDmaFenceEnableSignal);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_enable_signal, ParseDmaFenceEnableSignalEncoder);

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
void ParseDmaFenceInit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceInitData eventData {};
    ParseDmaFenceInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_init_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
void ParseDmaFenceInitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceInitData eventData {};
    ParseDmaFenceInitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_init_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_init, ParseDmaFenceInit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_init, ParseDmaFenceInitEncoder);

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
void ParseDmaFenceSignaled(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceSignaledData eventData {};
    ParseDmaFenceSignaledData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_signaled_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
void ParseDmaFenceSignaledEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceSignaledData eventData {};
    ParseDmaFenceSignaledData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_signaled_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_signaled, ParseDmaFenceSignaled);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_signaled, ParseDmaFenceSignaledEncoder);

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
void ParseDmaFenceWaitEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceWaitEndData eventData {};
    ParseDmaFenceWaitEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_wait_end_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
void ParseDmaFenceWaitEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceWaitEndData eventData {};
    ParseDmaFenceWaitEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_wait_end_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_wait_end, ParseDmaFenceWaitEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_wait_end, ParseDmaFenceWaitEndEncoder);

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
void ParseDmaFenceWaitStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceWaitStartData eventData {};
    ParseDmaFenceWaitStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_wait_start_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
void ParseDmaFenceWaitStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    DmaFenceWaitStartData eventData {};
    ParseDmaFenceWaitStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_dma_fence_wait_start_format();
    msg->set_driver(eventData.driver);
    msg->set_timeline(eventData.timeline);
    msg->set_context(eventData.context);
    msg->set_seqno(eventData.seqno);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(dma_fence_wait_start, ParseDmaFenceWaitStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(dma_fence_wait_start, ParseDmaFenceWaitStartEncoder);
} // namespace
FTRACE_NS_END
