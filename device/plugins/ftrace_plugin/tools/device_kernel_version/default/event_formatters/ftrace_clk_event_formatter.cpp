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
    clk_disable,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_disable_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_disable_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_disable: %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clk_disable) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_disable_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_disable_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_disable_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_disable_complete: %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(clk_disable_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_enable,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_enable_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_enable_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_enable: %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clk_enable) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_enable_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_enable_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_enable_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_enable_complete: %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(clk_enable_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_prepare,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_prepare_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_prepare_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_prepare: %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clk_prepare) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_prepare_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_prepare_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_prepare_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_prepare_complete: %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(clk_prepare_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_set_parent,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_set_parent_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_set_parent_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_set_parent: %s %s", msg.name().c_str(), msg.pname().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clk_set_parent) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_set_parent_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_set_parent_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_set_parent_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_set_parent_complete: %s %s", msg.name().c_str(),
            msg.pname().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(clk_set_parent_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_set_phase,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_set_phase_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_set_phase_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_set_phase: %s %d", msg.name().c_str(), (int)msg.phase());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clk_set_phase) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_set_phase_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_set_phase_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_set_phase_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_set_phase_complete: %s %d", msg.name().c_str(),
            (int)msg.phase());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(clk_set_phase_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_set_rate,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_set_rate_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_set_rate_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_set_rate: %s %lu", msg.name().c_str(),
            (unsigned long)msg.rate());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clk_set_rate) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_set_rate_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_set_rate_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_set_rate_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_set_rate_complete: %s %lu", msg.name().c_str(),
            (unsigned long)msg.rate());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(clk_set_rate_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_unprepare,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_unprepare_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_unprepare_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_unprepare: %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clk_unprepare) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clk_unprepare_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clk_unprepare_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clk_unprepare_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clk_unprepare_complete: %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(clk_unprepare_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
