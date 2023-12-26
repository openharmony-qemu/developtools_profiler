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
#include <cinttypes>

#include "event_formatter.h"
#include "logging.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    mmc_request_done,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mmc_request_done_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mmc_request_done_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mmc_request_done: %s: end struct mmc_request[%p]: cmd_opcode=%u cmd_err=%d cmd_resp=0x%x 0x%x 0x%x 0x%x "
            "cmd_retries=%u stop_opcode=%u stop_err=%d stop_resp=0x%x 0x%x 0x%x 0x%x stop_retries=%u sbc_opcode=%u "
            "sbc_err=%d sbc_resp=0x%x 0x%x 0x%x 0x%x sbc_retries=%u bytes_xfered=%u data_err=%d tag=%d can_retune=%u "
            "doing_retune=%u retune_now=%u need_retune=%d hold_retune=%d retune_period=%u",
            msg.name().c_str(), msg.mrq(), msg.cmd_opcode(), msg.cmd_err(), msg.cmd_resp()[0], msg.cmd_resp()[1],
            msg.cmd_resp()[2], msg.cmd_resp()[3], msg.cmd_retries(), msg.stop_opcode(), msg.stop_err(),
            msg.stop_resp()[0], msg.stop_resp()[1], msg.stop_resp()[2], msg.stop_resp()[3], msg.stop_retries(),
            msg.sbc_opcode(), msg.sbc_err(), msg.sbc_resp()[0], msg.sbc_resp()[1], msg.sbc_resp()[2], msg.sbc_resp()[3],
            msg.sbc_retries(), msg.bytes_xfered(), msg.data_err(), msg.tag(), msg.can_retune(), msg.doing_retune(),
            msg.retune_now(), msg.need_retune(), msg.hold_retune(), msg.retune_period());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(mmc_request_done) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mmc_request_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mmc_request_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mmc_request_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mmc_request_start: %s: start struct mmc_request[%p]: cmd_opcode=%u cmd_arg=0x%x cmd_flags=0x%x "
            "cmd_retries=%u stop_opcode=%u stop_arg=0x%x stop_flags=0x%x stop_retries=%u sbc_opcode=%u sbc_arg=0x%x "
            "sbc_flags=0x%x sbc_retires=%u blocks=%u block_size=%u blk_addr=%u data_flags=0x%x tag=%d can_retune=%u "
            "doing_retune=%u retune_now=%u need_retune=%d hold_retune=%d retune_period=%u",
            msg.name().c_str(), msg.mrq(), msg.cmd_opcode(), msg.cmd_arg(), msg.cmd_flags(), msg.cmd_retries(),
            msg.stop_opcode(), msg.stop_arg(), msg.stop_flags(), msg.stop_retries(), msg.sbc_opcode(), msg.sbc_arg(),
            msg.sbc_flags(), msg.sbc_retries(), msg.blocks(), msg.blksz(), msg.blk_addr(), msg.data_flags(), msg.tag(),
            msg.can_retune(), msg.doing_retune(), msg.retune_now(), msg.need_retune(), msg.hold_retune(),
            msg.retune_period());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(mmc_request_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
