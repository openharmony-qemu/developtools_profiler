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
struct BputsData {
    uint64_t ip {};
    std::string str {};
};
__attribute__((noinline)) void ParseBputsData(BputsData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ip = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.str = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseBputs(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BputsData eventData {};
    ParseBputsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_bputs_format();
    msg->set_ip(eventData.ip);
    msg->set_str(eventData.str);
}
void ParseBputsEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BputsData eventData {};
    ParseBputsData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_bputs_format();
    msg->set_ip(eventData.ip);
    msg->set_str(eventData.str);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(bputs, ParseBputs);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(bputs, ParseBputsEncoder);

struct BranchData {
    uint32_t line {};
    std::string func {};
    std::string file {};
    uint32_t correct {};
    uint32_t constant {};
};
__attribute__((noinline)) void ParseBranchData(BranchData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.line = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.func = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.file = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.correct = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.constant = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseBranch(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BranchData eventData {};
    ParseBranchData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_branch_format();
    msg->set_line(eventData.line);
    msg->set_func(eventData.func);
    msg->set_file(eventData.file);
    msg->set_correct(eventData.correct);
    msg->set_constant(eventData.constant);
}
void ParseBranchEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    BranchData eventData {};
    ParseBranchData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_branch_format();
    msg->set_line(eventData.line);
    msg->set_func(eventData.func);
    msg->set_file(eventData.file);
    msg->set_correct(eventData.correct);
    msg->set_constant(eventData.constant);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(branch, ParseBranch);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(branch, ParseBranchEncoder);

struct ContextSwitchData {
    uint32_t prev_pid {};
    uint32_t next_pid {};
    uint32_t next_cpu {};
    uint32_t prev_prio {};
    uint32_t prev_state {};
    uint32_t next_prio {};
    uint32_t next_state {};
};
__attribute__((noinline)) void ParseContextSwitchData(ContextSwitchData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.prev_pid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.next_pid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.next_cpu = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prev_prio = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prev_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.next_prio = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.next_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseContextSwitch(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ContextSwitchData eventData {};
    ParseContextSwitchData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_context_switch_format();
    msg->set_prev_pid(eventData.prev_pid);
    msg->set_next_pid(eventData.next_pid);
    msg->set_next_cpu(eventData.next_cpu);
    msg->set_prev_prio(eventData.prev_prio);
    msg->set_prev_state(eventData.prev_state);
    msg->set_next_prio(eventData.next_prio);
    msg->set_next_state(eventData.next_state);
}
void ParseContextSwitchEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    ContextSwitchData eventData {};
    ParseContextSwitchData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_context_switch_format();
    msg->set_prev_pid(eventData.prev_pid);
    msg->set_next_pid(eventData.next_pid);
    msg->set_next_cpu(eventData.next_cpu);
    msg->set_prev_prio(eventData.prev_prio);
    msg->set_prev_state(eventData.prev_state);
    msg->set_next_prio(eventData.next_prio);
    msg->set_next_state(eventData.next_state);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(context_switch, ParseContextSwitch);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(context_switch, ParseContextSwitchEncoder);

struct FuncgraphEntryData {
    uint64_t func {};
    int32_t depth {};
};
__attribute__((noinline)) void ParseFuncgraphEntryData(FuncgraphEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.func = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.depth = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseFuncgraphEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    FuncgraphEntryData eventData {};
    ParseFuncgraphEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_funcgraph_entry_format();
    msg->set_func(eventData.func);
    msg->set_depth(eventData.depth);
}
void ParseFuncgraphEntryEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    FuncgraphEntryData eventData {};
    ParseFuncgraphEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_funcgraph_entry_format();
    msg->set_func(eventData.func);
    msg->set_depth(eventData.depth);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(funcgraph_entry, ParseFuncgraphEntry);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(funcgraph_entry, ParseFuncgraphEntryEncoder);

struct FuncgraphExitData {
    uint64_t func {};
    int32_t depth {};
    uint32_t overrun {};
    uint64_t calltime {};
    uint64_t rettime {};
};
__attribute__((noinline)) void ParseFuncgraphExitData(FuncgraphExitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.func = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.depth = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.overrun = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.calltime = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.rettime = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseFuncgraphExit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    FuncgraphExitData eventData {};
    ParseFuncgraphExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_funcgraph_exit_format();
    msg->set_func(eventData.func);
    msg->set_depth(eventData.depth);
    msg->set_overrun(eventData.overrun);
    msg->set_calltime(eventData.calltime);
    msg->set_rettime(eventData.rettime);
}
void ParseFuncgraphExitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    FuncgraphExitData eventData {};
    ParseFuncgraphExitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_funcgraph_exit_format();
    msg->set_func(eventData.func);
    msg->set_depth(eventData.depth);
    msg->set_overrun(eventData.overrun);
    msg->set_calltime(eventData.calltime);
    msg->set_rettime(eventData.rettime);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(funcgraph_exit, ParseFuncgraphExit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(funcgraph_exit, ParseFuncgraphExitEncoder);

struct FunctionData {
    uint64_t ip {};
    uint64_t parent_ip {};
};
__attribute__((noinline)) void ParseFunctionData(FunctionData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ip = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.parent_ip = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseFunction(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    FunctionData eventData {};
    ParseFunctionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_function_format();
    msg->set_ip(eventData.ip);
    msg->set_parent_ip(eventData.parent_ip);
}
void ParseFunctionEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    FunctionData eventData {};
    ParseFunctionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_function_format();
    msg->set_ip(eventData.ip);
    msg->set_parent_ip(eventData.parent_ip);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(function, ParseFunction);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(function, ParseFunctionEncoder);

struct KernelStackData {
    int32_t size {};
    std::vector<uint64_t> caller {};
};
__attribute__((noinline)) void ParseKernelStackData(KernelStackData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.size = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.caller = FtraceFieldParser::ParseVectorIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseKernelStack(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    KernelStackData eventData {};
    ParseKernelStackData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_kernel_stack_format();
    msg->set_size(eventData.size);
    for (size_t i = 0; i < eventData.caller.size(); i++) {
        msg->add_caller(eventData.caller[i]);
    }
}
void ParseKernelStackEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    KernelStackData eventData {};
    ParseKernelStackData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_kernel_stack_format();
    msg->set_size(eventData.size);
    for (size_t i = 0; i < eventData.caller.size(); i++) {
        msg->add_caller(eventData.caller[i]);
    }
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kernel_stack, ParseKernelStack);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kernel_stack, ParseKernelStackEncoder);

struct MmiotraceMapData {
    uint64_t phys {};
    uint64_t virt {};
    uint64_t len {};
    int32_t map_id {};
    uint32_t opcode {};
};
__attribute__((noinline)) void ParseMmiotraceMapData(MmiotraceMapData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.phys = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.virt = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.map_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.opcode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseMmiotraceMap(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmiotraceMapData eventData {};
    ParseMmiotraceMapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mmiotrace_map_format();
    msg->set_phys(eventData.phys);
    msg->set_virt(eventData.virt);
    msg->set_len(eventData.len);
    msg->set_map_id(eventData.map_id);
    msg->set_opcode(eventData.opcode);
}
void ParseMmiotraceMapEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmiotraceMapData eventData {};
    ParseMmiotraceMapData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mmiotrace_map_format();
    msg->set_phys(eventData.phys);
    msg->set_virt(eventData.virt);
    msg->set_len(eventData.len);
    msg->set_map_id(eventData.map_id);
    msg->set_opcode(eventData.opcode);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mmiotrace_map, ParseMmiotraceMap);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mmiotrace_map, ParseMmiotraceMapEncoder);

struct MmiotraceRwData {
    uint64_t phys {};
    uint64_t value {};
    uint64_t pc {};
    int32_t map_id {};
    uint32_t opcode {};
    uint32_t width {};
};
__attribute__((noinline)) void ParseMmiotraceRwData(MmiotraceRwData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.phys = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.value = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pc = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.map_id = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.opcode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.width = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseMmiotraceRw(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmiotraceRwData eventData {};
    ParseMmiotraceRwData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mmiotrace_rw_format();
    msg->set_phys(eventData.phys);
    msg->set_value(eventData.value);
    msg->set_pc(eventData.pc);
    msg->set_map_id(eventData.map_id);
    msg->set_opcode(eventData.opcode);
    msg->set_width(eventData.width);
}
void ParseMmiotraceRwEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmiotraceRwData eventData {};
    ParseMmiotraceRwData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mmiotrace_rw_format();
    msg->set_phys(eventData.phys);
    msg->set_value(eventData.value);
    msg->set_pc(eventData.pc);
    msg->set_map_id(eventData.map_id);
    msg->set_opcode(eventData.opcode);
    msg->set_width(eventData.width);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mmiotrace_rw, ParseMmiotraceRw);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mmiotrace_rw, ParseMmiotraceRwEncoder);

struct PrintData {
    uint64_t ip {};
    std::string buf {};
};
__attribute__((noinline)) void ParsePrintData(PrintData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.ip = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.buf = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParsePrint(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PrintData eventData {};
    ParsePrintData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_print_format();
    msg->set_ip(eventData.ip);
    msg->set_buf(eventData.buf);
}
void ParsePrintEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    PrintData eventData {};
    ParsePrintData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_print_format();
    msg->set_ip(eventData.ip);
    msg->set_buf(eventData.buf);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(print, ParsePrint);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(print, ParsePrintEncoder);

struct UserStackData {
    uint32_t tgid {};
    std::vector<uint64_t> caller {};
};
__attribute__((noinline)) void ParseUserStackData(UserStackData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.tgid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.caller = FtraceFieldParser::ParseVectorIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseUserStack(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    UserStackData eventData {};
    ParseUserStackData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_user_stack_format();
    msg->set_tgid(eventData.tgid);
    for (size_t i = 0; i < eventData.caller.size(); i++) {
        msg->add_caller(eventData.caller[i]);
    }
}
void ParseUserStackEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    UserStackData eventData {};
    ParseUserStackData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_user_stack_format();
    msg->set_tgid(eventData.tgid);
    for (size_t i = 0; i < eventData.caller.size(); i++) {
        msg->add_caller(eventData.caller[i]);
    }
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(user_stack, ParseUserStack);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(user_stack, ParseUserStackEncoder);

struct WakeupData {
    uint32_t prev_pid {};
    uint32_t next_pid {};
    uint32_t next_cpu {};
    uint32_t prev_prio {};
    uint32_t prev_state {};
    uint32_t next_prio {};
    uint32_t next_state {};
};
__attribute__((noinline)) void ParseWakeupData(WakeupData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.prev_pid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.next_pid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.next_cpu = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prev_prio = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prev_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.next_prio = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.next_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseWakeup(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WakeupData eventData {};
    ParseWakeupData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_wakeup_format();
    msg->set_prev_pid(eventData.prev_pid);
    msg->set_next_pid(eventData.next_pid);
    msg->set_next_cpu(eventData.next_cpu);
    msg->set_prev_prio(eventData.prev_prio);
    msg->set_prev_state(eventData.prev_state);
    msg->set_next_prio(eventData.next_prio);
    msg->set_next_state(eventData.next_state);
}
void ParseWakeupEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    WakeupData eventData {};
    ParseWakeupData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_wakeup_format();
    msg->set_prev_pid(eventData.prev_pid);
    msg->set_next_pid(eventData.next_pid);
    msg->set_next_cpu(eventData.next_cpu);
    msg->set_prev_prio(eventData.prev_prio);
    msg->set_prev_state(eventData.prev_state);
    msg->set_next_prio(eventData.next_prio);
    msg->set_next_state(eventData.next_state);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(wakeup, ParseWakeup);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(wakeup, ParseWakeupEncoder);
}  // namespace
FTRACE_NS_END
