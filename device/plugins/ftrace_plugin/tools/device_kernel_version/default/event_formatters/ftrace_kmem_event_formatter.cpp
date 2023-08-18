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
    kfree,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_kfree_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.kfree_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "kfree: call_site=%p ptr=%p", msg.call_site(), msg.ptr());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(kfree) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    kmalloc,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_kmalloc_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.kmalloc_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "kmalloc: call_site=%p ptr=%p bytes_req=%" PRIu64 " bytes_alloc=%" PRIu64 " gfp_flags=%s", msg.call_site(),
            msg.ptr(), msg.bytes_req(), msg.bytes_alloc(),
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
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(kmalloc) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    kmalloc_node,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_kmalloc_node_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.kmalloc_node_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "kmalloc_node: call_site=%p ptr=%p bytes_req=%" PRIu64 " bytes_alloc=%" PRIu64 " gfp_flags=%s node=%d",
            msg.call_site(), msg.ptr(), msg.bytes_req(), msg.bytes_alloc(),
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
            msg.node());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(kmalloc_node) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    kmem_cache_alloc,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_kmem_cache_alloc_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.kmem_cache_alloc_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "kmem_cache_alloc: call_site=%p ptr=%p bytes_req=%" PRIu64 " bytes_alloc=%" PRIu64 " gfp_flags=%s",
            msg.call_site(), msg.ptr(), msg.bytes_req(), msg.bytes_alloc(),
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
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(kmem_cache_alloc) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    kmem_cache_alloc_node,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_kmem_cache_alloc_node_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.kmem_cache_alloc_node_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "kmem_cache_alloc_node: call_site=%p ptr=%p bytes_req=%" PRIu64 " bytes_alloc=%" PRIu64
            " gfp_flags=%s node=%d",
            msg.call_site(), msg.ptr(), msg.bytes_req(), msg.bytes_alloc(),
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
            msg.node());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(kmem_cache_alloc_node) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    kmem_cache_free,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_kmem_cache_free_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.kmem_cache_free_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "kmem_cache_free: call_site=%p ptr=%p", msg.call_site(), msg.ptr());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(kmem_cache_free) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_page_alloc,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_page_alloc_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_page_alloc_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_page_alloc: page=%s pfn=%" PRIu64 " order=%d migratetype=%d gfp_flags=%s", "0000000000000000",
            msg.pfn() != -1UL ? msg.pfn() : 0, msg.order(), msg.migratetype(),
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
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(mm_page_alloc) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_page_alloc_extfrag,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_page_alloc_extfrag_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_page_alloc_extfrag_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_page_alloc_extfrag: page=%s pfn=%" PRIu64
            " alloc_order=%d fallback_order=%d pageblock_order=%d alloc_migratetype=%d fallback_migratetype=%d "
            "fragmenting=%d change_ownership=%d",
            "0000000000000000", msg.pfn(), msg.alloc_order(), msg.fallback_order(), (11 - 1), msg.alloc_migratetype(),
            msg.fallback_migratetype(), msg.fallback_order() < (11 - 1), msg.change_ownership());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_page_alloc_extfrag) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_page_alloc_zone_locked,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_page_alloc_zone_locked_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_page_alloc_zone_locked_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_page_alloc_zone_locked: page=%s pfn=%" PRIu64 " order=%u migratetype=%d percpu_refill=%d",
            "0000000000000000", msg.pfn() != -1UL ? msg.pfn() : 0, msg.order(), msg.migratetype(), msg.order() == 0);
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(mm_page_alloc_zone_locked) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_page_free,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_page_free_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_page_free_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "mm_page_free: page=%s pfn=%" PRIu64 " order=%d",
            "0000000000000000", msg.pfn(), msg.order());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(mm_page_free) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_page_free_batched,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_page_free_batched_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_page_free_batched_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_page_free_batched: page=%s pfn=%" PRIu64 " order=0", "0000000000000000", msg.pfn());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_page_free_batched) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_page_pcpu_drain,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_page_pcpu_drain_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_page_pcpu_drain_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_page_pcpu_drain: page=%s pfn=%" PRIu64 " order=%d migratetype=%d", "0000000000000000", msg.pfn(),
            msg.order(), msg.migratetype());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(mm_page_pcpu_drain) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rss_stat,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rss_stat_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rss_stat_format();
        char buffer[BUFFER_SIZE];
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "rss_stat: mm_id=%u curr=%d member=%d size=%" PRIu64 "B",
                msg.mm_id(), msg.curr(), msg.member(), msg.size());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(rss_stat) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
