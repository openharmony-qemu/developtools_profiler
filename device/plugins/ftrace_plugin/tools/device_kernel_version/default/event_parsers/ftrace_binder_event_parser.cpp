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
void ParseBinderAllocLruEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderAllocLruEndData eventData {};
    ParseBinderAllocLruEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_lru_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderAllocLruEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderAllocLruEndData eventData {};
    ParseBinderAllocLruEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_lru_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_lru_end, ParseBinderAllocLruEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_lru_end, ParseBinderAllocLruEndEncoder);

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
void ParseBinderAllocLruStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderAllocLruStartData eventData {};
    ParseBinderAllocLruStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_lru_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderAllocLruStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderAllocLruStartData eventData {};
    ParseBinderAllocLruStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_lru_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_lru_start, ParseBinderAllocLruStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_lru_start, ParseBinderAllocLruStartEncoder);

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
void ParseBinderAllocPageEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderAllocPageEndData eventData {};
    ParseBinderAllocPageEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_page_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderAllocPageEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderAllocPageEndData eventData {};
    ParseBinderAllocPageEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_page_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_page_end, ParseBinderAllocPageEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_page_end, ParseBinderAllocPageEndEncoder);

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
void ParseBinderAllocPageStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderAllocPageStartData eventData {};
    ParseBinderAllocPageStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_page_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderAllocPageStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderAllocPageStartData eventData {};
    ParseBinderAllocPageStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_alloc_page_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_page_start, ParseBinderAllocPageStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_page_start, ParseBinderAllocPageStartEncoder);

struct BinderCommandData {
    uint32_t cmd {};
};
__attribute__((noinline)) void ParseBinderCommandData(BinderCommandData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cmd = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseBinderCommand(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderCommandData eventData {};
    ParseBinderCommandData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_command_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
}
void ParseBinderCommandEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderCommandData eventData {};
    ParseBinderCommandData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_command_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_command, ParseBinderCommand);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_command, ParseBinderCommandEncoder);

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
void ParseBinderFreeLruEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderFreeLruEndData eventData {};
    ParseBinderFreeLruEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_free_lru_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderFreeLruEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderFreeLruEndData eventData {};
    ParseBinderFreeLruEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_free_lru_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_free_lru_end, ParseBinderFreeLruEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_free_lru_end, ParseBinderFreeLruEndEncoder);

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
void ParseBinderFreeLruStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderFreeLruStartData eventData {};
    ParseBinderFreeLruStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_free_lru_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderFreeLruStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderFreeLruStartData eventData {};
    ParseBinderFreeLruStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_free_lru_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_free_lru_start, ParseBinderFreeLruStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_free_lru_start, ParseBinderFreeLruStartEncoder);

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
void ParseBinderIoctl(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderIoctlData eventData {};
    ParseBinderIoctlData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_ioctl_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
    msg->set_arg(eventData.arg);
}
void ParseBinderIoctlEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderIoctlData eventData {};
    ParseBinderIoctlData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_ioctl_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
    msg->set_arg(eventData.arg);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_ioctl, ParseBinderIoctl);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_ioctl, ParseBinderIoctlEncoder);

struct BinderIoctlDoneData {
    int32_t ret {};
};
__attribute__((noinline)) void ParseBinderIoctlDoneData(BinderIoctlDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseBinderIoctlDone(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderIoctlDoneData eventData {};
    ParseBinderIoctlDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_ioctl_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
void ParseBinderIoctlDoneEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderIoctlDoneData eventData {};
    ParseBinderIoctlDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_ioctl_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_ioctl_done, ParseBinderIoctlDone);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_ioctl_done, ParseBinderIoctlDoneEncoder);

struct BinderLockData {
    std::string tag {};
};
__attribute__((noinline)) void ParseBinderLockData(BinderLockData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.tag = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBinderLock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderLockData eventData {};
    ParseBinderLockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_lock_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
void ParseBinderLockEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderLockData eventData {};
    ParseBinderLockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_lock_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_lock, ParseBinderLock);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_lock, ParseBinderLockEncoder);

struct BinderLockedData {
    std::string tag {};
};
__attribute__((noinline)) void ParseBinderLockedData(BinderLockedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.tag = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBinderLocked(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderLockedData eventData {};
    ParseBinderLockedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_locked_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
void ParseBinderLockedEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderLockedData eventData {};
    ParseBinderLockedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_locked_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_locked, ParseBinderLocked);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_locked, ParseBinderLockedEncoder);

struct BinderReadDoneData {
    int32_t ret {};
};
__attribute__((noinline)) void ParseBinderReadDoneData(BinderReadDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseBinderReadDone(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderReadDoneData eventData {};
    ParseBinderReadDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_read_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
void ParseBinderReadDoneEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderReadDoneData eventData {};
    ParseBinderReadDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_read_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_read_done, ParseBinderReadDone);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_read_done, ParseBinderReadDoneEncoder);

struct BinderReturnData {
    uint32_t cmd {};
};
__attribute__((noinline)) void ParseBinderReturnData(BinderReturnData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cmd = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseBinderReturn(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderReturnData eventData {};
    ParseBinderReturnData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_return_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
}
void ParseBinderReturnEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderReturnData eventData {};
    ParseBinderReturnData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_return_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_return, ParseBinderReturn);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_return, ParseBinderReturnEncoder);

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
    int8_t reply = FtraceFieldParser::ParseIntField<int8_t>(format.fields, i++, data, size);
    eventData.reply = static_cast<int32_t>(reply);
    eventData.code = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseBinderTransaction(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderTransactionEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction, ParseBinderTransaction);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction, ParseBinderTransactionEncoder);

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
void ParseBinderTransactionAllocBuf(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderTransactionAllocBufEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_alloc_buf, ParseBinderTransactionAllocBuf);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_alloc_buf, ParseBinderTransactionAllocBufEncoder);

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
void ParseBinderTransactionBufferRelease(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderTransactionBufferReleaseEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_buffer_release, ParseBinderTransactionBufferRelease);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_buffer_release, ParseBinderTransactionBufferReleaseEncoder);

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
void ParseBinderTransactionFailedBufferRelease(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderTransactionFailedBufferReleaseEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_failed_buffer_release, ParseBinderTransactionFailedBufferRelease);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_failed_buffer_release, ParseBinderTransactionFailedBufferReleaseEncoder);

struct BinderTransactionFdData {
    int32_t debug_id {};
    int32_t src_fd {};
    int32_t dest_fd {};
};
__attribute__((noinline)) void ParseBinderTransactionFdData(BinderTransactionFdData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.src_fd = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dest_fd = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseBinderTransactionFd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderTransactionFdData eventData {};
    ParseBinderTransactionFdData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_fd_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_src_fd(eventData.src_fd);
    msg->set_dest_fd(eventData.dest_fd);
}
void ParseBinderTransactionFdEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderTransactionFdData eventData {};
    ParseBinderTransactionFdData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_fd_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
    msg->set_src_fd(eventData.src_fd);
    msg->set_dest_fd(eventData.dest_fd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_fd, ParseBinderTransactionFd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_fd, ParseBinderTransactionFdEncoder);

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
void ParseBinderTransactionNodeToRef(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderTransactionNodeToRefEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_node_to_ref, ParseBinderTransactionNodeToRef);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_node_to_ref, ParseBinderTransactionNodeToRefEncoder);

struct BinderTransactionReceivedData {
    int32_t debug_id {};
};
__attribute__((noinline)) void ParseBinderTransactionReceivedData(BinderTransactionReceivedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.debug_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseBinderTransactionReceived(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderTransactionReceivedData eventData {};
    ParseBinderTransactionReceivedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_received_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
}
void ParseBinderTransactionReceivedEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderTransactionReceivedData eventData {};
    ParseBinderTransactionReceivedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_transaction_received_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_debug_id(eventData.debug_id);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_received, ParseBinderTransactionReceived);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_received, ParseBinderTransactionReceivedEncoder);

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
void ParseBinderTransactionRefToNode(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderTransactionRefToNodeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_ref_to_node, ParseBinderTransactionRefToNode);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_ref_to_node, ParseBinderTransactionRefToNodeEncoder);

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
void ParseBinderTransactionRefToRef(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderTransactionRefToRefEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_ref_to_ref, ParseBinderTransactionRefToRef);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_ref_to_ref, ParseBinderTransactionRefToRefEncoder);

struct BinderUnlockData {
    std::string tag {};
};
__attribute__((noinline)) void ParseBinderUnlockData(BinderUnlockData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.tag = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBinderUnlock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnlockData eventData {};
    ParseBinderUnlockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unlock_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
void ParseBinderUnlockEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnlockData eventData {};
    ParseBinderUnlockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unlock_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_tag(eventData.tag);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unlock, ParseBinderUnlock);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unlock, ParseBinderUnlockEncoder);

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
void ParseBinderUnmapKernelEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnmapKernelEndData eventData {};
    ParseBinderUnmapKernelEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_kernel_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderUnmapKernelEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnmapKernelEndData eventData {};
    ParseBinderUnmapKernelEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_kernel_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_kernel_end, ParseBinderUnmapKernelEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_kernel_end, ParseBinderUnmapKernelEndEncoder);

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
void ParseBinderUnmapKernelStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnmapKernelStartData eventData {};
    ParseBinderUnmapKernelStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_kernel_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderUnmapKernelStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnmapKernelStartData eventData {};
    ParseBinderUnmapKernelStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_kernel_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_kernel_start, ParseBinderUnmapKernelStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_kernel_start, ParseBinderUnmapKernelStartEncoder);

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
void ParseBinderUnmapUserEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnmapUserEndData eventData {};
    ParseBinderUnmapUserEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_user_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderUnmapUserEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnmapUserEndData eventData {};
    ParseBinderUnmapUserEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_user_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_user_end, ParseBinderUnmapUserEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_user_end, ParseBinderUnmapUserEndEncoder);

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
void ParseBinderUnmapUserStart(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnmapUserStartData eventData {};
    ParseBinderUnmapUserStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_user_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
void ParseBinderUnmapUserStartEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderUnmapUserStartData eventData {};
    ParseBinderUnmapUserStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_unmap_user_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_proc(eventData.proc);
    msg->set_page_index(eventData.page_index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_user_start, ParseBinderUnmapUserStart);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_user_start, ParseBinderUnmapUserStartEncoder);

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
void ParseBinderUpdatePageRange(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderUpdatePageRangeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_update_page_range, ParseBinderUpdatePageRange);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_update_page_range, ParseBinderUpdatePageRangeEncoder);

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
void ParseBinderWaitForWork(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
void ParseBinderWaitForWorkEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_wait_for_work, ParseBinderWaitForWork);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_wait_for_work, ParseBinderWaitForWorkEncoder);

struct BinderWriteDoneData {
    int32_t ret {};
};
__attribute__((noinline)) void ParseBinderWriteDoneData(BinderWriteDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseBinderWriteDone(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderWriteDoneData eventData {};
    ParseBinderWriteDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_write_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
void ParseBinderWriteDoneEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BinderWriteDoneData eventData {};
    ParseBinderWriteDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_binder_write_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_write_done, ParseBinderWriteDone);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_write_done, ParseBinderWriteDoneEncoder);
} // namespace
FTRACE_NS_END
