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
template <typename FtraceEventType>
void ParseBlockBioBackmerge(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockBioBackmergeData eventData {};
    ParseBlockBioBackmergeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_backmerge_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_backmerge, ParseBlockBioBackmerge<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_backmerge, ParseBlockBioBackmerge<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockBioBounce(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockBioBounceData eventData {};
    ParseBlockBioBounceData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_bounce_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_bounce, ParseBlockBioBounce<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_bounce, ParseBlockBioBounce<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockBioComplete(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockBioCompleteData eventData {};
    ParseBlockBioCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_complete_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_error(eventData.error);
    msg->set_rwbs(eventData.rwbs);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_complete, ParseBlockBioComplete<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_complete, ParseBlockBioComplete<ProtoEncoder::FtraceEvent>);

struct BlockBioFrontmergeData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    std::string rwbs {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockBioFrontmergeData(
    BlockBioFrontmergeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBlockBioFrontmerge(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockBioFrontmergeData eventData {};
    ParseBlockBioFrontmergeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_frontmerge_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_frontmerge, ParseBlockBioFrontmerge<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_frontmerge, ParseBlockBioFrontmerge<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockBioQueue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockBioQueueData eventData {};
    ParseBlockBioQueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_queue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_queue, ParseBlockBioQueue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_queue, ParseBlockBioQueue<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockBioRemap(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockBioRemapData eventData {};
    ParseBlockBioRemapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_bio_remap_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_old_dev(eventData.old_dev);
    msg->set_old_sector(eventData.old_sector);
    msg->set_rwbs(eventData.rwbs);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_bio_remap, ParseBlockBioRemap<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_bio_remap, ParseBlockBioRemap<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockDirtyBuffer(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockDirtyBufferData eventData {};
    ParseBlockDirtyBufferData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_dirty_buffer_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_dirty_buffer, ParseBlockDirtyBuffer<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_dirty_buffer, ParseBlockDirtyBuffer<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockGetrq(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockGetrqData eventData {};
    ParseBlockGetrqData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_getrq_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_getrq, ParseBlockGetrq<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_getrq, ParseBlockGetrq<ProtoEncoder::FtraceEvent>);

struct BlockPlugData {
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockPlugData(BlockPlugData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBlockPlug(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockPlugData eventData {};
    ParseBlockPlugData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_plug_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_plug, ParseBlockPlug<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_plug, ParseBlockPlug<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockRqComplete(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockRqCompleteData eventData {};
    ParseBlockRqCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_complete_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_error(eventData.error);
    msg->set_rwbs(eventData.rwbs);
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_complete, ParseBlockRqComplete<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_complete, ParseBlockRqComplete<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockRqInsert(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockRqInsertData eventData {};
    ParseBlockRqInsertData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_insert_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_bytes(eventData.bytes);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_insert, ParseBlockRqInsert<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_insert, ParseBlockRqInsert<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockRqIssue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockRqIssueData eventData {};
    ParseBlockRqIssueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_issue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_bytes(eventData.bytes);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_issue, ParseBlockRqIssue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_issue, ParseBlockRqIssue<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockRqRemap(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockRqRemapData eventData {};
    ParseBlockRqRemapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_remap_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_old_dev(eventData.old_dev);
    msg->set_old_sector(eventData.old_sector);
    msg->set_nr_bios(eventData.nr_bios);
    msg->set_rwbs(eventData.rwbs);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_remap, ParseBlockRqRemap<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_remap, ParseBlockRqRemap<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockRqRequeue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockRqRequeueData eventData {};
    ParseBlockRqRequeueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_rq_requeue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_cmd(eventData.cmd);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_rq_requeue, ParseBlockRqRequeue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_rq_requeue, ParseBlockRqRequeue<ProtoEncoder::FtraceEvent>);

struct BlockSleeprqData {
    uint64_t dev {};
    uint64_t sector {};
    uint32_t nr_sector {};
    std::string rwbs {};
    std::string comm {};
};
__attribute__((noinline)) void ParseBlockSleeprqData(BlockSleeprqData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sector = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_sector = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rwbs = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.comm = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBlockSleeprq(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockSleeprqData eventData {};
    ParseBlockSleeprqData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_sleeprq_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_nr_sector(eventData.nr_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_sleeprq, ParseBlockSleeprq<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_sleeprq, ParseBlockSleeprq<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockSplit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockSplitData eventData {};
    ParseBlockSplitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_split_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_new_sector(eventData.new_sector);
    msg->set_rwbs(eventData.rwbs);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_split, ParseBlockSplit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_split, ParseBlockSplit<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockTouchBuffer(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockTouchBufferData eventData {};
    ParseBlockTouchBufferData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_touch_buffer_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_sector(eventData.sector);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_touch_buffer, ParseBlockTouchBuffer<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_touch_buffer, ParseBlockTouchBuffer<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseBlockUnplug(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BlockUnplugData eventData {};
    ParseBlockUnplugData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_block_unplug_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nr_rq(eventData.nr_rq);
    msg->set_comm(eventData.comm);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(block_unplug, ParseBlockUnplug<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(block_unplug, ParseBlockUnplug<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
