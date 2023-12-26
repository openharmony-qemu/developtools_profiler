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
#include "pq.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    balance_dirty_pages,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_balance_dirty_pages_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.balance_dirty_pages_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "balance_dirty_pages: bdi %s: limit=%" PRIu64 " setpoint=%" PRIu64 " dirty=%" PRIu64
            " bdi_setpoint=%" PRIu64 " bdi_dirty=%" PRIu64 " dirty_ratelimit=%" PRIu64 " task_ratelimit=%" PRIu64
            " dirtied=%u dirtied_pause=%u paused=%" PRIu64 " pause=%" PRIu64 " period=%" PRIu64 " think=%" PRIu64
            " cgroup_ino=%" PRId32 "",
            msg.bdi().c_str(), msg.limit(), msg.setpoint(), msg.dirty(), msg.bdi_setpoint(), msg.bdi_dirty(),
            msg.dirty_ratelimit(), msg.task_ratelimit(), msg.dirtied(), msg.dirtied_pause(), msg.paused(), msg.pause(),
            msg.period(), msg.think(), msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(balance_dirty_pages) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    bdi_dirty_ratelimit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_bdi_dirty_ratelimit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.bdi_dirty_ratelimit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "bdi_dirty_ratelimit: bdi %s: write_bw=%" PRIu64 " awrite_bw=%" PRIu64 " dirty_rate=%" PRIu64
            " dirty_ratelimit=%" PRIu64 " task_ratelimit=%" PRIu64 " balanced_dirty_ratelimit=%" PRIu64
            " cgroup_ino=%" PRId32 "",
            msg.bdi().c_str(), msg.write_bw(), msg.avg_write_bw(), msg.dirty_rate(), msg.dirty_ratelimit(),
            msg.task_ratelimit(), msg.balanced_dirty_ratelimit(), msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(bdi_dirty_ratelimit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    global_dirty_state,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_global_dirty_state_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.global_dirty_state_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "global_dirty_state: dirty=%" PRIu64 " writeback=%" PRIu64 " bg_thresh=%" PRIu64 " thresh=%" PRIu64
            " limit=%" PRIu64 " dirtied=%" PRIu64 " written=%" PRIu64 "",
            msg.nr_dirty(), msg.nr_writeback(), msg.background_thresh(), msg.dirty_thresh(), msg.dirty_limit(),
            msg.nr_dirtied(), msg.nr_written());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(global_dirty_state) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    wbc_writepage,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_wbc_writepage_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.wbc_writepage_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "wbc_writepage: bdi %s: towrt=%" PRIu64 " skip=%" PRIu64
            " mode=%d kupd=%d bgrd=%d reclm=%d cyclic=%d start=0x%" PRIx64 " end=0x%" PRIx64 " cgroup_ino=%" PRId32 "",
            msg.name().c_str(), msg.nr_to_write(), msg.pages_skipped(), msg.sync_mode(), msg.for_kupdate(),
            msg.for_background(), msg.for_reclaim(), msg.range_cyclic(), msg.range_start(), msg.range_end(),
            msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(wbc_writepage) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_bdi_register,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_bdi_register_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_bdi_register_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "writeback_bdi_register: bdi %s", msg.name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_bdi_register) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_congestion_wait,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_congestion_wait_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_congestion_wait_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_congestion_wait: usec_timeout=%u usec_delayed=%u", msg.usec_timeout(), msg.usec_delayed());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_congestion_wait) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_dirty_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_dirty_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_dirty_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_dirty_inode: bdi %s: ino=%" PRIu64 " state=%s flags=%s", msg.name().c_str(), msg.ino(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            __print_flags(msg.flags(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_dirty_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_dirty_inode_enqueue,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_dirty_inode_enqueue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_dirty_inode_enqueue_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_dirty_inode_enqueue: dev %d,%d ino %" PRIu64 " dirtied %" PRIu64 " state %s mode 0%o",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.dirtied_when(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            msg.mode());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_dirty_inode_enqueue) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_dirty_inode_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_dirty_inode_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_dirty_inode_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_dirty_inode_start: bdi %s: ino=%" PRIu64 " state=%s flags=%s", msg.name().c_str(), msg.ino(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            __print_flags(msg.flags(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_dirty_inode_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_dirty_page,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_dirty_page_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_dirty_page_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_dirty_page: bdi %s: ino=%" PRIu64 " index=%" PRIu64 "", msg.name().c_str(), msg.ino(),
            msg.index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_dirty_page) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_exec,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_exec_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_exec_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_exec: bdi %s: sb_dev %d:%d nr_pages=%" PRIu64
            " sync_mode=%d kupdate=%d range_cyclic=%d background=%d reason=%s cgroup_ino=%" PRId32 "",
            msg.name().c_str(), ((unsigned int)((msg.sb_dev()) >> 20)),
            ((unsigned int)((msg.sb_dev()) & ((1U << 20) - 1))), msg.nr_pages(), msg.sync_mode(), msg.for_kupdate(),
            msg.range_cyclic(), msg.for_background(),
            __print_symbolic(msg.reason(), {0, "background"}, {1, "vmscan"}, {2, "sync"}, {3, "periodic"},
                {4, "laptop_timer"}, {5, "fs_free_space"}, {6, "forker_thread"}),
            msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(writeback_exec) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_lazytime,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_lazytime_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_lazytime_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_lazytime: dev %d,%d ino %" PRIu64 " dirtied %" PRIu64 " state %s mode 0%o",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.dirtied_when(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            msg.mode());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_lazytime) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_lazytime_iput,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_lazytime_iput_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_lazytime_iput_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_lazytime_iput: dev %d,%d ino %" PRIu64 " dirtied %" PRIu64 " state %s mode 0%o",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.dirtied_when(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            msg.mode());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_lazytime_iput) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_mark_inode_dirty,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_mark_inode_dirty_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_mark_inode_dirty_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_mark_inode_dirty: bdi %s: ino=%" PRIu64 " state=%s flags=%s", msg.name().c_str(), msg.ino(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            __print_flags(msg.flags(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_mark_inode_dirty) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_pages_written,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_pages_written_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_pages_written_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "writeback_pages_written: %" PRIu64 "", msg.pages());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_pages_written) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_queue,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_queue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_queue_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_queue: bdi %s: sb_dev %d:%d nr_pages=%" PRIu64
            " sync_mode=%d kupdate=%d range_cyclic=%d background=%d reason=%s cgroup_ino=%" PRId32 "",
            msg.name().c_str(), ((unsigned int)((msg.sb_dev()) >> 20)),
            ((unsigned int)((msg.sb_dev()) & ((1U << 20) - 1))), msg.nr_pages(), msg.sync_mode(), msg.for_kupdate(),
            msg.range_cyclic(), msg.for_background(),
            __print_symbolic(msg.reason(), {0, "background"}, {1, "vmscan"}, {2, "sync"}, {3, "periodic"},
                {4, "laptop_timer"}, {5, "fs_free_space"}, {6, "forker_thread"}),
            msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(writeback_queue) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_queue_io,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_queue_io_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_queue_io_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_queue_io: bdi %s: older=%" PRIu64 " age=%" PRIu64 " enqueue=%d reason=%s cgroup_ino=%" PRId32 "",
            msg.name().c_str(), msg.older(), msg.age(), msg.moved(),
            __print_symbolic(msg.reason(), {0, "background"}, {1, "vmscan"}, {2, "sync"}, {3, "periodic"},
                {4, "laptop_timer"}, {5, "fs_free_space"}, {6, "forker_thread"}),
            msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_queue_io) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_sb_inodes_requeue,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_sb_inodes_requeue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_sb_inodes_requeue_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_sb_inodes_requeue: bdi %s: ino=%" PRIu64 " state=%s dirtied_when=%" PRIu64 " age=%" PRIu64
            " cgroup_ino=%" PRId32 "",
            msg.name().c_str(), msg.ino(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            msg.dirtied_when(), (jiffies - msg.dirtied_when()) / 300, msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_sb_inodes_requeue) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_single_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_single_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_single_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_single_inode: bdi %s: ino=%" PRIu64 " state=%s dirtied_when=%" PRIu64 " age=%" PRIu64
            " index=%" PRIu64 " to_write=%" PRIu64 " wrote=%" PRIu64 " cgroup_ino=%" PRId32 "",
            msg.name().c_str(), msg.ino(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            msg.dirtied_when(), (jiffies - msg.dirtied_when()) / 300, msg.writeback_index(), msg.nr_to_write(),
            msg.wrote(), msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_single_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_single_inode_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_single_inode_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_single_inode_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_single_inode_start: bdi %s: ino=%" PRIu64 " state=%s dirtied_when=%" PRIu64 " age=%" PRIu64
            " index=%" PRIu64 " to_write=%" PRIu64 " wrote=%" PRIu64 " cgroup_ino=%" PRId32 "",
            msg.name().c_str(), msg.ino(),
            __print_flags(msg.state(), "|", {(1 << 0), "I_DIRTY_SYNC"}, {(1 << 1), "I_DIRTY_DATASYNC"},
                {(1 << 2), "I_DIRTY_PAGES"}, {(1 << 3), "I_NEW"}, {(1 << 4), "I_WILL_FREE"}, {(1 << 5), "I_FREEING"},
                {(1 << 6), "I_CLEAR"}, {(1 << 7), "I_SYNC"}, {(1 << 11), "I_DIRTY_TIME"}, {(1 << 8), "I_REFERENCED"}),
            msg.dirtied_when(), (jiffies - msg.dirtied_when()) / 300, msg.writeback_index(), msg.nr_to_write(),
            msg.wrote(), msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_single_inode_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_start: bdi %s: sb_dev %d:%d nr_pages=%" PRIu64
            " sync_mode=%d kupdate=%d range_cyclic=%d background=%d reason=%s cgroup_ino=%" PRId32 "",
            msg.name().c_str(), ((unsigned int)((msg.sb_dev()) >> 20)),
            ((unsigned int)((msg.sb_dev()) & ((1U << 20) - 1))), msg.nr_pages(), msg.sync_mode(), msg.for_kupdate(),
            msg.range_cyclic(), msg.for_background(),
            __print_symbolic(msg.reason(), {0, "background"}, {1, "vmscan"}, {2, "sync"}, {3, "periodic"},
                {4, "laptop_timer"}, {5, "fs_free_space"}, {6, "forker_thread"}),
            msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(writeback_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_wait,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_wait_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_wait_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_wait: bdi %s: sb_dev %d:%d nr_pages=%" PRIu64
            " sync_mode=%d kupdate=%d range_cyclic=%d background=%d reason=%s cgroup_ino=%" PRId32 "",
            msg.name().c_str(), ((unsigned int)((msg.sb_dev()) >> 20)),
            ((unsigned int)((msg.sb_dev()) & ((1U << 20) - 1))), msg.nr_pages(), msg.sync_mode(), msg.for_kupdate(),
            msg.range_cyclic(), msg.for_background(),
            __print_symbolic(msg.reason(), {0, "background"}, {1, "vmscan"}, {2, "sync"}, {3, "periodic"},
                {4, "laptop_timer"}, {5, "fs_free_space"}, {6, "forker_thread"}),
            msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(writeback_wait) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_wait_iff_congested,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_wait_iff_congested_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_wait_iff_congested_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_wait_iff_congested: usec_timeout=%u usec_delayed=%u", msg.usec_timeout(), msg.usec_delayed());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_wait_iff_congested) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_wake_background,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_wake_background_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_wake_background_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_wake_background: bdi %s: cgroup_ino=%" PRId32 "", msg.name().c_str(), msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_wake_background) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_write_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_write_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_write_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_write_inode: bdi %s: ino=%" PRIu64 " sync_mode=%d cgroup_ino=%" PRId32 "", msg.name().c_str(),
            msg.ino(), msg.sync_mode(), msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(writeback_write_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_write_inode_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_write_inode_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_write_inode_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_write_inode_start: bdi %s: ino=%" PRIu64 " sync_mode=%d cgroup_ino=%" PRId32 "",
            msg.name().c_str(), msg.ino(), msg.sync_mode(), msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(writeback_write_inode_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    writeback_written,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_writeback_written_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.writeback_written_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "writeback_written: bdi %s: sb_dev %d:%d nr_pages=%" PRIu64
            " sync_mode=%d kupdate=%d range_cyclic=%d background=%d reason=%s cgroup_ino=%" PRId32 "",
            msg.name().c_str(), ((unsigned int)((msg.sb_dev()) >> 20)),
            ((unsigned int)((msg.sb_dev()) & ((1U << 20) - 1))), msg.nr_pages(), msg.sync_mode(), msg.for_kupdate(),
            msg.range_cyclic(), msg.for_background(),
            __print_symbolic(msg.reason(), {0, "background"}, {1, "vmscan"}, {2, "sync"}, {3, "periodic"},
                {4, "laptop_timer"}, {5, "fs_free_space"}, {6, "forker_thread"}),
            msg.cgroup_ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(writeback_written) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
