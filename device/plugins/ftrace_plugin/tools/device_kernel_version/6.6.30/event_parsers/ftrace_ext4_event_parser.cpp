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
void ParseExt4AllocDaBlocks(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4AllocDaBlocksData eventData {};
    ParseExt4AllocDaBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_alloc_da_blocks_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_data_blocks(eventData.data_blocks);
}
void ParseExt4AllocDaBlocksEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4AllocDaBlocksData eventData {};
    ParseExt4AllocDaBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_alloc_da_blocks_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_data_blocks(eventData.data_blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_alloc_da_blocks, ParseExt4AllocDaBlocks);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_alloc_da_blocks, ParseExt4AllocDaBlocksEncoder);

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
void ParseExt4AllocateBlocks(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4AllocateBlocksData eventData {};
    ParseExt4AllocateBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_allocate_blocks_format();
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
void ParseExt4AllocateBlocksEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4AllocateBlocksData eventData {};
    ParseExt4AllocateBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_allocate_blocks_format();
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_allocate_blocks, ParseExt4AllocateBlocks);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_allocate_blocks, ParseExt4AllocateBlocksEncoder);

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
void ParseExt4AllocateInode(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4AllocateInodeData eventData {};
    ParseExt4AllocateInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_allocate_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_dir(eventData.dir);
    msg->set_mode(eventData.mode);
}
void ParseExt4AllocateInodeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4AllocateInodeData eventData {};
    ParseExt4AllocateInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_allocate_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_dir(eventData.dir);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_allocate_inode, ParseExt4AllocateInode);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_allocate_inode, ParseExt4AllocateInodeEncoder);

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
void ParseExt4BeginOrderedTruncate(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4BeginOrderedTruncateData eventData {};
    ParseExt4BeginOrderedTruncateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_begin_ordered_truncate_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_new_size(eventData.new_size);
}
void ParseExt4BeginOrderedTruncateEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4BeginOrderedTruncateData eventData {};
    ParseExt4BeginOrderedTruncateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_begin_ordered_truncate_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_new_size(eventData.new_size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_begin_ordered_truncate, ParseExt4BeginOrderedTruncate);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_begin_ordered_truncate, ParseExt4BeginOrderedTruncateEncoder);

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
void ParseExt4CollapseRange(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4CollapseRangeData eventData {};
    ParseExt4CollapseRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_collapse_range_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
}
void ParseExt4CollapseRangeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4CollapseRangeData eventData {};
    ParseExt4CollapseRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_collapse_range_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_collapse_range, ParseExt4CollapseRange);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_collapse_range, ParseExt4CollapseRangeEncoder);

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
void ParseExt4DaReleaseSpace(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaReleaseSpaceData eventData {};
    ParseExt4DaReleaseSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_release_space_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_freed_blocks(eventData.freed_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_mode(eventData.mode);
}
void ParseExt4DaReleaseSpaceEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaReleaseSpaceData eventData {};
    ParseExt4DaReleaseSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_release_space_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_freed_blocks(eventData.freed_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_release_space, ParseExt4DaReleaseSpace);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_release_space, ParseExt4DaReleaseSpaceEncoder);

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
void ParseExt4DaReserveSpace(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaReserveSpaceData eventData {};
    ParseExt4DaReserveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_reserve_space_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_mode(eventData.mode);
}
void ParseExt4DaReserveSpaceEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaReserveSpaceData eventData {};
    ParseExt4DaReserveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_reserve_space_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_reserve_space, ParseExt4DaReserveSpace);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_reserve_space, ParseExt4DaReserveSpaceEncoder);

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
void ParseExt4DaUpdateReserveSpace(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaUpdateReserveSpaceData eventData {};
    ParseExt4DaUpdateReserveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_update_reserve_space_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_used_blocks(eventData.used_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_quota_claim(eventData.quota_claim);
    msg->set_mode(eventData.mode);
}
void ParseExt4DaUpdateReserveSpaceEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaUpdateReserveSpaceData eventData {};
    ParseExt4DaUpdateReserveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_update_reserve_space_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_i_blocks(eventData.i_blocks);
    msg->set_used_blocks(eventData.used_blocks);
    msg->set_reserved_data_blocks(eventData.reserved_data_blocks);
    msg->set_quota_claim(eventData.quota_claim);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_update_reserve_space, ParseExt4DaUpdateReserveSpace);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_update_reserve_space, ParseExt4DaUpdateReserveSpaceEncoder);

struct Ext4DaWriteBeginData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
};
__attribute__((noinline)) void ParseExt4DaWriteBeginData(Ext4DaWriteBeginData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseExt4DaWriteBegin(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaWriteBeginData eventData {};
    ParseExt4DaWriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_begin_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
}
void ParseExt4DaWriteBeginEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaWriteBeginData eventData {};
    ParseExt4DaWriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_begin_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_begin, ParseExt4DaWriteBegin);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_begin, ParseExt4DaWriteBeginEncoder);

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
void ParseExt4DaWriteEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaWriteEndData eventData {};
    ParseExt4DaWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_end_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
void ParseExt4DaWriteEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaWriteEndData eventData {};
    ParseExt4DaWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_end_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_end, ParseExt4DaWriteEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_end, ParseExt4DaWriteEndEncoder);

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
void ParseExt4DaWritePages(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaWritePagesData eventData {};
    ParseExt4DaWritePagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_pages_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_first_page(eventData.first_page);
    msg->set_nr_to_write(eventData.nr_to_write);
    msg->set_sync_mode(eventData.sync_mode);
}
void ParseExt4DaWritePagesEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaWritePagesData eventData {};
    ParseExt4DaWritePagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_pages_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_first_page(eventData.first_page);
    msg->set_nr_to_write(eventData.nr_to_write);
    msg->set_sync_mode(eventData.sync_mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_pages, ParseExt4DaWritePages);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_pages, ParseExt4DaWritePagesEncoder);

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
void ParseExt4DaWritePagesExtent(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaWritePagesExtentData eventData {};
    ParseExt4DaWritePagesExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_pages_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
void ParseExt4DaWritePagesExtentEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DaWritePagesExtentData eventData {};
    ParseExt4DaWritePagesExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_da_write_pages_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_da_write_pages_extent, ParseExt4DaWritePagesExtent);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_da_write_pages_extent, ParseExt4DaWritePagesExtentEncoder);

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
void ParseExt4DiscardBlocks(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DiscardBlocksData eventData {};
    ParseExt4DiscardBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_discard_blocks_format();
    msg->set_dev(eventData.dev);
    msg->set_blk(eventData.blk);
    msg->set_count(eventData.count);
}
void ParseExt4DiscardBlocksEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DiscardBlocksData eventData {};
    ParseExt4DiscardBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_discard_blocks_format();
    msg->set_dev(eventData.dev);
    msg->set_blk(eventData.blk);
    msg->set_count(eventData.count);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_discard_blocks, ParseExt4DiscardBlocks);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_discard_blocks, ParseExt4DiscardBlocksEncoder);

struct Ext4DiscardPreallocationsData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t len {};
    uint32_t needed {};
};
__attribute__((noinline)) void ParseExt4DiscardPreallocationsData(Ext4DiscardPreallocationsData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.needed = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseExt4DiscardPreallocations(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DiscardPreallocationsData eventData {};
    ParseExt4DiscardPreallocationsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_discard_preallocations_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_len(eventData.len);
    msg->set_needed(eventData.needed);
}
void ParseExt4DiscardPreallocationsEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DiscardPreallocationsData eventData {};
    ParseExt4DiscardPreallocationsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_discard_preallocations_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_len(eventData.len);
    msg->set_needed(eventData.needed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_discard_preallocations, ParseExt4DiscardPreallocations);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_discard_preallocations, ParseExt4DiscardPreallocationsEncoder);

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
void ParseExt4DropInode(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DropInodeData eventData {};
    ParseExt4DropInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_drop_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_drop(eventData.drop);
}
void ParseExt4DropInodeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4DropInodeData eventData {};
    ParseExt4DropInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_drop_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_drop(eventData.drop);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_drop_inode, ParseExt4DropInode);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_drop_inode, ParseExt4DropInodeEncoder);

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
void ParseExt4EsCacheExtent(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsCacheExtentData eventData {};
    ParseExt4EsCacheExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_cache_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
}
void ParseExt4EsCacheExtentEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsCacheExtentData eventData {};
    ParseExt4EsCacheExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_cache_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_cache_extent, ParseExt4EsCacheExtent);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_cache_extent, ParseExt4EsCacheExtentEncoder);

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
void ParseExt4EsInsertExtent(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsInsertExtentData eventData {};
    ParseExt4EsInsertExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_insert_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
}
void ParseExt4EsInsertExtentEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsInsertExtentData eventData {};
    ParseExt4EsInsertExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_insert_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_insert_extent, ParseExt4EsInsertExtent);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_insert_extent, ParseExt4EsInsertExtentEncoder);

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
void ParseExt4EsLookupExtentEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsLookupExtentEnterData eventData {};
    ParseExt4EsLookupExtentEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
}
void ParseExt4EsLookupExtentEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsLookupExtentEnterData eventData {};
    ParseExt4EsLookupExtentEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_lookup_extent_enter, ParseExt4EsLookupExtentEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_lookup_extent_enter, ParseExt4EsLookupExtentEnterEncoder);

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
void ParseExt4EsLookupExtentExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsLookupExtentExitData eventData {};
    ParseExt4EsLookupExtentExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
    msg->set_found(eventData.found);
}
void ParseExt4EsLookupExtentExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsLookupExtentExitData eventData {};
    ParseExt4EsLookupExtentExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_lookup_extent_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_pblk(eventData.pblk);
    msg->set_status(eventData.status);
    msg->set_found(eventData.found);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_lookup_extent_exit, ParseExt4EsLookupExtentExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_lookup_extent_exit, ParseExt4EsLookupExtentExitEncoder);

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
void ParseExt4EsRemoveExtent(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsRemoveExtentData eventData {};
    ParseExt4EsRemoveExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_remove_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
}
void ParseExt4EsRemoveExtentEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsRemoveExtentData eventData {};
    ParseExt4EsRemoveExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_remove_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_remove_extent, ParseExt4EsRemoveExtent);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_remove_extent, ParseExt4EsRemoveExtentEncoder);

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
void ParseExt4EsShrink(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsShrinkData eventData {};
    ParseExt4EsShrinkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_format();
    msg->set_dev(eventData.dev);
    msg->set_nr_shrunk(eventData.nr_shrunk);
    msg->set_scan_time(eventData.scan_time);
    msg->set_nr_skipped(eventData.nr_skipped);
    msg->set_retried(eventData.retried);
}
void ParseExt4EsShrinkEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsShrinkData eventData {};
    ParseExt4EsShrinkData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_format();
    msg->set_dev(eventData.dev);
    msg->set_nr_shrunk(eventData.nr_shrunk);
    msg->set_scan_time(eventData.scan_time);
    msg->set_nr_skipped(eventData.nr_skipped);
    msg->set_retried(eventData.retried);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink, ParseExt4EsShrink);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink, ParseExt4EsShrinkEncoder);

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
void ParseExt4EsShrinkCount(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsShrinkCountData eventData {};
    ParseExt4EsShrinkCountData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_count_format();
    msg->set_dev(eventData.dev);
    msg->set_nr_to_scan(eventData.nr_to_scan);
    msg->set_cache_cnt(eventData.cache_cnt);
}
void ParseExt4EsShrinkCountEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsShrinkCountData eventData {};
    ParseExt4EsShrinkCountData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_count_format();
    msg->set_dev(eventData.dev);
    msg->set_nr_to_scan(eventData.nr_to_scan);
    msg->set_cache_cnt(eventData.cache_cnt);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_count, ParseExt4EsShrinkCount);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_count, ParseExt4EsShrinkCountEncoder);

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
void ParseExt4EsShrinkScanEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsShrinkScanEnterData eventData {};
    ParseExt4EsShrinkScanEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_nr_to_scan(eventData.nr_to_scan);
    msg->set_cache_cnt(eventData.cache_cnt);
}
void ParseExt4EsShrinkScanEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsShrinkScanEnterData eventData {};
    ParseExt4EsShrinkScanEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_nr_to_scan(eventData.nr_to_scan);
    msg->set_cache_cnt(eventData.cache_cnt);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_scan_enter, ParseExt4EsShrinkScanEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_scan_enter, ParseExt4EsShrinkScanEnterEncoder);

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
void ParseExt4EsShrinkScanExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsShrinkScanExitData eventData {};
    ParseExt4EsShrinkScanExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_nr_shrunk(eventData.nr_shrunk);
    msg->set_cache_cnt(eventData.cache_cnt);
}
void ParseExt4EsShrinkScanExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EsShrinkScanExitData eventData {};
    ParseExt4EsShrinkScanExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_es_shrink_scan_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_nr_shrunk(eventData.nr_shrunk);
    msg->set_cache_cnt(eventData.cache_cnt);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_es_shrink_scan_exit, ParseExt4EsShrinkScanExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_es_shrink_scan_exit, ParseExt4EsShrinkScanExitEncoder);

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
void ParseExt4EvictInode(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EvictInodeData eventData {};
    ParseExt4EvictInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_evict_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_nlink(eventData.nlink);
}
void ParseExt4EvictInodeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4EvictInodeData eventData {};
    ParseExt4EvictInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_evict_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_nlink(eventData.nlink);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_evict_inode, ParseExt4EvictInode);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_evict_inode, ParseExt4EvictInodeEncoder);

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
void ParseExt4ExtConvertToInitializedEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtConvertToInitializedEnterData eventData {};
    ParseExt4ExtConvertToInitializedEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_m_lblk(eventData.m_lblk);
    msg->set_m_len(eventData.m_len);
    msg->set_u_lblk(eventData.u_lblk);
    msg->set_u_len(eventData.u_len);
    msg->set_u_pblk(eventData.u_pblk);
}
void ParseExt4ExtConvertToInitializedEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtConvertToInitializedEnterData eventData {};
    ParseExt4ExtConvertToInitializedEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_m_lblk(eventData.m_lblk);
    msg->set_m_len(eventData.m_len);
    msg->set_u_lblk(eventData.u_lblk);
    msg->set_u_len(eventData.u_len);
    msg->set_u_pblk(eventData.u_pblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_convert_to_initialized_enter, ParseExt4ExtConvertToInitializedEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_convert_to_initialized_enter, ParseExt4ExtConvertToInitializedEnterEncoder);

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
void ParseExt4ExtConvertToInitializedFastpath(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtConvertToInitializedFastpathData eventData {};
    ParseExt4ExtConvertToInitializedFastpathData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_fastpath_format();
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
void ParseExt4ExtConvertToInitializedFastpathEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtConvertToInitializedFastpathData eventData {};
    ParseExt4ExtConvertToInitializedFastpathData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_convert_to_initialized_fastpath_format();
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_convert_to_initialized_fastpath, ParseExt4ExtConvertToInitializedFastpath);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_convert_to_initialized_fastpath, ParseExt4ExtConvertToInitializedFastpathEncoder);

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
void ParseExt4ExtHandleUnwrittenExtents(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtHandleUnwrittenExtentsData eventData {};
    ParseExt4ExtHandleUnwrittenExtentsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_handle_unwritten_extents_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_lblk(eventData.lblk);
    msg->set_pblk(eventData.pblk);
    msg->set_len(eventData.len);
    msg->set_allocated(eventData.allocated);
    msg->set_newblk(eventData.newblk);
}
void ParseExt4ExtHandleUnwrittenExtentsEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtHandleUnwrittenExtentsData eventData {};
    ParseExt4ExtHandleUnwrittenExtentsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_handle_unwritten_extents_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_lblk(eventData.lblk);
    msg->set_pblk(eventData.pblk);
    msg->set_len(eventData.len);
    msg->set_allocated(eventData.allocated);
    msg->set_newblk(eventData.newblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_handle_unwritten_extents, ParseExt4ExtHandleUnwrittenExtents);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_handle_unwritten_extents, ParseExt4ExtHandleUnwrittenExtentsEncoder);

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
void ParseExt4ExtLoadExtent(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtLoadExtentData eventData {};
    ParseExt4ExtLoadExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_load_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
}
void ParseExt4ExtLoadExtentEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtLoadExtentData eventData {};
    ParseExt4ExtLoadExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_load_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_load_extent, ParseExt4ExtLoadExtent);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_load_extent, ParseExt4ExtLoadExtentEncoder);

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
void ParseExt4ExtMapBlocksEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtMapBlocksEnterData eventData {};
    ParseExt4ExtMapBlocksEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
void ParseExt4ExtMapBlocksEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtMapBlocksEnterData eventData {};
    ParseExt4ExtMapBlocksEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_map_blocks_enter, ParseExt4ExtMapBlocksEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_map_blocks_enter, ParseExt4ExtMapBlocksEnterEncoder);

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
void ParseExt4ExtMapBlocksExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtMapBlocksExitData eventData {};
    ParseExt4ExtMapBlocksExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_mflags(eventData.mflags);
    msg->set_ret(eventData.ret);
}
void ParseExt4ExtMapBlocksExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtMapBlocksExitData eventData {};
    ParseExt4ExtMapBlocksExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_map_blocks_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_mflags(eventData.mflags);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_map_blocks_exit, ParseExt4ExtMapBlocksExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_map_blocks_exit, ParseExt4ExtMapBlocksExitEncoder);

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
void ParseExt4ExtRemoveSpace(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtRemoveSpaceData eventData {};
    ParseExt4ExtRemoveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_remove_space_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_start(eventData.start);
    msg->set_end(eventData.end);
    msg->set_depth(eventData.depth);
}
void ParseExt4ExtRemoveSpaceEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtRemoveSpaceData eventData {};
    ParseExt4ExtRemoveSpaceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_remove_space_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_start(eventData.start);
    msg->set_end(eventData.end);
    msg->set_depth(eventData.depth);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_remove_space, ParseExt4ExtRemoveSpace);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_remove_space, ParseExt4ExtRemoveSpaceEncoder);

struct Ext4ExtRemoveSpaceDoneData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t start {};
    uint32_t end {};
    int32_t depth {};
    uint64_t pc_pclu {};
    uint32_t pc_lblk {};
    int32_t pc_state {};
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
    eventData.pc_pclu = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pc_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pc_state = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.eh_entries = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseExt4ExtRemoveSpaceDone(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtRemoveSpaceDoneData eventData {};
    ParseExt4ExtRemoveSpaceDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_remove_space_done_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_start(eventData.start);
    msg->set_end(eventData.end);
    msg->set_depth(eventData.depth);
    msg->set_pc_pclu(eventData.pc_pclu);
    msg->set_pc_lblk(eventData.pc_lblk);
    msg->set_pc_state(eventData.pc_state);
    msg->set_eh_entries(eventData.eh_entries);
}
void ParseExt4ExtRemoveSpaceDoneEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtRemoveSpaceDoneData eventData {};
    ParseExt4ExtRemoveSpaceDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_remove_space_done_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_start(eventData.start);
    msg->set_end(eventData.end);
    msg->set_depth(eventData.depth);
    msg->set_pc_pclu(eventData.pc_pclu);
    msg->set_pc_lblk(eventData.pc_lblk);
    msg->set_pc_state(eventData.pc_state);
    msg->set_eh_entries(eventData.eh_entries);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_remove_space_done, ParseExt4ExtRemoveSpaceDone);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_remove_space_done, ParseExt4ExtRemoveSpaceDoneEncoder);

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
void ParseExt4ExtRmIdx(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtRmIdxData eventData {};
    ParseExt4ExtRmIdxData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_rm_idx_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
}
void ParseExt4ExtRmIdxEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtRmIdxData eventData {};
    ParseExt4ExtRmIdxData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_rm_idx_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_rm_idx, ParseExt4ExtRmIdx);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_rm_idx, ParseExt4ExtRmIdxEncoder);

struct Ext4ExtRmLeafData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t start {};
    uint32_t ee_lblk {};
    uint64_t ee_pblk {};
    int32_t ee_len {};
    uint64_t pc_pclu {};
    uint32_t pc_lblk {};
    int32_t pc_state {};
};
__attribute__((noinline)) void ParseExt4ExtRmLeafData(Ext4ExtRmLeafData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.start = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ee_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ee_pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ee_len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.pc_pclu = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pc_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pc_state = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseExt4ExtRmLeaf(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtRmLeafData eventData {};
    ParseExt4ExtRmLeafData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_rm_leaf_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_start(eventData.start);
    msg->set_ee_lblk(eventData.ee_lblk);
    msg->set_ee_pblk(eventData.ee_pblk);
    msg->set_ee_len(eventData.ee_len);
    msg->set_pc_pclu(eventData.pc_pclu);
    msg->set_pc_lblk(eventData.pc_lblk);
    msg->set_pc_state(eventData.pc_state);
}
void ParseExt4ExtRmLeafEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtRmLeafData eventData {};
    ParseExt4ExtRmLeafData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_rm_leaf_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_start(eventData.start);
    msg->set_ee_lblk(eventData.ee_lblk);
    msg->set_ee_pblk(eventData.ee_pblk);
    msg->set_ee_len(eventData.ee_len);
    msg->set_pc_pclu(eventData.pc_pclu);
    msg->set_pc_lblk(eventData.pc_lblk);
    msg->set_pc_state(eventData.pc_state);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_rm_leaf, ParseExt4ExtRmLeaf);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_rm_leaf, ParseExt4ExtRmLeafEncoder);

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
void ParseExt4ExtShowExtent(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtShowExtentData eventData {};
    ParseExt4ExtShowExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_show_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
}
void ParseExt4ExtShowExtentEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ExtShowExtentData eventData {};
    ParseExt4ExtShowExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ext_show_extent_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ext_show_extent, ParseExt4ExtShowExtent);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ext_show_extent, ParseExt4ExtShowExtentEncoder);

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
void ParseExt4FallocateEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4FallocateEnterData eventData {};
    ParseExt4FallocateEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_fallocate_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
void ParseExt4FallocateEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4FallocateEnterData eventData {};
    ParseExt4FallocateEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_fallocate_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_fallocate_enter, ParseExt4FallocateEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_fallocate_enter, ParseExt4FallocateEnterEncoder);

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
void ParseExt4FallocateExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4FallocateExitData eventData {};
    ParseExt4FallocateExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_fallocate_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_blocks(eventData.blocks);
    msg->set_ret(eventData.ret);
}
void ParseExt4FallocateExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4FallocateExitData eventData {};
    ParseExt4FallocateExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_fallocate_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_blocks(eventData.blocks);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_fallocate_exit, ParseExt4FallocateExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_fallocate_exit, ParseExt4FallocateExitEncoder);

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
void ParseExt4Forget(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ForgetData eventData {};
    ParseExt4ForgetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_forget_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_is_metadata(eventData.is_metadata);
    msg->set_mode(eventData.mode);
}
void ParseExt4ForgetEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ForgetData eventData {};
    ParseExt4ForgetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_forget_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_is_metadata(eventData.is_metadata);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_forget, ParseExt4Forget);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_forget, ParseExt4ForgetEncoder);

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
void ParseExt4FreeBlocks(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4FreeBlocksData eventData {};
    ParseExt4FreeBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_free_blocks_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_count(eventData.count);
    msg->set_flags(eventData.flags);
    msg->set_mode(eventData.mode);
}
void ParseExt4FreeBlocksEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4FreeBlocksData eventData {};
    ParseExt4FreeBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_free_blocks_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_count(eventData.count);
    msg->set_flags(eventData.flags);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_free_blocks, ParseExt4FreeBlocks);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_free_blocks, ParseExt4FreeBlocksEncoder);

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
void ParseExt4FreeInode(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4FreeInodeData eventData {};
    ParseExt4FreeInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_free_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_uid(eventData.uid);
    msg->set_gid(eventData.gid);
    msg->set_blocks(eventData.blocks);
    msg->set_mode(eventData.mode);
}
void ParseExt4FreeInodeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4FreeInodeData eventData {};
    ParseExt4FreeInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_free_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_uid(eventData.uid);
    msg->set_gid(eventData.gid);
    msg->set_blocks(eventData.blocks);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_free_inode, ParseExt4FreeInode);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_free_inode, ParseExt4FreeInodeEncoder);

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
void ParseExt4GetImpliedClusterAllocExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4GetImpliedClusterAllocExitData eventData {};
    ParseExt4GetImpliedClusterAllocExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_get_implied_cluster_alloc_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_flags(eventData.flags);
    msg->set_lblk(eventData.lblk);
    msg->set_pblk(eventData.pblk);
    msg->set_len(eventData.len);
    msg->set_ret(eventData.ret);
}
void ParseExt4GetImpliedClusterAllocExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4GetImpliedClusterAllocExitData eventData {};
    ParseExt4GetImpliedClusterAllocExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_get_implied_cluster_alloc_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_flags(eventData.flags);
    msg->set_lblk(eventData.lblk);
    msg->set_pblk(eventData.pblk);
    msg->set_len(eventData.len);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_get_implied_cluster_alloc_exit, ParseExt4GetImpliedClusterAllocExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_get_implied_cluster_alloc_exit, ParseExt4GetImpliedClusterAllocExitEncoder);

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
void ParseExt4IndMapBlocksEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4IndMapBlocksEnterData eventData {};
    ParseExt4IndMapBlocksEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
void ParseExt4IndMapBlocksEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4IndMapBlocksEnterData eventData {};
    ParseExt4IndMapBlocksEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ind_map_blocks_enter, ParseExt4IndMapBlocksEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ind_map_blocks_enter, ParseExt4IndMapBlocksEnterEncoder);

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
void ParseExt4IndMapBlocksExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4IndMapBlocksExitData eventData {};
    ParseExt4IndMapBlocksExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_mflags(eventData.mflags);
    msg->set_ret(eventData.ret);
}
void ParseExt4IndMapBlocksExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4IndMapBlocksExitData eventData {};
    ParseExt4IndMapBlocksExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_ind_map_blocks_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_flags(eventData.flags);
    msg->set_pblk(eventData.pblk);
    msg->set_lblk(eventData.lblk);
    msg->set_len(eventData.len);
    msg->set_mflags(eventData.mflags);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_ind_map_blocks_exit, ParseExt4IndMapBlocksExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_ind_map_blocks_exit, ParseExt4IndMapBlocksExitEncoder);

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
void ParseExt4InsertRange(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4InsertRangeData eventData {};
    ParseExt4InsertRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_insert_range_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
}
void ParseExt4InsertRangeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4InsertRangeData eventData {};
    ParseExt4InsertRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_insert_range_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_insert_range, ParseExt4InsertRange);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_insert_range, ParseExt4InsertRangeEncoder);

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
void ParseExt4JournalStartReserved(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4JournalStartReservedData eventData {};
    ParseExt4JournalStartReservedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_journal_start_reserved_format();
    msg->set_dev(eventData.dev);
    msg->set_ip(eventData.ip);
    msg->set_blocks(eventData.blocks);
}
void ParseExt4JournalStartReservedEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4JournalStartReservedData eventData {};
    ParseExt4JournalStartReservedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_journal_start_reserved_format();
    msg->set_dev(eventData.dev);
    msg->set_ip(eventData.ip);
    msg->set_blocks(eventData.blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journal_start_reserved, ParseExt4JournalStartReserved);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journal_start_reserved, ParseExt4JournalStartReservedEncoder);

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
void ParseExt4JournalledWriteEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4JournalledWriteEndData eventData {};
    ParseExt4JournalledWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_journalled_write_end_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
void ParseExt4JournalledWriteEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4JournalledWriteEndData eventData {};
    ParseExt4JournalledWriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_journalled_write_end_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_journalled_write_end, ParseExt4JournalledWriteEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_journalled_write_end, ParseExt4JournalledWriteEndEncoder);

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
void ParseExt4LoadInode(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4LoadInodeData eventData {};
    ParseExt4LoadInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_load_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
}
void ParseExt4LoadInodeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4LoadInodeData eventData {};
    ParseExt4LoadInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_load_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_load_inode, ParseExt4LoadInode);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_load_inode, ParseExt4LoadInodeEncoder);

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
void ParseExt4LoadInodeBitmap(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4LoadInodeBitmapData eventData {};
    ParseExt4LoadInodeBitmapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_load_inode_bitmap_format();
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
void ParseExt4LoadInodeBitmapEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4LoadInodeBitmapData eventData {};
    ParseExt4LoadInodeBitmapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_load_inode_bitmap_format();
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_load_inode_bitmap, ParseExt4LoadInodeBitmap);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_load_inode_bitmap, ParseExt4LoadInodeBitmapEncoder);

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
void ParseExt4MarkInodeDirty(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MarkInodeDirtyData eventData {};
    ParseExt4MarkInodeDirtyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mark_inode_dirty_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ip(eventData.ip);
}
void ParseExt4MarkInodeDirtyEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MarkInodeDirtyData eventData {};
    ParseExt4MarkInodeDirtyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mark_inode_dirty_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ip(eventData.ip);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mark_inode_dirty, ParseExt4MarkInodeDirty);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mark_inode_dirty, ParseExt4MarkInodeDirtyEncoder);

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
void ParseExt4MbBitmapLoad(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbBitmapLoadData eventData {};
    ParseExt4MbBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_bitmap_load_format();
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
void ParseExt4MbBitmapLoadEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbBitmapLoadData eventData {};
    ParseExt4MbBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_bitmap_load_format();
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_bitmap_load, ParseExt4MbBitmapLoad);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_bitmap_load, ParseExt4MbBitmapLoadEncoder);

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
void ParseExt4MbBuddyBitmapLoad(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbBuddyBitmapLoadData eventData {};
    ParseExt4MbBuddyBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_buddy_bitmap_load_format();
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
void ParseExt4MbBuddyBitmapLoadEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbBuddyBitmapLoadData eventData {};
    ParseExt4MbBuddyBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_buddy_bitmap_load_format();
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_buddy_bitmap_load, ParseExt4MbBuddyBitmapLoad);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_buddy_bitmap_load, ParseExt4MbBuddyBitmapLoadEncoder);

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
void ParseExt4MbDiscardPreallocations(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbDiscardPreallocationsData eventData {};
    ParseExt4MbDiscardPreallocationsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_discard_preallocations_format();
    msg->set_dev(eventData.dev);
    msg->set_needed(eventData.needed);
}
void ParseExt4MbDiscardPreallocationsEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbDiscardPreallocationsData eventData {};
    ParseExt4MbDiscardPreallocationsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_discard_preallocations_format();
    msg->set_dev(eventData.dev);
    msg->set_needed(eventData.needed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_discard_preallocations, ParseExt4MbDiscardPreallocations);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_discard_preallocations, ParseExt4MbDiscardPreallocationsEncoder);

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
void ParseExt4MbNewGroupPa(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbNewGroupPaData eventData {};
    ParseExt4MbNewGroupPaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_new_group_pa_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_lstart(eventData.pa_lstart);
    msg->set_pa_len(eventData.pa_len);
}
void ParseExt4MbNewGroupPaEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbNewGroupPaData eventData {};
    ParseExt4MbNewGroupPaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_new_group_pa_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_lstart(eventData.pa_lstart);
    msg->set_pa_len(eventData.pa_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_new_group_pa, ParseExt4MbNewGroupPa);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_new_group_pa, ParseExt4MbNewGroupPaEncoder);

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
void ParseExt4MbNewInodePa(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbNewInodePaData eventData {};
    ParseExt4MbNewInodePaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_new_inode_pa_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_lstart(eventData.pa_lstart);
    msg->set_pa_len(eventData.pa_len);
}
void ParseExt4MbNewInodePaEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbNewInodePaData eventData {};
    ParseExt4MbNewInodePaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_new_inode_pa_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_lstart(eventData.pa_lstart);
    msg->set_pa_len(eventData.pa_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_new_inode_pa, ParseExt4MbNewInodePa);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_new_inode_pa, ParseExt4MbNewInodePaEncoder);

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
void ParseExt4MbReleaseGroupPa(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbReleaseGroupPaData eventData {};
    ParseExt4MbReleaseGroupPaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_release_group_pa_format();
    msg->set_dev(eventData.dev);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_len(eventData.pa_len);
}
void ParseExt4MbReleaseGroupPaEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbReleaseGroupPaData eventData {};
    ParseExt4MbReleaseGroupPaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_release_group_pa_format();
    msg->set_dev(eventData.dev);
    msg->set_pa_pstart(eventData.pa_pstart);
    msg->set_pa_len(eventData.pa_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_release_group_pa, ParseExt4MbReleaseGroupPa);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_release_group_pa, ParseExt4MbReleaseGroupPaEncoder);

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
void ParseExt4MbReleaseInodePa(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbReleaseInodePaData eventData {};
    ParseExt4MbReleaseInodePaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_release_inode_pa_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_count(eventData.count);
}
void ParseExt4MbReleaseInodePaEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MbReleaseInodePaData eventData {};
    ParseExt4MbReleaseInodePaData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mb_release_inode_pa_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_block(eventData.block);
    msg->set_count(eventData.count);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mb_release_inode_pa, ParseExt4MbReleaseInodePa);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mb_release_inode_pa, ParseExt4MbReleaseInodePaEncoder);

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
void ParseExt4MballocAlloc(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MballocAllocData eventData {};
    ParseExt4MballocAllocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_alloc_format();
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
void ParseExt4MballocAllocEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MballocAllocData eventData {};
    ParseExt4MballocAllocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_alloc_format();
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_alloc, ParseExt4MballocAlloc);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_alloc, ParseExt4MballocAllocEncoder);

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
void ParseExt4MballocDiscard(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MballocDiscardData eventData {};
    ParseExt4MballocDiscardData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_discard_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_result_start(eventData.result_start);
    msg->set_result_group(eventData.result_group);
    msg->set_result_len(eventData.result_len);
}
void ParseExt4MballocDiscardEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MballocDiscardData eventData {};
    ParseExt4MballocDiscardData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_discard_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_result_start(eventData.result_start);
    msg->set_result_group(eventData.result_group);
    msg->set_result_len(eventData.result_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_discard, ParseExt4MballocDiscard);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_discard, ParseExt4MballocDiscardEncoder);

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
void ParseExt4MballocFree(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MballocFreeData eventData {};
    ParseExt4MballocFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_free_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_result_start(eventData.result_start);
    msg->set_result_group(eventData.result_group);
    msg->set_result_len(eventData.result_len);
}
void ParseExt4MballocFreeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MballocFreeData eventData {};
    ParseExt4MballocFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_free_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_result_start(eventData.result_start);
    msg->set_result_group(eventData.result_group);
    msg->set_result_len(eventData.result_len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_free, ParseExt4MballocFree);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_free, ParseExt4MballocFreeEncoder);

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
void ParseExt4MballocPrealloc(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MballocPreallocData eventData {};
    ParseExt4MballocPreallocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_prealloc_format();
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
void ParseExt4MballocPreallocEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4MballocPreallocData eventData {};
    ParseExt4MballocPreallocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_mballoc_prealloc_format();
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_mballoc_prealloc, ParseExt4MballocPrealloc);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_mballoc_prealloc, ParseExt4MballocPreallocEncoder);

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
void ParseExt4OtherInodeUpdateTime(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4OtherInodeUpdateTimeData eventData {};
    ParseExt4OtherInodeUpdateTimeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_other_inode_update_time_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_orig_ino(eventData.orig_ino);
    msg->set_uid(eventData.uid);
    msg->set_gid(eventData.gid);
    msg->set_mode(eventData.mode);
}
void ParseExt4OtherInodeUpdateTimeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4OtherInodeUpdateTimeData eventData {};
    ParseExt4OtherInodeUpdateTimeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_other_inode_update_time_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_orig_ino(eventData.orig_ino);
    msg->set_uid(eventData.uid);
    msg->set_gid(eventData.gid);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_other_inode_update_time, ParseExt4OtherInodeUpdateTime);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_other_inode_update_time, ParseExt4OtherInodeUpdateTimeEncoder);

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
void ParseExt4PunchHole(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4PunchHoleData eventData {};
    ParseExt4PunchHoleData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_punch_hole_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
void ParseExt4PunchHoleEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4PunchHoleData eventData {};
    ParseExt4PunchHoleData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_punch_hole_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_punch_hole, ParseExt4PunchHole);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_punch_hole, ParseExt4PunchHoleEncoder);

struct Ext4ReadBlockBitmapLoadData {
    uint64_t dev {};
    uint32_t group {};
    uint32_t prefetch {};
};
__attribute__((noinline)) void ParseExt4ReadBlockBitmapLoadData(Ext4ReadBlockBitmapLoadData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.group = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prefetch = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseExt4ReadBlockBitmapLoad(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ReadBlockBitmapLoadData eventData {};
    ParseExt4ReadBlockBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_read_block_bitmap_load_format();
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
    msg->set_prefetch(eventData.prefetch);
}
void ParseExt4ReadBlockBitmapLoadEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ReadBlockBitmapLoadData eventData {};
    ParseExt4ReadBlockBitmapLoadData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_read_block_bitmap_load_format();
    msg->set_dev(eventData.dev);
    msg->set_group(eventData.group);
    msg->set_prefetch(eventData.prefetch);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_read_block_bitmap_load, ParseExt4ReadBlockBitmapLoad);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_read_block_bitmap_load, ParseExt4ReadBlockBitmapLoadEncoder);

struct Ext4RemoveBlocksData {
    uint64_t dev {};
    uint64_t ino {};
    uint32_t from {};
    uint32_t to {};
    uint64_t ee_pblk {};
    uint32_t ee_lblk {};
    uint32_t ee_len {};
    uint64_t pc_pclu {};
    uint32_t pc_lblk {};
    int32_t pc_state {};
};
__attribute__((noinline)) void ParseExt4RemoveBlocksData(Ext4RemoveBlocksData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.from = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.to = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ee_pblk = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ee_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ee_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pc_pclu = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pc_lblk = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.pc_state = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseExt4RemoveBlocks(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4RemoveBlocksData eventData {};
    ParseExt4RemoveBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_remove_blocks_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_from(eventData.from);
    msg->set_to(eventData.to);
    msg->set_ee_pblk(eventData.ee_pblk);
    msg->set_ee_lblk(eventData.ee_lblk);
    msg->set_ee_len(eventData.ee_len);
    msg->set_pc_pclu(eventData.pc_pclu);
    msg->set_pc_lblk(eventData.pc_lblk);
    msg->set_pc_state(eventData.pc_state);
}
void ParseExt4RemoveBlocksEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4RemoveBlocksData eventData {};
    ParseExt4RemoveBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_remove_blocks_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_from(eventData.from);
    msg->set_to(eventData.to);
    msg->set_ee_pblk(eventData.ee_pblk);
    msg->set_ee_lblk(eventData.ee_lblk);
    msg->set_ee_len(eventData.ee_len);
    msg->set_pc_pclu(eventData.pc_pclu);
    msg->set_pc_lblk(eventData.pc_lblk);
    msg->set_pc_state(eventData.pc_state);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_remove_blocks, ParseExt4RemoveBlocks);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_remove_blocks, ParseExt4RemoveBlocksEncoder);

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
void ParseExt4RequestBlocks(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4RequestBlocksData eventData {};
    ParseExt4RequestBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_request_blocks_format();
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
void ParseExt4RequestBlocksEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4RequestBlocksData eventData {};
    ParseExt4RequestBlocksData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_request_blocks_format();
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_request_blocks, ParseExt4RequestBlocks);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_request_blocks, ParseExt4RequestBlocksEncoder);

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
void ParseExt4RequestInode(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4RequestInodeData eventData {};
    ParseExt4RequestInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_request_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_dir(eventData.dir);
    msg->set_mode(eventData.mode);
}
void ParseExt4RequestInodeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4RequestInodeData eventData {};
    ParseExt4RequestInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_request_inode_format();
    msg->set_dev(eventData.dev);
    msg->set_dir(eventData.dir);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_request_inode, ParseExt4RequestInode);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_request_inode, ParseExt4RequestInodeEncoder);

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
void ParseExt4SyncFileEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4SyncFileEnterData eventData {};
    ParseExt4SyncFileEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_file_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_parent(eventData.parent);
    msg->set_datasync(eventData.datasync);
}
void ParseExt4SyncFileEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4SyncFileEnterData eventData {};
    ParseExt4SyncFileEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_file_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_parent(eventData.parent);
    msg->set_datasync(eventData.datasync);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_file_enter, ParseExt4SyncFileEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_file_enter, ParseExt4SyncFileEnterEncoder);

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
void ParseExt4SyncFileExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4SyncFileExitData eventData {};
    ParseExt4SyncFileExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_file_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
}
void ParseExt4SyncFileExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4SyncFileExitData eventData {};
    ParseExt4SyncFileExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_file_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_file_exit, ParseExt4SyncFileExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_file_exit, ParseExt4SyncFileExitEncoder);

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
void ParseExt4SyncFs(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4SyncFsData eventData {};
    ParseExt4SyncFsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_fs_format();
    msg->set_dev(eventData.dev);
    msg->set_wait(eventData.wait);
}
void ParseExt4SyncFsEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4SyncFsData eventData {};
    ParseExt4SyncFsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_sync_fs_format();
    msg->set_dev(eventData.dev);
    msg->set_wait(eventData.wait);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_sync_fs, ParseExt4SyncFs);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_sync_fs, ParseExt4SyncFsEncoder);

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
void ParseExt4TrimAllFree(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4TrimAllFreeData eventData {};
    ParseExt4TrimAllFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_trim_all_free_format();
    msg->set_dev_major(eventData.dev_major);
    msg->set_dev_minor(eventData.dev_minor);
    msg->set_group(eventData.group);
    msg->set_start(eventData.start);
    msg->set_len(eventData.len);
}
void ParseExt4TrimAllFreeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4TrimAllFreeData eventData {};
    ParseExt4TrimAllFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_trim_all_free_format();
    msg->set_dev_major(eventData.dev_major);
    msg->set_dev_minor(eventData.dev_minor);
    msg->set_group(eventData.group);
    msg->set_start(eventData.start);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_trim_all_free, ParseExt4TrimAllFree);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_trim_all_free, ParseExt4TrimAllFreeEncoder);

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
void ParseExt4TrimExtent(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4TrimExtentData eventData {};
    ParseExt4TrimExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_trim_extent_format();
    msg->set_dev_major(eventData.dev_major);
    msg->set_dev_minor(eventData.dev_minor);
    msg->set_group(eventData.group);
    msg->set_start(eventData.start);
    msg->set_len(eventData.len);
}
void ParseExt4TrimExtentEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4TrimExtentData eventData {};
    ParseExt4TrimExtentData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_trim_extent_format();
    msg->set_dev_major(eventData.dev_major);
    msg->set_dev_minor(eventData.dev_minor);
    msg->set_group(eventData.group);
    msg->set_start(eventData.start);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_trim_extent, ParseExt4TrimExtent);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_trim_extent, ParseExt4TrimExtentEncoder);

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
void ParseExt4TruncateEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4TruncateEnterData eventData {};
    ParseExt4TruncateEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_truncate_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_blocks(eventData.blocks);
}
void ParseExt4TruncateEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4TruncateEnterData eventData {};
    ParseExt4TruncateEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_truncate_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_blocks(eventData.blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_truncate_enter, ParseExt4TruncateEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_truncate_enter, ParseExt4TruncateEnterEncoder);

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
void ParseExt4TruncateExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4TruncateExitData eventData {};
    ParseExt4TruncateExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_truncate_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_blocks(eventData.blocks);
}
void ParseExt4TruncateExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4TruncateExitData eventData {};
    ParseExt4TruncateExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_truncate_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_blocks(eventData.blocks);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_truncate_exit, ParseExt4TruncateExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_truncate_exit, ParseExt4TruncateExitEncoder);

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
void ParseExt4UnlinkEnter(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4UnlinkEnterData eventData {};
    ParseExt4UnlinkEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_unlink_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_parent(eventData.parent);
    msg->set_size(eventData.size);
}
void ParseExt4UnlinkEnterEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4UnlinkEnterData eventData {};
    ParseExt4UnlinkEnterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_unlink_enter_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_parent(eventData.parent);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_unlink_enter, ParseExt4UnlinkEnter);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_unlink_enter, ParseExt4UnlinkEnterEncoder);

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
void ParseExt4UnlinkExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4UnlinkExitData eventData {};
    ParseExt4UnlinkExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_unlink_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
}
void ParseExt4UnlinkExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4UnlinkExitData eventData {};
    ParseExt4UnlinkExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_unlink_exit_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_unlink_exit, ParseExt4UnlinkExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_unlink_exit, ParseExt4UnlinkExitEncoder);

struct Ext4WriteBeginData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t pos {};
    uint32_t len {};
};
__attribute__((noinline)) void ParseExt4WriteBeginData(Ext4WriteBeginData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pos = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseExt4WriteBegin(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4WriteBeginData eventData {};
    ParseExt4WriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_write_begin_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
}
void ParseExt4WriteBeginEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4WriteBeginData eventData {};
    ParseExt4WriteBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_write_begin_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_write_begin, ParseExt4WriteBegin);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_write_begin, ParseExt4WriteBeginEncoder);

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
void ParseExt4WriteEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4WriteEndData eventData {};
    ParseExt4WriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_write_end_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
void ParseExt4WriteEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4WriteEndData eventData {};
    ParseExt4WriteEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_write_end_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_pos(eventData.pos);
    msg->set_len(eventData.len);
    msg->set_copied(eventData.copied);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_write_end, ParseExt4WriteEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_write_end, ParseExt4WriteEndEncoder);

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
void ParseExt4Writepages(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4WritepagesData eventData {};
    ParseExt4WritepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_writepages_format();
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
void ParseExt4WritepagesEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4WritepagesData eventData {};
    ParseExt4WritepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_writepages_format();
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_writepages, ParseExt4Writepages);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_writepages, ParseExt4WritepagesEncoder);

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
void ParseExt4WritepagesResult(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4WritepagesResultData eventData {};
    ParseExt4WritepagesResultData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_writepages_result_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
    msg->set_pages_written(eventData.pages_written);
    msg->set_pages_skipped(eventData.pages_skipped);
    msg->set_writeback_index(eventData.writeback_index);
    msg->set_sync_mode(eventData.sync_mode);
}
void ParseExt4WritepagesResultEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4WritepagesResultData eventData {};
    ParseExt4WritepagesResultData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_writepages_result_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_ret(eventData.ret);
    msg->set_pages_written(eventData.pages_written);
    msg->set_pages_skipped(eventData.pages_skipped);
    msg->set_writeback_index(eventData.writeback_index);
    msg->set_sync_mode(eventData.sync_mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_writepages_result, ParseExt4WritepagesResult);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_writepages_result, ParseExt4WritepagesResultEncoder);

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
void ParseExt4ZeroRange(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ZeroRangeData eventData {};
    ParseExt4ZeroRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_zero_range_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
void ParseExt4ZeroRangeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    Ext4ZeroRangeData eventData {};
    ParseExt4ZeroRangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_ext4_zero_range_format();
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_offset(eventData.offset);
    msg->set_len(eventData.len);
    msg->set_mode(eventData.mode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(ext4_zero_range, ParseExt4ZeroRange);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(ext4_zero_range, ParseExt4ZeroRangeEncoder);
}  // namespace
FTRACE_NS_END
