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
template <typename T>
void binder_alloc_lru_end_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_lru_end, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                              const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_alloc_lru_end_format();
    binder_alloc_lru_end_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_lru_end,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_alloc_lru_end_format();
    binder_alloc_lru_end_func(msg, data, size, format);
});

template <typename T>
void binder_alloc_lru_start_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_lru_start, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_alloc_lru_start_format();
    binder_alloc_lru_start_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_lru_start,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_binder_alloc_lru_start_format();
                                                binder_alloc_lru_start_func(msg, data, size, format);
                                            });

template <typename T>
void binder_alloc_page_end_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_page_end, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                               const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_alloc_page_end_format();
    binder_alloc_page_end_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_page_end,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_binder_alloc_page_end_format();
                                                binder_alloc_page_end_func(msg, data, size, format);
                                            });

template <typename T>
void binder_alloc_page_start_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_alloc_page_start, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                 const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_alloc_page_start_format();
    binder_alloc_page_start_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_alloc_page_start,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_binder_alloc_page_start_format();
                                                binder_alloc_page_start_func(msg, data, size, format);
                                            });

template <typename T>
void binder_command_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_cmd(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_command, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                        const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_command_format();
    binder_command_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_command, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_command_format();
    binder_command_func(msg, data, size, format);
});

template <typename T>
void binder_free_lru_end_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_free_lru_end, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                             const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_free_lru_end_format();
    binder_free_lru_end_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_free_lru_end,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_free_lru_end_format();
    binder_free_lru_end_func(msg, data, size, format);
});

template <typename T>
void binder_free_lru_start_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_free_lru_start, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                               const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_free_lru_start_format();
    binder_free_lru_start_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_free_lru_start,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_free_lru_start_format();
    binder_free_lru_start_func(msg, data, size, format);
});

template <typename T>
void binder_ioctl_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_cmd(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_arg(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_ioctl, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                      const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_ioctl_format();
    binder_ioctl_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_ioctl, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                              size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_ioctl_format();
    binder_ioctl_func(msg, data, size, format);
});

template <typename T>
void binder_ioctl_done_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_ioctl_done, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                           const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_ioctl_done_format();
    binder_ioctl_done_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_ioctl_done,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_ioctl_done_format();
    binder_ioctl_done_func(msg, data, size, format);
});

template <typename T>
void binder_lock_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_tag(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_lock, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                     const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_lock_format();
    binder_lock_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_lock, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                             size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_lock_format();
    binder_lock_func(msg, data, size, format);
});

template <typename T>
void binder_locked_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_tag(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_locked, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_locked_format();
    binder_locked_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_locked, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_locked_format();
    binder_locked_func(msg, data, size, format);
});

template <typename T>
void binder_read_done_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_read_done, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_read_done_format();
    binder_read_done_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_read_done,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_read_done_format();
    binder_read_done_func(msg, data, size, format);
});

template <typename T>
void binder_return_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_cmd(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_return, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_return_format();
    binder_return_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_return, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_return_format();
    binder_return_func(msg, data, size, format);
});

template <typename T>
void binder_transaction_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_target_node(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_to_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_to_thread(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_reply(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_code(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_format();
    binder_transaction_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_format();
    binder_transaction_func(msg, data, size, format);
});

template <typename T>
void binder_transaction_alloc_buf_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_data_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offsets_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_extra_buffers_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_alloc_buf, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                      size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_alloc_buf_format();
    binder_transaction_alloc_buf_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_alloc_buf,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_alloc_buf_format();
    binder_transaction_alloc_buf_func(msg, data, size, format);
});

template <typename T>
void binder_transaction_buffer_release_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_data_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offsets_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_extra_buffers_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_buffer_release, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                           size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_buffer_release_format();
    binder_transaction_buffer_release_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_buffer_release,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_buffer_release_format();
    binder_transaction_buffer_release_func(msg, data, size, format);
});

template <typename T>
void binder_transaction_failed_buffer_release_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_data_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offsets_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_extra_buffers_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_failed_buffer_release, [](FtraceEvent &ftraceEvent,
                                                                                  uint8_t data[], size_t size,
                                                                                  const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_failed_buffer_release_format();
    binder_transaction_failed_buffer_release_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(
    binder_transaction_failed_buffer_release,
    [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size, const EventFormat &format) {
        auto msg = ftraceEvent.mutable_binder_transaction_failed_buffer_release_format();
        binder_transaction_failed_buffer_release_func(msg, data, size, format);
    });

template <typename T>
void binder_transaction_node_to_ref_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_node_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_node_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ref_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ref_desc(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_node_to_ref, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                        size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_node_to_ref_format();
    binder_transaction_node_to_ref_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_node_to_ref,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_node_to_ref_format();
    binder_transaction_node_to_ref_func(msg, data, size, format);
});

template <typename T>
void binder_transaction_received_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_received, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                     size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_received_format();
    binder_transaction_received_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_received,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_received_format();
    binder_transaction_received_func(msg, data, size, format);
});

template <typename T>
void binder_transaction_ref_to_node_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ref_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ref_desc(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_node_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_node_ptr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_ref_to_node, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                        size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_ref_to_node_format();
    binder_transaction_ref_to_node_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_ref_to_node,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_binder_transaction_ref_to_node_format();
                                                binder_transaction_ref_to_node_func(msg, data, size, format);
                                            });

template <typename T>
void binder_transaction_ref_to_ref_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_node_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_src_ref_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_src_ref_desc(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_dest_ref_debug_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_dest_ref_desc(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_transaction_ref_to_ref, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                       size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_ref_to_ref_format();
    binder_transaction_ref_to_ref_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_transaction_ref_to_ref,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_transaction_ref_to_ref_format();
    binder_transaction_ref_to_ref_func(msg, data, size, format);
});

template <typename T>
void binder_unlock_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_tag(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unlock, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_unlock_format();
    binder_unlock_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unlock, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_unlock_format();
    binder_unlock_func(msg, data, size, format);
});

template <typename T>
void binder_unmap_kernel_end_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_kernel_end, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                 const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_unmap_kernel_end_format();
    binder_unmap_kernel_end_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_kernel_end,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_binder_unmap_kernel_end_format();
                                                binder_unmap_kernel_end_func(msg, data, size, format);
                                            });

template <typename T>
void binder_unmap_kernel_start_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_kernel_start, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                   size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_unmap_kernel_start_format();
    binder_unmap_kernel_start_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_kernel_start,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_binder_unmap_kernel_start_format();
                                                binder_unmap_kernel_start_func(msg, data, size, format);
                                            });

template <typename T>
void binder_unmap_user_end_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_user_end, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                               const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_unmap_user_end_format();
    binder_unmap_user_end_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_user_end,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_binder_unmap_user_end_format();
                                                binder_unmap_user_end_func(msg, data, size, format);
                                            });

template <typename T>
void binder_unmap_user_start_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_page_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_unmap_user_start, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                 const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_unmap_user_start_format();
    binder_unmap_user_start_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_unmap_user_start,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_unmap_user_start_format();
    binder_unmap_user_start_func(msg, data, size, format);
});

template <typename T>
void binder_update_page_range_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_allocate(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_offset(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_update_page_range, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                  const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_update_page_range_format();
    binder_update_page_range_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_update_page_range,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_update_page_range_format();
    binder_update_page_range_func(msg, data, size, format);
});

template <typename T>
void binder_wait_for_work_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_proc_work(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_transaction_stack(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_thread_todo(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_wait_for_work, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                              const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_wait_for_work_format();
    binder_wait_for_work_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_wait_for_work,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_wait_for_work_format();
    binder_wait_for_work_func(msg, data, size, format);
});

template <typename T>
void binder_write_done_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(binder_write_done, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                           const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_write_done_format();
    binder_write_done_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(binder_write_done,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_binder_write_done_format();
    binder_write_done_func(msg, data, size, format);
});
}  // namespace
FTRACE_NS_END
