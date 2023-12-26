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
    irq_handler_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_irq_handler_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.irq_handler_entry_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "irq_handler_entry: irq=%d name=%s", msg.irq(), msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(irq_handler_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    irq_handler_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_irq_handler_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.irq_handler_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "irq_handler_exit: irq=%d ret=%s", msg.irq(),
            msg.ret() ? "handled" : "unhandled");
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(irq_handler_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    softirq_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_softirq_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.softirq_entry_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "softirq_entry: vec=%u [action=%s]", msg.vec(),
            __print_symbolic(msg.vec(), {0, "HI"}, {1, "TIMER"}, {2, "NET_TX"}, {3, "NET_RX"}, {4, "BLOCK"},
                {5, "IRQ_POLL"}, {6, "TASKLET"}, {7, "SCHED"}, {8, "HRTIMER"}, {9, "RCU"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(softirq_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    softirq_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_softirq_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.softirq_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "softirq_exit: vec=%u [action=%s]", msg.vec(),
            __print_symbolic(msg.vec(), {0, "HI"}, {1, "TIMER"}, {2, "NET_TX"}, {3, "NET_RX"}, {4, "BLOCK"},
                {5, "IRQ_POLL"}, {6, "TASKLET"}, {7, "SCHED"}, {8, "HRTIMER"}, {9, "RCU"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(softirq_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    softirq_raise,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_softirq_raise_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.softirq_raise_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "softirq_raise: vec=%u [action=%s]", msg.vec(),
            __print_symbolic(msg.vec(), {0, "HI"}, {1, "TIMER"}, {2, "NET_TX"}, {3, "NET_RX"}, {4, "BLOCK"},
                {5, "IRQ_POLL"}, {6, "TASKLET"}, {7, "SCHED"}, {8, "HRTIMER"}, {9, "RCU"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(softirq_raise) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
