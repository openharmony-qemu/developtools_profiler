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
struct BreakLeaseBlockData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_next {};
    uint32_t fl_owner {};
    uint32_t fl_flags {};
    uint32_t fl_type {};
    uint64_t fl_break_time {};
    uint64_t fl_downgrade_time {};
};
__attribute__((noinline)) void ParseBreakLeaseBlockData(BreakLeaseBlockData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.fl = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_next = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBreakLeaseBlock(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BreakLeaseBlockData eventData {};
    ParseBreakLeaseBlockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_block_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_next(eventData.fl_next);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_block, ParseBreakLeaseBlock<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_block, ParseBreakLeaseBlock<ProtoEncoder::FtraceEvent>);

struct BreakLeaseNoblockData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_next {};
    uint32_t fl_owner {};
    uint32_t fl_flags {};
    uint32_t fl_type {};
    uint64_t fl_break_time {};
    uint64_t fl_downgrade_time {};
};
__attribute__((noinline)) void ParseBreakLeaseNoblockData(BreakLeaseNoblockData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.fl = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_next = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBreakLeaseNoblock(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BreakLeaseNoblockData eventData {};
    ParseBreakLeaseNoblockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_noblock_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_next(eventData.fl_next);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_noblock, ParseBreakLeaseNoblock<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_noblock, ParseBreakLeaseNoblock<ProtoEncoder::FtraceEvent>);

struct BreakLeaseUnblockData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_next {};
    uint32_t fl_owner {};
    uint32_t fl_flags {};
    uint32_t fl_type {};
    uint64_t fl_break_time {};
    uint64_t fl_downgrade_time {};
};
__attribute__((noinline)) void ParseBreakLeaseUnblockData(BreakLeaseUnblockData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.fl = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_next = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseBreakLeaseUnblock(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    BreakLeaseUnblockData eventData {};
    ParseBreakLeaseUnblockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_unblock_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_next(eventData.fl_next);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_unblock, ParseBreakLeaseUnblock<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_unblock, ParseBreakLeaseUnblock<ProtoEncoder::FtraceEvent>);

struct GenericAddLeaseData {
    uint64_t i_ino {};
    int32_t wcount {};
    int32_t dcount {};
    int32_t icount {};
    uint64_t s_dev {};
    uint32_t fl_owner {};
    uint32_t fl_flags {};
    uint32_t fl_type {};
};
__attribute__((noinline)) void ParseGenericAddLeaseData(GenericAddLeaseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.wcount = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.dcount = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.icount = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseGenericAddLease(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    GenericAddLeaseData eventData {};
    ParseGenericAddLeaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_generic_add_lease_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_i_ino(eventData.i_ino);
    msg->set_wcount(eventData.wcount);
    msg->set_dcount(eventData.dcount);
    msg->set_icount(eventData.icount);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(generic_add_lease, ParseGenericAddLease<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(generic_add_lease, ParseGenericAddLease<ProtoEncoder::FtraceEvent>);

struct GenericDeleteLeaseData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_next {};
    uint32_t fl_owner {};
    uint32_t fl_flags {};
    uint32_t fl_type {};
    uint64_t fl_break_time {};
    uint64_t fl_downgrade_time {};
};
__attribute__((noinline)) void ParseGenericDeleteLeaseData(
    GenericDeleteLeaseData& eventData, uint8_t data[], size_t size,
    const EventFormat& format)
{
    int i = 0;
    eventData.fl = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_next = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseGenericDeleteLease(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    GenericDeleteLeaseData eventData {};
    ParseGenericDeleteLeaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_generic_delete_lease_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_next(eventData.fl_next);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(generic_delete_lease, ParseGenericDeleteLease<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(generic_delete_lease, ParseGenericDeleteLease<ProtoEncoder::FtraceEvent>);

struct TimeOutLeasesData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_next {};
    uint32_t fl_owner {};
    uint32_t fl_flags {};
    uint32_t fl_type {};
    uint64_t fl_break_time {};
    uint64_t fl_downgrade_time {};
};
__attribute__((noinline)) void ParseTimeOutLeasesData(TimeOutLeasesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.fl = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_next = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseTimeOutLeases(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    TimeOutLeasesData eventData {};
    ParseTimeOutLeasesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_time_out_leases_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_next(eventData.fl_next);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(time_out_leases, ParseTimeOutLeases<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(time_out_leases, ParseTimeOutLeases<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
