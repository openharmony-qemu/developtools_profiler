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
#include "hrtimer.h"
#include "logging.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    hrtimer_cancel,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hrtimer_cancel_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hrtimer_cancel_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "hrtimer_cancel: hrtimer=%p", msg.hrtimer());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(hrtimer_cancel) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    hrtimer_expire_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hrtimer_expire_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hrtimer_expire_entry_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.function()) > 0) {
            functionStr = kernelSymbols[msg.function()];
        }
        if (functionStr != "") {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "hrtimer_expire_entry: hrtimer=%p function=%s now=%" PRIu64 "", msg.hrtimer(), functionStr.c_str(),
                msg.now());
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "hrtimer_expire_entry: hrtimer=%p function=%p now=%" PRIu64 "", msg.hrtimer(), msg.function(),
                msg.now());
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(hrtimer_expire_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    hrtimer_expire_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hrtimer_expire_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hrtimer_expire_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "hrtimer_expire_exit: hrtimer=%p", msg.hrtimer());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(hrtimer_expire_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    hrtimer_init,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hrtimer_init_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hrtimer_init_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "hrtimer_init: hrtimer=%p clockid=%s mode=%s",
            msg.hrtimer(),
            __print_symbolic(
                msg.clockid(), {0, "CLOCK_REALTIME"}, {1, "CLOCK_MONOTONIC"}, {7, "CLOCK_BOOTTIME"}, {11, "CLOCK_TAI"}),
            __print_symbolic(msg.mode(), {HRTIMER_MODE_ABS, "ABS"}, {HRTIMER_MODE_REL, "REL"},
                {HRTIMER_MODE_ABS_PINNED, "ABS|PINNED"}, {HRTIMER_MODE_REL_PINNED, "REL|PINNED"},
                {HRTIMER_MODE_ABS_SOFT, "ABS|SOFT"}, {HRTIMER_MODE_REL_SOFT, "REL|SOFT"},
                {HRTIMER_MODE_ABS_PINNED_SOFT, "ABS|PINNED|SOFT"}, {HRTIMER_MODE_REL_PINNED_SOFT, "REL|PINNED|SOFT"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(hrtimer_init) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    hrtimer_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_hrtimer_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.hrtimer_start_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.function()) > 0) {
            functionStr = kernelSymbols[msg.function()];
        }
        if (functionStr != "") {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "hrtimer_start: hrtimer=%p function=%s expires=%" PRIu64 " softexpires=%" PRIu64 " mode=%s",
                msg.hrtimer(), functionStr.c_str(), msg.expires(), msg.softexpires(),
                __print_symbolic(msg.mode(), {HRTIMER_MODE_ABS, "ABS"}, {HRTIMER_MODE_REL, "REL"},
                    {HRTIMER_MODE_ABS_PINNED, "ABS|PINNED"}, {HRTIMER_MODE_REL_PINNED, "REL|PINNED"},
                    {HRTIMER_MODE_ABS_SOFT, "ABS|SOFT"}, {HRTIMER_MODE_REL_SOFT, "REL|SOFT"},
                    {HRTIMER_MODE_ABS_PINNED_SOFT, "ABS|PINNED|SOFT"},
                    {HRTIMER_MODE_REL_PINNED_SOFT, "REL|PINNED|SOFT"}));
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "hrtimer_start: hrtimer=%p function=%p expires=%" PRIu64 " softexpires=%" PRIu64 " mode=%s",
                msg.hrtimer(), msg.function(), msg.expires(), msg.softexpires(),
                __print_symbolic(msg.mode(), {HRTIMER_MODE_ABS, "ABS"}, {HRTIMER_MODE_REL, "REL"},
                    {HRTIMER_MODE_ABS_PINNED, "ABS|PINNED"}, {HRTIMER_MODE_REL_PINNED, "REL|PINNED"},
                    {HRTIMER_MODE_ABS_SOFT, "ABS|SOFT"}, {HRTIMER_MODE_REL_SOFT, "REL|SOFT"},
                    {HRTIMER_MODE_ABS_PINNED_SOFT, "ABS|PINNED|SOFT"},
                    {HRTIMER_MODE_REL_PINNED_SOFT, "REL|PINNED|SOFT"}));
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(hrtimer_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    itimer_expire,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_itimer_expire_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.itimer_expire_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "itimer_expire: which=%d pid=%d now=%" PRIu64 "",
            msg.which(), (int)msg.pid(), msg.now());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(itimer_expire) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    itimer_state,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_itimer_state_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.itimer_state_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "itimer_state: which=%d expires=%" PRIu64 " it_value=%" PRIu64 "it_interval=%" PRIu64 "", msg.which(),
            msg.expires(), msg.value_sec(), msg.interval_sec());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(itimer_state) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    timer_cancel,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_timer_cancel_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.timer_cancel_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "timer_cancel: timer=%p", msg.timer());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(timer_cancel) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    timer_expire_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_timer_expire_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.timer_expire_entry_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.function()) > 0) {
            functionStr = kernelSymbols[msg.function()];
        }
        if (functionStr != "") {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "timer_expire_entry: timer=%p function=%s now=%" PRIu64 "", msg.timer(), functionStr.c_str(),
                msg.now());
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "timer_expire_entry: timer=%p function=%p now=%" PRIu64 "", msg.timer(), msg.function(), msg.now());
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(timer_expire_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    timer_expire_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_timer_expire_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.timer_expire_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "timer_expire_exit: timer=%p", msg.timer());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(timer_expire_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    timer_init,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_timer_init_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.timer_init_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "timer_init: timer=%p", msg.timer());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(timer_init) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    timer_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_timer_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.timer_start_format();
        char buffer[BUFFER_SIZE];
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.function()) > 0) {
            functionStr = kernelSymbols[msg.function()];
        }
        if (functionStr != "") {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "timer_start: timer=%p function=%s expires=%" PRIu64 " [timeout=%" PRIu64 "] cpu=%u idx=%u flags=%s",
                msg.timer(), functionStr.c_str(), msg.expires(), msg.expires() - msg.now(), msg.flags() & 0x0003FFFF,
                msg.flags() >> 22,
                __print_flags(msg.flags() & (0x00040000 | 0x00080000 | 0x00100000 | 0x00200000), "|", {0x00040000, "M"},
                    {0x00080000, "D"}, {0x00100000, "P"}, {0x00200000, "I"}));
        } else {
            len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "timer_start: timer=%p function=%p expires=%" PRIu64 " [timeout=%" PRIu64 "] cpu=%u idx=%u flags=%s",
                msg.timer(), msg.function(), msg.expires(), msg.expires() - msg.now(), msg.flags() & 0x0003FFFF,
                msg.flags() >> 22,
                __print_flags(msg.flags() & (0x00040000 | 0x00080000 | 0x00100000 | 0x00200000), "|", {0x00040000, "M"},
                    {0x00080000, "D"}, {0x00100000, "P"}, {0x00200000, "I"}));
        }
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(timer_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
