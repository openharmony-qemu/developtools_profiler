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
    mmc_request_done,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mmc_request_done_format();
        msg->set_cmd_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_cmd_err(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_cmd_resp(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_cmd_retries(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_stop_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_stop_err(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_stop_resp(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_stop_retries(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sbc_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sbc_err(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_sbc_resp(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_sbc_retries(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_bytes_xfered(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_data_err(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_tag(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_can_retune(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_doing_retune(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_retune_now(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_need_retune(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_hold_retune(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_retune_period(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mrq(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    mmc_request_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_mmc_request_start_format();
        msg->set_cmd_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_cmd_arg(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_cmd_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_cmd_retries(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_stop_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_stop_arg(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_stop_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_stop_retries(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sbc_opcode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sbc_arg(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sbc_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sbc_retries(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_blocks(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_blk_addr(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_blksz(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_data_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_tag(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_can_retune(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_doing_retune(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_retune_now(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_need_retune(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_hold_retune(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_retune_period(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mrq(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END

