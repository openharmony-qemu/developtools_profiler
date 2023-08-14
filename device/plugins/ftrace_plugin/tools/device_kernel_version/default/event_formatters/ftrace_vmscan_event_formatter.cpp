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
#include "type.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_shrink_slab_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_shrink_slab_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_shrink_slab_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_shrink_slab_end: %p %p: nid: %d unused scan count %" PRIu64 " new scan count %" PRIu64
            " total_scan %" PRIu64 " last shrinker return val %d",
            msg.shrink(), msg.shr(), msg.nid(), msg.unused_scan(), msg.new_scan(), msg.total_scan(), msg.retval());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_shrink_slab_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_shrink_slab_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_shrink_slab_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_shrink_slab_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_shrink_slab_start: %p %p: nid: %d objects to shrink %" PRIu64 " gfp_flags %s cache items %" PRIu64
            " delta %" PRIu64 " total_scan %" PRIu64 " priority %d",
            msg.shrink(), msg.shr(), msg.nid(), msg.nr_objects_to_shrink(),
            (msg.gfp_flags())
                ? __print_flags(msg.gfp_flags(), "|",
                      {(unsigned long)((((((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                               ((gfp_t)0x100000u)) |
                                              ((gfp_t)0x02u)) |
                                             ((gfp_t)0x08u)) |
                                            ((gfp_t)0x40000u) | ((gfp_t)0x80000u) | ((gfp_t)0x2000u)) &
                                           ~((gfp_t)(0x400u | 0x800u))) |
                                       ((gfp_t)0x400u)),
                          "GFP_TRANSHUGE"},
                      {(unsigned long)(((((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                              ((gfp_t)0x100000u)) |
                                             ((gfp_t)0x02u)) |
                                            ((gfp_t)0x08u)) |
                                           ((gfp_t)0x40000u) | ((gfp_t)0x80000u) | ((gfp_t)0x2000u)) &
                                       ~((gfp_t)(0x400u | 0x800u))),
                          "GFP_TRANSHUGE_LIGHT"},
                      {(unsigned long)(((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                            ((gfp_t)0x100000u)) |
                                           ((gfp_t)0x02u)) |
                                       ((gfp_t)0x08u)),
                          "GFP_HIGHUSER_MOVABLE"},
                      {(unsigned long)((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                           ((gfp_t)0x100000u)) |
                                       ((gfp_t)0x02u)),
                          "GFP_HIGHUSER"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                       ((gfp_t)0x100000u)),
                          "GFP_USER"},
                      {(unsigned long)((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u)) |
                                       ((gfp_t)0x400000u)),
                          "GFP_KERNEL_ACCOUNT"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u)), "GFP_KERNEL"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u)), "GFP_NOFS"},
                      {(unsigned long)(((gfp_t)0x20u) | ((gfp_t)0x200u) | ((gfp_t)0x800u)), "GFP_ATOMIC"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u))), "GFP_NOIO"},
                      {(unsigned long)(((gfp_t)0x800u)), "GFP_NOWAIT"}, {(unsigned long)((gfp_t)0x01u), "GFP_DMA"},
                      {(unsigned long)((gfp_t)0x02u), "__GFP_HIGHMEM"}, {(unsigned long)((gfp_t)0x04u), "GFP_DMA32"},
                      {(unsigned long)((gfp_t)0x20u), "__GFP_HIGH"}, {(unsigned long)((gfp_t)0x200u), "__GFP_ATOMIC"},
                      {(unsigned long)((gfp_t)0x40u), "__GFP_IO"}, {(unsigned long)((gfp_t)0x80u), "__GFP_FS"},
                      {(unsigned long)((gfp_t)0x2000u), "__GFP_NOWARN"},
                      {(unsigned long)((gfp_t)0x4000u), "__GFP_RETRY_MAYFAIL"},
                      {(unsigned long)((gfp_t)0x8000u), "__GFP_NOFAIL"},
                      {(unsigned long)((gfp_t)0x10000u), "__GFP_NORETRY"},
                      {(unsigned long)((gfp_t)0x40000u), "__GFP_COMP"}, {(unsigned long)((gfp_t)0x100u), "__GFP_ZERO"},
                      {(unsigned long)((gfp_t)0x80000u), "__GFP_NOMEMALLOC"},
                      {(unsigned long)((gfp_t)0x20000u), "__GFP_MEMALLOC"},
                      {(unsigned long)((gfp_t)0x100000u), "__GFP_HARDWALL"},
                      {(unsigned long)((gfp_t)0x200000u), "__GFP_THISNODE"},
                      {(unsigned long)((gfp_t)0x10u), "__GFP_RECLAIMABLE"},
                      {(unsigned long)((gfp_t)0x08u), "__GFP_MOVABLE"},
                      {(unsigned long)((gfp_t)0x400000u), "__GFP_ACCOUNT"},
                      {(unsigned long)((gfp_t)0x1000u), "__GFP_WRITE"},
                      {(unsigned long)((gfp_t)(0x400u | 0x800u)), "__GFP_RECLAIM"},
                      {(unsigned long)((gfp_t)0x400u), "__GFP_DIRECT_RECLAIM"},
                      {(unsigned long)((gfp_t)0x800u), "__GFP_KSWAPD_RECLAIM"})
                : "none",
            msg.cache_items(), msg.delta(), msg.total_scan(), msg.priority());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_shrink_slab_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_vmscan_direct_reclaim_begin,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_vmscan_direct_reclaim_begin_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_vmscan_direct_reclaim_begin_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_vmscan_direct_reclaim_begin: order=%d gfp_flags=%s", msg.order(),
            (msg.gfp_flags())
                ? __print_flags(msg.gfp_flags(), "|",
                      {(unsigned long)((((((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                               ((gfp_t)0x100000u)) |
                                              ((gfp_t)0x02u)) |
                                             ((gfp_t)0x08u)) |
                                            ((gfp_t)0x40000u) | ((gfp_t)0x80000u) | ((gfp_t)0x2000u)) &
                                           ~((gfp_t)(0x400u | 0x800u))) |
                                       ((gfp_t)0x400u)),
                          "GFP_TRANSHUGE"},
                      {(unsigned long)(((((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                              ((gfp_t)0x100000u)) |
                                             ((gfp_t)0x02u)) |
                                            ((gfp_t)0x08u)) |
                                           ((gfp_t)0x40000u) | ((gfp_t)0x80000u) | ((gfp_t)0x2000u)) &
                                       ~((gfp_t)(0x400u | 0x800u))),
                          "GFP_TRANSHUGE_LIGHT"},
                      {(unsigned long)(((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                            ((gfp_t)0x100000u)) |
                                           ((gfp_t)0x02u)) |
                                       ((gfp_t)0x08u)),
                          "GFP_HIGHUSER_MOVABLE"},
                      {(unsigned long)((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                           ((gfp_t)0x100000u)) |
                                       ((gfp_t)0x02u)),
                          "GFP_HIGHUSER"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                       ((gfp_t)0x100000u)),
                          "GFP_USER"},
                      {(unsigned long)((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u)) |
                                       ((gfp_t)0x400000u)),
                          "GFP_KERNEL_ACCOUNT"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u)), "GFP_KERNEL"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u)), "GFP_NOFS"},
                      {(unsigned long)(((gfp_t)0x20u) | ((gfp_t)0x200u) | ((gfp_t)0x800u)), "GFP_ATOMIC"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u))), "GFP_NOIO"},
                      {(unsigned long)(((gfp_t)0x800u)), "GFP_NOWAIT"}, {(unsigned long)((gfp_t)0x01u), "GFP_DMA"},
                      {(unsigned long)((gfp_t)0x02u), "__GFP_HIGHMEM"}, {(unsigned long)((gfp_t)0x04u), "GFP_DMA32"},
                      {(unsigned long)((gfp_t)0x20u), "__GFP_HIGH"}, {(unsigned long)((gfp_t)0x200u), "__GFP_ATOMIC"},
                      {(unsigned long)((gfp_t)0x40u), "__GFP_IO"}, {(unsigned long)((gfp_t)0x80u), "__GFP_FS"},
                      {(unsigned long)((gfp_t)0x2000u), "__GFP_NOWARN"},
                      {(unsigned long)((gfp_t)0x4000u), "__GFP_RETRY_MAYFAIL"},
                      {(unsigned long)((gfp_t)0x8000u), "__GFP_NOFAIL"},
                      {(unsigned long)((gfp_t)0x10000u), "__GFP_NORETRY"},
                      {(unsigned long)((gfp_t)0x40000u), "__GFP_COMP"}, {(unsigned long)((gfp_t)0x100u), "__GFP_ZERO"},
                      {(unsigned long)((gfp_t)0x80000u), "__GFP_NOMEMALLOC"},
                      {(unsigned long)((gfp_t)0x20000u), "__GFP_MEMALLOC"},
                      {(unsigned long)((gfp_t)0x100000u), "__GFP_HARDWALL"},
                      {(unsigned long)((gfp_t)0x200000u), "__GFP_THISNODE"},
                      {(unsigned long)((gfp_t)0x10u), "__GFP_RECLAIMABLE"},
                      {(unsigned long)((gfp_t)0x08u), "__GFP_MOVABLE"},
                      {(unsigned long)((gfp_t)0x400000u), "__GFP_ACCOUNT"},
                      {(unsigned long)((gfp_t)0x1000u), "__GFP_WRITE"},
                      {(unsigned long)((gfp_t)(0x400u | 0x800u)), "__GFP_RECLAIM"},
                      {(unsigned long)((gfp_t)0x400u), "__GFP_DIRECT_RECLAIM"},
                      {(unsigned long)((gfp_t)0x800u), "__GFP_KSWAPD_RECLAIM"})
                : "none");
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(mm_vmscan_direct_reclaim_begin) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_vmscan_direct_reclaim_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_vmscan_direct_reclaim_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_vmscan_direct_reclaim_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_vmscan_direct_reclaim_end: nr_reclaimed=%" PRIu64 "", msg.nr_reclaimed());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(mm_vmscan_direct_reclaim_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_vmscan_kswapd_sleep,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_vmscan_kswapd_sleep_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_vmscan_kswapd_sleep_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "mm_vmscan_kswapd_sleep: nid=%d", msg.nid());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_vmscan_kswapd_sleep) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_vmscan_kswapd_wake,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_vmscan_kswapd_wake_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_vmscan_kswapd_wake_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "mm_vmscan_kswapd_wake: nid=%d order=%d", msg.nid(), msg.order());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_vmscan_kswapd_wake) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_vmscan_lru_isolate,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_vmscan_lru_isolate_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_vmscan_lru_isolate_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_vmscan_lru_isolate: isolate_mode=%d classzone=%d order=%d nr_requested=%" PRIu64 " nr_scanned=%" PRIu64
            " nr_skipped=%" PRIu64 " nr_taken=%" PRIu64 " lru=%s",
            msg.isolate_mode(), msg.classzone_idx(), msg.order(), msg.nr_requested(), msg.nr_scanned(),
            msg.nr_skipped(), msg.nr_taken(),
            __print_symbolic(msg.lru(), {0, "inactive_anon"}, {1, "active_anon"}, {2, "inactive_file"},
                {3, "active_file"}, {4, "unevictable"}));
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_vmscan_lru_isolate) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_vmscan_lru_shrink_inactive,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_vmscan_lru_shrink_inactive_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_vmscan_lru_shrink_inactive_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_vmscan_lru_shrink_inactive: nid=%d nr_scanned=%" PRIu64 " nr_reclaimed=%" PRIu64 " nr_dirty=%" PRIu64
            " nr_writeback=%" PRIu64 " nr_congested=%" PRIu64 " nr_immediate=%" PRIu64 " nr_activate=%" PRIu64
            " nr_ref_keep=%" PRIu64 " nr_unmap_fail=%" PRIu64 " priority=%d flags=%s",
            msg.nid(), msg.nr_scanned(), msg.nr_reclaimed(), msg.nr_dirty(), msg.nr_writeback(), msg.nr_congested(),
            msg.nr_immediate(), msg.nr_activate(), msg.nr_ref_keep(), msg.nr_unmap_fail(), msg.priority(),
            (msg.reclaim_flags())
                ? __print_flags(msg.reclaim_flags(), "|", {0x0001u, "RECLAIM_WB_ANON"}, {0x0002u, "RECLAIM_WB_FILE"},
                      {0x0010u, "RECLAIM_WB_MIXED"}, {0x0004u, "RECLAIM_WB_SYNC"}, {0x0008u, "RECLAIM_WB_ASYNC"})
                : "RECLAIM_WB_NONE");
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(mm_vmscan_lru_shrink_inactive) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_vmscan_wakeup_kswapd,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_vmscan_wakeup_kswapd_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_vmscan_wakeup_kswapd_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_vmscan_wakeup_kswapd: nid=%d order=%d gfp_flags=%s", msg.nid(), msg.order(),
            (msg.gfp_flags())
                ? __print_flags(msg.gfp_flags(), "|",
                      {(unsigned long)((((((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                               ((gfp_t)0x100000u)) |
                                              ((gfp_t)0x02u)) |
                                             ((gfp_t)0x08u)) |
                                            ((gfp_t)0x40000u) | ((gfp_t)0x80000u) | ((gfp_t)0x2000u)) &
                                           ~((gfp_t)(0x400u | 0x800u))) |
                                       ((gfp_t)0x400u)),
                          "GFP_TRANSHUGE"},
                      {(unsigned long)(((((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                              ((gfp_t)0x100000u)) |
                                             ((gfp_t)0x02u)) |
                                            ((gfp_t)0x08u)) |
                                           ((gfp_t)0x40000u) | ((gfp_t)0x80000u) | ((gfp_t)0x2000u)) &
                                       ~((gfp_t)(0x400u | 0x800u))),
                          "GFP_TRANSHUGE_LIGHT"},
                      {(unsigned long)(((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                            ((gfp_t)0x100000u)) |
                                           ((gfp_t)0x02u)) |
                                       ((gfp_t)0x08u)),
                          "GFP_HIGHUSER_MOVABLE"},
                      {(unsigned long)((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                           ((gfp_t)0x100000u)) |
                                       ((gfp_t)0x02u)),
                          "GFP_HIGHUSER"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u) |
                                       ((gfp_t)0x100000u)),
                          "GFP_USER"},
                      {(unsigned long)((((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u)) |
                                       ((gfp_t)0x400000u)),
                          "GFP_KERNEL_ACCOUNT"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u) | ((gfp_t)0x80u)), "GFP_KERNEL"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u)) | ((gfp_t)0x40u)), "GFP_NOFS"},
                      {(unsigned long)(((gfp_t)0x20u) | ((gfp_t)0x200u) | ((gfp_t)0x800u)), "GFP_ATOMIC"},
                      {(unsigned long)(((gfp_t)(0x400u | 0x800u))), "GFP_NOIO"},
                      {(unsigned long)(((gfp_t)0x800u)), "GFP_NOWAIT"}, {(unsigned long)((gfp_t)0x01u), "GFP_DMA"},
                      {(unsigned long)((gfp_t)0x02u), "__GFP_HIGHMEM"}, {(unsigned long)((gfp_t)0x04u), "GFP_DMA32"},
                      {(unsigned long)((gfp_t)0x20u), "__GFP_HIGH"}, {(unsigned long)((gfp_t)0x200u), "__GFP_ATOMIC"},
                      {(unsigned long)((gfp_t)0x40u), "__GFP_IO"}, {(unsigned long)((gfp_t)0x80u), "__GFP_FS"},
                      {(unsigned long)((gfp_t)0x2000u), "__GFP_NOWARN"},
                      {(unsigned long)((gfp_t)0x4000u), "__GFP_RETRY_MAYFAIL"},
                      {(unsigned long)((gfp_t)0x8000u), "__GFP_NOFAIL"},
                      {(unsigned long)((gfp_t)0x10000u), "__GFP_NORETRY"},
                      {(unsigned long)((gfp_t)0x40000u), "__GFP_COMP"}, {(unsigned long)((gfp_t)0x100u), "__GFP_ZERO"},
                      {(unsigned long)((gfp_t)0x80000u), "__GFP_NOMEMALLOC"},
                      {(unsigned long)((gfp_t)0x20000u), "__GFP_MEMALLOC"},
                      {(unsigned long)((gfp_t)0x100000u), "__GFP_HARDWALL"},
                      {(unsigned long)((gfp_t)0x200000u), "__GFP_THISNODE"},
                      {(unsigned long)((gfp_t)0x10u), "__GFP_RECLAIMABLE"},
                      {(unsigned long)((gfp_t)0x08u), "__GFP_MOVABLE"},
                      {(unsigned long)((gfp_t)0x400000u), "__GFP_ACCOUNT"},
                      {(unsigned long)((gfp_t)0x1000u), "__GFP_WRITE"},
                      {(unsigned long)((gfp_t)(0x400u | 0x800u)), "__GFP_RECLAIM"},
                      {(unsigned long)((gfp_t)0x400u), "__GFP_DIRECT_RECLAIM"},
                      {(unsigned long)((gfp_t)0x800u), "__GFP_KSWAPD_RECLAIM"})
                : "none");
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_vmscan_wakeup_kswapd) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_vmscan_writepage,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_vmscan_writepage_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_vmscan_writepage_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_vmscan_writepage: page=%s pfn=%" PRIu64 " flags=%s", "0000000000000000", msg.pfn(),
            (msg.reclaim_flags())
                ? __print_flags(msg.reclaim_flags(), "|", {0x0001u, "RECLAIM_WB_ANON"}, {0x0002u, "RECLAIM_WB_FILE"},
                      {0x0010u, "RECLAIM_WB_MIXED"}, {0x0004u, "RECLAIM_WB_SYNC"}, {0x0008u, "RECLAIM_WB_ASYNC"})
                : "RECLAIM_WB_NONE");
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_vmscan_writepage) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
