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
    i2c_read,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_i2c_read_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.i2c_read_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "i2c_read: i2c-%d #%u a=%03x f=%04x l=%u",
            msg.adapter_nr(), msg.msg_nr(), msg.addr(), msg.flags(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(i2c_read) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    i2c_reply,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_i2c_reply_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.i2c_reply_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "i2c_reply: i2c-%d #%u a=%03x f=%04x l=%" PRId32 " [%" PRId32 "]", msg.adapter_nr(), msg.msg_nr(),
            msg.addr(), msg.flags(), msg.len(), msg.buf());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(i2c_reply) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    i2c_result,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_i2c_result_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.i2c_result_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "i2c_result: i2c-%d n=%u ret=%d", msg.adapter_nr(),
            msg.nr_msgs(), msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(i2c_result) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    i2c_write,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_i2c_write_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.i2c_write_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "i2c_write: i2c-%d #%u a=%03x f=%04x l=%" PRId32 " [%" PRId32 "]", msg.adapter_nr(), msg.msg_nr(),
            msg.addr(), msg.flags(), msg.len(), msg.buf());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(i2c_write) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
