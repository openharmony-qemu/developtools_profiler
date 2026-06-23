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
struct BlockBioBackmergeData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    std::string rwbs {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockBioBackmergeData(BlockBioBackmergeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockBioBackmerge(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioBackmergeData eventData {};
    ParseBlockBioBackmergeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_backmerge_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
void ParseBlockBioBackmergeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioBackmergeData eventData {};
    ParseBlockBioBackmergeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_backmerge_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_backmerge, ParseBlockBioBackmerge);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_backmerge, ParseBlockBioBackmergeEncoder);

struct BlockBioBounceData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    std::string rwbs {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockBioBounceData(BlockBioBounceData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockBioBounce(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioBounceData eventData {};
    ParseBlockBioBounceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_bounce_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
void ParseBlockBioBounceEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioBounceData eventData {};
    ParseBlockBioBounceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_bounce_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_bounce, ParseBlockBioBounce);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_bounce, ParseBlockBioBounceEncoder);

struct BlockBioCompleteData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    int32_t error {};
    std::string rwbs {};
};
__attribute__((noinline)) void ParseBlockBioCompleteData(BlockBioCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockBioComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioCompleteData eventData {};
    ParseBlockBioCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_complete_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_error(eventData.error);
    msg->set_rwbs(eventData.rwbs);
}
void ParseBlockBioCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioCompleteData eventData {};
    ParseBlockBioCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_complete_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_error(eventData.error);
    msg->set_rwbs(eventData.rwbs);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_complete, ParseBlockBioComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_complete, ParseBlockBioCompleteEncoder);

struct BlockBioFrontmergeData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    std::string rwbs {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockBioFrontmergeData(BlockBioFrontmergeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockBioFrontmerge(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioFrontmergeData eventData {};
    ParseBlockBioFrontmergeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_frontmerge_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
void ParseBlockBioFrontmergeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioFrontmergeData eventData {};
    ParseBlockBioFrontmergeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_frontmerge_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_frontmerge, ParseBlockBioFrontmerge);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_frontmerge, ParseBlockBioFrontmergeEncoder);

struct BlockBioQueueData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    std::string rwbs {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockBioQueueData(BlockBioQueueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockBioQueue(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioQueueData eventData {};
    ParseBlockBioQueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_queue_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
void ParseBlockBioQueueEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioQueueData eventData {};
    ParseBlockBioQueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_queue_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_queue, ParseBlockBioQueue);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_queue, ParseBlockBioQueueEncoder);

struct BlockBioRemapData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    uint64_t old_dev {};
    uint64_t old_sector {};
    std::string rwbs {};
};
__attribute__((noinline)) void ParseBlockBioRemapData(BlockBioRemapData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.old_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.old_sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockBioRemap(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioRemapData eventData {};
    ParseBlockBioRemapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_remap_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_old_dev(eventData.old_dev);
    msg->set_old_sector(eventData.old_sector);
    msg->set_rwbs(eventData.rwbs);
}
void ParseBlockBioRemapEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockBioRemapData eventData {};
    ParseBlockBioRemapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_remap_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_old_dev(eventData.old_dev);
    msg->set_old_sector(eventData.old_sector);
    msg->set_rwbs(eventData.rwbs);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_remap, ParseBlockBioRemap);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_remap, ParseBlockBioRemapEncoder);

struct BlockDirtyBufferData {
    uint64_t dev {};
    uint64_t sector {};
    uint64_t size {};
};
__attribute__((noinline)) void ParseBlockDirtyBufferData(BlockDirtyBufferData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseBlockDirtyBuffer(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockDirtyBufferData eventData {};
    ParseBlockDirtyBufferData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_dirty_buffer_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_size(eventData.size);
}
void ParseBlockDirtyBufferEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockDirtyBufferData eventData {};
    ParseBlockDirtyBufferData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_dirty_buffer_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_dirty_buffer, ParseBlockDirtyBuffer);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_dirty_buffer, ParseBlockDirtyBufferEncoder);

struct BlockGetrqData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    std::string rwbs {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockGetrqData(BlockGetrqData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockGetrq(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockGetrqData eventData {};
    ParseBlockGetrqData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_getrq_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
void ParseBlockGetrqEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockGetrqData eventData {};
    ParseBlockGetrqData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_getrq_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_getrq, ParseBlockGetrq);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_getrq, ParseBlockGetrqEncoder);

struct BlockPlugData {
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockPlugData(BlockPlugData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockPlug(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockPlugData eventData {};
    ParseBlockPlugData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_plug_format();
    msg->set_comm(eventData.comm);
}
void ParseBlockPlugEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockPlugData eventData {};
    ParseBlockPlugData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_plug_format();
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_plug, ParseBlockPlug);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_plug, ParseBlockPlugEncoder);

struct BlockRqCompleteData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    int32_t error {};
    std::string rwbs {};
    std::string cmd {};
};
__attribute__((noinline)) void ParseBlockRqCompleteData(BlockRqCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.cmd = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockRqComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqCompleteData eventData {};
    ParseBlockRqCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_complete_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_error(eventData.error);
    msg->set_rwbs(eventData.rwbs);
    msg->set_cmd(eventData.cmd);
}
void ParseBlockRqCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqCompleteData eventData {};
    ParseBlockRqCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_complete_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_error(eventData.error);
    msg->set_rwbs(eventData.rwbs);
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_complete, ParseBlockRqComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_complete, ParseBlockRqCompleteEncoder);

struct BlockRqInsertData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    uint32_t bytes {};
    std::string rwbs {};
    std::string comm {};
    std::string cmd {};
};
__attribute__((noinline)) void ParseBlockRqInsertData(BlockRqInsertData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.bytes = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.cmd = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockRqInsert(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqInsertData eventData {};
    ParseBlockRqInsertData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_insert_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_bytes(eventData.bytes);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
    msg->set_cmd(eventData.cmd);
}
void ParseBlockRqInsertEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqInsertData eventData {};
    ParseBlockRqInsertData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_insert_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_bytes(eventData.bytes);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_insert, ParseBlockRqInsert);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_insert, ParseBlockRqInsertEncoder);

struct BlockRqIssueData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    uint32_t bytes {};
    std::string rwbs {};
    std::string comm {};
    std::string cmd {};
};
__attribute__((noinline)) void ParseBlockRqIssueData(BlockRqIssueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.bytes = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.cmd = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockRqIssue(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqIssueData eventData {};
    ParseBlockRqIssueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_issue_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_bytes(eventData.bytes);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
    msg->set_cmd(eventData.cmd);
}
void ParseBlockRqIssueEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqIssueData eventData {};
    ParseBlockRqIssueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_issue_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_bytes(eventData.bytes);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_issue, ParseBlockRqIssue);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_issue, ParseBlockRqIssueEncoder);

struct BlockRqRemapData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    uint64_t old_dev {};
    uint64_t old_sector {};
    uint32_t nr_bios {};
    std::string rwbs {};
};
__attribute__((noinline)) void ParseBlockRqRemapData(BlockRqRemapData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.old_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.old_sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_bios = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockRqRemap(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqRemapData eventData {};
    ParseBlockRqRemapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_remap_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_old_dev(eventData.old_dev);
    msg->set_old_sector(eventData.old_sector);
    msg->set_nr_bios(eventData.nr_bios);
    msg->set_rwbs(eventData.rwbs);
}
void ParseBlockRqRemapEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqRemapData eventData {};
    ParseBlockRqRemapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_remap_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_old_dev(eventData.old_dev);
    msg->set_old_sector(eventData.old_sector);
    msg->set_nr_bios(eventData.nr_bios);
    msg->set_rwbs(eventData.rwbs);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_remap, ParseBlockRqRemap);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_remap, ParseBlockRqRemapEncoder);

struct BlockRqRequeueData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    std::string rwbs {};
    std::string cmd {};
};
__attribute__((noinline)) void ParseBlockRqRequeueData(BlockRqRequeueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.cmd = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockRqRequeue(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqRequeueData eventData {};
    ParseBlockRqRequeueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_requeue_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_cmd(eventData.cmd);
}
void ParseBlockRqRequeueEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockRqRequeueData eventData {};
    ParseBlockRqRequeueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_requeue_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_requeue, ParseBlockRqRequeue);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_requeue, ParseBlockRqRequeueEncoder);

struct BlockSplitData {
    uint64_t dev {};
    uint64_t sector {};
    uint64_t new_sector {};
    std::string rwbs {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockSplitData(BlockSplitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.new_sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockSplit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockSplitData eventData {};
    ParseBlockSplitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_split_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_new_sector(eventData.new_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
void ParseBlockSplitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockSplitData eventData {};
    ParseBlockSplitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_split_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_new_sector(eventData.new_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_split, ParseBlockSplit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_split, ParseBlockSplitEncoder);

struct BlockTouchBufferData {
    uint64_t dev {};
    uint64_t sector {};
    uint64_t size {};
};
__attribute__((noinline)) void ParseBlockTouchBufferData(BlockTouchBufferData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseBlockTouchBuffer(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockTouchBufferData eventData {};
    ParseBlockTouchBufferData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_touch_buffer_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_size(eventData.size);
}
void ParseBlockTouchBufferEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockTouchBufferData eventData {};
    ParseBlockTouchBufferData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_touch_buffer_format();
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_touch_buffer, ParseBlockTouchBuffer);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_touch_buffer, ParseBlockTouchBufferEncoder);

struct BlockUnplugData {
    int32_t nr_rq {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockUnplugData(BlockUnplugData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nr_rq = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBlockUnplug(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockUnplugData eventData {};
    ParseBlockUnplugData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_unplug_format();
    msg->set_nr_rq(eventData.nr_rq);
    msg->set_comm(eventData.comm);
}
void ParseBlockUnplugEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BlockUnplugData eventData {};
    ParseBlockUnplugData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_unplug_format();
    msg->set_nr_rq(eventData.nr_rq);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_unplug, ParseBlockUnplug);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_unplug, ParseBlockUnplugEncoder);
}  // namespace
FTRACE_NS_END
