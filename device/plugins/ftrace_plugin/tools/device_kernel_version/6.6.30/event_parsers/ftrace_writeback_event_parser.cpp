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
struct BalanceDirtyPagesData {
    std::string bdi {};
    uint64_t limit {};
    uint64_t setpoint {};
    uint64_t dirty {};
    uint64_t bdi_setpoint {};
    uint64_t bdi_dirty {};
    uint64_t dirty_ratelimit {};
    uint64_t task_ratelimit {};
    uint32_t dirtied {};
    uint32_t dirtied_pause {};
    uint64_t paused {};
    uint64_t pause {};
    uint64_t period {};
    uint64_t think {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseBalanceDirtyPagesData(BalanceDirtyPagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.bdi = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.limit = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.setpoint = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirty = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bdi_setpoint = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bdi_dirty = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirty_ratelimit = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.task_ratelimit = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirtied = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.dirtied_pause = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.paused = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pause = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.period = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.think = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBalanceDirtyPages(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BalanceDirtyPagesData eventData {};
    ParseBalanceDirtyPagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_balance_dirty_pages_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_bdi(eventData.bdi);
    msg->set_limit(eventData.limit);
    msg->set_setpoint(eventData.setpoint);
    msg->set_dirty(eventData.dirty);
    msg->set_bdi_setpoint(eventData.bdi_setpoint);
    msg->set_bdi_dirty(eventData.bdi_dirty);
    msg->set_dirty_ratelimit(eventData.dirty_ratelimit);
    msg->set_task_ratelimit(eventData.task_ratelimit);
    msg->set_dirtied(eventData.dirtied);
    msg->set_dirtied_pause(eventData.dirtied_pause);
    msg->set_paused(eventData.paused);
    msg->set_pause(eventData.pause);
    msg->set_period(eventData.period);
    msg->set_think(eventData.think);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(balance_dirty_pages, ParseBalanceDirtyPages<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(balance_dirty_pages, ParseBalanceDirtyPages<ProtoEncoder::FtraceEvent>);

struct BdiDirtyRatelimitData {
    std::string bdi {};
    uint64_t write_bw {};
    uint64_t avg_write_bw {};
    uint64_t dirty_rate {};
    uint64_t dirty_ratelimit {};
    uint64_t task_ratelimit {};
    uint64_t balanced_dirty_ratelimit {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseBdiDirtyRatelimitData(BdiDirtyRatelimitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.bdi = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.write_bw = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.avg_write_bw = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirty_rate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirty_ratelimit = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.task_ratelimit = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.balanced_dirty_ratelimit = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBdiDirtyRatelimit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BdiDirtyRatelimitData eventData {};
    ParseBdiDirtyRatelimitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_bdi_dirty_ratelimit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_bdi(eventData.bdi);
    msg->set_write_bw(eventData.write_bw);
    msg->set_avg_write_bw(eventData.avg_write_bw);
    msg->set_dirty_rate(eventData.dirty_rate);
    msg->set_dirty_ratelimit(eventData.dirty_ratelimit);
    msg->set_task_ratelimit(eventData.task_ratelimit);
    msg->set_balanced_dirty_ratelimit(eventData.balanced_dirty_ratelimit);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(bdi_dirty_ratelimit, ParseBdiDirtyRatelimit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(bdi_dirty_ratelimit, ParseBdiDirtyRatelimit<ProtoEncoder::FtraceEvent>);

struct GlobalDirtyStateData {
    uint64_t nr_dirty {};
    uint64_t nr_writeback {};
    uint64_t background_thresh {};
    uint64_t dirty_thresh {};
    uint64_t dirty_limit {};
    uint64_t nr_dirtied {};
    uint64_t nr_written {};
};
__attribute__((noinline)) void ParseGlobalDirtyStateData(GlobalDirtyStateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nr_dirty = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_writeback = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.background_thresh = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirty_thresh = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirty_limit = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_dirtied = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_written = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseGlobalDirtyState(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    GlobalDirtyStateData eventData {};
    ParseGlobalDirtyStateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_global_dirty_state_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nr_dirty(eventData.nr_dirty);
    msg->set_nr_writeback(eventData.nr_writeback);
    msg->set_background_thresh(eventData.background_thresh);
    msg->set_dirty_thresh(eventData.dirty_thresh);
    msg->set_dirty_limit(eventData.dirty_limit);
    msg->set_nr_dirtied(eventData.nr_dirtied);
    msg->set_nr_written(eventData.nr_written);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(global_dirty_state, ParseGlobalDirtyState<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(global_dirty_state, ParseGlobalDirtyState<ProtoEncoder::FtraceEvent>);

struct WbcWritepageData {
    std::string name {};
    uint64_t nr_to_write {};
    uint64_t pages_skipped {};
    int32_t sync_mode {};
    int32_t for_kupdate {};
    int32_t for_background {};
    int32_t for_reclaim {};
    int32_t range_cyclic {};
    uint64_t range_start {};
    uint64_t range_end {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWbcWritepageData(WbcWritepageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.nr_to_write = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pages_skipped = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_kupdate = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_background = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_reclaim = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.range_cyclic = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.range_start = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.range_end = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWbcWritepage(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WbcWritepageData eventData {};
    ParseWbcWritepageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_wbc_writepage_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_nr_to_write(eventData.nr_to_write);
    msg->set_pages_skipped(eventData.pages_skipped);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_for_kupdate(eventData.for_kupdate);
    msg->set_for_background(eventData.for_background);
    msg->set_for_reclaim(eventData.for_reclaim);
    msg->set_range_cyclic(eventData.range_cyclic);
    msg->set_range_start(eventData.range_start);
    msg->set_range_end(eventData.range_end);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(wbc_writepage, ParseWbcWritepage<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(wbc_writepage, ParseWbcWritepage<ProtoEncoder::FtraceEvent>);

struct WritebackBdiRegisterData {
    std::string name {};
};
__attribute__((noinline)) void ParseWritebackBdiRegisterData(
    WritebackBdiRegisterData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackBdiRegister(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackBdiRegisterData eventData {};
    ParseWritebackBdiRegisterData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_bdi_register_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_bdi_register, ParseWritebackBdiRegister<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_bdi_register,
    ParseWritebackBdiRegister<ProtoEncoder::FtraceEvent>);

struct WritebackDirtyInodeData {
    std::string name {};
    uint64_t ino {};
    uint64_t state {};
    uint64_t flags {};
};
__attribute__((noinline)) void ParseWritebackDirtyInodeData(
    WritebackDirtyInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackDirtyInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackDirtyInodeData eventData {};
    ParseWritebackDirtyInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_dirty_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_dirty_inode, ParseWritebackDirtyInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_dirty_inode,
    ParseWritebackDirtyInode<ProtoEncoder::FtraceEvent>);

struct WritebackDirtyInodeEnqueueData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t state {};
    uint32_t mode {};
    uint64_t dirtied_when {};
};
__attribute__((noinline)) void ParseWritebackDirtyInodeEnqueueData(
    WritebackDirtyInodeEnqueueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.dirtied_when = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackDirtyInodeEnqueue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackDirtyInodeEnqueueData eventData {};
    ParseWritebackDirtyInodeEnqueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_dirty_inode_enqueue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_mode(eventData.mode);
    msg->set_dirtied_when(eventData.dirtied_when);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_dirty_inode_enqueue, ParseWritebackDirtyInodeEnqueue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_dirty_inode_enqueue,
    ParseWritebackDirtyInodeEnqueue<ProtoEncoder::FtraceEvent>);

struct WritebackDirtyInodeStartData {
    std::string name {};
    uint64_t ino {};
    uint64_t state {};
    uint64_t flags {};
};
__attribute__((noinline)) void ParseWritebackDirtyInodeStartData(
    WritebackDirtyInodeStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackDirtyInodeStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackDirtyInodeStartData eventData {};
    ParseWritebackDirtyInodeStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_dirty_inode_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_dirty_inode_start, ParseWritebackDirtyInodeStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_dirty_inode_start,
    ParseWritebackDirtyInodeStart<ProtoEncoder::FtraceEvent>);

struct WritebackExecData {
    std::string name {};
    uint64_t nr_pages {};
    uint64_t sb_dev {};
    int32_t sync_mode {};
    int32_t for_kupdate {};
    int32_t range_cyclic {};
    int32_t for_background {};
    int32_t reason {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackExecData(WritebackExecData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.nr_pages = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sb_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_kupdate = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.range_cyclic = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_background = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reason = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackExec(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackExecData eventData {};
    ParseWritebackExecData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_exec_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_nr_pages(eventData.nr_pages);
    msg->set_sb_dev(eventData.sb_dev);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_for_kupdate(eventData.for_kupdate);
    msg->set_range_cyclic(eventData.range_cyclic);
    msg->set_for_background(eventData.for_background);
    msg->set_reason(eventData.reason);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_exec, ParseWritebackExec<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_exec, ParseWritebackExec<ProtoEncoder::FtraceEvent>);

struct WritebackLazytimeData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t state {};
    uint32_t mode {};
    uint64_t dirtied_when {};
};
__attribute__((noinline)) void ParseWritebackLazytimeData(WritebackLazytimeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.dirtied_when = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackLazytime(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackLazytimeData eventData {};
    ParseWritebackLazytimeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_lazytime_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_mode(eventData.mode);
    msg->set_dirtied_when(eventData.dirtied_when);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_lazytime, ParseWritebackLazytime<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_lazytime, ParseWritebackLazytime<ProtoEncoder::FtraceEvent>);

struct WritebackLazytimeIputData {
    uint64_t dev {};
    uint64_t ino {};
    uint64_t state {};
    uint32_t mode {};
    uint64_t dirtied_when {};
};
__attribute__((noinline)) void ParseWritebackLazytimeIputData(
    WritebackLazytimeIputData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.dirtied_when = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackLazytimeIput(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackLazytimeIputData eventData {};
    ParseWritebackLazytimeIputData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_lazytime_iput_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_dev(eventData.dev);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_mode(eventData.mode);
    msg->set_dirtied_when(eventData.dirtied_when);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_lazytime_iput, ParseWritebackLazytimeIput<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_lazytime_iput,
    ParseWritebackLazytimeIput<ProtoEncoder::FtraceEvent>);

struct WritebackMarkInodeDirtyData {
    std::string name {};
    uint64_t ino {};
    uint64_t state {};
    uint64_t flags {};
};
__attribute__((noinline)) void ParseWritebackMarkInodeDirtyData(
    WritebackMarkInodeDirtyData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackMarkInodeDirty(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackMarkInodeDirtyData eventData {};
    ParseWritebackMarkInodeDirtyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_mark_inode_dirty_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_mark_inode_dirty, ParseWritebackMarkInodeDirty<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_mark_inode_dirty,
    ParseWritebackMarkInodeDirty<ProtoEncoder::FtraceEvent>);

struct WritebackPagesWrittenData {
    uint64_t pages {};
};
__attribute__((noinline)) void ParseWritebackPagesWrittenData(
    WritebackPagesWrittenData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pages = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackPagesWritten(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackPagesWrittenData eventData {};
    ParseWritebackPagesWrittenData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_pages_written_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pages(eventData.pages);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_pages_written, ParseWritebackPagesWritten<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_pages_written,
    ParseWritebackPagesWritten<ProtoEncoder::FtraceEvent>);

struct WritebackQueueData {
    std::string name {};
    uint64_t nr_pages {};
    uint64_t sb_dev {};
    int32_t sync_mode {};
    int32_t for_kupdate {};
    int32_t range_cyclic {};
    int32_t for_background {};
    int32_t reason {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackQueueData(WritebackQueueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.nr_pages = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sb_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_kupdate = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.range_cyclic = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_background = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reason = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackQueue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackQueueData eventData {};
    ParseWritebackQueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_queue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_nr_pages(eventData.nr_pages);
    msg->set_sb_dev(eventData.sb_dev);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_for_kupdate(eventData.for_kupdate);
    msg->set_range_cyclic(eventData.range_cyclic);
    msg->set_for_background(eventData.for_background);
    msg->set_reason(eventData.reason);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_queue, ParseWritebackQueue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_queue, ParseWritebackQueue<ProtoEncoder::FtraceEvent>);

struct WritebackQueueIoData {
    std::string name {};
    uint64_t older {};
    uint64_t age {};
    int32_t moved {};
    int32_t reason {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackQueueIoData(WritebackQueueIoData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.older = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.age = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.moved = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reason = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackQueueIo(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackQueueIoData eventData {};
    ParseWritebackQueueIoData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_queue_io_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_older(eventData.older);
    msg->set_age(eventData.age);
    msg->set_moved(eventData.moved);
    msg->set_reason(eventData.reason);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_queue_io, ParseWritebackQueueIo<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_queue_io, ParseWritebackQueueIo<ProtoEncoder::FtraceEvent>);

struct WritebackSbInodesRequeueData {
    std::string name {};
    uint64_t ino {};
    uint64_t state {};
    uint64_t dirtied_when {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackSbInodesRequeueData(
    WritebackSbInodesRequeueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirtied_when = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackSbInodesRequeue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackSbInodesRequeueData eventData {};
    ParseWritebackSbInodesRequeueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_sb_inodes_requeue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_dirtied_when(eventData.dirtied_when);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_sb_inodes_requeue, ParseWritebackSbInodesRequeue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_sb_inodes_requeue,
    ParseWritebackSbInodesRequeue<ProtoEncoder::FtraceEvent>);

struct WritebackSingleInodeData {
    std::string name {};
    uint64_t ino {};
    uint64_t state {};
    uint64_t dirtied_when {};
    uint64_t writeback_index {};
    uint64_t nr_to_write {};
    uint64_t wrote {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackSingleInodeData(
    WritebackSingleInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirtied_when = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.writeback_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_to_write = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.wrote = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackSingleInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackSingleInodeData eventData {};
    ParseWritebackSingleInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_single_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_dirtied_when(eventData.dirtied_when);
    msg->set_writeback_index(eventData.writeback_index);
    msg->set_nr_to_write(eventData.nr_to_write);
    msg->set_wrote(eventData.wrote);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_single_inode, ParseWritebackSingleInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_single_inode,
    ParseWritebackSingleInode<ProtoEncoder::FtraceEvent>);

struct WritebackSingleInodeStartData {
    std::string name {};
    uint64_t ino {};
    uint64_t state {};
    uint64_t dirtied_when {};
    uint64_t writeback_index {};
    uint64_t nr_to_write {};
    uint64_t wrote {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackSingleInodeStartData(
    WritebackSingleInodeStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.state = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dirtied_when = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.writeback_index = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_to_write = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.wrote = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackSingleInodeStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackSingleInodeStartData eventData {};
    ParseWritebackSingleInodeStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_single_inode_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_ino(eventData.ino);
    msg->set_state(eventData.state);
    msg->set_dirtied_when(eventData.dirtied_when);
    msg->set_writeback_index(eventData.writeback_index);
    msg->set_nr_to_write(eventData.nr_to_write);
    msg->set_wrote(eventData.wrote);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_single_inode_start, ParseWritebackSingleInodeStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_single_inode_start,
    ParseWritebackSingleInodeStart<ProtoEncoder::FtraceEvent>);

struct WritebackStartData {
    std::string name {};
    uint64_t nr_pages {};
    uint64_t sb_dev {};
    int32_t sync_mode {};
    int32_t for_kupdate {};
    int32_t range_cyclic {};
    int32_t for_background {};
    int32_t reason {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackStartData(WritebackStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.nr_pages = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sb_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_kupdate = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.range_cyclic = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_background = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reason = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackStartData eventData {};
    ParseWritebackStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_nr_pages(eventData.nr_pages);
    msg->set_sb_dev(eventData.sb_dev);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_for_kupdate(eventData.for_kupdate);
    msg->set_range_cyclic(eventData.range_cyclic);
    msg->set_for_background(eventData.for_background);
    msg->set_reason(eventData.reason);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_start, ParseWritebackStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_start, ParseWritebackStart<ProtoEncoder::FtraceEvent>);

struct WritebackWaitData {
    std::string name {};
    uint64_t nr_pages {};
    uint64_t sb_dev {};
    int32_t sync_mode {};
    int32_t for_kupdate {};
    int32_t range_cyclic {};
    int32_t for_background {};
    int32_t reason {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackWaitData(WritebackWaitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.nr_pages = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sb_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_kupdate = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.range_cyclic = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_background = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reason = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackWait(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackWaitData eventData {};
    ParseWritebackWaitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_wait_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_nr_pages(eventData.nr_pages);
    msg->set_sb_dev(eventData.sb_dev);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_for_kupdate(eventData.for_kupdate);
    msg->set_range_cyclic(eventData.range_cyclic);
    msg->set_for_background(eventData.for_background);
    msg->set_reason(eventData.reason);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_wait, ParseWritebackWait<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_wait, ParseWritebackWait<ProtoEncoder::FtraceEvent>);

struct WritebackWakeBackgroundData {
    std::string name {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackWakeBackgroundData(
    WritebackWakeBackgroundData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackWakeBackground(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackWakeBackgroundData eventData {};
    ParseWritebackWakeBackgroundData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_wake_background_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_wake_background, ParseWritebackWakeBackground<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_wake_background,
    ParseWritebackWakeBackground<ProtoEncoder::FtraceEvent>);

struct WritebackWriteInodeData {
    std::string name {};
    uint64_t ino {};
    int32_t sync_mode {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackWriteInodeData(
    WritebackWriteInodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackWriteInode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackWriteInodeData eventData {};
    ParseWritebackWriteInodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_write_inode_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_ino(eventData.ino);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_write_inode, ParseWritebackWriteInode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_write_inode,
    ParseWritebackWriteInode<ProtoEncoder::FtraceEvent>);

struct WritebackWriteInodeStartData {
    std::string name {};
    uint64_t ino {};
    int32_t sync_mode {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackWriteInodeStartData(
    WritebackWriteInodeStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackWriteInodeStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackWriteInodeStartData eventData {};
    ParseWritebackWriteInodeStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_write_inode_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_ino(eventData.ino);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_write_inode_start, ParseWritebackWriteInodeStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_write_inode_start,
    ParseWritebackWriteInodeStart<ProtoEncoder::FtraceEvent>);

struct WritebackWrittenData {
    std::string name {};
    uint64_t nr_pages {};
    uint64_t sb_dev {};
    int32_t sync_mode {};
    int32_t for_kupdate {};
    int32_t range_cyclic {};
    int32_t for_background {};
    int32_t reason {};
    uint64_t cgroup_ino {};
};
__attribute__((noinline)) void ParseWritebackWrittenData(WritebackWrittenData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.nr_pages = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sb_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync_mode = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_kupdate = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.range_cyclic = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.for_background = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reason = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cgroup_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseWritebackWritten(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    WritebackWrittenData eventData {};
    ParseWritebackWrittenData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_writeback_written_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_name(eventData.name);
    msg->set_nr_pages(eventData.nr_pages);
    msg->set_sb_dev(eventData.sb_dev);
    msg->set_sync_mode(eventData.sync_mode);
    msg->set_for_kupdate(eventData.for_kupdate);
    msg->set_range_cyclic(eventData.range_cyclic);
    msg->set_for_background(eventData.for_background);
    msg->set_reason(eventData.reason);
    msg->set_cgroup_ino(eventData.cgroup_ino);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(writeback_written, ParseWritebackWritten<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(writeback_written, ParseWritebackWritten<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
