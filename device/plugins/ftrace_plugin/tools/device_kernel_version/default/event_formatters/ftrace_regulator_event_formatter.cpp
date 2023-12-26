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
    regulator_bypass_disable,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_bypass_disable_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_bypass_disable_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_bypass_disable: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(regulator_bypass_disable) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_bypass_disable_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_bypass_disable_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_bypass_disable_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_bypass_disable_complete: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(regulator_bypass_disable_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_bypass_enable,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_bypass_enable_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_bypass_enable_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_bypass_enable: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(regulator_bypass_enable) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_bypass_enable_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_bypass_enable_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_bypass_enable_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_bypass_enable_complete: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(regulator_bypass_enable_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_disable,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_disable_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_disable_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_disable: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(regulator_disable) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_disable_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_disable_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_disable_complete_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_disable_complete: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(regulator_disable_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_enable,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_enable_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_enable_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_enable: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(regulator_enable) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_enable_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_enable_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_enable_complete_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_enable_complete: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(regulator_enable_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_enable_delay,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_enable_delay_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_enable_delay_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_enable_delay: name=%s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(regulator_enable_delay) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_set_voltage,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_set_voltage_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_set_voltage_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_set_voltage: name=%s (%d-%d)",
            msg.name().c_str(), (int)msg.min(), (int)msg.max());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(regulator_set_voltage) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    regulator_set_voltage_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_regulator_set_voltage_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.regulator_set_voltage_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "regulator_set_voltage_complete: name=%s, val=%u",
            msg.name().c_str(), (int)msg.val());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(regulator_set_voltage_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
