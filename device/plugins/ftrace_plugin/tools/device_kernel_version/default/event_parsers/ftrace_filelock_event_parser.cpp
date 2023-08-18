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
    break_lease_block,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_break_lease_block_format();
        msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_next(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    break_lease_noblock,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_break_lease_noblock_format();
        msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_next(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    break_lease_unblock,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_break_lease_unblock_format();
        msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_next(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    generic_add_lease,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_generic_add_lease_format();
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_wcount(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_dcount(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_icount(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    generic_delete_lease,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_generic_delete_lease_format();
        msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_next(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    time_out_leases,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_time_out_leases_format();
        msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_next(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
