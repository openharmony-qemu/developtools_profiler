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
    bputs,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_bputs_format();
        msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_str(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    branch,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_branch_format();
        msg->set_line(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_func(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_file(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_correct(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_constant(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    context_switch,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_context_switch_format();
        msg->set_prev_pid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_next_pid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_next_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_prev_prio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_prev_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_next_prio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_next_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    funcgraph_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_funcgraph_entry_format();
        msg->set_func(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_depth(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    funcgraph_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_funcgraph_exit_format();
        msg->set_func(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_calltime(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_rettime(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_overrun(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_depth(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    function,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_function_format();
        msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_parent_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    kernel_stack,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_kernel_stack_format();
        msg->set_size(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        std::vector<uint64_t> retvalVec =
            FtraceFieldParser::ParseVectorIntField<uint64_t>(format.fields, i++, data, size);
        for (size_t i = 0; i < retvalVec.size(); i++) {
            msg->add_caller(retvalVec[i]);
        }
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mmiotrace_map,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mmiotrace_map_format();
        msg->set_phys(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_virt(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_map_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mmiotrace_rw,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mmiotrace_rw_format();
        msg->set_phys(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_value(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_pc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_map_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_width(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    print,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_print_format();
        msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_buf(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    user_stack,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_user_stack_format();
        msg->set_tgid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        std::vector<uint64_t> retvalVec =
            FtraceFieldParser::ParseVectorIntField<uint64_t>(format.fields, i++, data, size);
        for (size_t i = 0; i < retvalVec.size(); i++) {
            msg->add_caller(retvalVec[i]);
        }
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    wakeup,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_wakeup_format();
        msg->set_prev_pid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_next_pid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_next_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_prev_prio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_prev_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_next_prio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_next_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
