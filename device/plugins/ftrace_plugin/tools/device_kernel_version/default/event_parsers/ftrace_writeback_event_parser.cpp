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
    balance_dirty_pages,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_balance_dirty_pages_format();
        msg->set_bdi(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_limit(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_setpoint(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirty(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bdi_setpoint(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_bdi_dirty(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirty_ratelimit(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_task_ratelimit(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirtied(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_dirtied_pause(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_paused(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_pause(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_period(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_think(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    bdi_dirty_ratelimit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_bdi_dirty_ratelimit_format();
        msg->set_bdi(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_write_bw(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_avg_write_bw(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirty_rate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirty_ratelimit(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_task_ratelimit(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_balanced_dirty_ratelimit(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    global_dirty_state,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_global_dirty_state_format();
        msg->set_nr_dirty(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_writeback(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_unstable(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_background_thresh(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirty_thresh(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirty_limit(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_dirtied(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_written(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    wbc_writepage,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_wbc_writepage_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_nr_to_write(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_pages_skipped(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_kupdate(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_background(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_reclaim(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_range_cyclic(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_range_start(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_range_end(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_bdi_register,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_bdi_register_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_congestion_wait,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_congestion_wait_format();
        msg->set_usec_timeout(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_usec_delayed(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_dirty_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_dirty_inode_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_dirty_inode_enqueue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_dirty_inode_enqueue_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_dirtied_when(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_dirty_inode_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_dirty_inode_start_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_dirty_page,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_dirty_page_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_exec,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_exec_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_nr_pages(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sb_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_kupdate(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_range_cyclic(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_background(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_reason(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_lazytime,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_lazytime_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_dirtied_when(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_lazytime_iput,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_lazytime_iput_format();
        msg->set_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_dirtied_when(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_mark_inode_dirty,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_mark_inode_dirty_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_pages_written,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_pages_written_format();
        msg->set_pages(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_queue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_queue_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_nr_pages(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sb_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_kupdate(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_range_cyclic(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_background(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_reason(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_queue_io,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_queue_io_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_older(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_age(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_moved(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_reason(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_sb_inodes_requeue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_sb_inodes_requeue_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirtied_when(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_single_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_single_inode_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirtied_when(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_writeback_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_to_write(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_wrote(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_single_inode_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_single_inode_start_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_state(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dirtied_when(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_writeback_index(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_nr_to_write(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_wrote(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_start_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_nr_pages(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sb_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_kupdate(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_range_cyclic(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_background(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_reason(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_wait,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_wait_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_nr_pages(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sb_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_kupdate(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_range_cyclic(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_background(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_reason(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_wait_iff_congested,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_wait_iff_congested_format();
        msg->set_usec_timeout(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_usec_delayed(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_wake_background,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_wake_background_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_write_inode,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_write_inode_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_write_inode_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_write_inode_start_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    writeback_written,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_writeback_written_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_nr_pages(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sb_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_sync_mode(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_kupdate(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_range_cyclic(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_for_background(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_reason(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cgroup_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
