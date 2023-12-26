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
    cpuhp_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cpuhp_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cpuhp_enter_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.fun()) > 0) {
            functionStr = kernelSymbols[msg.fun()];
        }
        if (functionStr != "") {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cpuhp_enter: cpu: %04u target: %3d step: %3d (%s)",
                msg.cpu(), msg.target(), msg.idx(), functionStr.c_str());
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cpuhp_enter: cpu: %04u target: %3d step: %3d (%p)",
                msg.cpu(), msg.target(), msg.idx(), msg.fun());
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cpuhp_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cpuhp_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cpuhp_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cpuhp_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "cpuhp_exit:  cpu: %04u  state: %3d step: %3d ret: %d", msg.cpu(), msg.state(), msg.idx(), msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cpuhp_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cpuhp_multi_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cpuhp_multi_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cpuhp_multi_enter_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.fun()) > 0) {
            functionStr = kernelSymbols[msg.fun()];
        }
        if (functionStr != "") {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "cpuhp_multi_enter: cpu: %04u target: %3d step: %3d (%s)", msg.cpu(), msg.target(), msg.idx(),
                functionStr.c_str());
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "cpuhp_multi_enter: cpu: %04u target: %3d step: %3d (%p)", msg.cpu(), msg.target(), msg.idx(),
                msg.fun());
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cpuhp_multi_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
