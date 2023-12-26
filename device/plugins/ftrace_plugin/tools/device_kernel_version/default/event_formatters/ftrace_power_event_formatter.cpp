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
#include "pm_qos.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    clock_disable,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clock_disable_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clock_disable_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clock_disable: %s state=%lu cpu_id=%lu",
            msg.name().c_str(), (unsigned long)msg.state(), (unsigned long)msg.cpu_id());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clock_disable) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clock_enable,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clock_enable_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clock_enable_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clock_enable: %s state=%lu cpu_id=%lu",
            msg.name().c_str(), (unsigned long)msg.state(), (unsigned long)msg.cpu_id());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clock_enable) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    clock_set_rate,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_clock_set_rate_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.clock_set_rate_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "clock_set_rate: %s state=%lu cpu_id=%lu",
            msg.name().c_str(), (unsigned long)msg.state(), (unsigned long)msg.cpu_id());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(clock_set_rate) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cpu_frequency,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cpu_frequency_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cpu_frequency_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cpu_frequency: state=%lu cpu_id=%lu",
            (unsigned long)msg.state(), (unsigned long)msg.cpu_id());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cpu_frequency) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cpu_frequency_limits,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cpu_frequency_limits_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cpu_frequency_limits_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cpu_frequency_limits: min=%lu max=%lu cpu_id=%lu",
            (unsigned long)msg.min_freq(), (unsigned long)msg.max_freq(), (unsigned long)msg.cpu_id());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(cpu_frequency_limits) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cpu_idle,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cpu_idle_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cpu_idle_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cpu_idle: state=%lu cpu_id=%lu",
            (unsigned long)msg.state(), (unsigned long)msg.cpu_id());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cpu_idle) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dev_pm_qos_add_request,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dev_pm_qos_add_request_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dev_pm_qos_add_request_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dev_pm_qos_add_request: device=%s type=%s new_value=%d", msg.name().c_str(),
            __print_symbolic(msg.type(), {DEV_PM_QOS_RESUME_LATENCY, "DEV_PM_QOS_RESUME_LATENCY"},
                {DEV_PM_QOS_FLAGS, "DEV_PM_QOS_FLAGS"}),
            msg.new_value());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(dev_pm_qos_add_request) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dev_pm_qos_remove_request,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dev_pm_qos_remove_request_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dev_pm_qos_remove_request_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dev_pm_qos_remove_request: device=%s type=%s new_value=%d", msg.name().c_str(),
            __print_symbolic(msg.type(), {DEV_PM_QOS_RESUME_LATENCY, "DEV_PM_QOS_RESUME_LATENCY"},
                {DEV_PM_QOS_FLAGS, "DEV_PM_QOS_FLAGS"}),
            msg.new_value());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(dev_pm_qos_remove_request) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dev_pm_qos_update_request,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dev_pm_qos_update_request_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dev_pm_qos_update_request_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dev_pm_qos_update_request: device=%s type=%s new_value=%d", msg.name().c_str(),
            __print_symbolic(msg.type(), {DEV_PM_QOS_RESUME_LATENCY, "DEV_PM_QOS_RESUME_LATENCY"},
                {DEV_PM_QOS_FLAGS, "DEV_PM_QOS_FLAGS"}),
            msg.new_value());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(dev_pm_qos_update_request) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    device_pm_callback_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_device_pm_callback_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.device_pm_callback_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "device_pm_callback_end: %s %s, err=%d",
            msg.driver().c_str(), msg.device().c_str(), msg.error());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(device_pm_callback_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    device_pm_callback_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_device_pm_callback_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.device_pm_callback_start_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "device_pm_callback_start: %s %s, parent: %s, %s[%s]",
                msg.driver().c_str(), msg.device().c_str(), msg.parent().c_str(), msg.pm_ops().c_str(),
                __print_symbolic(msg.event(), {0x0002, "suspend"}, {0x0010, "resume"}, {0x0001, "freeze"},
                    {0x0008, "quiesce"}, {0x0004, "hibernate"}, {0x0020, "thaw"}, {0x0040, "restore"},
                    {0x0080, "recover"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(device_pm_callback_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    pm_qos_add_request,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_pm_qos_add_request_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.pm_qos_add_request_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "pm_qos_add_request: CPU_DMA_LATENCY value=%d", msg.value());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(pm_qos_add_request) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    pm_qos_remove_request,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_pm_qos_remove_request_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.pm_qos_remove_request_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "pm_qos_remove_request: CPU_DMA_LATENCY value=%d", msg.value());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(pm_qos_remove_request) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    pm_qos_update_flags,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_pm_qos_update_flags_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.pm_qos_update_flags_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "pm_qos_update_flags: action=%s prev_value=0x%x curr_value=0x%x",
            __print_symbolic(msg.action(), {PM_QOS_ADD_REQ, "ADD_REQ"}, {PM_QOS_UPDATE_REQ, "UPDATE_REQ"},
                {PM_QOS_REMOVE_REQ, "REMOVE_REQ"}),
            msg.prev_value(), msg.curr_value());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(pm_qos_update_flags) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    pm_qos_update_request,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_pm_qos_update_request_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.pm_qos_update_request_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "pm_qos_update_request: CPU_DMA_LATENCY value=%d", msg.value());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(pm_qos_update_request) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    pm_qos_update_target,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_pm_qos_update_target_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.pm_qos_update_target_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "pm_qos_update_target: action=%s prev_value=%d curr_value=%d",
            __print_symbolic(msg.action(), {PM_QOS_ADD_REQ, "ADD_REQ"}, {PM_QOS_UPDATE_REQ, "UPDATE_REQ"},
                {PM_QOS_REMOVE_REQ, "REMOVE_REQ"}),
            msg.prev_value(), msg.curr_value());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(pm_qos_update_target) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    power_domain_target,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_power_domain_target_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.power_domain_target_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "power_domain_target: %s state=%lu cpu_id=%lu",
            msg.name().c_str(), (unsigned long)msg.state(), (unsigned long)msg.cpu_id());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(power_domain_target) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    pstate_sample,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_pstate_sample_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.pstate_sample_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "pstate_sample: core_busy=%lu scaled=%lu from=%lu to=%lu mperf=%" PRIu64 " aperf=%" PRIu64 " tsc=%" PRIu64
            " freq=%lu io_boost=%lu",
            (unsigned long)msg.core_busy(), (unsigned long)msg.scaled_busy(), (unsigned long)msg.from(),
            (unsigned long)msg.to(), msg.mperf(), msg.aperf(), msg.tsc(), (unsigned long)msg.freq(),
            (unsigned long)msg.io_boost());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(pstate_sample) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    suspend_resume,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_suspend_resume_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.suspend_resume_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "suspend_resume: %s[%u] %s", msg.action().c_str(),
            (unsigned int)msg.val(), (msg.start()) ? "begin" : "end");
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(suspend_resume) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    wakeup_source_activate,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_wakeup_source_activate_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.wakeup_source_activate_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "wakeup_source_activate: %s state=0x%" PRIx64 "",
            msg.name().c_str(), msg.state());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(wakeup_source_activate) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    wakeup_source_deactivate,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_wakeup_source_deactivate_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.wakeup_source_deactivate_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "wakeup_source_deactivate: %s state=0x%" PRIx64 "",
            msg.name().c_str(), msg.state());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(wakeup_source_deactivate) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
