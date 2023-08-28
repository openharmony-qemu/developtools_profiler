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
template <typename T> void ext4_alloc_da_blocks_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_data_blocks(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_alloc_da_blocks,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_alloc_da_blocks_format();
        ext4_alloc_da_blocks_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_alloc_da_blocks,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_alloc_da_blocks_format();
        ext4_alloc_da_blocks_func(msg, data, size, format);
    });

template <typename T> void ext4_allocate_blocks_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_block(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_logical(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_lleft(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_lright(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_goal(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pleft(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pright(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_allocate_blocks,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_allocate_blocks_format();
        ext4_allocate_blocks_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_allocate_blocks,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_allocate_blocks_format();
        ext4_allocate_blocks_func(msg, data, size, format);
    });

template <typename T> void ext4_allocate_inode_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_dir(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_allocate_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_allocate_inode_format();
        ext4_allocate_inode_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_allocate_inode,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_allocate_inode_format();
        ext4_allocate_inode_func(msg, data, size, format);
    });

template <typename T>
void ext4_begin_ordered_truncate_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_new_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_begin_ordered_truncate,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_begin_ordered_truncate_format();
        ext4_begin_ordered_truncate_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_begin_ordered_truncate,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_begin_ordered_truncate_format();
        ext4_begin_ordered_truncate_func(msg, data, size, format);
    });

template <typename T> void ext4_collapse_range_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offset(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_collapse_range,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_collapse_range_format();
        ext4_collapse_range_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_collapse_range,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_collapse_range_format();
        ext4_collapse_range_func(msg, data, size, format);
    });

template <typename T> void ext4_da_release_space_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_blocks(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_freed_blocks(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_reserved_data_blocks(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_release_space,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_release_space_format();
        ext4_da_release_space_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_release_space,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_release_space_format();
        ext4_da_release_space_func(msg, data, size, format);
    });

template <typename T> void ext4_da_reserve_space_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_blocks(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_reserved_data_blocks(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_reserve_space,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_reserve_space_format();
        ext4_da_reserve_space_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_reserve_space,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_reserve_space_format();
        ext4_da_reserve_space_func(msg, data, size, format);
    });

template <typename T>
void ext4_da_update_reserve_space_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_blocks(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_used_blocks(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_reserved_data_blocks(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_quota_claim(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_update_reserve_space,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_update_reserve_space_format();
        ext4_da_update_reserve_space_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_update_reserve_space,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_update_reserve_space_format();
        ext4_da_update_reserve_space_func(msg, data, size, format);
    });

template <typename T> void ext4_da_write_begin_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_begin,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_write_begin_format();
        ext4_da_write_begin_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_begin,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_write_begin_format();
        ext4_da_write_begin_func(msg, data, size, format);
    });

template <typename T> void ext4_da_write_end_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_copied(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_write_end_format();
        ext4_da_write_end_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_end,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_write_end_format();
        ext4_da_write_end_func(msg, data, size, format);
    });

template <typename T> void ext4_da_write_pages_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_first_page(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_to_write(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_pages,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_write_pages_format();
        ext4_da_write_pages_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_pages,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_write_pages_format();
        ext4_da_write_pages_func(msg, data, size, format);
    });

template <typename T>
void ext4_da_write_pages_extent_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_pages_extent,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_write_pages_extent_format();
        ext4_da_write_pages_extent_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_pages_extent,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_da_write_pages_extent_format();
        ext4_da_write_pages_extent_func(msg, data, size, format);
    });

template <typename T> void ext4_direct_IO_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_rw(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_direct_IO_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_direct_io_enter_format();
        ext4_direct_IO_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_direct_IO_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_direct_io_enter_format();
        ext4_direct_IO_enter_func(msg, data, size, format);
    });

template <typename T> void ext4_direct_IO_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_rw(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_direct_IO_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_direct_io_exit_format();
        ext4_direct_IO_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_direct_IO_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_direct_io_exit_format();
        ext4_direct_IO_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_discard_blocks_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_blk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_count(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_discard_blocks,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_discard_blocks_format();
        ext4_discard_blocks_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_discard_blocks,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_discard_blocks_format();
        ext4_discard_blocks_func(msg, data, size, format);
    });

template <typename T>
void ext4_discard_preallocations_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_discard_preallocations,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_discard_preallocations_format();
        ext4_discard_preallocations_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_discard_preallocations,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_discard_preallocations_format();
        ext4_discard_preallocations_func(msg, data, size, format);
    });

template <typename T> void ext4_drop_inode_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_drop(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_drop_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_drop_inode_format();
        ext4_drop_inode_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_drop_inode,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_drop_inode_format();
        ext4_drop_inode_func(msg, data, size, format);
    });

template <typename T> void ext4_es_cache_extent_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_cache_extent,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_cache_extent_format();
        ext4_es_cache_extent_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_cache_extent,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_cache_extent_format();
        ext4_es_cache_extent_func(msg, data, size, format);
    });

template <typename T>
void ext4_es_find_delayed_extent_range_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_find_delayed_extent_range_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_find_delayed_extent_range_enter_format();
        ext4_es_find_delayed_extent_range_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_find_delayed_extent_range_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_find_delayed_extent_range_enter_format();
        ext4_es_find_delayed_extent_range_enter_func(msg, data, size, format);
    });

template <typename T>
void ext4_es_find_delayed_extent_range_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_find_delayed_extent_range_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_find_delayed_extent_range_exit_format();
        ext4_es_find_delayed_extent_range_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_find_delayed_extent_range_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_find_delayed_extent_range_exit_format();
        ext4_es_find_delayed_extent_range_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_es_insert_extent_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_insert_extent,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_insert_extent_format();
        ext4_es_insert_extent_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_insert_extent,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_insert_extent_format();
        ext4_es_insert_extent_func(msg, data, size, format);
    });

template <typename T>
void ext4_es_lookup_extent_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_lookup_extent_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_enter_format();
        ext4_es_lookup_extent_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_lookup_extent_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_enter_format();
        ext4_es_lookup_extent_enter_func(msg, data, size, format);
    });

template <typename T>
void ext4_es_lookup_extent_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_found(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_lookup_extent_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_exit_format();
        ext4_es_lookup_extent_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_lookup_extent_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_exit_format();
        ext4_es_lookup_extent_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_es_remove_extent_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_remove_extent,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_remove_extent_format();
        ext4_es_remove_extent_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_remove_extent,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_remove_extent_format();
        ext4_es_remove_extent_func(msg, data, size, format);
    });

template <typename T> void ext4_es_shrink_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_shrunk(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_scan_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_skipped(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_retried(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_shrink_format();
        ext4_es_shrink_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_shrink_format();
        ext4_es_shrink_func(msg, data, size, format);
    });

template <typename T> void ext4_es_shrink_count_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_to_scan(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_cache_cnt(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_count,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_shrink_count_format();
        ext4_es_shrink_count_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_count,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_shrink_count_format();
        ext4_es_shrink_count_func(msg, data, size, format);
    });

template <typename T>
void ext4_es_shrink_scan_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_to_scan(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_cache_cnt(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_scan_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_enter_format();
        ext4_es_shrink_scan_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_scan_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_enter_format();
        ext4_es_shrink_scan_enter_func(msg, data, size, format);
    });

template <typename T> void ext4_es_shrink_scan_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_shrunk(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_cache_cnt(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_scan_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_exit_format();
        ext4_es_shrink_scan_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_scan_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_exit_format();
        ext4_es_shrink_scan_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_evict_inode_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nlink(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_evict_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_evict_inode_format();
        ext4_evict_inode_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_evict_inode,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_evict_inode_format();
        ext4_evict_inode_func(msg, data, size, format);
    });

template <typename T>
void ext4_ext_convert_to_initialized_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_m_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_m_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_u_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_u_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_u_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_convert_to_initialized_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_enter_format();
        ext4_ext_convert_to_initialized_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_convert_to_initialized_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_enter_format();
        ext4_ext_convert_to_initialized_enter_func(msg, data, size, format);
    });

template <typename T>
void ext4_ext_convert_to_initialized_fastpath_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_m_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_m_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_u_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_u_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_u_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_i_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_i_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_convert_to_initialized_fastpath,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_fastpath_format();
        ext4_ext_convert_to_initialized_fastpath_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_convert_to_initialized_fastpath,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_fastpath_format();
        ext4_ext_convert_to_initialized_fastpath_func(msg, data, size, format);
    });

template <typename T>
void ext4_ext_handle_unwritten_extents_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_allocated(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_newblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_handle_unwritten_extents,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_handle_unwritten_extents_format();
        ext4_ext_handle_unwritten_extents_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_handle_unwritten_extents,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_handle_unwritten_extents_format();
        ext4_ext_handle_unwritten_extents_func(msg, data, size, format);
    });

template <typename T> void ext4_ext_in_cache_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_in_cache,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_in_cache_format();
        ext4_ext_in_cache_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_in_cache,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_in_cache_format();
        ext4_ext_in_cache_func(msg, data, size, format);
    });

template <typename T> void ext4_ext_load_extent_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_load_extent,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_load_extent_format();
        ext4_ext_load_extent_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_load_extent,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_load_extent_format();
        ext4_ext_load_extent_func(msg, data, size, format);
    });

template <typename T>
void ext4_ext_map_blocks_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_map_blocks_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_enter_format();
        ext4_ext_map_blocks_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_map_blocks_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_enter_format();
        ext4_ext_map_blocks_enter_func(msg, data, size, format);
    });

template <typename T> void ext4_ext_map_blocks_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_mflags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_map_blocks_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_exit_format();
        ext4_ext_map_blocks_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_map_blocks_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_exit_format();
        ext4_ext_map_blocks_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_ext_put_in_cache_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_start(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_put_in_cache,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_put_in_cache_format();
        ext4_ext_put_in_cache_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_put_in_cache,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_put_in_cache_format();
        ext4_ext_put_in_cache_func(msg, data, size, format);
    });

template <typename T> void ext4_ext_remove_space_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_start(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_end(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_depth(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_remove_space,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_remove_space_format();
        ext4_ext_remove_space_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_remove_space,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_remove_space_format();
        ext4_ext_remove_space_func(msg, data, size, format);
    });

template <typename T>
void ext4_ext_remove_space_done_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_start(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_end(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_depth(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_partial(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_eh_entries(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_remove_space_done,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_remove_space_done_format();
        ext4_ext_remove_space_done_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_remove_space_done,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_remove_space_done_format();
        ext4_ext_remove_space_done_func(msg, data, size, format);
    });

template <typename T> void ext4_ext_rm_idx_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_rm_idx,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_rm_idx_format();
        ext4_ext_rm_idx_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_rm_idx,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_rm_idx_format();
        ext4_ext_rm_idx_func(msg, data, size, format);
    });

template <typename T> void ext4_ext_rm_leaf_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_partial(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_start(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ee_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ee_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ee_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_rm_leaf,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_rm_leaf_format();
        ext4_ext_rm_leaf_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_rm_leaf,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_rm_leaf_format();
        ext4_ext_rm_leaf_func(msg, data, size, format);
    });

template <typename T> void ext4_ext_show_extent_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_show_extent,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_show_extent_format();
        ext4_ext_show_extent_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_show_extent,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ext_show_extent_format();
        ext4_ext_show_extent_func(msg, data, size, format);
    });

template <typename T> void ext4_fallocate_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offset(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_fallocate_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_fallocate_enter_format();
        ext4_fallocate_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_fallocate_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_fallocate_enter_format();
        ext4_fallocate_enter_func(msg, data, size, format);
    });

template <typename T> void ext4_fallocate_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_blocks(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_fallocate_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_fallocate_exit_format();
        ext4_fallocate_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_fallocate_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_fallocate_exit_format();
        ext4_fallocate_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_find_delalloc_range_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_from(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_to(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_reverse(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_found(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_found_blk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_find_delalloc_range,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_find_delalloc_range_format();
        ext4_find_delalloc_range_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_find_delalloc_range,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_find_delalloc_range_format();
        ext4_find_delalloc_range_func(msg, data, size, format);
    });

template <typename T> void ext4_forget_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_block(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_is_metadata(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_forget,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_forget_format();
        ext4_forget_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_forget,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_forget_format();
        ext4_forget_func(msg, data, size, format);
    });

template <typename T> void ext4_free_blocks_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_block(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_count(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_free_blocks,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_free_blocks_format();
        ext4_free_blocks_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_free_blocks,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_free_blocks_format();
        ext4_free_blocks_func(msg, data, size, format);
    });

template <typename T> void ext4_free_inode_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_uid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_gid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_blocks(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_free_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_free_inode_format();
        ext4_free_inode_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_free_inode,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_free_inode_format();
        ext4_free_inode_func(msg, data, size, format);
    });

template <typename T>
void ext4_get_implied_cluster_alloc_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_get_implied_cluster_alloc_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_get_implied_cluster_alloc_exit_format();
        ext4_get_implied_cluster_alloc_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_get_implied_cluster_alloc_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_get_implied_cluster_alloc_exit_format();
        ext4_get_implied_cluster_alloc_exit_func(msg, data, size, format);
    });

template <typename T>
void ext4_get_reserved_cluster_alloc_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_get_reserved_cluster_alloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_get_reserved_cluster_alloc_format();
        ext4_get_reserved_cluster_alloc_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_get_reserved_cluster_alloc,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_get_reserved_cluster_alloc_format();
        ext4_get_reserved_cluster_alloc_func(msg, data, size, format);
    });

template <typename T>
void ext4_ind_map_blocks_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ind_map_blocks_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_enter_format();
        ext4_ind_map_blocks_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ind_map_blocks_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_enter_format();
        ext4_ind_map_blocks_enter_func(msg, data, size, format);
    });

template <typename T> void ext4_ind_map_blocks_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_mflags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ind_map_blocks_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_exit_format();
        ext4_ind_map_blocks_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ind_map_blocks_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_exit_format();
        ext4_ind_map_blocks_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_insert_range_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offset(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_insert_range,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_insert_range_format();
        ext4_insert_range_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_insert_range,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_insert_range_format();
        ext4_insert_range_func(msg, data, size, format);
    });

template <typename T> void ext4_invalidatepage_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offset(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_length(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_invalidatepage,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_invalidatepage_format();
        ext4_invalidatepage_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_invalidatepage,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_invalidatepage_format();
        ext4_invalidatepage_func(msg, data, size, format);
    });

template <typename T> void ext4_journal_start_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_blocks(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_rsv_blocks(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journal_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_journal_start_format();
        ext4_journal_start_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journal_start,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_journal_start_format();
        ext4_journal_start_func(msg, data, size, format);
    });

template <typename T>
void ext4_journal_start_reserved_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_blocks(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journal_start_reserved,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_journal_start_reserved_format();
        ext4_journal_start_reserved_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journal_start_reserved,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_journal_start_reserved_format();
        ext4_journal_start_reserved_func(msg, data, size, format);
    });

template <typename T>
void ext4_journalled_invalidatepage_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offset(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_length(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journalled_invalidatepage,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_journalled_invalidatepage_format();
        ext4_journalled_invalidatepage_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journalled_invalidatepage,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_journalled_invalidatepage_format();
        ext4_journalled_invalidatepage_func(msg, data, size, format);
    });

template <typename T>
void ext4_journalled_write_end_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_copied(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journalled_write_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_journalled_write_end_format();
        ext4_journalled_write_end_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journalled_write_end,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_journalled_write_end_format();
        ext4_journalled_write_end_func(msg, data, size, format);
    });

template <typename T> void ext4_load_inode_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_load_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_load_inode_format();
        ext4_load_inode_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_load_inode,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_load_inode_format();
        ext4_load_inode_func(msg, data, size, format);
    });

template <typename T> void ext4_load_inode_bitmap_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_load_inode_bitmap,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_load_inode_bitmap_format();
        ext4_load_inode_bitmap_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_load_inode_bitmap,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_load_inode_bitmap_format();
        ext4_load_inode_bitmap_func(msg, data, size, format);
    });

template <typename T> void ext4_mark_inode_dirty_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mark_inode_dirty,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mark_inode_dirty_format();
        ext4_mark_inode_dirty_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mark_inode_dirty,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mark_inode_dirty_format();
        ext4_mark_inode_dirty_func(msg, data, size, format);
    });

template <typename T> void ext4_mb_bitmap_load_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_bitmap_load,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_bitmap_load_format();
        ext4_mb_bitmap_load_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_bitmap_load,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_bitmap_load_format();
        ext4_mb_bitmap_load_func(msg, data, size, format);
    });

template <typename T>
void ext4_mb_buddy_bitmap_load_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_buddy_bitmap_load,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_buddy_bitmap_load_format();
        ext4_mb_buddy_bitmap_load_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_buddy_bitmap_load,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_buddy_bitmap_load_format();
        ext4_mb_buddy_bitmap_load_func(msg, data, size, format);
    });

template <typename T>
void ext4_mb_discard_preallocations_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_needed(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_discard_preallocations,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_discard_preallocations_format();
        ext4_mb_discard_preallocations_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_discard_preallocations,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_discard_preallocations_format();
        ext4_mb_discard_preallocations_func(msg, data, size, format);
    });

template <typename T> void ext4_mb_new_group_pa_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pa_pstart(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pa_lstart(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pa_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_new_group_pa,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_new_group_pa_format();
        ext4_mb_new_group_pa_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_new_group_pa,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_new_group_pa_format();
        ext4_mb_new_group_pa_func(msg, data, size, format);
    });

template <typename T> void ext4_mb_new_inode_pa_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pa_pstart(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pa_lstart(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pa_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_new_inode_pa,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_new_inode_pa_format();
        ext4_mb_new_inode_pa_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_new_inode_pa,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_new_inode_pa_format();
        ext4_mb_new_inode_pa_func(msg, data, size, format);
    });

template <typename T> void ext4_mb_release_group_pa_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pa_pstart(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pa_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_release_group_pa,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_release_group_pa_format();
        ext4_mb_release_group_pa_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_release_group_pa,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_release_group_pa_format();
        ext4_mb_release_group_pa_func(msg, data, size, format);
    });

template <typename T> void ext4_mb_release_inode_pa_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_block(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_count(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_release_inode_pa,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_release_inode_pa_format();
        ext4_mb_release_inode_pa_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_release_inode_pa,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mb_release_inode_pa_format();
        ext4_mb_release_inode_pa_func(msg, data, size, format);
    });

template <typename T> void ext4_mballoc_alloc_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_orig_logical(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_orig_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_orig_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_orig_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_goal_logical(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_goal_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_goal_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_goal_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_result_logical(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_result_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_result_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_result_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_found(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_groups(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_buddy(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_tail(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_cr(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_alloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mballoc_alloc_format();
        ext4_mballoc_alloc_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_alloc,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mballoc_alloc_format();
        ext4_mballoc_alloc_func(msg, data, size, format);
    });

template <typename T> void ext4_mballoc_discard_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_result_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_result_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_result_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_discard,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mballoc_discard_format();
        ext4_mballoc_discard_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_discard,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mballoc_discard_format();
        ext4_mballoc_discard_func(msg, data, size, format);
    });

template <typename T> void ext4_mballoc_free_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_result_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_result_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_result_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_free,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mballoc_free_format();
        ext4_mballoc_free_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_free,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mballoc_free_format();
        ext4_mballoc_free_func(msg, data, size, format);
    });

template <typename T> void ext4_mballoc_prealloc_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_orig_logical(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_orig_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_orig_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_orig_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_result_logical(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_result_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_result_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_result_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_prealloc,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mballoc_prealloc_format();
        ext4_mballoc_prealloc_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_prealloc,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_mballoc_prealloc_format();
        ext4_mballoc_prealloc_func(msg, data, size, format);
    });

template <typename T>
void ext4_other_inode_update_time_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_orig_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_uid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_gid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_other_inode_update_time,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_other_inode_update_time_format();
        ext4_other_inode_update_time_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_other_inode_update_time,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_other_inode_update_time_format();
        ext4_other_inode_update_time_func(msg, data, size, format);
    });

template <typename T> void ext4_punch_hole_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offset(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_punch_hole,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_punch_hole_format();
        ext4_punch_hole_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_punch_hole,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_punch_hole_format();
        ext4_punch_hole_func(msg, data, size, format);
    });

template <typename T>
void ext4_read_block_bitmap_load_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_read_block_bitmap_load,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_read_block_bitmap_load_format();
        ext4_read_block_bitmap_load_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_read_block_bitmap_load,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_read_block_bitmap_load_format();
        ext4_read_block_bitmap_load_func(msg, data, size, format);
    });

template <typename T> void ext4_readpage_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_readpage,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_readpage_format();
        ext4_readpage_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_readpage,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_readpage_format();
        ext4_readpage_func(msg, data, size, format);
    });

template <typename T> void ext4_releasepage_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_releasepage,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_releasepage_format();
        ext4_releasepage_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_releasepage,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_releasepage_format();
        ext4_releasepage_func(msg, data, size, format);
    });

template <typename T> void ext4_remove_blocks_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_from(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_to(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_partial(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ee_pblk(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ee_lblk(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ee_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_remove_blocks,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_remove_blocks_format();
        ext4_remove_blocks_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_remove_blocks,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_remove_blocks_format();
        ext4_remove_blocks_func(msg, data, size, format);
    });

template <typename T> void ext4_request_blocks_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_logical(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_lleft(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_lright(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_goal(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pleft(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pright(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_request_blocks,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_request_blocks_format();
        ext4_request_blocks_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_request_blocks,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_request_blocks_format();
        ext4_request_blocks_func(msg, data, size, format);
    });

template <typename T> void ext4_request_inode_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_dir(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_request_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_request_inode_format();
        ext4_request_inode_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_request_inode,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_request_inode_format();
        ext4_request_inode_func(msg, data, size, format);
    });

template <typename T> void ext4_sync_file_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_parent(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_datasync(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_file_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_sync_file_enter_format();
        ext4_sync_file_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_file_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_sync_file_enter_format();
        ext4_sync_file_enter_func(msg, data, size, format);
    });

template <typename T> void ext4_sync_file_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_file_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_sync_file_exit_format();
        ext4_sync_file_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_file_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_sync_file_exit_format();
        ext4_sync_file_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_sync_fs_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_wait(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_fs,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_sync_fs_format();
        ext4_sync_fs_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_fs,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_sync_fs_format();
        ext4_sync_fs_func(msg, data, size, format);
    });

template <typename T> void ext4_trim_all_free_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev_major(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_dev_minor(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_trim_all_free,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_trim_all_free_format();
        ext4_trim_all_free_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_trim_all_free,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_trim_all_free_format();
        ext4_trim_all_free_func(msg, data, size, format);
    });

template <typename T> void ext4_trim_extent_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev_major(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_dev_minor(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_group(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_start(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_trim_extent,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_trim_extent_format();
        ext4_trim_extent_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_trim_extent,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_trim_extent_format();
        ext4_trim_extent_func(msg, data, size, format);
    });

template <typename T> void ext4_truncate_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_blocks(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_truncate_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_truncate_enter_format();
        ext4_truncate_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_truncate_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_truncate_enter_format();
        ext4_truncate_enter_func(msg, data, size, format);
    });

template <typename T> void ext4_truncate_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_blocks(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_truncate_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_truncate_exit_format();
        ext4_truncate_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_truncate_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_truncate_exit_format();
        ext4_truncate_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_unlink_enter_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_parent(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_unlink_enter,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_unlink_enter_format();
        ext4_unlink_enter_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_unlink_enter,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_unlink_enter_format();
        ext4_unlink_enter_func(msg, data, size, format);
    });

template <typename T> void ext4_unlink_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_unlink_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_unlink_exit_format();
        ext4_unlink_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_unlink_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_unlink_exit_format();
        ext4_unlink_exit_func(msg, data, size, format);
    });

template <typename T> void ext4_write_begin_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_write_begin,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_write_begin_format();
        ext4_write_begin_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_write_begin,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_write_begin_format();
        ext4_write_begin_func(msg, data, size, format);
    });

template <typename T> void ext4_write_end_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pos(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_copied(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_write_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_write_end_format();
        ext4_write_end_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_write_end,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_write_end_format();
        ext4_write_end_func(msg, data, size, format);
    });

template <typename T> void ext4_writepage_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_writepage,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_writepage_format();
        ext4_writepage_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_writepage,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_writepage_format();
        ext4_writepage_func(msg, data, size, format);
    });

template <typename T> void ext4_writepages_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_to_write(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pages_skipped(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_range_start(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_range_end(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_writeback_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_for_kupdate(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_range_cyclic(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_writepages,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_writepages_format();
        ext4_writepages_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_writepages,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_writepages_format();
        ext4_writepages_func(msg, data, size, format);
    });

template <typename T> void ext4_writepages_result_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_pages_written(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_pages_skipped(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_writeback_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_writepages_result,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_writepages_result_format();
        ext4_writepages_result_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_writepages_result,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_writepages_result_format();
        ext4_writepages_result_func(msg, data, size, format);
    });

template <typename T> void ext4_zero_range_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_offset(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_zero_range,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_zero_range_format();
        ext4_zero_range_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_zero_range,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_ext4_zero_range_format();
        ext4_zero_range_func(msg, data, size, format);
    });
} // namespace
FTRACE_NS_END
