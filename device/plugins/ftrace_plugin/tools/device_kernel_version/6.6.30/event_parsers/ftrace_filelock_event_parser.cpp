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
    uint64_t fl_blocker {};
    uint64_t fl_owner {};
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
    eventData.fl_blocker = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseBreakLeaseBlock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BreakLeaseBlockData eventData {};
    ParseBreakLeaseBlockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_block_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
void ParseBreakLeaseBlockEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BreakLeaseBlockData eventData {};
    ParseBreakLeaseBlockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_block_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_block, ParseBreakLeaseBlock);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_block, ParseBreakLeaseBlockEncoder);

struct BreakLeaseNoblockData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_blocker {};
    uint64_t fl_owner {};
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
    eventData.fl_blocker = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseBreakLeaseNoblock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BreakLeaseNoblockData eventData {};
    ParseBreakLeaseNoblockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_noblock_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
void ParseBreakLeaseNoblockEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BreakLeaseNoblockData eventData {};
    ParseBreakLeaseNoblockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_noblock_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_noblock, ParseBreakLeaseNoblock);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_noblock, ParseBreakLeaseNoblockEncoder);

struct BreakLeaseUnblockData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_blocker {};
    uint64_t fl_owner {};
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
    eventData.fl_blocker = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseBreakLeaseUnblock(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BreakLeaseUnblockData eventData {};
    ParseBreakLeaseUnblockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_unblock_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
void ParseBreakLeaseUnblockEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BreakLeaseUnblockData eventData {};
    ParseBreakLeaseUnblockData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_break_lease_unblock_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_unblock, ParseBreakLeaseUnblock);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_unblock, ParseBreakLeaseUnblockEncoder);

struct GenericAddLeaseData {
    uint64_t i_ino {};
    int32_t wcount {};
    int32_t rcount {};
    int32_t icount {};
    uint64_t s_dev {};
    uint64_t fl_owner {};
    uint32_t fl_flags {};
    uint32_t fl_type {};
};
__attribute__((noinline)) void ParseGenericAddLeaseData(GenericAddLeaseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.wcount = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.rcount = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.icount = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseGenericAddLease(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    GenericAddLeaseData eventData {};
    ParseGenericAddLeaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_generic_add_lease_format();
    msg->set_i_ino(eventData.i_ino);
    msg->set_wcount(eventData.wcount);
    msg->set_rcount(eventData.rcount);
    msg->set_icount(eventData.icount);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
}
void ParseGenericAddLeaseEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    GenericAddLeaseData eventData {};
    ParseGenericAddLeaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_generic_add_lease_format();
    msg->set_i_ino(eventData.i_ino);
    msg->set_wcount(eventData.wcount);
    msg->set_rcount(eventData.rcount);
    msg->set_icount(eventData.icount);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(generic_add_lease, ParseGenericAddLease);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(generic_add_lease, ParseGenericAddLeaseEncoder);

struct GenericDeleteLeaseData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_blocker {};
    uint64_t fl_owner {};
    uint32_t fl_flags {};
    uint32_t fl_type {};
    uint64_t fl_break_time {};
    uint64_t fl_downgrade_time {};
};
__attribute__((noinline)) void ParseGenericDeleteLeaseData(GenericDeleteLeaseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.fl = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.i_ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.s_dev = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_blocker = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseGenericDeleteLease(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    GenericDeleteLeaseData eventData {};
    ParseGenericDeleteLeaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_generic_delete_lease_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
void ParseGenericDeleteLeaseEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    GenericDeleteLeaseData eventData {};
    ParseGenericDeleteLeaseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_generic_delete_lease_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(generic_delete_lease, ParseGenericDeleteLease);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(generic_delete_lease, ParseGenericDeleteLeaseEncoder);

struct TimeOutLeasesData {
    uint64_t fl {};
    uint64_t i_ino {};
    uint64_t s_dev {};
    uint64_t fl_blocker {};
    uint64_t fl_owner {};
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
    eventData.fl_blocker = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_owner = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.fl_break_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.fl_downgrade_time = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseTimeOutLeases(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimeOutLeasesData eventData {};
    ParseTimeOutLeasesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_time_out_leases_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
void ParseTimeOutLeasesEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    TimeOutLeasesData eventData {};
    ParseTimeOutLeasesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_time_out_leases_format();
    msg->set_fl(eventData.fl);
    msg->set_i_ino(eventData.i_ino);
    msg->set_s_dev(eventData.s_dev);
    msg->set_fl_blocker(eventData.fl_blocker);
    msg->set_fl_owner(eventData.fl_owner);
    msg->set_fl_flags(eventData.fl_flags);
    msg->set_fl_type(eventData.fl_type);
    msg->set_fl_break_time(eventData.fl_break_time);
    msg->set_fl_downgrade_time(eventData.fl_downgrade_time);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(time_out_leases, ParseTimeOutLeases);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(time_out_leases, ParseTimeOutLeasesEncoder);
}  // namespace
FTRACE_NS_END
