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
using namespace OHOS::Developtools::Profiler;
template <typename T>
void break_lease_block_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_blocker(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_block, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                           const EventFormat &format) {
    auto msg = ftraceEvent.mutable_break_lease_block_format();
    break_lease_block_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_block,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_break_lease_block_format();
    break_lease_block_func(msg, data, size, format);
});

template <typename T>
void break_lease_noblock_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_blocker(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_noblock, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                             const EventFormat &format) {
    auto msg = ftraceEvent.mutable_break_lease_noblock_format();
    break_lease_noblock_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_noblock,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_break_lease_noblock_format();
    break_lease_noblock_func(msg, data, size, format);
});

template <typename T>
void break_lease_unblock_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_blocker(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(break_lease_unblock, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                             const EventFormat &format) {
    auto msg = ftraceEvent.mutable_break_lease_unblock_format();
    break_lease_unblock_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(break_lease_unblock,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_break_lease_unblock_format();
    break_lease_unblock_func(msg, data, size, format);
});

template <typename T>
void generic_add_lease_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_wcount(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_rcount(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_icount(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(generic_add_lease, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                           const EventFormat &format) {
    auto msg = ftraceEvent.mutable_generic_add_lease_format();
    generic_add_lease_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(generic_add_lease,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_generic_add_lease_format();
    generic_add_lease_func(msg, data, size, format);
});

template <typename T>
void generic_delete_lease_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_blocker(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(generic_delete_lease, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                              const EventFormat &format) {
    auto msg = ftraceEvent.mutable_generic_delete_lease_format();
    generic_delete_lease_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(generic_delete_lease,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_generic_delete_lease_format();
    generic_delete_lease_func(msg, data, size, format);
});

template <typename T>
void time_out_leases_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_fl(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_i_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_s_dev(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_blocker(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_owner(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_fl_break_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_fl_downgrade_time(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(time_out_leases, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                         const EventFormat &format) {
    auto msg = ftraceEvent.mutable_time_out_leases_format();
    time_out_leases_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(time_out_leases, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                 size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_time_out_leases_format();
    time_out_leases_func(msg, data, size, format);
});
}  // namespace
FTRACE_NS_END
