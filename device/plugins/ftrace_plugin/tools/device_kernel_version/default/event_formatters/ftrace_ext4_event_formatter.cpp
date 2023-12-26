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
    ext4_alloc_da_blocks,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_alloc_da_blocks_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_alloc_da_blocks_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_alloc_da_blocks: dev %d,%d ino %" PRIu64 " reserved_data_blocks %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.data_blocks());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_alloc_da_blocks) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_allocate_blocks,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_allocate_blocks_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_allocate_blocks_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_allocate_blocks: dev %d,%d ino %" PRIu64 " flags %s len %u block %" PRIu64 " lblk %u goal %" PRIu64
            " lleft %u lright %u pleft %" PRIu64 " pright %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            __print_flags(msg.flags(), "|", {0x0001, "HINT_MERGE"}, {0x0002, "HINT_RESV"}, {0x0004, "HINT_MDATA"},
                {0x0008, "HINT_FIRST"}, {0x0010, "HINT_BEST"}, {0x0020, "HINT_DATA"}, {0x0040, "HINT_NOPREALLOC"},
                {0x0080, "HINT_GRP_ALLOC"}, {0x0100, "HINT_GOAL_ONLY"}, {0x0200, "HINT_TRY_GOAL"},
                {0x0400, "DELALLOC_RESV"}, {0x0800, "STREAM_ALLOC"}, {0x1000, "USE_ROOT_BLKS"}, {0x2000, "USE_RESV"},
                {0x4000, "STRICT_CHECK"}),
            msg.len(), msg.block(), msg.logical(), msg.goal(), msg.lleft(), msg.lright(), msg.pleft(), msg.pright());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_allocate_blocks) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_allocate_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_allocate_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_allocate_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_allocate_inode: dev %d,%d ino %" PRIu64 " dir %" PRIu64 " mode 0%o",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.dir(),
            msg.mode());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_allocate_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_begin_ordered_truncate,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_begin_ordered_truncate_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_begin_ordered_truncate_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_begin_ordered_truncate: dev %d,%d ino %" PRIu64 " new_size %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.new_size());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_begin_ordered_truncate) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_collapse_range,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_collapse_range_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_collapse_range_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_collapse_range: dev %d,%d ino %" PRIu64 " offset %" PRIu64 " len %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.offset(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_collapse_range) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_da_release_space,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_da_release_space_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_da_release_space_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_da_release_space: dev %d,%d ino %" PRIu64 " mode 0%o i_blocks %" PRIu64
            " freed_blocks %d reserved_data_blocks %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.mode(), msg.i_blocks(), msg.freed_blocks(), msg.reserved_data_blocks());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_da_release_space) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_da_reserve_space,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_da_reserve_space_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_da_reserve_space_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_da_reserve_space: dev %d,%d ino %" PRIu64 " mode 0%o i_blocks %" PRIu64 " reserved_data_blocks %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.mode(), msg.i_blocks(), msg.reserved_data_blocks());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_da_reserve_space) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_da_update_reserve_space,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_da_update_reserve_space_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_da_update_reserve_space_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_da_update_reserve_space: dev %d,%d ino %" PRIu64 " mode 0%o i_blocks %" PRIu64
            " used_blocks %d reserved_data_blocks %d quota_claim %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.mode(), msg.i_blocks(), msg.used_blocks(), msg.reserved_data_blocks(), msg.quota_claim());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_da_update_reserve_space) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_da_write_begin,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_da_write_begin_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_da_write_begin_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_da_write_begin: dev %d,%d ino %" PRIu64 " pos %" PRIu64 " len %u flags %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pos(),
            msg.len(), msg.flags());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_da_write_begin) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_da_write_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_da_write_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_da_write_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_da_write_end: dev %d,%d ino %" PRIu64 " pos %" PRIu64 " len %u copied %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pos(),
            msg.len(), msg.copied());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_da_write_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_da_write_pages,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_da_write_pages_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_da_write_pages_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_da_write_pages: dev %d,%d ino %" PRIu64 " first_page %" PRIu64 " nr_to_write %" PRIu64
            " sync_mode %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.first_page(), msg.nr_to_write(), msg.sync_mode());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_da_write_pages) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_da_write_pages_extent,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_da_write_pages_extent_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_da_write_pages_extent_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_da_write_pages_extent: dev %d,%d ino %" PRIu64 " lblk %" PRIu64 " len %u flags %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.lblk(), msg.len(),
            __print_flags(msg.flags(), "", {((((1UL))) << (5)), "N"}, {((((1UL))) << (4)), "M"},
                {((((1UL))) << (11)), "U"}, {((((1UL))) << (9)), "B"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_da_write_pages_extent) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_direct_io_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_direct_io_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_direct_io_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_direct_IO_enter: dev %d,%d ino %" PRIu64 " pos %" PRIu64 " len %" PRIu64 " rw %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pos(),
            msg.len(), msg.rw());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_direct_IO_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_direct_io_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_direct_io_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_direct_io_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_direct_IO_exit: dev %d,%d ino %" PRIu64 " pos %" PRIu64 " len %" PRIu64 " rw %d ret %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pos(),
            msg.len(), msg.rw(), msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_direct_IO_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_discard_blocks,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_discard_blocks_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_discard_blocks_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_discard_blocks: dev %d,%d blk %" PRIu64 " count %" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.blk(), msg.count());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_discard_blocks) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_discard_preallocations,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_discard_preallocations_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_discard_preallocations_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_discard_preallocations: dev %d,%d ino %" PRIu64 "",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_discard_preallocations) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_drop_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_drop_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_drop_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_drop_inode: dev %d,%d ino %" PRIu64 " drop %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.drop());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_drop_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_cache_extent,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_cache_extent_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_cache_extent_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_cache_extent: dev %d,%d ino %" PRIu64 " es [%u/%u) mapped %" PRIu64 " status %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.lblk(), msg.len(), msg.pblk(),
            __print_flags(
                msg.status(), "", {(1 << 0), "W"}, {(1 << 1), "U"}, {(1 << 2), "D"}, {(1 << 3), "H"}, {(1 << 4), "R"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_es_cache_extent) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_insert_extent,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_insert_extent_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_insert_extent_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_insert_extent: dev %d,%d ino %" PRIu64 " es [%u/%u) mapped %" PRIu64 " status %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.lblk(), msg.len(), msg.pblk(),
            __print_flags(
                msg.status(), "", {(1 << 0), "W"}, {(1 << 1), "U"}, {(1 << 2), "D"}, {(1 << 3), "H"}, {(1 << 4), "R"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_es_insert_extent) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_lookup_extent_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_lookup_extent_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_lookup_extent_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_lookup_extent_enter: dev %d,%d ino %" PRIu64 " lblk %u", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.lblk());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_es_lookup_extent_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_lookup_extent_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_lookup_extent_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_lookup_extent_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_lookup_extent_exit: dev %d,%d ino %" PRIu64 " found %d [%u/%u) %" PRIu64 " %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.found(), msg.lblk(), msg.len(), msg.found() ? msg.pblk() : 0,
            __print_flags(msg.found() ? msg.status() : 0, "", {(1 << 0), "W"}, {(1 << 1), "U"}, {(1 << 2), "D"},
                {(1 << 3), "H"}, {(1 << 4), "R"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_es_lookup_extent_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_remove_extent,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_remove_extent_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_remove_extent_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_remove_extent: dev %d,%d ino %" PRIu64 " es [%" PRIu64 "/%" PRIu64 ")",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.lblk(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_es_remove_extent) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_shrink,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_shrink_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_shrink_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_shrink: dev %d,%d nr_shrunk %d, scan_time %" PRIu64 " nr_skipped %d retried %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.nr_shrunk(),
            msg.scan_time(), msg.nr_skipped(), msg.retried());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_es_shrink) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_shrink_count,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_shrink_count_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_shrink_count_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_shrink_count: dev %d,%d nr_to_scan %d cache_cnt %d", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.nr_to_scan(), msg.cache_cnt());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_es_shrink_count) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_shrink_scan_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_shrink_scan_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_shrink_scan_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_shrink_scan_enter: dev %d,%d nr_to_scan %d cache_cnt %d", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.nr_to_scan(), msg.cache_cnt());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_es_shrink_scan_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_es_shrink_scan_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_es_shrink_scan_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_es_shrink_scan_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_es_shrink_scan_exit: dev %d,%d nr_shrunk %d cache_cnt %d", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.nr_shrunk(), msg.cache_cnt());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_es_shrink_scan_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_evict_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_evict_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_evict_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_evict_inode: dev %d,%d ino %" PRIu64 " nlink %d", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.nlink());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_evict_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_convert_to_initialized_enter,
    [](const ForStandard::FtraceEvent& event) -> bool {
        return event.has_ext4_ext_convert_to_initialized_enter_format();
    },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_convert_to_initialized_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_convert_to_initialized_enter: dev %d,%d ino %" PRIu64
            " m_lblk %u m_len %u u_lblk %u u_len %u u_pblk %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.m_lblk(), msg.m_len(), msg.u_lblk(), msg.u_len(), msg.u_pblk());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_ext_convert_to_initialized_enter) msg had be cut off in "
                "outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_convert_to_initialized_fastpath,
    [](const ForStandard::FtraceEvent& event) -> bool {
        return event.has_ext4_ext_convert_to_initialized_fastpath_format();
    },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_convert_to_initialized_fastpath_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_convert_to_initialized_fastpath: dev %d,%d ino %" PRIu64
            " m_lblk %u m_len %u u_lblk %u u_len %u u_pblk %" PRIu64 " i_lblk %u i_len %u i_pblk %" PRIu64 " ",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.m_lblk(), msg.m_len(), msg.u_lblk(), msg.u_len(), msg.u_pblk(), msg.i_lblk(), msg.i_len(),
            msg.i_pblk());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_ext_convert_to_initialized_fastpath) msg had be cut off in "
                "outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_handle_unwritten_extents,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_handle_unwritten_extents_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_handle_unwritten_extents_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_handle_unwritten_extents: dev %d,%d ino %" PRIu64 " m_lblk %u m_pblk %" PRIu64
            " m_len %u flags %s allocated %d newblock %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.lblk(), msg.pblk(), msg.len(),
            __print_flags(msg.flags(), "|", {0x0001, "CREATE"}, {0x0002, "UNWRIT"}, {0x0004, "DELALLOC"},
                {0x0008, "PRE_IO"}, {0x0010, "CONVERT"}, {0x0020, "METADATA_NOFAIL"}, {0x0040, "NO_NORMALIZE"},
                {0x0100, "CONVERT_UNWRITTEN"}, {0x0200, "ZERO"}, {0x0400, "IO_SUBMIT"}, {0x40000000, "EX_NOCACHE"}),
            (unsigned int)msg.allocated(), msg.newblk());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_ext_handle_unwritten_extents) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_in_cache,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_in_cache_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_in_cache_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_in_cache: dev %d,%d ino %" PRIu64 " lblk %u ret %d", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), (unsigned)msg.lblk(), msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_ext_in_cache) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_load_extent,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_load_extent_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_load_extent_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_load_extent: dev %d,%d ino %" PRIu64 " lblk %u pblk %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.lblk(), msg.pblk());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_ext_load_extent) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_map_blocks_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_map_blocks_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_map_blocks_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_map_blocks_enter: dev %d,%d ino %" PRIu64 " lblk %u len %u flags %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.lblk(), msg.len(),
            __print_flags(msg.flags(), "|", {0x0001, "CREATE"}, {0x0002, "UNWRIT"}, {0x0004, "DELALLOC"},
                {0x0008, "PRE_IO"}, {0x0010, "CONVERT"}, {0x0020, "METADATA_NOFAIL"}, {0x0040, "NO_NORMALIZE"},
                {0x0100, "CONVERT_UNWRITTEN"}, {0x0200, "ZERO"}, {0x0400, "IO_SUBMIT"}, {0x40000000, "EX_NOCACHE"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_ext_map_blocks_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_map_blocks_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_map_blocks_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_map_blocks_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_map_blocks_exit: dev %d,%d ino %" PRIu64 " flags %s lblk %u pblk %" PRIu64
            " len %u mflags %s ret %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            __print_flags(msg.flags(), "|", {0x0001, "CREATE"}, {0x0002, "UNWRIT"}, {0x0004, "DELALLOC"},
                {0x0008, "PRE_IO"}, {0x0010, "CONVERT"}, {0x0020, "METADATA_NOFAIL"}, {0x0040, "NO_NORMALIZE"},
                {0x0100, "CONVERT_UNWRITTEN"}, {0x0200, "ZERO"}, {0x0400, "IO_SUBMIT"}, {0x40000000, "EX_NOCACHE"}),
            msg.lblk(), msg.pblk(), msg.len(),
            __print_flags(msg.mflags(), "", {((((1UL))) << (5)), "N"}, {((((1UL))) << (4)), "M"},
                {((((1UL))) << (11)), "U"}, {((((1UL))) << (9)), "B"}),
            msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_ext_map_blocks_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_put_in_cache,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_put_in_cache_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_put_in_cache_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_put_in_cache: dev %d,%d ino %" PRIu64 " lblk %u len %u start %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.lblk(), msg.len(), msg.start());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_ext_put_in_cache) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_remove_space,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_remove_space_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_remove_space_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_remove_space: dev %d,%d ino %" PRIu64 " since %u end %u depth %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.start(), (unsigned)msg.end(), msg.depth());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_ext_remove_space) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_remove_space_done,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_remove_space_done_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_remove_space_done_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_remove_space_done: dev %d,%d ino %" PRIu64 " since %u end %u depth %d partial%" PRIu64
            " remaining_entries %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.start(), (unsigned)msg.end(), msg.depth(), msg.partial(), (unsigned short)msg.eh_entries());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_ext_remove_space_done) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_rm_idx,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_rm_idx_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_rm_idx_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_rm_idx: dev %d,%d ino %" PRIu64 " index_pblk %" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pblk());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_ext_rm_idx) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_rm_leaf,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_rm_leaf_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_rm_leaf_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_rm_leaf: dev %d,%d ino %" PRIu64 " start_lblk %u last_extent [%u(%" PRIu64 "), %u]partial%" PRIu64
            "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.start(), (unsigned)msg.ee_lblk(), msg.ee_pblk(), (unsigned short)msg.ee_len(), msg.partial());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_ext_rm_leaf) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ext_show_extent,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ext_show_extent_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ext_show_extent_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ext_show_extent: dev %d,%d ino %" PRIu64 " lblk %u pblk %" PRIu64 " len %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.lblk(), msg.pblk(), (unsigned short)msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_ext_show_extent) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_fallocate_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_fallocate_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_fallocate_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_fallocate_enter: dev %d,%d ino %" PRIu64 " offset %" PRIu64 " len %" PRIu64 " mode %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.offset(), msg.len(),
            __print_flags(msg.mode(), "|", {0x01, "KEEP_SIZE"}, {0x02, "PUNCH_HOLE"}, {0x04, "NO_HIDE_STALE"},
                {0x08, "COLLAPSE_RANGE"}, {0x10, "ZERO_RANGE"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_fallocate_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_fallocate_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_fallocate_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_fallocate_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_fallocate_exit: dev %d,%d ino %" PRIu64 " pos %" PRIu64 " blocks %u ret %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pos(),
            msg.blocks(), msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_fallocate_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_find_delalloc_range,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_find_delalloc_range_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_find_delalloc_range_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_find_delalloc_range: dev %d,%d ino %" PRIu64 " from %u to %u reverse %d found %d (blk = %u)",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.from(), (unsigned)msg.to(), msg.reverse(), msg.found(), (unsigned)msg.found_blk());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_find_delalloc_range) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_forget,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_forget_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_forget_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_forget: dev %d,%d ino %" PRIu64 " mode 0%o is_metadata %d block %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.mode(), msg.is_metadata(), msg.block());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_forget) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_free_blocks,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_free_blocks_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_free_blocks_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_free_blocks: dev %d,%d ino %" PRIu64 " mode 0%o block %" PRIu64 " count %" PRIu64 " flags %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.mode(), msg.block(), msg.count(),
            __print_flags(msg.flags(), "|", {0x0001, "METADATA"}, {0x0002, "FORGET"}, {0x0004, "VALIDATED"},
                {0x0008, "NO_QUOTA"}, {0x0010, "1ST_CLUSTER"}, {0x0020, "LAST_CLUSTER"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_free_blocks) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_free_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_free_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_free_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_free_inode: dev %d,%d ino %" PRIu64 " mode 0%o uid %u gid %u blocks %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.mode(), msg.uid(), msg.gid(), msg.blocks());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_free_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_get_implied_cluster_alloc_exit,
    [](const ForStandard::FtraceEvent& event) -> bool {
        return event.has_ext4_get_implied_cluster_alloc_exit_format();
    },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_get_implied_cluster_alloc_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_get_implied_cluster_alloc_exit: dev %d,%d m_lblk %u m_pblk %" PRIu64 " m_len %u m_flags %s ret %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.lblk(),
            msg.pblk(), msg.len(),
            __print_flags(msg.flags(), "", {((((1UL))) << (5)), "N"}, {((((1UL))) << (4)), "M"},
                {((((1UL))) << (11)), "U"}, {((((1UL))) << (9)), "B"}),
            msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_get_implied_cluster_alloc_exit) msg had be cut off in "
                "outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_get_reserved_cluster_alloc,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_get_reserved_cluster_alloc_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_get_reserved_cluster_alloc_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_get_reserved_cluster_alloc: dev %d,%d ino %" PRIu64 " lblk %u len %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.lblk(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_get_reserved_cluster_alloc) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ind_map_blocks_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ind_map_blocks_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ind_map_blocks_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ind_map_blocks_enter: dev %d,%d ino %" PRIu64 " lblk %u len %u flags %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.lblk(), msg.len(),
            __print_flags(msg.flags(), "|", {0x0001, "CREATE"}, {0x0002, "UNWRIT"}, {0x0004, "DELALLOC"},
                {0x0008, "PRE_IO"}, {0x0010, "CONVERT"}, {0x0020, "METADATA_NOFAIL"}, {0x0040, "NO_NORMALIZE"},
                {0x0100, "CONVERT_UNWRITTEN"}, {0x0200, "ZERO"}, {0x0400, "IO_SUBMIT"}, {0x40000000, "EX_NOCACHE"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_ind_map_blocks_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_ind_map_blocks_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_ind_map_blocks_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_ind_map_blocks_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_ind_map_blocks_exit: dev %d,%d ino %" PRIu64 " flags %s lblk %u pblk %" PRIu64
            " len %u mflags %s ret %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            __print_flags(msg.flags(), "|", {0x0001, "CREATE"}, {0x0002, "UNWRIT"}, {0x0004, "DELALLOC"},
                {0x0008, "PRE_IO"}, {0x0010, "CONVERT"}, {0x0020, "METADATA_NOFAIL"}, {0x0040, "NO_NORMALIZE"},
                {0x0100, "CONVERT_UNWRITTEN"}, {0x0200, "ZERO"}, {0x0400, "IO_SUBMIT"}, {0x40000000, "EX_NOCACHE"}),
            msg.lblk(), msg.pblk(), msg.len(),
            __print_flags(msg.mflags(), "", {((((1UL))) << (5)), "N"}, {((((1UL))) << (4)), "M"},
                {((((1UL))) << (11)), "U"}, {((((1UL))) << (9)), "B"}),
            msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_ind_map_blocks_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_insert_range,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_insert_range_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_insert_range_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_insert_range: dev %d,%d ino %" PRIu64 " offset %" PRIu64 " len %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.offset(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_insert_range) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_invalidatepage,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_invalidatepage_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_invalidatepage_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_invalidatepage: dev %d,%d ino %" PRIu64 " page_index %" PRIu64 " offset %u length %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.index(), msg.offset(), msg.length());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_invalidatepage) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_journal_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_journal_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_journal_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_journal_start: dev %d,%d blocks %d, rsv_blocks %d, caller %p", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.blocks(), msg.rsv_blocks(), (void*)msg.ip());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_journal_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_journal_start_reserved,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_journal_start_reserved_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_journal_start_reserved_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_journal_start_reserved: dev %d,%d blocks, %d caller %p", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.blocks(), (void*)msg.ip());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_journal_start_reserved) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_journalled_invalidatepage,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_journalled_invalidatepage_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_journalled_invalidatepage_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_journalled_invalidatepage: dev %d,%d ino %" PRIu64 " page_index %" PRIu64 " offset %u length %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.index(), msg.offset(), msg.length());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_journalled_invalidatepage) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_journalled_write_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_journalled_write_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_journalled_write_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_journalled_write_end: dev %d,%d ino %" PRIu64 " pos %" PRIu64 " len %u copied %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pos(),
            msg.len(), msg.copied());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_journalled_write_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_load_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_load_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_load_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_load_inode: dev %d,%d ino %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_load_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_load_inode_bitmap,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_load_inode_bitmap_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_load_inode_bitmap_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_load_inode_bitmap: dev %d,%d group %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.group());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_load_inode_bitmap) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mark_inode_dirty,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mark_inode_dirty_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mark_inode_dirty_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mark_inode_dirty: dev %d,%d ino %" PRIu64 " caller %p", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), (void*)msg.ip());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mark_inode_dirty) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mb_bitmap_load,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mb_bitmap_load_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mb_bitmap_load_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_mb_bitmap_load: dev %d,%d group %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.group());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mb_bitmap_load) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mb_buddy_bitmap_load,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mb_buddy_bitmap_load_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mb_buddy_bitmap_load_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_mb_buddy_bitmap_load: dev %d,%d group %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.group());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_mb_buddy_bitmap_load) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mb_discard_preallocations,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mb_discard_preallocations_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mb_discard_preallocations_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_mb_discard_preallocations: dev %d,%d needed %d",
                ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.needed());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_mb_discard_preallocations) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mb_new_group_pa,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mb_new_group_pa_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mb_new_group_pa_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mb_new_group_pa: dev %d,%d ino %" PRIu64 " pstart %" PRIu64 " len %u lstart %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.pa_pstart(), msg.pa_len(), msg.pa_lstart());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mb_new_group_pa) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mb_new_inode_pa,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mb_new_inode_pa_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mb_new_inode_pa_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mb_new_inode_pa: dev %d,%d ino %" PRIu64 " pstart %" PRIu64 " len %u lstart %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.pa_pstart(), msg.pa_len(), msg.pa_lstart());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mb_new_inode_pa) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mb_release_group_pa,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mb_release_group_pa_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mb_release_group_pa_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mb_release_group_pa: dev %d,%d pstart %" PRIu64 " len %u", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.pa_pstart(), msg.pa_len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mb_release_group_pa) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mb_release_inode_pa,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mb_release_inode_pa_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mb_release_inode_pa_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mb_release_inode_pa: dev %d,%d ino %" PRIu64 " block %" PRIu64 " count %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.block(), msg.count());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mb_release_inode_pa) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mballoc_alloc,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mballoc_alloc_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mballoc_alloc_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mballoc_alloc: dev %d,%d inode %" PRIu64
            " orig %u/%d/%u@%u goal %u/%d/%u@%u result %u/%d/%u@%u blks %u grps %u cr %u flags %s tail %u broken %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.orig_group(), msg.orig_start(), msg.orig_len(), msg.orig_logical(), msg.goal_group(), msg.goal_start(),
            msg.goal_len(), msg.goal_logical(), msg.result_group(), msg.result_start(), msg.result_len(),
            msg.result_logical(), msg.found(), msg.groups(), msg.cr(),
            __print_flags(msg.flags(), "|", {0x0001, "HINT_MERGE"}, {0x0002, "HINT_RESV"}, {0x0004, "HINT_MDATA"},
                {0x0008, "HINT_FIRST"}, {0x0010, "HINT_BEST"}, {0x0020, "HINT_DATA"}, {0x0040, "HINT_NOPREALLOC"},
                {0x0080, "HINT_GRP_ALLOC"}, {0x0100, "HINT_GOAL_ONLY"}, {0x0200, "HINT_TRY_GOAL"},
                {0x0400, "DELALLOC_RESV"}, {0x0800, "STREAM_ALLOC"}, {0x1000, "USE_ROOT_BLKS"}, {0x2000, "USE_RESV"},
                {0x4000, "STRICT_CHECK"}),
            msg.tail(), msg.buddy() ? 1 << msg.buddy() : 0);
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mballoc_alloc) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mballoc_discard,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mballoc_discard_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mballoc_discard_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mballoc_discard: dev %d,%d inode %" PRIu64 " extent %u/%d/%d ", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.result_group(), msg.result_start(),
            msg.result_len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mballoc_discard) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mballoc_free,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mballoc_free_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mballoc_free_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mballoc_free: dev %d,%d inode %" PRIu64 " extent %u/%d/%d ", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.result_group(), msg.result_start(),
            msg.result_len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_mballoc_free) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_mballoc_prealloc,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_mballoc_prealloc_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_mballoc_prealloc_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_mballoc_prealloc: dev %d,%d inode %" PRIu64 " orig %u/%d/%u@%u result %u/%d/%u@%u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.orig_group(), msg.orig_start(), msg.orig_len(), msg.orig_logical(), msg.result_group(),
            msg.result_start(), msg.result_len(), msg.result_logical());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_mballoc_prealloc) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_other_inode_update_time,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_other_inode_update_time_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_other_inode_update_time_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_other_inode_update_time: dev %d,%d orig_ino %" PRIu64 " ino %" PRIu64 " mode 0%o uid %u gid %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.orig_ino(),
            msg.ino(), msg.mode(), msg.uid(), msg.gid());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_other_inode_update_time) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_punch_hole,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_punch_hole_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_punch_hole_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_punch_hole: dev %d,%d ino %" PRIu64 " offset %" PRIu64 " len %" PRIu64 " mode %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.offset(), msg.len(),
            __print_flags(msg.mode(), "|", {0x01, "KEEP_SIZE"}, {0x02, "PUNCH_HOLE"}, {0x04, "NO_HIDE_STALE"},
                {0x08, "COLLAPSE_RANGE"}, {0x10, "ZERO_RANGE"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_punch_hole) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_read_block_bitmap_load,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_read_block_bitmap_load_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_read_block_bitmap_load_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_read_block_bitmap_load: dev %d,%d group %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.group());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(ext4_read_block_bitmap_load) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_readpage,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_readpage_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_readpage_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_readpage: dev %d,%d ino %" PRIu64 " page_index %" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_readpage) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_releasepage,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_releasepage_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_releasepage_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_releasepage: dev %d,%d ino %" PRIu64 " page_index %" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_releasepage) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_remove_blocks,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_remove_blocks_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_remove_blocks_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_remove_blocks: dev %d,%d ino %" PRIu64 " extent [%u(%" PRIu64 "), %u]from %u to %u partial%" PRIu64
            "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            (unsigned)msg.ee_lblk(), msg.ee_pblk(), (unsigned short)msg.ee_len(), (unsigned)msg.from(),
            (unsigned)msg.to(), msg.partial());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_remove_blocks) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_request_blocks,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_request_blocks_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_request_blocks_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_request_blocks: dev %d,%d ino %" PRIu64 " flags %s len %u lblk %u goal %" PRIu64
            " lleft %u lright %u pleft %" PRIu64 " pright %" PRIu64 " ",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            __print_flags(msg.flags(), "|", {0x0001, "HINT_MERGE"}, {0x0002, "HINT_RESV"}, {0x0004, "HINT_MDATA"},
                {0x0008, "HINT_FIRST"}, {0x0010, "HINT_BEST"}, {0x0020, "HINT_DATA"}, {0x0040, "HINT_NOPREALLOC"},
                {0x0080, "HINT_GRP_ALLOC"}, {0x0100, "HINT_GOAL_ONLY"}, {0x0200, "HINT_TRY_GOAL"},
                {0x0400, "DELALLOC_RESV"}, {0x0800, "STREAM_ALLOC"}, {0x1000, "USE_ROOT_BLKS"}, {0x2000, "USE_RESV"},
                {0x4000, "STRICT_CHECK"}),
            msg.len(), msg.logical(), msg.goal(), msg.lleft(), msg.lright(), msg.pleft(), msg.pright());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_request_blocks) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_request_inode,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_request_inode_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_request_inode_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_request_inode: dev %d,%d dir %" PRIu64 " mode 0%o", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.dir(), msg.mode());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_request_inode) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_sync_file_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_sync_file_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_sync_file_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_sync_file_enter: dev %d,%d ino %" PRIu64 " parent %" PRIu64 " datasync %d ",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.parent(), msg.datasync());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_sync_file_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_sync_file_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_sync_file_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_sync_file_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_sync_file_exit: dev %d,%d ino %" PRIu64 " ret %d", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_sync_file_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_sync_fs,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_sync_fs_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_sync_fs_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_sync_fs: dev %d,%d wait %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.wait());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_sync_fs) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_trim_all_free,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_trim_all_free_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_trim_all_free_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_trim_all_free: dev %d,%d group %u, start %d, len %d",
                msg.dev_major(), msg.dev_minor(), msg.group(), msg.start(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_trim_all_free) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_trim_extent,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_trim_extent_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_trim_extent_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_trim_extent: dev %d,%d group %u, start %d, len %d",
                msg.dev_major(), msg.dev_minor(), msg.group(), msg.start(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_trim_extent) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_truncate_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_truncate_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_truncate_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_truncate_enter: dev %d,%d ino %" PRIu64 " blocks %" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.blocks());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_truncate_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_truncate_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_truncate_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_truncate_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_truncate_exit: dev %d,%d ino %" PRIu64 " blocks %" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.blocks());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_truncate_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_unlink_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_unlink_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_unlink_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_unlink_enter: dev %d,%d ino %" PRIu64 " size %" PRIu64 " parent %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.size(), msg.parent());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_unlink_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_unlink_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_unlink_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_unlink_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "ext4_unlink_exit: dev %d,%d ino %" PRIu64 " ret %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_unlink_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_write_begin,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_write_begin_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_write_begin_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_write_begin: dev %d,%d ino %" PRIu64 " pos %" PRIu64 " len %u flags %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pos(),
            msg.len(), msg.flags());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_write_begin) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_write_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_write_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_write_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_write_end: dev %d,%d ino %" PRIu64 " pos %" PRIu64 " len %u copied %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.pos(),
            msg.len(), msg.copied());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_write_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_writepage,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_writepage_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_writepage_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_writepage: dev %d,%d ino %" PRIu64 " page_index %" PRIu64 "", ((unsigned int)((msg.dev()) >> 20)),
            ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_writepage) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_writepages,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_writepages_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_writepages_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_writepages: dev %d,%d ino %" PRIu64 " nr_to_write %" PRIu64 " pages_skipped %" PRIu64
            " range_start %" PRIu64 " range_end %" PRIu64
            " sync_mode %d for_kupdate %d range_cyclic %d writeback_index %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.nr_to_write(), msg.pages_skipped(), msg.range_start(), msg.range_end(), msg.sync_mode(),
            msg.for_kupdate(), msg.range_cyclic(), msg.writeback_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_writepages) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_writepages_result,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_writepages_result_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_writepages_result_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_writepages_result: dev %d,%d ino %" PRIu64 " ret %d pages_written %d pages_skipped %" PRIu64
            " sync_mode %d writeback_index %" PRIu64 "",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(), msg.ret(),
            msg.pages_written(), msg.pages_skipped(), msg.sync_mode(), msg.writeback_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(ext4_writepages_result) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    ext4_zero_range,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_ext4_zero_range_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.ext4_zero_range_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "ext4_zero_range: dev %d,%d ino %" PRIu64 " offset %" PRIu64 " len %" PRIu64 " mode %s",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))), msg.ino(),
            msg.offset(), msg.len(),
            __print_flags(msg.mode(), "|", {0x01, "KEEP_SIZE"}, {0x02, "PUNCH_HOLE"}, {0x04, "NO_HIDE_STALE"},
                {0x08, "COLLAPSE_RANGE"}, {0x10, "ZERO_RANGE"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(ext4_zero_range) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
