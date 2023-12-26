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
    block_bio_backmerge,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_bio_backmerge_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_bio_backmerge_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_bio_backmerge: %d,%d %s %" PRIu64 " + %u [%s]",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.rwbs().c_str(), msg.sector(), msg.nr_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(block_bio_backmerge) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_bio_bounce,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_bio_bounce_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_bio_bounce_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_bio_bounce: %d,%d %s %" PRIu64 " + %u [%s]",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.rwbs().c_str(),
            msg.sector(), msg.nr_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_bio_bounce) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_bio_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_bio_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_bio_complete_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_bio_complete: %d,%d %s %" PRIu64 " + %u [%d]",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.rwbs().c_str(),
            msg.sector(), msg.nr_sector(), msg.error());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(block_bio_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_bio_frontmerge,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_bio_frontmerge_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_bio_frontmerge_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_bio_frontmerge: %d,%d %s %" PRIu64 " + %u [%s]",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.rwbs().c_str(), msg.sector(), msg.nr_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(block_bio_frontmerge) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_bio_queue,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_bio_queue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_bio_queue_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_bio_queue: %d,%d %s %" PRIu64 " + %u [%s]",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.rwbs().c_str(),
            msg.sector(), msg.nr_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_bio_queue) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_bio_remap,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_bio_remap_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_bio_remap_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "block_bio_remap: %d,%d %s %" PRIu64 " + %u <- (%d,%d) %" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.rwbs().c_str(), msg.sector(), msg.nr_sector(),
            ((unsigned int)((msg.old_dev()) >> 20)), ((unsigned int)((msg.old_dev()) & ((1U << 20) - 1))),
            msg.old_sector());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_bio_remap) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_dirty_buffer,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_dirty_buffer_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_dirty_buffer_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "block_dirty_buffer: %d,%d sector=%" PRIu64 " size=%" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.sector(), msg.size());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(block_dirty_buffer) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_getrq,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_getrq_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_getrq_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_getrq: %d,%d %s %" PRIu64 " + %u [%s]",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.rwbs().c_str(),
            msg.sector(), msg.nr_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_getrq) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_plug,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_plug_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_plug_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_plug: [%s]", msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_plug) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_rq_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_rq_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_rq_complete_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_rq_complete: %d,%d %s (%s) %" PRIu64 " + %u [%d]",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.rwbs().c_str(), msg.cmd().c_str(), msg.sector(), msg.nr_sector(), msg.error());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_rq_complete) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_rq_insert,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_rq_insert_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_rq_insert_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_rq_insert: %d,%d %s %u (%s) %" PRIu64 " + %u [%s]",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.rwbs().c_str(), msg.bytes(), msg.cmd().c_str(), msg.sector(), msg.nr_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_rq_insert) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_rq_issue,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_rq_issue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_rq_issue_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_rq_issue: %d,%d %s %u (%s) %" PRIu64 " + %u [%s]",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.rwbs().c_str(), msg.bytes(), msg.cmd().c_str(), msg.sector(), msg.nr_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_rq_issue) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_rq_remap,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_rq_remap_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_rq_remap_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "block_rq_remap: %d,%d %s %" PRIu64 " + %u <- (%d,%d) %" PRIu64 " %u", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.rwbs().c_str(), msg.sector(), msg.nr_sector(),
            ((unsigned int)((msg.old_dev()) >> 20)), ((unsigned int)((msg.old_dev()) & ((1U << 20) - 1))),
            msg.old_sector(), msg.nr_bios());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_rq_remap) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_rq_requeue,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_rq_requeue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_rq_requeue_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_rq_requeue: %d,%d %s (%s) %" PRIu64 " + %u [%d]",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.rwbs().c_str(), msg.cmd().c_str(), msg.sector(), msg.nr_sector(), 0);
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_rq_requeue) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_sleeprq,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_sleeprq_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_sleeprq_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_sleeprq: %d,%d %s %" PRIu64 " + %u [%s]",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.rwbs().c_str(),
            msg.sector(), msg.nr_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_sleeprq) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_split,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_split_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_split_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_split: %d,%d %s %" PRIu64 " / %" PRIu64 " [%s]",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
                msg.rwbs().c_str(), msg.sector(), msg.new_sector(), msg.comm().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_split) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_touch_buffer,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_touch_buffer_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_touch_buffer_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "block_touch_buffer: %d,%d sector=%" PRIu64 " size=%" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.sector(), msg.size());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(block_touch_buffer) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    block_unplug,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_block_unplug_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.block_unplug_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "block_unplug: [%s] %d", msg.comm().c_str(), msg.nr_rq());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(block_unplug) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
