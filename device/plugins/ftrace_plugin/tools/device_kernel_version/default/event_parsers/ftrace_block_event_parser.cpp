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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_bio_backmerge,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_bio_backmerge_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_bio_bounce,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_bio_bounce_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_bio_complete,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_bio_complete_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_bio_frontmerge,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_bio_frontmerge_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_bio_queue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_bio_queue_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_bio_remap,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_bio_remap_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_old_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_old_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_dirty_buffer,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_dirty_buffer_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_getrq,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_getrq_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_plug,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_plug_format();
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_rq_complete,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_rq_complete_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_cmd(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_rq_insert,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_rq_insert_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_bytes(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_cmd(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_rq_issue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_rq_issue_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_bytes(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_cmd(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_rq_remap,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_rq_remap_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_old_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_old_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_bios(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_rq_requeue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_rq_requeue_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_cmd(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_sleeprq,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_sleeprq_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_sector(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_split,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_split_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_new_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_rwbs(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_touch_buffer,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_touch_buffer_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sector(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_size(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    block_unplug,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_block_unplug_format();
        msg->set_nr_rq(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_comm(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
