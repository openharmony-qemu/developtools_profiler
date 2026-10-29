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
struct MmcRequestDoneData {
    uint32_t cmd_opcode {};
    int32_t cmd_err {};
    std::string cmd_resp {};
    uint32_t cmd_retries {};
    uint32_t stop_opcode {};
    int32_t stop_err {};
    std::string stop_resp {};
    uint32_t stop_retries {};
    uint32_t sbc_opcode {};
    int32_t sbc_err {};
    std::string sbc_resp {};
    uint32_t sbc_retries {};
    uint32_t bytes_xfered {};
    int32_t data_err {};
    int32_t tag {};
    uint32_t can_retune {};
    uint32_t doing_retune {};
    uint32_t retune_now {};
    int32_t need_retune {};
    int32_t hold_retune {};
    uint32_t retune_period {};
    uint64_t mrq {};
    std::string name {};
};
__attribute__((noinline)) void ParseMmcRequestDoneData(MmcRequestDoneData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cmd_opcode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cmd_err = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.cmd_resp = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.cmd_retries = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.stop_opcode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.stop_err = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.stop_resp = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.stop_retries = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sbc_opcode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sbc_err = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.sbc_resp = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.sbc_retries = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.bytes_xfered = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.data_err = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.tag = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.can_retune = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.doing_retune = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.retune_now = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.need_retune = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.hold_retune = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.retune_period = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mrq = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmcRequestDone(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmcRequestDoneData eventData {};
    ParseMmcRequestDoneData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mmc_request_done_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd_opcode(eventData.cmd_opcode);
    msg->set_cmd_err(eventData.cmd_err);
    msg->set_cmd_resp(eventData.cmd_resp);
    msg->set_cmd_retries(eventData.cmd_retries);
    msg->set_stop_opcode(eventData.stop_opcode);
    msg->set_stop_err(eventData.stop_err);
    msg->set_stop_resp(eventData.stop_resp);
    msg->set_stop_retries(eventData.stop_retries);
    msg->set_sbc_opcode(eventData.sbc_opcode);
    msg->set_sbc_err(eventData.sbc_err);
    msg->set_sbc_resp(eventData.sbc_resp);
    msg->set_sbc_retries(eventData.sbc_retries);
    msg->set_bytes_xfered(eventData.bytes_xfered);
    msg->set_data_err(eventData.data_err);
    msg->set_tag(eventData.tag);
    msg->set_can_retune(eventData.can_retune);
    msg->set_doing_retune(eventData.doing_retune);
    msg->set_retune_now(eventData.retune_now);
    msg->set_need_retune(eventData.need_retune);
    msg->set_hold_retune(eventData.hold_retune);
    msg->set_retune_period(eventData.retune_period);
    msg->set_mrq(eventData.mrq);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mmc_request_done, ParseMmcRequestDone<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mmc_request_done, ParseMmcRequestDone<ProtoEncoder::FtraceEvent>);

struct MmcRequestStartData {
    uint32_t cmd_opcode {};
    uint32_t cmd_arg {};
    uint32_t cmd_flags {};
    uint32_t cmd_retries {};
    uint32_t stop_opcode {};
    uint32_t stop_arg {};
    uint32_t stop_flags {};
    uint32_t stop_retries {};
    uint32_t sbc_opcode {};
    uint32_t sbc_arg {};
    uint32_t sbc_flags {};
    uint32_t sbc_retries {};
    uint32_t blocks {};
    uint32_t blk_addr {};
    uint32_t blksz {};
    uint32_t data_flags {};
    int32_t tag {};
    uint32_t can_retune {};
    uint32_t doing_retune {};
    uint32_t retune_now {};
    int32_t need_retune {};
    int32_t hold_retune {};
    uint32_t retune_period {};
    uint64_t mrq {};
    std::string name {};
};
__attribute__((noinline)) void ParseMmcRequestStartData(MmcRequestStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.cmd_opcode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cmd_arg = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cmd_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cmd_retries = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.stop_opcode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.stop_arg = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.stop_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.stop_retries = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sbc_opcode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sbc_arg = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sbc_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sbc_retries = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.blocks = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.blk_addr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.blksz = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.data_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.tag = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.can_retune = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.doing_retune = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.retune_now = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.need_retune = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.hold_retune = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.retune_period = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mrq = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmcRequestStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmcRequestStartData eventData {};
    ParseMmcRequestStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mmc_request_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_cmd_opcode(eventData.cmd_opcode);
    msg->set_cmd_arg(eventData.cmd_arg);
    msg->set_cmd_flags(eventData.cmd_flags);
    msg->set_cmd_retries(eventData.cmd_retries);
    msg->set_stop_opcode(eventData.stop_opcode);
    msg->set_stop_arg(eventData.stop_arg);
    msg->set_stop_flags(eventData.stop_flags);
    msg->set_stop_retries(eventData.stop_retries);
    msg->set_sbc_opcode(eventData.sbc_opcode);
    msg->set_sbc_arg(eventData.sbc_arg);
    msg->set_sbc_flags(eventData.sbc_flags);
    msg->set_sbc_retries(eventData.sbc_retries);
    msg->set_blocks(eventData.blocks);
    msg->set_blk_addr(eventData.blk_addr);
    msg->set_blksz(eventData.blksz);
    msg->set_data_flags(eventData.data_flags);
    msg->set_tag(eventData.tag);
    msg->set_can_retune(eventData.can_retune);
    msg->set_doing_retune(eventData.doing_retune);
    msg->set_retune_now(eventData.retune_now);
    msg->set_need_retune(eventData.need_retune);
    msg->set_hold_retune(eventData.hold_retune);
    msg->set_retune_period(eventData.retune_period);
    msg->set_mrq(eventData.mrq);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mmc_request_start, ParseMmcRequestStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mmc_request_start, ParseMmcRequestStart<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
