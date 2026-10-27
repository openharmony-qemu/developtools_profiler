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
#include "sub_event_parser.h"

#include <string>
#include <vector>

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
struct KfreeData {
    uint64_t call_site {};
    uint64_t ptr {};
};
__attribute__((noinline)) void ParseKfreeData(KfreeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.call_site = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ptr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseKfree(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    KfreeData eventData {};
    ParseKfreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_kfree_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_call_site(eventData.call_site);
    msg->set_ptr(eventData.ptr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kfree, ParseKfree<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kfree, ParseKfree<ProtoEncoder::FtraceEvent>);

struct KmallocData {
    uint64_t call_site {};
    uint64_t ptr {};
    uint64_t bytes_req {};
    uint64_t bytes_alloc {};
    uint32_t gfp_flags {};
};
__attribute__((noinline)) void ParseKmallocData(KmallocData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.call_site = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ptr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bytes_req = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bytes_alloc = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.gfp_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseKmalloc(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    KmallocData eventData {};
    ParseKmallocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_kmalloc_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_call_site(eventData.call_site);
    msg->set_ptr(eventData.ptr);
    msg->set_bytes_req(eventData.bytes_req);
    msg->set_bytes_alloc(eventData.bytes_alloc);
    msg->set_gfp_flags(eventData.gfp_flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmalloc, ParseKmalloc<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmalloc, ParseKmalloc<ProtoEncoder::FtraceEvent>);

struct KmallocNodeData {
    uint64_t call_site {};
    uint64_t ptr {};
    uint64_t bytes_req {};
    uint64_t bytes_alloc {};
    uint32_t gfp_flags {};
    int32_t node {};
};
__attribute__((noinline)) void ParseKmallocNodeData(KmallocNodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.call_site = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ptr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bytes_req = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bytes_alloc = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.gfp_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.node = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseKmallocNode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    KmallocNodeData eventData {};
    ParseKmallocNodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_kmalloc_node_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_call_site(eventData.call_site);
    msg->set_ptr(eventData.ptr);
    msg->set_bytes_req(eventData.bytes_req);
    msg->set_bytes_alloc(eventData.bytes_alloc);
    msg->set_gfp_flags(eventData.gfp_flags);
    msg->set_node(eventData.node);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmalloc_node, ParseKmallocNode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmalloc_node, ParseKmallocNode<ProtoEncoder::FtraceEvent>);

struct KmemCacheAllocData {
    uint64_t call_site {};
    uint64_t ptr {};
    uint64_t bytes_req {};
    uint64_t bytes_alloc {};
    uint32_t gfp_flags {};
};
__attribute__((noinline)) void ParseKmemCacheAllocData(KmemCacheAllocData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.call_site = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ptr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bytes_req = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bytes_alloc = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.gfp_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseKmemCacheAlloc(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    KmemCacheAllocData eventData {};
    ParseKmemCacheAllocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_kmem_cache_alloc_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_call_site(eventData.call_site);
    msg->set_ptr(eventData.ptr);
    msg->set_bytes_req(eventData.bytes_req);
    msg->set_bytes_alloc(eventData.bytes_alloc);
    msg->set_gfp_flags(eventData.gfp_flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmem_cache_alloc, ParseKmemCacheAlloc<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmem_cache_alloc, ParseKmemCacheAlloc<ProtoEncoder::FtraceEvent>);

struct KmemCacheAllocNodeData {
    uint64_t call_site {};
    uint64_t ptr {};
    uint64_t bytes_req {};
    uint64_t bytes_alloc {};
    uint32_t gfp_flags {};
    int32_t node {};
};
__attribute__((noinline)) void ParseKmemCacheAllocNodeData(KmemCacheAllocNodeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.call_site = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ptr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bytes_req = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.bytes_alloc = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.gfp_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.node = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseKmemCacheAllocNode(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    KmemCacheAllocNodeData eventData {};
    ParseKmemCacheAllocNodeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_kmem_cache_alloc_node_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_call_site(eventData.call_site);
    msg->set_ptr(eventData.ptr);
    msg->set_bytes_req(eventData.bytes_req);
    msg->set_bytes_alloc(eventData.bytes_alloc);
    msg->set_gfp_flags(eventData.gfp_flags);
    msg->set_node(eventData.node);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmem_cache_alloc_node, ParseKmemCacheAllocNode<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmem_cache_alloc_node, ParseKmemCacheAllocNode<ProtoEncoder::FtraceEvent>);

struct KmemCacheFreeData {
    uint64_t call_site {};
    uint64_t ptr {};
};
__attribute__((noinline)) void ParseKmemCacheFreeData(KmemCacheFreeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.call_site = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.ptr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseKmemCacheFree(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    KmemCacheFreeData eventData {};
    ParseKmemCacheFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_kmem_cache_free_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_call_site(eventData.call_site);
    msg->set_ptr(eventData.ptr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(kmem_cache_free, ParseKmemCacheFree<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(kmem_cache_free, ParseKmemCacheFree<ProtoEncoder::FtraceEvent>);

struct MmPageAllocData {
    uint64_t pfn {};
    uint32_t order {};
    uint32_t gfp_flags {};
    int32_t migratetype {};
};
__attribute__((noinline)) void ParseMmPageAllocData(MmPageAllocData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gfp_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.migratetype = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmPageAlloc(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmPageAllocData eventData {};
    ParseMmPageAllocData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_page_alloc_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pfn(eventData.pfn);
    msg->set_order(eventData.order);
    msg->set_gfp_flags(eventData.gfp_flags);
    msg->set_migratetype(eventData.migratetype);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_alloc, ParseMmPageAlloc<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_alloc, ParseMmPageAlloc<ProtoEncoder::FtraceEvent>);

struct MmPageAllocExtfragData {
    uint64_t pfn {};
    int32_t alloc_order {};
    int32_t fallback_order {};
    int32_t alloc_migratetype {};
    int32_t fallback_migratetype {};
    int32_t change_ownership {};
};
__attribute__((noinline)) void ParseMmPageAllocExtfragData(MmPageAllocExtfragData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.alloc_order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.fallback_order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.alloc_migratetype = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.fallback_migratetype = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.change_ownership = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmPageAllocExtfrag(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmPageAllocExtfragData eventData {};
    ParseMmPageAllocExtfragData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_page_alloc_extfrag_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pfn(eventData.pfn);
    msg->set_alloc_order(eventData.alloc_order);
    msg->set_fallback_order(eventData.fallback_order);
    msg->set_alloc_migratetype(eventData.alloc_migratetype);
    msg->set_fallback_migratetype(eventData.fallback_migratetype);
    msg->set_change_ownership(eventData.change_ownership);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_alloc_extfrag, ParseMmPageAllocExtfrag<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_alloc_extfrag, ParseMmPageAllocExtfrag<ProtoEncoder::FtraceEvent>);

struct MmPageAllocZoneLockedData {
    uint64_t pfn {};
    uint32_t order {};
    int32_t migratetype {};
};
__attribute__((noinline)) void ParseMmPageAllocZoneLockedData(MmPageAllocZoneLockedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.migratetype = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmPageAllocZoneLocked(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmPageAllocZoneLockedData eventData {};
    ParseMmPageAllocZoneLockedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_page_alloc_zone_locked_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pfn(eventData.pfn);
    msg->set_order(eventData.order);
    msg->set_migratetype(eventData.migratetype);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_alloc_zone_locked, ParseMmPageAllocZoneLocked<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_alloc_zone_locked, ParseMmPageAllocZoneLocked<ProtoEncoder::FtraceEvent>);

struct MmPageFreeData {
    uint64_t pfn {};
    uint32_t order {};
};
__attribute__((noinline)) void ParseMmPageFreeData(MmPageFreeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmPageFree(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmPageFreeData eventData {};
    ParseMmPageFreeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_page_free_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pfn(eventData.pfn);
    msg->set_order(eventData.order);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_free, ParseMmPageFree<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_free, ParseMmPageFree<ProtoEncoder::FtraceEvent>);

struct MmPageFreeBatchedData {
    uint64_t pfn {};
};
__attribute__((noinline)) void ParseMmPageFreeBatchedData(MmPageFreeBatchedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmPageFreeBatched(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmPageFreeBatchedData eventData {};
    ParseMmPageFreeBatchedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_page_free_batched_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pfn(eventData.pfn);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_free_batched, ParseMmPageFreeBatched<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_free_batched, ParseMmPageFreeBatched<ProtoEncoder::FtraceEvent>);

struct MmPagePcpuDrainData {
    uint64_t pfn {};
    uint32_t order {};
    int32_t migratetype {};
};
__attribute__((noinline)) void ParseMmPagePcpuDrainData(MmPagePcpuDrainData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.migratetype = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmPagePcpuDrain(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmPagePcpuDrainData eventData {};
    ParseMmPagePcpuDrainData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_page_pcpu_drain_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pfn(eventData.pfn);
    msg->set_order(eventData.order);
    msg->set_migratetype(eventData.migratetype);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_page_pcpu_drain, ParseMmPagePcpuDrain<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_page_pcpu_drain, ParseMmPagePcpuDrain<ProtoEncoder::FtraceEvent>);

struct RssStatData {
    uint32_t mm_id {};
    uint32_t curr {};
    int32_t member {};
    uint64_t size {};
};
__attribute__((noinline)) void ParseRssStatData(RssStatData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.mm_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.curr = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.member = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.size = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRssStat(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RssStatData eventData {};
    ParseRssStatData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rss_stat_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_mm_id(eventData.mm_id);
    msg->set_curr(eventData.curr);
    msg->set_member(eventData.member);
    msg->set_size(eventData.size);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rss_stat, ParseRssStat<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rss_stat, ParseRssStat<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
