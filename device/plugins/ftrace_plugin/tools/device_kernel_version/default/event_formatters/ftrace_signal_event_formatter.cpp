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
    signal_deliver,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_signal_deliver_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.signal_deliver_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "signal_deliver: sig=%d errno=%d code=%d sa_handler=%" PRIx64 " sa_flags=%" PRIx64 "", msg.sig(),
            msg.error_code(), msg.code(), msg.sig_handler(), msg.sig_flags());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(signal_deliver) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    signal_generate,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_signal_generate_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.signal_generate_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "signal_generate: sig=%d errno=%d code=%d comm=%s pid=%d grp=%d res=%d", msg.sig(), msg.error_code(),
            msg.code(), msg.comm().c_str(), msg.pid(), msg.group(), msg.result());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(signal_generate) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
