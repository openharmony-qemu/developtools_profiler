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
template <typename T> void bputs_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_str(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(bputs,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_bputs_format();
        bputs_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(bputs,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_bputs_format();
        bputs_func(msg, data, size, format);
    });

template <typename T> void branch_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_line(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_func(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_file(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_correct(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_constant(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(branch,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_branch_format();
        branch_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(branch,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_branch_format();
        branch_func(msg, data, size, format);
    });

template <typename T> void context_switch_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_prev_pid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_next_pid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_next_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_prev_prio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_prev_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_next_prio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_next_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(context_switch,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_context_switch_format();
        context_switch_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(context_switch,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_context_switch_format();
        context_switch_func(msg, data, size, format);
    });

template <typename T> void funcgraph_entry_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_func(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_depth(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(funcgraph_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_funcgraph_entry_format();
        funcgraph_entry_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(funcgraph_entry,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_funcgraph_entry_format();
        funcgraph_entry_func(msg, data, size, format);
    });

template <typename T> void funcgraph_exit_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_func(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_calltime(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_rettime(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_overrun(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_depth(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(funcgraph_exit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_funcgraph_exit_format();
        funcgraph_exit_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(funcgraph_exit,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_funcgraph_exit_format();
        funcgraph_exit_func(msg, data, size, format);
    });

template <typename T> void function_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_parent_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(function,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_function_format();
        function_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(function,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_function_format();
        function_func(msg, data, size, format);
    });

template <typename T> void kernel_stack_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int index = 0;
    msg->set_size(FtraceFieldParser::ParseIntField<int32_t>(format.fields, index++, data, size));
    std::vector<uint64_t> retvalVec = FtraceFieldParser::ParseVectorIntField<uint64_t>(format.fields, index++,
                                                                                       data, size);
    for (size_t i = 0; i < retvalVec.size(); i++) {
        msg->add_caller(retvalVec[i]);
    }
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kernel_stack,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kernel_stack_format();
        kernel_stack_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kernel_stack,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_kernel_stack_format();
        kernel_stack_func(msg, data, size, format);
    });

template <typename T> void mmiotrace_map_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_phys(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_virt(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_len(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_map_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mmiotrace_map,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mmiotrace_map_format();
        mmiotrace_map_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mmiotrace_map,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mmiotrace_map_format();
        mmiotrace_map_func(msg, data, size, format);
    });

template <typename T> void mmiotrace_rw_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_phys(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_value(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_pc(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_map_id(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_width(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mmiotrace_rw,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mmiotrace_rw_format();
        mmiotrace_rw_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mmiotrace_rw,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mmiotrace_rw_format();
        mmiotrace_rw_func(msg, data, size, format);
    });

template <typename T> void print_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_ip(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_buf(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(print,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_print_format();
        print_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(print,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_print_format();
        print_func(msg, data, size, format);
    });

template <typename T> void tracing_mark_write_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_buf(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(tracing_mark_write,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_print_format();
        tracing_mark_write_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(tracing_mark_write,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_print_format();
        tracing_mark_write_func(msg, data, size, format);
    });

template <typename T> void user_stack_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int index = 0;
    msg->set_tgid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, index++, data, size));
    std::vector<uint64_t> retvalVec = FtraceFieldParser::ParseVectorIntField<uint64_t>(format.fields, index++,
                                                                                       data, size);
    for (size_t i = 0; i < retvalVec.size(); i++) {
        msg->add_caller(retvalVec[i]);
    }
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(user_stack,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_user_stack_format();
        user_stack_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(user_stack,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_user_stack_format();
        user_stack_func(msg, data, size, format);
    });

template <typename T> void wakeup_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_prev_pid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_next_pid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_next_cpu(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_prev_prio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_prev_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_next_prio(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_next_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(wakeup,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_wakeup_format();
        wakeup_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(wakeup,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_wakeup_format();
        wakeup_func(msg, data, size, format);
    });
} // namespace
FTRACE_NS_END
