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
    cgroup_attach_task,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_attach_task_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_attach_task_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "cgroup_attach_task: dst_root=%d dst_id=%d dst_level=%d dst_path=%s pid=%d comm=%s", msg.dst_root(),
            msg.dst_id(), msg.dst_level(), msg.dst_path().c_str(), msg.pid(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(cgroup_attach_task) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_destroy_root,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_destroy_root_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_destroy_root_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_destroy_root: root=%d ss_mask=%#x name=%s",
            msg.root(), msg.ss_mask(), msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(cgroup_destroy_root) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_freeze,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_freeze_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_freeze_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_freeze: root=%d id=%d level=%d path=%s",
            msg.root(), msg.id(), msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cgroup_freeze) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_mkdir,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_mkdir_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_mkdir_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_mkdir: root=%d id=%d level=%d path=%s",
            msg.root(), msg.id(), msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cgroup_mkdir) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_notify_frozen,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_notify_frozen_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_notify_frozen_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "cgroup_notify_frozen: root=%d id=%d level=%d path=%s val=%d", msg.root(), msg.id(), msg.level(),
            msg.path().c_str(), msg.val());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(cgroup_notify_frozen) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_notify_populated,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_notify_populated_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_notify_populated_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "cgroup_notify_populated: root=%d id=%d level=%d path=%s val=%d", msg.root(), msg.id(), msg.level(),
            msg.path().c_str(), msg.val());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(cgroup_notify_populated) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_release,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_release_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_release_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_release: root=%d id=%d level=%d path=%s",
            msg.root(), msg.id(), msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cgroup_release) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_remount,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_remount_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_remount_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_remount: root=%d ss_mask=%#x name=%s",
            msg.root(), msg.ss_mask(), msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cgroup_remount) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_rename,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_rename_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_rename_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_rename: root=%d id=%d level=%d path=%s",
            msg.root(), msg.id(), msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cgroup_rename) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_rmdir,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_rmdir_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_rmdir_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_rmdir: root=%d id=%d level=%d path=%s",
            msg.root(), msg.id(), msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cgroup_rmdir) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_setup_root,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_setup_root_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_setup_root_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_setup_root: root=%d ss_mask=%#x name=%s",
            msg.root(), msg.ss_mask(), msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cgroup_setup_root) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_transfer_tasks,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_transfer_tasks_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_transfer_tasks_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "cgroup_transfer_tasks: dst_root=%d dst_id=%d dst_level=%d dst_path=%s pid=%d comm=%s", msg.dst_root(),
            msg.dst_id(), msg.dst_level(), msg.dst_path().c_str(), msg.pid(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(cgroup_transfer_tasks) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    cgroup_unfreeze,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_cgroup_unfreeze_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.cgroup_unfreeze_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "cgroup_unfreeze: root=%d id=%d level=%d path=%s",
            msg.root(), msg.id(), msg.level(), msg.path().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(cgroup_unfreeze) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
