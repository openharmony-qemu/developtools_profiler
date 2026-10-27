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
struct Ext4AllocDaBlocksData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t data_blocks {};
};
__attribute__((noinline)) void ParseExt4AllocDaBlocksData(Ext4AllocDaBlocksData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.data_blocks = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4AllocDaBlocks(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4AllocDaBlocksData eventData {};
    ParseExt4AllocDaBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_alloc_da_blocks_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_data_blocks(eventData.data_blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_alloc_da_blocks, ParseExt4AllocDaBlocks<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_alloc_da_blocks, ParseExt4AllocDaBlocks<ProtoEncoder::FtraceEvent>);

struct Ext4AllocateBlocksData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t block {};
    uint32_t len {};
    uint32_t logical {};
    uint32_t lleft {};
    uint32_t lright {};
    uint64_t goal {};
    uint64_t pleft {};
    uint64_t pright {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseExt4AllocateBlocksData(Ext4AllocateBlocksData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.block = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.logical = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.lleft = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.lright = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.goal = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pleft = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pright = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4AllocateBlocks(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4AllocateBlocksData eventData {};
    ParseExt4AllocateBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_allocate_blocks_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_len(eventData.len);
    msg->set_logical(eventData.logical);
    msg->set_lleft(eventData.lleft);
    msg->set_lright(eventData.lright);
    msg->set_goal(eventData.goal);
    msg->set_pleft(eventData.pleft);
    msg->set_pright(eventData.pright);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_allocate_blocks, ParseExt4AllocateBlocks<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_allocate_blocks, ParseExt4AllocateBlocks<ProtoEncoder::FtraceEvent>);

struct Ext4AllocateInodeData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t dir {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4AllocateInodeData(Ext4AllocateInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dir = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4AllocateInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4AllocateInodeData eventData {};
    ParseExt4AllocateInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_allocate_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_dir(eventData.dir);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_allocate_inode, ParseExt4AllocateInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_allocate_inode, ParseExt4AllocateInode<ProtoEncoder::FtraceEvent>);

struct Ext4BeginOrderedTruncateData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t new_size {};
};
__attribute__((noinline)) void ParseExt4BeginOrderedTruncateData(Ext4BeginOrderedTruncateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.new_size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4BeginOrderedTruncate(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4BeginOrderedTruncateData eventData {};
    ParseExt4BeginOrderedTruncateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_begin_ordered_truncate_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_new_size(eventData.new_size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_begin_ordered_truncate, ParseExt4BeginOrderedTruncate<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_begin_ordered_truncate, ParseExt4BeginOrderedTruncate<ProtoEncoder::FtraceEvent>);

struct Ext4CollapseRangeData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t offset {};
    uint64_t len {};
};
__attribute__((noinline)) void ParseExt4CollapseRangeData(Ext4CollapseRangeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4CollapseRange(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4CollapseRangeData eventData {};
    ParseExt4CollapseRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_collapse_range_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_collapse_range, ParseExt4CollapseRange<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_collapse_range, ParseExt4CollapseRange<ProtoEncoder::FtraceEvent>);

struct Ext4DaReleaseSpaceData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t i_blocks {};
    int32_t freed_blocks {};
    int32_t reserved_data_blocks {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4DaReleaseSpaceData(Ext4DaReleaseSpaceData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_blocks = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.freed_blocks = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reserved_data_blocks = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DaReleaseSpace(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DaReleaseSpaceData eventData {};
    ParseExt4DaReleaseSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_release_space_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_freed_blocks(eventData.freed_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_release_space, ParseExt4DaReleaseSpace<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_release_space, ParseExt4DaReleaseSpace<ProtoEncoder::FtraceEvent>);

struct Ext4DaReserveSpaceData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t i_blocks {};
    int32_t reserved_data_blocks {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4DaReserveSpaceData(Ext4DaReserveSpaceData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_blocks = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.reserved_data_blocks = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DaReserveSpace(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DaReserveSpaceData eventData {};
    ParseExt4DaReserveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_reserve_space_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_reserve_space, ParseExt4DaReserveSpace<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_reserve_space, ParseExt4DaReserveSpace<ProtoEncoder::FtraceEvent>);

struct Ext4DaUpdateReserveSpaceData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t i_blocks {};
    int32_t used_blocks {};
    int32_t reserved_data_blocks {};
    int32_t quota_claim {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4DaUpdateReserveSpaceData(Ext4DaUpdateReserveSpaceData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_blocks = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.used_blocks = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reserved_data_blocks = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.quota_claim = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DaUpdateReserveSpace(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DaUpdateReserveSpaceData eventData {};
    ParseExt4DaUpdateReserveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_update_reserve_space_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_used_blocks(eventData.used_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_quota_claim(eventData.quota_claim);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_update_reserve_space, ParseExt4DaUpdateReserveSpace<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_update_reserve_space, ParseExt4DaUpdateReserveSpace<ProtoEncoder::FtraceEvent>);

struct Ext4DaWriteBeginData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseExt4DaWriteBeginData(Ext4DaWriteBeginData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DaWriteBegin(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DaWriteBeginData eventData {};
    ParseExt4DaWriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_begin_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_begin, ParseExt4DaWriteBegin<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_begin, ParseExt4DaWriteBegin<ProtoEncoder::FtraceEvent>);

struct Ext4DaWriteEndData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
    uint32_t copied {};
};
__attribute__((noinline)) void ParseExt4DaWriteEndData(Ext4DaWriteEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.copied = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DaWriteEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DaWriteEndData eventData {};
    ParseExt4DaWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_end, ParseExt4DaWriteEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_end, ParseExt4DaWriteEnd<ProtoEncoder::FtraceEvent>);

struct Ext4DaWritePagesData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t first_page {};
    uint64_t nr_to_write {};
    int32_t sync_mode {};
};
__attribute__((noinline)) void ParseExt4DaWritePagesData(Ext4DaWritePagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.first_page = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_to_write = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DaWritePages(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DaWritePagesData eventData {};
    ParseExt4DaWritePagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_pages_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_first_page(eventData.first_page);
    msg->set_nr_to_write(eventData.nr_to_write);
    msg->set_sync_mode(eventData.sync_mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_pages, ParseExt4DaWritePages<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_pages, ParseExt4DaWritePages<ProtoEncoder::FtraceEvent>);

struct Ext4DaWritePagesExtentData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t lblk {};
    uint32_t len {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseExt4DaWritePagesExtentData(Ext4DaWritePagesExtentData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DaWritePagesExtent(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DaWritePagesExtentData eventData {};
    ParseExt4DaWritePagesExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_pages_extent_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_pages_extent, ParseExt4DaWritePagesExtent<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_pages_extent, ParseExt4DaWritePagesExtent<ProtoEncoder::FtraceEvent>);

struct Ext4DirectIoEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint64_t len {};
    int32_t rw {};
};
__attribute__((noinline)) void ParseExt4DirectIoEnterData(Ext4DirectIoEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.rw = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DirectIoEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DirectIoEnterData eventData {};
    ParseExt4DirectIoEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_direct_io_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_rw(eventData.rw);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_direct_IO_enter, ParseExt4DirectIoEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_direct_IO_enter, ParseExt4DirectIoEnter<ProtoEncoder::FtraceEvent>);

struct Ext4DirectIoExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint64_t len {};
    int32_t rw {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseExt4DirectIoExitData(Ext4DirectIoExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.rw = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DirectIoExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DirectIoExitData eventData {};
    ParseExt4DirectIoExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_direct_io_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_rw(eventData.rw);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_direct_IO_exit, ParseExt4DirectIoExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_direct_IO_exit, ParseExt4DirectIoExit<ProtoEncoder::FtraceEvent>);

struct Ext4DiscardBlocksData {
    uint64_t dev {};
    uint64_t blk {};
    uint64_t count {};
};
__attribute__((noinline)) void ParseExt4DiscardBlocksData(Ext4DiscardBlocksData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.blk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.count = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DiscardBlocks(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DiscardBlocksData eventData {};
    ParseExt4DiscardBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_discard_blocks_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_blk(eventData.blk);
    msg->set_count(eventData.count);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_discard_blocks, ParseExt4DiscardBlocks<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_discard_blocks, ParseExt4DiscardBlocks<ProtoEncoder::FtraceEvent>);

struct Ext4DiscardPreallocationsData {
    uint64_t dev {};
    uint64_t ino {};
};
__attribute__((noinline)) void ParseExt4DiscardPreallocationsData(Ext4DiscardPreallocationsData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DiscardPreallocations(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DiscardPreallocationsData eventData {};
    ParseExt4DiscardPreallocationsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_discard_preallocations_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_discard_preallocations, ParseExt4DiscardPreallocations<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_discard_preallocations, ParseExt4DiscardPreallocations<ProtoEncoder::FtraceEvent>);

struct Ext4DropInodeData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t drop {};
};
__attribute__((noinline)) void ParseExt4DropInodeData(Ext4DropInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.drop = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4DropInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4DropInodeData eventData {};
    ParseExt4DropInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_drop_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_drop(eventData.drop);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_drop_inode, ParseExt4DropInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_drop_inode, ParseExt4DropInode<ProtoEncoder::FtraceEvent>);

struct Ext4EsCacheExtentData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    uint32_t len {};
    uint64_t pblk {};
    uint32_t status {};
};
__attribute__((noinline)) void ParseExt4EsCacheExtentData(Ext4EsCacheExtentData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsCacheExtent(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsCacheExtentData eventData {};
    ParseExt4EsCacheExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_cache_extent_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_cache_extent, ParseExt4EsCacheExtent<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_cache_extent, ParseExt4EsCacheExtent<ProtoEncoder::FtraceEvent>);

struct Ext4EsFindDelayedExtentRangeEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
};
__attribute__((noinline)) void ParseExt4EsFindDelayedExtentRangeEnterData(Ext4EsFindDelayedExtentRangeEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsFindDelayedExtentRangeEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsFindDelayedExtentRangeEnterData eventData {};
    ParseExt4EsFindDelayedExtentRangeEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_find_delayed_extent_range_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_find_delayed_extent_range_enter, ParseExt4EsFindDelayedExtentRangeEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_find_delayed_extent_range_enter, ParseExt4EsFindDelayedExtentRangeEnter<ProtoEncoder::FtraceEvent>);

struct Ext4EsFindDelayedExtentRangeExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    uint32_t len {};
    uint64_t pblk {};
    uint32_t status {};
};
__attribute__((noinline)) void ParseExt4EsFindDelayedExtentRangeExitData(Ext4EsFindDelayedExtentRangeExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsFindDelayedExtentRangeExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsFindDelayedExtentRangeExitData eventData {};
    ParseExt4EsFindDelayedExtentRangeExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_find_delayed_extent_range_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_find_delayed_extent_range_exit, ParseExt4EsFindDelayedExtentRangeExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_find_delayed_extent_range_exit, ParseExt4EsFindDelayedExtentRangeExit<ProtoEncoder::FtraceEvent>);

struct Ext4EsInsertExtentData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    uint32_t len {};
    uint64_t pblk {};
    uint32_t status {};
};
__attribute__((noinline)) void ParseExt4EsInsertExtentData(Ext4EsInsertExtentData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsInsertExtent(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsInsertExtentData eventData {};
    ParseExt4EsInsertExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_insert_extent_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_insert_extent, ParseExt4EsInsertExtent<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_insert_extent, ParseExt4EsInsertExtent<ProtoEncoder::FtraceEvent>);

struct Ext4EsLookupExtentEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
};
__attribute__((noinline)) void ParseExt4EsLookupExtentEnterData(Ext4EsLookupExtentEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsLookupExtentEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsLookupExtentEnterData eventData {};
    ParseExt4EsLookupExtentEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_lookup_extent_enter, ParseExt4EsLookupExtentEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_lookup_extent_enter, ParseExt4EsLookupExtentEnter<ProtoEncoder::FtraceEvent>);

struct Ext4EsLookupExtentExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    uint32_t len {};
    uint64_t pblk {};
    uint32_t status {};
    int32_t found {};
};
__attribute__((noinline)) void ParseExt4EsLookupExtentExitData(Ext4EsLookupExtentExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.found = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsLookupExtentExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsLookupExtentExitData eventData {};
    ParseExt4EsLookupExtentExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
    msg->set_found(eventData.found);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_lookup_extent_exit, ParseExt4EsLookupExtentExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_lookup_extent_exit, ParseExt4EsLookupExtentExit<ProtoEncoder::FtraceEvent>);

struct Ext4EsRemoveExtentData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t lblk {};
    uint64_t len {};
};
__attribute__((noinline)) void ParseExt4EsRemoveExtentData(Ext4EsRemoveExtentData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsRemoveExtent(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsRemoveExtentData eventData {};
    ParseExt4EsRemoveExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_remove_extent_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_remove_extent, ParseExt4EsRemoveExtent<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_remove_extent, ParseExt4EsRemoveExtent<ProtoEncoder::FtraceEvent>);

struct Ext4EsShrinkData {
    uint64_t dev {};
    int32_t nr_shrunk {};
    uint64_t scan_time {};
    int32_t nr_skipped {};
    int32_t retried {};
};
__attribute__((noinline)) void ParseExt4EsShrinkData(Ext4EsShrinkData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_shrunk = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.scan_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_skipped = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.retried = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsShrink(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsShrinkData eventData {};
    ParseExt4EsShrinkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_nr_shrunk(eventData.nr_shrunk);
    msg->set_scan_time(eventData.scan_time);
    msg->set_nr_skipped(eventData.nr_skipped);
    msg->set_retried(eventData.retried);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink, ParseExt4EsShrink<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink, ParseExt4EsShrink<ProtoEncoder::FtraceEvent>);

struct Ext4EsShrinkCountData {
    uint64_t dev {};
    int32_t nr_to_scan {};
    int32_t cache_cnt {};
};
__attribute__((noinline)) void ParseExt4EsShrinkCountData(Ext4EsShrinkCountData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_to_scan = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cache_cnt = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsShrinkCount(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsShrinkCountData eventData {};
    ParseExt4EsShrinkCountData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_count_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_nr_to_scan(eventData.nr_to_scan);
    msg->set_cache_cnt(eventData.cache_cnt);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_count, ParseExt4EsShrinkCount<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_count, ParseExt4EsShrinkCount<ProtoEncoder::FtraceEvent>);

struct Ext4EsShrinkScanEnterData {
    uint64_t dev {};
    int32_t nr_to_scan {};
    int32_t cache_cnt {};
};
__attribute__((noinline)) void ParseExt4EsShrinkScanEnterData(Ext4EsShrinkScanEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_to_scan = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cache_cnt = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsShrinkScanEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsShrinkScanEnterData eventData {};
    ParseExt4EsShrinkScanEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_nr_to_scan(eventData.nr_to_scan);
    msg->set_cache_cnt(eventData.cache_cnt);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_scan_enter, ParseExt4EsShrinkScanEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_scan_enter, ParseExt4EsShrinkScanEnter<ProtoEncoder::FtraceEvent>);

struct Ext4EsShrinkScanExitData {
    uint64_t dev {};
    int32_t nr_shrunk {};
    int32_t cache_cnt {};
};
__attribute__((noinline)) void ParseExt4EsShrinkScanExitData(Ext4EsShrinkScanExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_shrunk = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cache_cnt = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EsShrinkScanExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EsShrinkScanExitData eventData {};
    ParseExt4EsShrinkScanExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_nr_shrunk(eventData.nr_shrunk);
    msg->set_cache_cnt(eventData.cache_cnt);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_scan_exit, ParseExt4EsShrinkScanExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_scan_exit, ParseExt4EsShrinkScanExit<ProtoEncoder::FtraceEvent>);

struct Ext4EvictInodeData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t nlink {};
};
__attribute__((noinline)) void ParseExt4EvictInodeData(Ext4EvictInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nlink = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4EvictInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4EvictInodeData eventData {};
    ParseExt4EvictInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_evict_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_nlink(eventData.nlink);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_evict_inode, ParseExt4EvictInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_evict_inode, ParseExt4EvictInode<ProtoEncoder::FtraceEvent>);

struct Ext4ExtConvertToInitializedEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t m_lblk {};
    uint32_t m_len {};
    uint32_t u_lblk {};
    uint32_t u_len {};
    uint64_t u_pblk {};
};
__attribute__((noinline)) void ParseExt4ExtConvertToInitializedEnterData(Ext4ExtConvertToInitializedEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.m_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.m_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.u_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.u_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.u_pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtConvertToInitializedEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtConvertToInitializedEnterData eventData {};
    ParseExt4ExtConvertToInitializedEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_m_lblk(eventData.m_lblk);
    msg->set_m_len(eventData.m_len);
    msg->set_u_lblk(eventData.u_lblk);
    msg->set_u_len(eventData.u_len);
    msg->set_u_pblk(eventData.u_pblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_convert_to_initialized_enter, ParseExt4ExtConvertToInitializedEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_convert_to_initialized_enter, ParseExt4ExtConvertToInitializedEnter<ProtoEncoder::FtraceEvent>);

struct Ext4ExtConvertToInitializedFastpathData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t m_lblk {};
    uint32_t m_len {};
    uint32_t u_lblk {};
    uint32_t u_len {};
    uint64_t u_pblk {};
    uint32_t i_lblk {};
    uint32_t i_len {};
    uint64_t i_pblk {};
};
__attribute__((noinline)) void ParseExt4ExtConvertToInitializedFastpathData(Ext4ExtConvertToInitializedFastpathData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.m_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.m_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.u_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.u_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.u_pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.i_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.i_pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtConvertToInitializedFastpath(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtConvertToInitializedFastpathData eventData {};
    ParseExt4ExtConvertToInitializedFastpathData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_fastpath_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_m_lblk(eventData.m_lblk);
    msg->set_m_len(eventData.m_len);
    msg->set_u_lblk(eventData.u_lblk);
    msg->set_u_len(eventData.u_len);
    msg->set_u_pblk(eventData.u_pblk);
    msg->set_i_lblk(eventData.i_lblk);
    msg->set_i_len(eventData.i_len);
    msg->set_i_pblk(eventData.i_pblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_convert_to_initialized_fastpath, ParseExt4ExtConvertToInitializedFastpath<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_convert_to_initialized_fastpath, ParseExt4ExtConvertToInitializedFastpath<ProtoEncoder::FtraceEvent>);

struct Ext4ExtHandleUnwrittenExtentsData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t flags {};
    uint32_t lblk {};
    uint64_t pblk {};
    uint32_t len {};
    uint32_t allocated {};
    uint64_t newblk {};
};
__attribute__((noinline)) void ParseExt4ExtHandleUnwrittenExtentsData(Ext4ExtHandleUnwrittenExtentsData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.allocated = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.newblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtHandleUnwrittenExtents(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtHandleUnwrittenExtentsData eventData {};
    ParseExt4ExtHandleUnwrittenExtentsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_handle_unwritten_extents_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_lblk(eventData.lblk);
    msg->set_pblk(eventData.pblk);
    msg->set_len(eventData.len);
    msg->set_allocated(eventData.allocated);
    msg->set_newblk(eventData.newblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_handle_unwritten_extents, ParseExt4ExtHandleUnwrittenExtents<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_handle_unwritten_extents, ParseExt4ExtHandleUnwrittenExtents<ProtoEncoder::FtraceEvent>);

struct Ext4ExtInCacheData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseExt4ExtInCacheData(Ext4ExtInCacheData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtInCache(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtInCacheData eventData {};
    ParseExt4ExtInCacheData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_in_cache_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_in_cache, ParseExt4ExtInCache<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_in_cache, ParseExt4ExtInCache<ProtoEncoder::FtraceEvent>);

struct Ext4ExtLoadExtentData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pblk {};
    uint32_t lblk {};
};
__attribute__((noinline)) void ParseExt4ExtLoadExtentData(Ext4ExtLoadExtentData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtLoadExtent(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtLoadExtentData eventData {};
    ParseExt4ExtLoadExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_load_extent_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_load_extent, ParseExt4ExtLoadExtent<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_load_extent, ParseExt4ExtLoadExtent<ProtoEncoder::FtraceEvent>);

struct Ext4ExtMapBlocksEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    uint32_t len {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseExt4ExtMapBlocksEnterData(Ext4ExtMapBlocksEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtMapBlocksEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtMapBlocksEnterData eventData {};
    ParseExt4ExtMapBlocksEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_map_blocks_enter, ParseExt4ExtMapBlocksEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_map_blocks_enter, ParseExt4ExtMapBlocksEnter<ProtoEncoder::FtraceEvent>);

struct Ext4ExtMapBlocksExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t flags {};
    uint64_t pblk {};
    uint32_t lblk {};
    uint32_t len {};
    uint32_t mflags {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseExt4ExtMapBlocksExitData(Ext4ExtMapBlocksExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mflags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtMapBlocksExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtMapBlocksExitData eventData {};
    ParseExt4ExtMapBlocksExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_mflags(eventData.mflags);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_map_blocks_exit, ParseExt4ExtMapBlocksExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_map_blocks_exit, ParseExt4ExtMapBlocksExit<ProtoEncoder::FtraceEvent>);

struct Ext4ExtPutInCacheData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    uint32_t len {};
    uint64_t start {};
};
__attribute__((noinline)) void ParseExt4ExtPutInCacheData(Ext4ExtPutInCacheData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.start = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtPutInCache(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtPutInCacheData eventData {};
    ParseExt4ExtPutInCacheData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_put_in_cache_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_start(eventData.start);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_put_in_cache, ParseExt4ExtPutInCache<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_put_in_cache, ParseExt4ExtPutInCache<ProtoEncoder::FtraceEvent>);

struct Ext4ExtRemoveSpaceData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t start {};
    uint32_t end {};
    int32_t depth {};
};
__attribute__((noinline)) void ParseExt4ExtRemoveSpaceData(Ext4ExtRemoveSpaceData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.start = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.end = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.depth = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtRemoveSpace(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtRemoveSpaceData eventData {};
    ParseExt4ExtRemoveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_remove_space_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_start(eventData.start);
    msg->set_end(eventData.end);
    msg->set_depth(eventData.depth);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_remove_space, ParseExt4ExtRemoveSpace<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_remove_space, ParseExt4ExtRemoveSpace<ProtoEncoder::FtraceEvent>);

struct Ext4ExtRemoveSpaceDoneData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t start {};
    uint32_t end {};
    int32_t depth {};
    uint64_t partial {};
    uint32_t eh_entries {};
};
__attribute__((noinline)) void ParseExt4ExtRemoveSpaceDoneData(Ext4ExtRemoveSpaceDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.start = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.end = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.depth = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.partial = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.eh_entries = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtRemoveSpaceDone(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtRemoveSpaceDoneData eventData {};
    ParseExt4ExtRemoveSpaceDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_remove_space_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_start(eventData.start);
    msg->set_end(eventData.end);
    msg->set_depth(eventData.depth);
    msg->set_partial(eventData.partial);
    msg->set_eh_entries(eventData.eh_entries);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_remove_space_done, ParseExt4ExtRemoveSpaceDone<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_remove_space_done, ParseExt4ExtRemoveSpaceDone<ProtoEncoder::FtraceEvent>);

struct Ext4ExtRmIdxData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pblk {};
};
__attribute__((noinline)) void ParseExt4ExtRmIdxData(Ext4ExtRmIdxData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtRmIdx(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtRmIdxData eventData {};
    ParseExt4ExtRmIdxData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_rm_idx_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_rm_idx, ParseExt4ExtRmIdx<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_rm_idx, ParseExt4ExtRmIdx<ProtoEncoder::FtraceEvent>);

struct Ext4ExtRmLeafData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t partial {};
    uint32_t start {};
    uint32_t ee_lblk {};
    uint64_t ee_pblk {};
    int32_t ee_len {};
};
__attribute__((noinline)) void ParseExt4ExtRmLeafData(Ext4ExtRmLeafData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.partial = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.start = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ee_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ee_pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ee_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtRmLeaf(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtRmLeafData eventData {};
    ParseExt4ExtRmLeafData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_rm_leaf_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_partial(eventData.partial);
    msg->set_start(eventData.start);
    msg->set_ee_lblk(eventData.ee_lblk);
    msg->set_ee_pblk(eventData.ee_pblk);
    msg->set_ee_len(eventData.ee_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_rm_leaf, ParseExt4ExtRmLeaf<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_rm_leaf, ParseExt4ExtRmLeaf<ProtoEncoder::FtraceEvent>);

struct Ext4ExtShowExtentData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pblk {};
    uint32_t lblk {};
    uint32_t len {};
};
__attribute__((noinline)) void ParseExt4ExtShowExtentData(Ext4ExtShowExtentData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ExtShowExtent(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ExtShowExtentData eventData {};
    ParseExt4ExtShowExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_show_extent_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_show_extent, ParseExt4ExtShowExtent<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_show_extent, ParseExt4ExtShowExtent<ProtoEncoder::FtraceEvent>);

struct Ext4FallocateEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t offset {};
    uint64_t len {};
    int32_t mode {};
};
__attribute__((noinline)) void ParseExt4FallocateEnterData(Ext4FallocateEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4FallocateEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4FallocateEnterData eventData {};
    ParseExt4FallocateEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_fallocate_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_fallocate_enter, ParseExt4FallocateEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_fallocate_enter, ParseExt4FallocateEnter<ProtoEncoder::FtraceEvent>);

struct Ext4FallocateExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t blocks {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseExt4FallocateExitData(Ext4FallocateExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.blocks = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4FallocateExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4FallocateExitData eventData {};
    ParseExt4FallocateExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_fallocate_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_blocks(eventData.blocks);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_fallocate_exit, ParseExt4FallocateExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_fallocate_exit, ParseExt4FallocateExit<ProtoEncoder::FtraceEvent>);

struct Ext4FindDelallocRangeData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t from {};
    uint32_t to {};
    int32_t reverse {};
    int32_t found {};
    uint32_t found_blk {};
};
__attribute__((noinline)) void ParseExt4FindDelallocRangeData(Ext4FindDelallocRangeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.from = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.to = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.reverse = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.found = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.found_blk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4FindDelallocRange(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4FindDelallocRangeData eventData {};
    ParseExt4FindDelallocRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_find_delalloc_range_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_from(eventData.from);
    msg->set_to(eventData.to);
    msg->set_reverse(eventData.reverse);
    msg->set_found(eventData.found);
    msg->set_found_blk(eventData.found_blk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_find_delalloc_range, ParseExt4FindDelallocRange<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_find_delalloc_range, ParseExt4FindDelallocRange<ProtoEncoder::FtraceEvent>);

struct Ext4ForgetData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t block {};
    int32_t is_metadata {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4ForgetData(Ext4ForgetData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.block = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.is_metadata = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4Forget(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ForgetData eventData {};
    ParseExt4ForgetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_forget_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_is_metadata(eventData.is_metadata);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_forget, ParseExt4Forget<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_forget, ParseExt4Forget<ProtoEncoder::FtraceEvent>);

struct Ext4FreeBlocksData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t block {};
    uint64_t count {};
    int32_t flags {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4FreeBlocksData(Ext4FreeBlocksData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.block = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.count = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4FreeBlocks(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4FreeBlocksData eventData {};
    ParseExt4FreeBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_free_blocks_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_count(eventData.count);
    msg->set_flags(eventData.flags);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_free_blocks, ParseExt4FreeBlocks<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_free_blocks, ParseExt4FreeBlocks<ProtoEncoder::FtraceEvent>);

struct Ext4FreeInodeData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t uid {};
    uint32_t gid {};
    uint64_t blocks {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4FreeInodeData(Ext4FreeInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.uid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.blocks = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4FreeInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4FreeInodeData eventData {};
    ParseExt4FreeInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_free_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_uid(eventData.uid);
    msg->set_gid(eventData.gid);
    msg->set_blocks(eventData.blocks);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_free_inode, ParseExt4FreeInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_free_inode, ParseExt4FreeInode<ProtoEncoder::FtraceEvent>);

struct Ext4GetImpliedClusterAllocExitData {
    uint64_t dev {};
    uint32_t flags {};
    uint32_t lblk {};
    uint64_t pblk {};
    uint32_t len {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseExt4GetImpliedClusterAllocExitData(Ext4GetImpliedClusterAllocExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4GetImpliedClusterAllocExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4GetImpliedClusterAllocExitData eventData {};
    ParseExt4GetImpliedClusterAllocExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_get_implied_cluster_alloc_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_flags(eventData.flags);
    msg->set_lblk(eventData.lblk);
    msg->set_pblk(eventData.pblk);
    msg->set_len(eventData.len);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_get_implied_cluster_alloc_exit, ParseExt4GetImpliedClusterAllocExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_get_implied_cluster_alloc_exit, ParseExt4GetImpliedClusterAllocExit<ProtoEncoder::FtraceEvent>);

struct Ext4GetReservedClusterAllocData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    uint32_t len {};
};
__attribute__((noinline)) void ParseExt4GetReservedClusterAllocData(Ext4GetReservedClusterAllocData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4GetReservedClusterAlloc(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4GetReservedClusterAllocData eventData {};
    ParseExt4GetReservedClusterAllocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_get_reserved_cluster_alloc_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_get_reserved_cluster_alloc, ParseExt4GetReservedClusterAlloc<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_get_reserved_cluster_alloc, ParseExt4GetReservedClusterAlloc<ProtoEncoder::FtraceEvent>);

struct Ext4IndMapBlocksEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t lblk {};
    uint32_t len {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseExt4IndMapBlocksEnterData(Ext4IndMapBlocksEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4IndMapBlocksEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4IndMapBlocksEnterData eventData {};
    ParseExt4IndMapBlocksEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ind_map_blocks_enter, ParseExt4IndMapBlocksEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ind_map_blocks_enter, ParseExt4IndMapBlocksEnter<ProtoEncoder::FtraceEvent>);

struct Ext4IndMapBlocksExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t flags {};
    uint64_t pblk {};
    uint32_t lblk {};
    uint32_t len {};
    uint32_t mflags {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseExt4IndMapBlocksExitData(Ext4IndMapBlocksExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mflags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4IndMapBlocksExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4IndMapBlocksExitData eventData {};
    ParseExt4IndMapBlocksExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_mflags(eventData.mflags);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ind_map_blocks_exit, ParseExt4IndMapBlocksExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ind_map_blocks_exit, ParseExt4IndMapBlocksExit<ProtoEncoder::FtraceEvent>);

struct Ext4InsertRangeData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t offset {};
    uint64_t len {};
};
__attribute__((noinline)) void ParseExt4InsertRangeData(Ext4InsertRangeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4InsertRange(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4InsertRangeData eventData {};
    ParseExt4InsertRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_insert_range_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_insert_range, ParseExt4InsertRange<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_insert_range, ParseExt4InsertRange<ProtoEncoder::FtraceEvent>);

struct Ext4InvalidatepageData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t index {};
    uint32_t offset {};
    uint32_t length {};
};
__attribute__((noinline)) void ParseExt4InvalidatepageData(Ext4InvalidatepageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.length = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4Invalidatepage(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4InvalidatepageData eventData {};
    ParseExt4InvalidatepageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_invalidatepage_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_index(eventData.index);
    msg->set_offset(eventData.offset);
    msg->set_length(eventData.length);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_invalidatepage, ParseExt4Invalidatepage<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_invalidatepage, ParseExt4Invalidatepage<ProtoEncoder::FtraceEvent>);

struct Ext4JournalStartData {
    uint64_t dev {};
    uint64_t ip {};
    int32_t blocks {};
    int32_t rsv_blocks {};
};
__attribute__((noinline)) void ParseExt4JournalStartData(Ext4JournalStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ip = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.blocks = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.rsv_blocks = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4JournalStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4JournalStartData eventData {};
    ParseExt4JournalStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_journal_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ip(eventData.ip);
    msg->set_blocks(eventData.blocks);
    msg->set_rsv_blocks(eventData.rsv_blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journal_start, ParseExt4JournalStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journal_start, ParseExt4JournalStart<ProtoEncoder::FtraceEvent>);

struct Ext4JournalStartReservedData {
    uint64_t dev {};
    uint64_t ip {};
    int32_t blocks {};
};
__attribute__((noinline)) void ParseExt4JournalStartReservedData(Ext4JournalStartReservedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ip = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.blocks = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4JournalStartReserved(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4JournalStartReservedData eventData {};
    ParseExt4JournalStartReservedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_journal_start_reserved_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ip(eventData.ip);
    msg->set_blocks(eventData.blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journal_start_reserved, ParseExt4JournalStartReserved<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journal_start_reserved, ParseExt4JournalStartReserved<ProtoEncoder::FtraceEvent>);

struct Ext4JournalledInvalidatepageData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t index {};
    uint32_t offset {};
    uint32_t length {};
};
__attribute__((noinline)) void ParseExt4JournalledInvalidatepageData(Ext4JournalledInvalidatepageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.length = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4JournalledInvalidatepage(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4JournalledInvalidatepageData eventData {};
    ParseExt4JournalledInvalidatepageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_journalled_invalidatepage_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_index(eventData.index);
    msg->set_offset(eventData.offset);
    msg->set_length(eventData.length);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journalled_invalidatepage, ParseExt4JournalledInvalidatepage<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journalled_invalidatepage, ParseExt4JournalledInvalidatepage<ProtoEncoder::FtraceEvent>);

struct Ext4JournalledWriteEndData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
    uint32_t copied {};
};
__attribute__((noinline)) void ParseExt4JournalledWriteEndData(Ext4JournalledWriteEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.copied = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4JournalledWriteEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4JournalledWriteEndData eventData {};
    ParseExt4JournalledWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_journalled_write_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journalled_write_end, ParseExt4JournalledWriteEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journalled_write_end, ParseExt4JournalledWriteEnd<ProtoEncoder::FtraceEvent>);

struct Ext4LoadInodeData {
    uint64_t dev {};
    uint64_t ino {};
};
__attribute__((noinline)) void ParseExt4LoadInodeData(Ext4LoadInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4LoadInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4LoadInodeData eventData {};
    ParseExt4LoadInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_load_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_load_inode, ParseExt4LoadInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_load_inode, ParseExt4LoadInode<ProtoEncoder::FtraceEvent>);

struct Ext4LoadInodeBitmapData {
    uint64_t dev {};
    uint32_t group {};
};
__attribute__((noinline)) void ParseExt4LoadInodeBitmapData(Ext4LoadInodeBitmapData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4LoadInodeBitmap(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4LoadInodeBitmapData eventData {};
    ParseExt4LoadInodeBitmapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_load_inode_bitmap_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_load_inode_bitmap, ParseExt4LoadInodeBitmap<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_load_inode_bitmap, ParseExt4LoadInodeBitmap<ProtoEncoder::FtraceEvent>);

struct Ext4MarkInodeDirtyData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t ip {};
};
__attribute__((noinline)) void ParseExt4MarkInodeDirtyData(Ext4MarkInodeDirtyData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ip = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MarkInodeDirty(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MarkInodeDirtyData eventData {};
    ParseExt4MarkInodeDirtyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mark_inode_dirty_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ip(eventData.ip);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mark_inode_dirty, ParseExt4MarkInodeDirty<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mark_inode_dirty, ParseExt4MarkInodeDirty<ProtoEncoder::FtraceEvent>);

struct Ext4MbBitmapLoadData {
    uint64_t dev {};
    uint32_t group {};
};
__attribute__((noinline)) void ParseExt4MbBitmapLoadData(Ext4MbBitmapLoadData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MbBitmapLoad(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MbBitmapLoadData eventData {};
    ParseExt4MbBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_bitmap_load_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_bitmap_load, ParseExt4MbBitmapLoad<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_bitmap_load, ParseExt4MbBitmapLoad<ProtoEncoder::FtraceEvent>);

struct Ext4MbBuddyBitmapLoadData {
    uint64_t dev {};
    uint32_t group {};
};
__attribute__((noinline)) void ParseExt4MbBuddyBitmapLoadData(Ext4MbBuddyBitmapLoadData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MbBuddyBitmapLoad(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MbBuddyBitmapLoadData eventData {};
    ParseExt4MbBuddyBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_buddy_bitmap_load_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_buddy_bitmap_load, ParseExt4MbBuddyBitmapLoad<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_buddy_bitmap_load, ParseExt4MbBuddyBitmapLoad<ProtoEncoder::FtraceEvent>);

struct Ext4MbDiscardPreallocationsData {
    uint64_t dev {};
    int32_t needed {};
};
__attribute__((noinline)) void ParseExt4MbDiscardPreallocationsData(Ext4MbDiscardPreallocationsData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.needed = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MbDiscardPreallocations(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MbDiscardPreallocationsData eventData {};
    ParseExt4MbDiscardPreallocationsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_discard_preallocations_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_needed(eventData.needed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_discard_preallocations, ParseExt4MbDiscardPreallocations<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_discard_preallocations, ParseExt4MbDiscardPreallocations<ProtoEncoder::FtraceEvent>);

struct Ext4MbNewGroupPaData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pa_pstart {};
    uint64_t pa_lstart {};
    uint32_t pa_len {};
};
__attribute__((noinline)) void ParseExt4MbNewGroupPaData(Ext4MbNewGroupPaData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pa_pstart = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pa_lstart = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pa_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MbNewGroupPa(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MbNewGroupPaData eventData {};
    ParseExt4MbNewGroupPaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_new_group_pa_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_lstart(eventData.pa_lstart);
    msg->set_pa_len(eventData.pa_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_new_group_pa, ParseExt4MbNewGroupPa<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_new_group_pa, ParseExt4MbNewGroupPa<ProtoEncoder::FtraceEvent>);

struct Ext4MbNewInodePaData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pa_pstart {};
    uint64_t pa_lstart {};
    uint32_t pa_len {};
};
__attribute__((noinline)) void ParseExt4MbNewInodePaData(Ext4MbNewInodePaData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pa_pstart = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pa_lstart = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pa_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MbNewInodePa(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MbNewInodePaData eventData {};
    ParseExt4MbNewInodePaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_new_inode_pa_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_lstart(eventData.pa_lstart);
    msg->set_pa_len(eventData.pa_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_new_inode_pa, ParseExt4MbNewInodePa<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_new_inode_pa, ParseExt4MbNewInodePa<ProtoEncoder::FtraceEvent>);

struct Ext4MbReleaseGroupPaData {
    uint64_t dev {};
    uint64_t pa_pstart {};
    uint32_t pa_len {};
};
__attribute__((noinline)) void ParseExt4MbReleaseGroupPaData(Ext4MbReleaseGroupPaData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pa_pstart = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pa_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MbReleaseGroupPa(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MbReleaseGroupPaData eventData {};
    ParseExt4MbReleaseGroupPaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_release_group_pa_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_len(eventData.pa_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_release_group_pa, ParseExt4MbReleaseGroupPa<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_release_group_pa, ParseExt4MbReleaseGroupPa<ProtoEncoder::FtraceEvent>);

struct Ext4MbReleaseInodePaData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t block {};
    uint32_t count {};
};
__attribute__((noinline)) void ParseExt4MbReleaseInodePaData(Ext4MbReleaseInodePaData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.block = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.count = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MbReleaseInodePa(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MbReleaseInodePaData eventData {};
    ParseExt4MbReleaseInodePaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_release_inode_pa_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_count(eventData.count);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_release_inode_pa, ParseExt4MbReleaseInodePa<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_release_inode_pa, ParseExt4MbReleaseInodePa<ProtoEncoder::FtraceEvent>);

struct Ext4MballocAllocData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t orig_logical {};
    int32_t orig_start {};
    uint32_t orig_group {};
    int32_t orig_len {};
    uint32_t goal_logical {};
    int32_t goal_start {};
    uint32_t goal_group {};
    int32_t goal_len {};
    uint32_t result_logical {};
    int32_t result_start {};
    uint32_t result_group {};
    int32_t result_len {};
    uint32_t found {};
    uint32_t groups {};
    uint32_t buddy {};
    uint32_t flags {};
    uint32_t tail {};
    uint32_t cr {};
};
__attribute__((noinline)) void ParseExt4MballocAllocData(Ext4MballocAllocData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.orig_logical = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.orig_start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.orig_group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.orig_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.goal_logical = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.goal_start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.goal_group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.goal_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.result_logical = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.result_start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.result_group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.result_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.found = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.groups = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.buddy = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.tail = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MballocAlloc(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MballocAllocData eventData {};
    ParseExt4MballocAllocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_alloc_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_orig_logical(eventData.orig_logical);
    msg->set_orig_start(eventData.orig_start);
    msg->set_orig_group(eventData.orig_group);
    msg->set_orig_len(eventData.orig_len);
    msg->set_goal_logical(eventData.goal_logical);
    msg->set_goal_start(eventData.goal_start);
    msg->set_goal_group(eventData.goal_group);
    msg->set_goal_len(eventData.goal_len);
    msg->set_result_logical(eventData.result_logical);
    msg->set_result_start(eventData.result_start);
    msg->set_result_group(eventData.result_group);
    msg->set_result_len(eventData.result_len);
    msg->set_found(eventData.found);
    msg->set_groups(eventData.groups);
    msg->set_buddy(eventData.buddy);
    msg->set_flags(eventData.flags);
    msg->set_tail(eventData.tail);
    msg->set_cr(eventData.cr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_alloc, ParseExt4MballocAlloc<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_alloc, ParseExt4MballocAlloc<ProtoEncoder::FtraceEvent>);

struct Ext4MballocDiscardData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t result_start {};
    uint32_t result_group {};
    int32_t result_len {};
};
__attribute__((noinline)) void ParseExt4MballocDiscardData(Ext4MballocDiscardData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.result_start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.result_group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.result_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MballocDiscard(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MballocDiscardData eventData {};
    ParseExt4MballocDiscardData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_discard_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_result_start(eventData.result_start);
    msg->set_result_group(eventData.result_group);
    msg->set_result_len(eventData.result_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_discard, ParseExt4MballocDiscard<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_discard, ParseExt4MballocDiscard<ProtoEncoder::FtraceEvent>);

struct Ext4MballocFreeData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t result_start {};
    uint32_t result_group {};
    int32_t result_len {};
};
__attribute__((noinline)) void ParseExt4MballocFreeData(Ext4MballocFreeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.result_start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.result_group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.result_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MballocFree(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MballocFreeData eventData {};
    ParseExt4MballocFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_free_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_result_start(eventData.result_start);
    msg->set_result_group(eventData.result_group);
    msg->set_result_len(eventData.result_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_free, ParseExt4MballocFree<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_free, ParseExt4MballocFree<ProtoEncoder::FtraceEvent>);

struct Ext4MballocPreallocData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t orig_logical {};
    int32_t orig_start {};
    uint32_t orig_group {};
    int32_t orig_len {};
    uint32_t result_logical {};
    int32_t result_start {};
    uint32_t result_group {};
    int32_t result_len {};
};
__attribute__((noinline)) void ParseExt4MballocPreallocData(Ext4MballocPreallocData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.orig_logical = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.orig_start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.orig_group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.orig_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.result_logical = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.result_start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.result_group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.result_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4MballocPrealloc(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4MballocPreallocData eventData {};
    ParseExt4MballocPreallocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_prealloc_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_orig_logical(eventData.orig_logical);
    msg->set_orig_start(eventData.orig_start);
    msg->set_orig_group(eventData.orig_group);
    msg->set_orig_len(eventData.orig_len);
    msg->set_result_logical(eventData.result_logical);
    msg->set_result_start(eventData.result_start);
    msg->set_result_group(eventData.result_group);
    msg->set_result_len(eventData.result_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_prealloc, ParseExt4MballocPrealloc<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_prealloc, ParseExt4MballocPrealloc<ProtoEncoder::FtraceEvent>);

struct Ext4OtherInodeUpdateTimeData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t orig_ino {};
    uint32_t uid {};
    uint32_t gid {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4OtherInodeUpdateTimeData(Ext4OtherInodeUpdateTimeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.orig_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.uid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4OtherInodeUpdateTime(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4OtherInodeUpdateTimeData eventData {};
    ParseExt4OtherInodeUpdateTimeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_other_inode_update_time_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_orig_ino(eventData.orig_ino);
    msg->set_uid(eventData.uid);
    msg->set_gid(eventData.gid);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_other_inode_update_time, ParseExt4OtherInodeUpdateTime<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_other_inode_update_time, ParseExt4OtherInodeUpdateTime<ProtoEncoder::FtraceEvent>);

struct Ext4PunchHoleData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t offset {};
    uint64_t len {};
    int32_t mode {};
};
__attribute__((noinline)) void ParseExt4PunchHoleData(Ext4PunchHoleData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4PunchHole(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4PunchHoleData eventData {};
    ParseExt4PunchHoleData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_punch_hole_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_punch_hole, ParseExt4PunchHole<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_punch_hole, ParseExt4PunchHole<ProtoEncoder::FtraceEvent>);

struct Ext4ReadBlockBitmapLoadData {
    uint64_t dev {};
    uint32_t group {};
};
__attribute__((noinline)) void ParseExt4ReadBlockBitmapLoadData(Ext4ReadBlockBitmapLoadData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ReadBlockBitmapLoad(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ReadBlockBitmapLoadData eventData {};
    ParseExt4ReadBlockBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_read_block_bitmap_load_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_read_block_bitmap_load, ParseExt4ReadBlockBitmapLoad<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_read_block_bitmap_load, ParseExt4ReadBlockBitmapLoad<ProtoEncoder::FtraceEvent>);

struct Ext4ReadpageData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t index {};
};
__attribute__((noinline)) void ParseExt4ReadpageData(Ext4ReadpageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4Readpage(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ReadpageData eventData {};
    ParseExt4ReadpageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_readpage_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_index(eventData.index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_readpage, ParseExt4Readpage<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_readpage, ParseExt4Readpage<ProtoEncoder::FtraceEvent>);

struct Ext4ReleasepageData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t index {};
};
__attribute__((noinline)) void ParseExt4ReleasepageData(Ext4ReleasepageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4Releasepage(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ReleasepageData eventData {};
    ParseExt4ReleasepageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_releasepage_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_index(eventData.index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_releasepage, ParseExt4Releasepage<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_releasepage, ParseExt4Releasepage<ProtoEncoder::FtraceEvent>);

struct Ext4RemoveBlocksData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t from {};
    uint32_t to {};
    uint64_t partial {};
    uint64_t ee_pblk {};
    uint32_t ee_lblk {};
    uint32_t ee_len {};
};
__attribute__((noinline)) void ParseExt4RemoveBlocksData(Ext4RemoveBlocksData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.from = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.to = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.partial = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ee_pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ee_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ee_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4RemoveBlocks(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4RemoveBlocksData eventData {};
    ParseExt4RemoveBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_remove_blocks_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_from(eventData.from);
    msg->set_to(eventData.to);
    msg->set_partial(eventData.partial);
    msg->set_ee_pblk(eventData.ee_pblk);
    msg->set_ee_lblk(eventData.ee_lblk);
    msg->set_ee_len(eventData.ee_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_remove_blocks, ParseExt4RemoveBlocks<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_remove_blocks, ParseExt4RemoveBlocks<ProtoEncoder::FtraceEvent>);

struct Ext4RequestBlocksData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t len {};
    uint32_t logical {};
    uint32_t lleft {};
    uint32_t lright {};
    uint64_t goal {};
    uint64_t pleft {};
    uint64_t pright {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseExt4RequestBlocksData(Ext4RequestBlocksData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.logical = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.lleft = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.lright = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.goal = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pleft = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pright = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4RequestBlocks(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4RequestBlocksData eventData {};
    ParseExt4RequestBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_request_blocks_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_len(eventData.len);
    msg->set_logical(eventData.logical);
    msg->set_lleft(eventData.lleft);
    msg->set_lright(eventData.lright);
    msg->set_goal(eventData.goal);
    msg->set_pleft(eventData.pleft);
    msg->set_pright(eventData.pright);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_request_blocks, ParseExt4RequestBlocks<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_request_blocks, ParseExt4RequestBlocks<ProtoEncoder::FtraceEvent>);

struct Ext4RequestInodeData {
    uint64_t dev {};
    uint64_t dir {};
    uint32_t mode {};
};
__attribute__((noinline)) void ParseExt4RequestInodeData(Ext4RequestInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dir = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4RequestInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4RequestInodeData eventData {};
    ParseExt4RequestInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_request_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_dir(eventData.dir);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_request_inode, ParseExt4RequestInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_request_inode, ParseExt4RequestInode<ProtoEncoder::FtraceEvent>);

struct Ext4SyncFileEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t parent {};
    int32_t datasync {};
};
__attribute__((noinline)) void ParseExt4SyncFileEnterData(Ext4SyncFileEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.parent = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.datasync = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4SyncFileEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4SyncFileEnterData eventData {};
    ParseExt4SyncFileEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_file_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_parent(eventData.parent);
    msg->set_datasync(eventData.datasync);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_file_enter, ParseExt4SyncFileEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_file_enter, ParseExt4SyncFileEnter<ProtoEncoder::FtraceEvent>);

struct Ext4SyncFileExitData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseExt4SyncFileExitData(Ext4SyncFileExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4SyncFileExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4SyncFileExitData eventData {};
    ParseExt4SyncFileExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_file_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_file_exit, ParseExt4SyncFileExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_file_exit, ParseExt4SyncFileExit<ProtoEncoder::FtraceEvent>);

struct Ext4SyncFsData {
    uint64_t dev {};
    int32_t wait {};
};
__attribute__((noinline)) void ParseExt4SyncFsData(Ext4SyncFsData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.wait = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4SyncFs(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4SyncFsData eventData {};
    ParseExt4SyncFsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_fs_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_wait(eventData.wait);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_fs, ParseExt4SyncFs<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_fs, ParseExt4SyncFs<ProtoEncoder::FtraceEvent>);

struct Ext4TrimAllFreeData {
    int32_t dev_major {};
    int32_t dev_minor {};
    uint32_t group {};
    int32_t start {};
    int32_t len {};
};
__attribute__((noinline)) void ParseExt4TrimAllFreeData(Ext4TrimAllFreeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev_major = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dev_minor = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4TrimAllFree(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4TrimAllFreeData eventData {};
    ParseExt4TrimAllFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_trim_all_free_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev_major(eventData.dev_major);
    msg->set_dev_minor(eventData.dev_minor);
    msg->set_group(eventData.group);
    msg->set_start(eventData.start);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_trim_all_free, ParseExt4TrimAllFree<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_trim_all_free, ParseExt4TrimAllFree<ProtoEncoder::FtraceEvent>);

struct Ext4TrimExtentData {
    int32_t dev_major {};
    int32_t dev_minor {};
    uint32_t group {};
    int32_t start {};
    int32_t len {};
};
__attribute__((noinline)) void ParseExt4TrimExtentData(Ext4TrimExtentData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev_major = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dev_minor = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.start = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4TrimExtent(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4TrimExtentData eventData {};
    ParseExt4TrimExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_trim_extent_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev_major(eventData.dev_major);
    msg->set_dev_minor(eventData.dev_minor);
    msg->set_group(eventData.group);
    msg->set_start(eventData.start);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_trim_extent, ParseExt4TrimExtent<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_trim_extent, ParseExt4TrimExtent<ProtoEncoder::FtraceEvent>);

struct Ext4TruncateEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t blocks {};
};
__attribute__((noinline)) void ParseExt4TruncateEnterData(Ext4TruncateEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.blocks = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4TruncateEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4TruncateEnterData eventData {};
    ParseExt4TruncateEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_truncate_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_blocks(eventData.blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_truncate_enter, ParseExt4TruncateEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_truncate_enter, ParseExt4TruncateEnter<ProtoEncoder::FtraceEvent>);

struct Ext4TruncateExitData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t blocks {};
};
__attribute__((noinline)) void ParseExt4TruncateExitData(Ext4TruncateExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.blocks = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4TruncateExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4TruncateExitData eventData {};
    ParseExt4TruncateExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_truncate_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_blocks(eventData.blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_truncate_exit, ParseExt4TruncateExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_truncate_exit, ParseExt4TruncateExit<ProtoEncoder::FtraceEvent>);

struct Ext4UnlinkEnterData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t parent {};
    uint64_t size {};
};
__attribute__((noinline)) void ParseExt4UnlinkEnterData(Ext4UnlinkEnterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.parent = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4UnlinkEnter(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4UnlinkEnterData eventData {};
    ParseExt4UnlinkEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_unlink_enter_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_parent(eventData.parent);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_unlink_enter, ParseExt4UnlinkEnter<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_unlink_enter, ParseExt4UnlinkEnter<ProtoEncoder::FtraceEvent>);

struct Ext4UnlinkExitData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseExt4UnlinkExitData(Ext4UnlinkExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4UnlinkExit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4UnlinkExitData eventData {};
    ParseExt4UnlinkExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_unlink_exit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_unlink_exit, ParseExt4UnlinkExit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_unlink_exit, ParseExt4UnlinkExit<ProtoEncoder::FtraceEvent>);

struct Ext4WriteBeginData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseExt4WriteBeginData(Ext4WriteBeginData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4WriteBegin(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4WriteBeginData eventData {};
    ParseExt4WriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_write_begin_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_write_begin, ParseExt4WriteBegin<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_write_begin, ParseExt4WriteBegin<ProtoEncoder::FtraceEvent>);

struct Ext4WriteEndData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
    uint32_t copied {};
};
__attribute__((noinline)) void ParseExt4WriteEndData(Ext4WriteEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.copied = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4WriteEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4WriteEndData eventData {};
    ParseExt4WriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_write_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_write_end, ParseExt4WriteEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_write_end, ParseExt4WriteEnd<ProtoEncoder::FtraceEvent>);

struct Ext4WritepageData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t index {};
};
__attribute__((noinline)) void ParseExt4WritepageData(Ext4WritepageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4Writepage(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4WritepageData eventData {};
    ParseExt4WritepageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_writepage_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_index(eventData.index);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_writepage, ParseExt4Writepage<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_writepage, ParseExt4Writepage<ProtoEncoder::FtraceEvent>);

struct Ext4WritepagesData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t nr_to_write {};
    uint64_t pages_skipped {};
    uint64_t range_start {};
    uint64_t range_end {};
    uint64_t writeback_index {};
    int32_t sync_mode {};
    uint32_t for_kupdate {};
    uint32_t range_cyclic {};
};
__attribute__((noinline)) void ParseExt4WritepagesData(Ext4WritepagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_to_write = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pages_skipped = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.range_start = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.range_end = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.writeback_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_kupdate = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.range_cyclic = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4Writepages(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4WritepagesData eventData {};
    ParseExt4WritepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_writepages_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_nr_to_write(eventData.nr_to_write);
    msg->set_pages_skipped(eventData.pages_skipped);
    msg->set_range_start(eventData.range_start);
    msg->set_range_end(eventData.range_end);
    msg->set_writeback_index(eventData.writeback_index);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_for_kupdate(eventData.for_kupdate);
    msg->set_range_cyclic(eventData.range_cyclic);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_writepages, ParseExt4Writepages<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_writepages, ParseExt4Writepages<ProtoEncoder::FtraceEvent>);

struct Ext4WritepagesResultData {
    uint64_t dev {};
    uint64_t ino {};
    int32_t ret {};
    int32_t pages_written {};
    uint64_t pages_skipped {};
    uint64_t writeback_index {};
    int32_t sync_mode {};
};
__attribute__((noinline)) void ParseExt4WritepagesResultData(Ext4WritepagesResultData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.pages_written = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.pages_skipped = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.writeback_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4WritepagesResult(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4WritepagesResultData eventData {};
    ParseExt4WritepagesResultData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_writepages_result_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
    msg->set_pages_written(eventData.pages_written);
    msg->set_pages_skipped(eventData.pages_skipped);
    msg->set_writeback_index(eventData.writeback_index);
    msg->set_sync_mode(eventData.sync_mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_writepages_result, ParseExt4WritepagesResult<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_writepages_result, ParseExt4WritepagesResult<ProtoEncoder::FtraceEvent>);

struct Ext4ZeroRangeData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t offset {};
    uint64_t len {};
    int32_t mode {};
};
__attribute__((noinline)) void ParseExt4ZeroRangeData(Ext4ZeroRangeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseExt4ZeroRange(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    Ext4ZeroRangeData eventData {};
    ParseExt4ZeroRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_zero_range_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_zero_range, ParseExt4ZeroRange<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_zero_range, ParseExt4ZeroRange<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
