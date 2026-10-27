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
struct BinderAllocLruEndData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderAllocLruEndData(BinderAllocLruEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderAllocLruEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderAllocLruEndData eventData {};
    ParseBinderAllocLruEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_lru_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_lru_end, ParseBinderAllocLruEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_lru_end, ParseBinderAllocLruEnd<ProtoEncoder::FtraceEvent>);

struct BinderAllocLruStartData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderAllocLruStartData(BinderAllocLruStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderAllocLruStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderAllocLruStartData eventData {};
    ParseBinderAllocLruStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_lru_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_lru_start, ParseBinderAllocLruStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_lru_start, ParseBinderAllocLruStart<ProtoEncoder::FtraceEvent>);

struct BinderAllocPageEndData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderAllocPageEndData(BinderAllocPageEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderAllocPageEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderAllocPageEndData eventData {};
    ParseBinderAllocPageEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_page_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_page_end, ParseBinderAllocPageEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_page_end, ParseBinderAllocPageEnd<ProtoEncoder::FtraceEvent>);

struct BinderAllocPageStartData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderAllocPageStartData(BinderAllocPageStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderAllocPageStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderAllocPageStartData eventData {};
    ParseBinderAllocPageStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_page_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_page_start, ParseBinderAllocPageStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_page_start, ParseBinderAllocPageStart<ProtoEncoder::FtraceEvent>);

struct BinderCommandData {
    uint32_t cmd {};
};
__attribute__((noinline)) void ParseBinderCommandData(BinderCommandData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cmd = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderCommand(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderCommandData eventData {};
    ParseBinderCommandData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_command_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_command, ParseBinderCommand<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_command, ParseBinderCommand<ProtoEncoder::FtraceEvent>);

struct BinderFreeLruEndData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderFreeLruEndData(BinderFreeLruEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderFreeLruEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderFreeLruEndData eventData {};
    ParseBinderFreeLruEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_free_lru_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_free_lru_end, ParseBinderFreeLruEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_free_lru_end, ParseBinderFreeLruEnd<ProtoEncoder::FtraceEvent>);

struct BinderFreeLruStartData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderFreeLruStartData(BinderFreeLruStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderFreeLruStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderFreeLruStartData eventData {};
    ParseBinderFreeLruStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_free_lru_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_free_lru_start, ParseBinderFreeLruStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_free_lru_start, ParseBinderFreeLruStart<ProtoEncoder::FtraceEvent>);

struct BinderIoctlData {
    uint32_t cmd {};
    uint64_t arg {};
};
__attribute__((noinline)) void ParseBinderIoctlData(BinderIoctlData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cmd = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.arg = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderIoctl(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderIoctlData eventData {};
    ParseBinderIoctlData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_ioctl_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
    msg->set_arg(eventData.arg);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_ioctl, ParseBinderIoctl<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_ioctl, ParseBinderIoctl<ProtoEncoder::FtraceEvent>);

struct BinderIoctlDoneData {
    int32_t ret {};
};
__attribute__((noinline)) void ParseBinderIoctlDoneData(BinderIoctlDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderIoctlDone(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderIoctlDoneData eventData {};
    ParseBinderIoctlDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_ioctl_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_ioctl_done, ParseBinderIoctlDone<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_ioctl_done, ParseBinderIoctlDone<ProtoEncoder::FtraceEvent>);

struct BinderLockData {
    std::string tag {};
};
__attribute__((noinline)) void ParseBinderLockData(BinderLockData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.tag = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderLock(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderLockData eventData {};
    ParseBinderLockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_lock_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_lock, ParseBinderLock<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_lock, ParseBinderLock<ProtoEncoder::FtraceEvent>);

struct BinderLockedData {
    std::string tag {};
};
__attribute__((noinline)) void ParseBinderLockedData(BinderLockedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.tag = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderLocked(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderLockedData eventData {};
    ParseBinderLockedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_locked_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_locked, ParseBinderLocked<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_locked, ParseBinderLocked<ProtoEncoder::FtraceEvent>);

struct BinderReadDoneData {
    int32_t ret {};
};
__attribute__((noinline)) void ParseBinderReadDoneData(BinderReadDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderReadDone(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderReadDoneData eventData {};
    ParseBinderReadDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_read_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_read_done, ParseBinderReadDone<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_read_done, ParseBinderReadDone<ProtoEncoder::FtraceEvent>);

struct BinderReturnData {
    uint32_t cmd {};
};
__attribute__((noinline)) void ParseBinderReturnData(BinderReturnData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cmd = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderReturn(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderReturnData eventData {};
    ParseBinderReturnData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_return_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_return, ParseBinderReturn<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_return, ParseBinderReturn<ProtoEncoder::FtraceEvent>);

struct BinderTransactionData {
    int32_t debug_id {};
    int32_t target_node {};
    int32_t to_proc {};
    int32_t to_thread {};
    int32_t reply {};
    uint32_t code {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseBinderTransactionData(BinderTransactionData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.target_node = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.to_proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.to_thread = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reply = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.code = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderTransaction(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderTransactionData eventData {};
    ParseBinderTransactionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_target_node(eventData.target_node);
    msg->set_to_proc(eventData.to_proc);
    msg->set_to_thread(eventData.to_thread);
    msg->set_reply(eventData.reply);
    msg->set_code(eventData.code);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction, ParseBinderTransaction<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction, ParseBinderTransaction<ProtoEncoder::FtraceEvent>);

struct BinderTransactionAllocBufData {
    int32_t debug_id {};
    uint64_t data_size {};
    uint64_t offsets_size {};
    uint64_t extra_buffers_size {};
};
__attribute__((noinline)) void ParseBinderTransactionAllocBufData(BinderTransactionAllocBufData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.data_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offsets_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.extra_buffers_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderTransactionAllocBuf(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderTransactionAllocBufData eventData {};
    ParseBinderTransactionAllocBufData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_alloc_buf_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_data_size(eventData.data_size);
    msg->set_offsets_size(eventData.offsets_size);
    msg->set_extra_buffers_size(eventData.extra_buffers_size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_alloc_buf, ParseBinderTransactionAllocBuf<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_alloc_buf, ParseBinderTransactionAllocBuf<ProtoEncoder::FtraceEvent>);

struct BinderTransactionBufferReleaseData {
    int32_t debug_id {};
    uint64_t data_size {};
    uint64_t offsets_size {};
    uint64_t extra_buffers_size {};
};
__attribute__((noinline)) void ParseBinderTransactionBufferReleaseData(BinderTransactionBufferReleaseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.data_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offsets_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.extra_buffers_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderTransactionBufferRelease(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderTransactionBufferReleaseData eventData {};
    ParseBinderTransactionBufferReleaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_buffer_release_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_data_size(eventData.data_size);
    msg->set_offsets_size(eventData.offsets_size);
    msg->set_extra_buffers_size(eventData.extra_buffers_size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_buffer_release, ParseBinderTransactionBufferRelease<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_buffer_release, ParseBinderTransactionBufferRelease<ProtoEncoder::FtraceEvent>);

struct BinderTransactionFailedBufferReleaseData {
    int32_t debug_id {};
    uint64_t data_size {};
    uint64_t offsets_size {};
    uint64_t extra_buffers_size {};
};
__attribute__((noinline)) void ParseBinderTransactionFailedBufferReleaseData(BinderTransactionFailedBufferReleaseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.data_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offsets_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.extra_buffers_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderTransactionFailedBufferRelease(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderTransactionFailedBufferReleaseData eventData {};
    ParseBinderTransactionFailedBufferReleaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_failed_buffer_release_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_data_size(eventData.data_size);
    msg->set_offsets_size(eventData.offsets_size);
    msg->set_extra_buffers_size(eventData.extra_buffers_size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_failed_buffer_release, ParseBinderTransactionFailedBufferRelease<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_failed_buffer_release, ParseBinderTransactionFailedBufferRelease<ProtoEncoder::FtraceEvent>);

struct BinderTransactionNodeToRefData {
    int32_t debug_id {};
    int32_t node_debug_id {};
    uint64_t node_ptr {};
    int32_t ref_debug_id {};
    uint32_t ref_desc {};
};
__attribute__((noinline)) void ParseBinderTransactionNodeToRefData(BinderTransactionNodeToRefData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.node_debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.node_ptr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ref_debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ref_desc = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderTransactionNodeToRef(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderTransactionNodeToRefData eventData {};
    ParseBinderTransactionNodeToRefData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_node_to_ref_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_node_debug_id(eventData.node_debug_id);
    msg->set_node_ptr(eventData.node_ptr);
    msg->set_ref_debug_id(eventData.ref_debug_id);
    msg->set_ref_desc(eventData.ref_desc);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_node_to_ref, ParseBinderTransactionNodeToRef<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_node_to_ref, ParseBinderTransactionNodeToRef<ProtoEncoder::FtraceEvent>);

struct BinderTransactionReceivedData {
    int32_t debug_id {};
};
__attribute__((noinline)) void ParseBinderTransactionReceivedData(BinderTransactionReceivedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderTransactionReceived(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderTransactionReceivedData eventData {};
    ParseBinderTransactionReceivedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_received_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_received, ParseBinderTransactionReceived<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_received, ParseBinderTransactionReceived<ProtoEncoder::FtraceEvent>);

struct BinderTransactionRefToNodeData {
    int32_t debug_id {};
    int32_t ref_debug_id {};
    uint32_t ref_desc {};
    int32_t node_debug_id {};
    uint64_t node_ptr {};
};
__attribute__((noinline)) void ParseBinderTransactionRefToNodeData(BinderTransactionRefToNodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ref_debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ref_desc = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.node_debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.node_ptr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderTransactionRefToNode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderTransactionRefToNodeData eventData {};
    ParseBinderTransactionRefToNodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_ref_to_node_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_ref_debug_id(eventData.ref_debug_id);
    msg->set_ref_desc(eventData.ref_desc);
    msg->set_node_debug_id(eventData.node_debug_id);
    msg->set_node_ptr(eventData.node_ptr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_ref_to_node, ParseBinderTransactionRefToNode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_ref_to_node, ParseBinderTransactionRefToNode<ProtoEncoder::FtraceEvent>);

struct BinderTransactionRefToRefData {
    int32_t debug_id {};
    int32_t node_debug_id {};
    int32_t src_ref_debug_id {};
    uint32_t src_ref_desc {};
    int32_t dest_ref_debug_id {};
    uint32_t dest_ref_desc {};
};
__attribute__((noinline)) void ParseBinderTransactionRefToRefData(BinderTransactionRefToRefData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.node_debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_ref_debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_ref_desc = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.dest_ref_debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dest_ref_desc = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderTransactionRefToRef(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderTransactionRefToRefData eventData {};
    ParseBinderTransactionRefToRefData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_ref_to_ref_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_node_debug_id(eventData.node_debug_id);
    msg->set_src_ref_debug_id(eventData.src_ref_debug_id);
    msg->set_src_ref_desc(eventData.src_ref_desc);
    msg->set_dest_ref_debug_id(eventData.dest_ref_debug_id);
    msg->set_dest_ref_desc(eventData.dest_ref_desc);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_ref_to_ref, ParseBinderTransactionRefToRef<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_ref_to_ref, ParseBinderTransactionRefToRef<ProtoEncoder::FtraceEvent>);

struct BinderUnlockData {
    std::string tag {};
};
__attribute__((noinline)) void ParseBinderUnlockData(BinderUnlockData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.tag = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderUnlock(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderUnlockData eventData {};
    ParseBinderUnlockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unlock_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unlock, ParseBinderUnlock<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unlock, ParseBinderUnlock<ProtoEncoder::FtraceEvent>);

struct BinderUnmapKernelEndData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderUnmapKernelEndData(BinderUnmapKernelEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderUnmapKernelEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderUnmapKernelEndData eventData {};
    ParseBinderUnmapKernelEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_kernel_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_kernel_end, ParseBinderUnmapKernelEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_kernel_end, ParseBinderUnmapKernelEnd<ProtoEncoder::FtraceEvent>);

struct BinderUnmapKernelStartData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderUnmapKernelStartData(BinderUnmapKernelStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderUnmapKernelStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderUnmapKernelStartData eventData {};
    ParseBinderUnmapKernelStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_kernel_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_kernel_start, ParseBinderUnmapKernelStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_kernel_start, ParseBinderUnmapKernelStart<ProtoEncoder::FtraceEvent>);

struct BinderUnmapUserEndData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderUnmapUserEndData(BinderUnmapUserEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderUnmapUserEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderUnmapUserEndData eventData {};
    ParseBinderUnmapUserEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_user_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_user_end, ParseBinderUnmapUserEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_user_end, ParseBinderUnmapUserEnd<ProtoEncoder::FtraceEvent>);

struct BinderUnmapUserStartData {
    int32_t proc {};
    uint64_t page_index {};
};
__attribute__((noinline)) void ParseBinderUnmapUserStartData(BinderUnmapUserStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.page_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderUnmapUserStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderUnmapUserStartData eventData {};
    ParseBinderUnmapUserStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_user_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_user_start, ParseBinderUnmapUserStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_user_start, ParseBinderUnmapUserStart<ProtoEncoder::FtraceEvent>);

struct BinderUpdatePageRangeData {
    int32_t proc {};
    uint32_t allocate {};
    uint64_t offset {};
    uint64_t size {};
};
__attribute__((noinline)) void ParseBinderUpdatePageRangeData(BinderUpdatePageRangeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.allocate = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderUpdatePageRange(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderUpdatePageRangeData eventData {};
    ParseBinderUpdatePageRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_update_page_range_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_allocate(eventData.allocate);
    msg->set_offset(eventData.offset);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_update_page_range, ParseBinderUpdatePageRange<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_update_page_range, ParseBinderUpdatePageRange<ProtoEncoder::FtraceEvent>);

struct BinderWaitForWorkData {
    uint32_t proc_work {};
    uint32_t transaction_stack {};
    uint32_t thread_todo {};
};
__attribute__((noinline)) void ParseBinderWaitForWorkData(BinderWaitForWorkData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.proc_work = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.transaction_stack = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.thread_todo = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderWaitForWork(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderWaitForWorkData eventData {};
    ParseBinderWaitForWorkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_wait_for_work_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc_work(eventData.proc_work);
    msg->set_transaction_stack(eventData.transaction_stack);
    msg->set_thread_todo(eventData.thread_todo);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_wait_for_work, ParseBinderWaitForWork<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_wait_for_work, ParseBinderWaitForWork<ProtoEncoder::FtraceEvent>);

struct BinderWriteDoneData {
    int32_t ret {};
};
__attribute__((noinline)) void ParseBinderWriteDoneData(BinderWriteDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBinderWriteDone(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BinderWriteDoneData eventData {};
    ParseBinderWriteDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_write_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_write_done, ParseBinderWriteDone<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_write_done, ParseBinderWriteDone<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
