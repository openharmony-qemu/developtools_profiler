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
struct MmShrinkSlabEndData {
    uint64_t shr {};
    int32_t nid {};
    uint64_t shrink {};
    uint64_t unused_scan {};
    uint64_t new_scan {};
    int32_t retval {};
    uint64_t total_scan {};
};
__attribute__((noinline)) void ParseMmShrinkSlabEndData(MmShrinkSlabEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.shr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.shrink = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.unused_scan = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.new_scan = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.retval = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.total_scan = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmShrinkSlabEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmShrinkSlabEndData eventData {};
    ParseMmShrinkSlabEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_shrink_slab_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_shr(eventData.shr);
    msg->set_nid(eventData.nid);
    msg->set_shrink(eventData.shrink);
    msg->set_unused_scan(eventData.unused_scan);
    msg->set_new_scan(eventData.new_scan);
    msg->set_retval(eventData.retval);
    msg->set_total_scan(eventData.total_scan);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_shrink_slab_end, ParseMmShrinkSlabEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_shrink_slab_end, ParseMmShrinkSlabEnd<ProtoEncoder::FtraceEvent>);

struct MmShrinkSlabStartData {
    uint64_t shr {};
    uint64_t shrink {};
    int32_t nid {};
    uint64_t nr_objects_to_shrink {};
    uint32_t gfp_flags {};
    uint64_t cache_items {};
    uint64_t delta {};
    uint64_t total_scan {};
    int32_t priority {};
};
__attribute__((noinline)) void ParseMmShrinkSlabStartData(MmShrinkSlabStartData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.shr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.shrink = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.nr_objects_to_shrink = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.gfp_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.cache_items = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.delta = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.total_scan = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.priority = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmShrinkSlabStart(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmShrinkSlabStartData eventData {};
    ParseMmShrinkSlabStartData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_shrink_slab_start_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_shr(eventData.shr);
    msg->set_shrink(eventData.shrink);
    msg->set_nid(eventData.nid);
    msg->set_nr_objects_to_shrink(eventData.nr_objects_to_shrink);
    msg->set_gfp_flags(eventData.gfp_flags);
    msg->set_cache_items(eventData.cache_items);
    msg->set_delta(eventData.delta);
    msg->set_total_scan(eventData.total_scan);
    msg->set_priority(eventData.priority);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_shrink_slab_start, ParseMmShrinkSlabStart<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_shrink_slab_start, ParseMmShrinkSlabStart<ProtoEncoder::FtraceEvent>);

struct MmVmscanDirectReclaimBeginData {
    int32_t order {};
    int32_t may_writepage {};
    uint32_t gfp_flags {};
    int32_t classzone_idx {};
};
__attribute__((noinline)) void ParseMmVmscanDirectReclaimBeginData(
    MmVmscanDirectReclaimBeginData& eventData, uint8_t data[], size_t size,
    const EventFormat& format)
{
    int i = 0;
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.may_writepage = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.gfp_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.classzone_idx = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmVmscanDirectReclaimBegin(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmVmscanDirectReclaimBeginData eventData {};
    ParseMmVmscanDirectReclaimBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_vmscan_direct_reclaim_begin_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_order(eventData.order);
    msg->set_may_writepage(eventData.may_writepage);
    msg->set_gfp_flags(eventData.gfp_flags);
    msg->set_classzone_idx(eventData.classzone_idx);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_direct_reclaim_begin, ParseMmVmscanDirectReclaimBegin<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_direct_reclaim_begin,
    ParseMmVmscanDirectReclaimBegin<ProtoEncoder::FtraceEvent>);

struct MmVmscanDirectReclaimEndData {
    uint64_t nr_reclaimed {};
};
__attribute__((noinline)) void ParseMmVmscanDirectReclaimEndData(
    MmVmscanDirectReclaimEndData& eventData, uint8_t data[], size_t size,
    const EventFormat& format)
{
    int i = 0;
    eventData.nr_reclaimed = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmVmscanDirectReclaimEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmVmscanDirectReclaimEndData eventData {};
    ParseMmVmscanDirectReclaimEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_vmscan_direct_reclaim_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nr_reclaimed(eventData.nr_reclaimed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_direct_reclaim_end, ParseMmVmscanDirectReclaimEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_direct_reclaim_end,
    ParseMmVmscanDirectReclaimEnd<ProtoEncoder::FtraceEvent>);

struct MmVmscanKswapdSleepData {
    int32_t nid {};
};
__attribute__((noinline)) void ParseMmVmscanKswapdSleepData(
    MmVmscanKswapdSleepData& eventData, uint8_t data[], size_t size,
    const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmVmscanKswapdSleep(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmVmscanKswapdSleepData eventData {};
    ParseMmVmscanKswapdSleepData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_vmscan_kswapd_sleep_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_kswapd_sleep, ParseMmVmscanKswapdSleep<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_kswapd_sleep,
    ParseMmVmscanKswapdSleep<ProtoEncoder::FtraceEvent>);

struct MmVmscanKswapdWakeData {
    int32_t nid {};
    int32_t zid {};
    int32_t order {};
};
__attribute__((noinline)) void ParseMmVmscanKswapdWakeData(
    MmVmscanKswapdWakeData& eventData, uint8_t data[], size_t size,
    const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.zid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmVmscanKswapdWake(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmVmscanKswapdWakeData eventData {};
    ParseMmVmscanKswapdWakeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_vmscan_kswapd_wake_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
    msg->set_zid(eventData.zid);
    msg->set_order(eventData.order);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_kswapd_wake, ParseMmVmscanKswapdWake<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_kswapd_wake, ParseMmVmscanKswapdWake<ProtoEncoder::FtraceEvent>);

struct MmVmscanLruIsolateData {
    int32_t classzone_idx {};
    int32_t order {};
    uint64_t nr_requested {};
    uint64_t nr_scanned {};
    uint64_t nr_skipped {};
    uint64_t nr_taken {};
    uint32_t isolate_mode {};
    int32_t lru {};
};
__attribute__((noinline)) void ParseMmVmscanLruIsolateData(
    MmVmscanLruIsolateData& eventData, uint8_t data[], size_t size,
    const EventFormat& format)
{
    int i = 0;
    eventData.classzone_idx = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.nr_requested = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_scanned = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_skipped = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_taken = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.isolate_mode = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.lru = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmVmscanLruIsolate(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmVmscanLruIsolateData eventData {};
    ParseMmVmscanLruIsolateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_vmscan_lru_isolate_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_classzone_idx(eventData.classzone_idx);
    msg->set_order(eventData.order);
    msg->set_nr_requested(eventData.nr_requested);
    msg->set_nr_scanned(eventData.nr_scanned);
    msg->set_nr_skipped(eventData.nr_skipped);
    msg->set_nr_taken(eventData.nr_taken);
    msg->set_isolate_mode(eventData.isolate_mode);
    msg->set_lru(eventData.lru);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_lru_isolate, ParseMmVmscanLruIsolate<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_lru_isolate, ParseMmVmscanLruIsolate<ProtoEncoder::FtraceEvent>);

struct MmVmscanLruShrinkInactiveData {
    int32_t nid {};
    uint64_t nr_scanned {};
    uint64_t nr_reclaimed {};
    uint64_t nr_dirty {};
    uint64_t nr_writeback {};
    uint64_t nr_congested {};
    uint64_t nr_immediate {};
    uint64_t nr_activate {};
    uint64_t nr_ref_keep {};
    uint64_t nr_unmap_fail {};
    int32_t priority {};
    int32_t reclaim_flags {};
};
__attribute__((noinline)) void ParseMmVmscanLruShrinkInactiveData(
    MmVmscanLruShrinkInactiveData& eventData, uint8_t data[], size_t size,
    const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.nr_scanned = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_reclaimed = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_dirty = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_writeback = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_congested = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_immediate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_activate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_ref_keep = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_unmap_fail = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.priority = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.reclaim_flags = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmVmscanLruShrinkInactive(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmVmscanLruShrinkInactiveData eventData {};
    ParseMmVmscanLruShrinkInactiveData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_vmscan_lru_shrink_inactive_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
    msg->set_nr_scanned(eventData.nr_scanned);
    msg->set_nr_reclaimed(eventData.nr_reclaimed);
    msg->set_nr_dirty(eventData.nr_dirty);
    msg->set_nr_writeback(eventData.nr_writeback);
    msg->set_nr_congested(eventData.nr_congested);
    msg->set_nr_immediate(eventData.nr_immediate);
    msg->set_nr_activate(eventData.nr_activate);
    msg->set_nr_ref_keep(eventData.nr_ref_keep);
    msg->set_nr_unmap_fail(eventData.nr_unmap_fail);
    msg->set_priority(eventData.priority);
    msg->set_reclaim_flags(eventData.reclaim_flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_lru_shrink_inactive, ParseMmVmscanLruShrinkInactive<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_lru_shrink_inactive,
    ParseMmVmscanLruShrinkInactive<ProtoEncoder::FtraceEvent>);

struct MmVmscanWakeupKswapdData {
    int32_t nid {};
    int32_t zid {};
    int32_t order {};
    uint32_t gfp_flags {};
};
__attribute__((noinline)) void ParseMmVmscanWakeupKswapdData(
    MmVmscanWakeupKswapdData& eventData, uint8_t data[], size_t size,
    const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.zid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.gfp_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmVmscanWakeupKswapd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmVmscanWakeupKswapdData eventData {};
    ParseMmVmscanWakeupKswapdData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_vmscan_wakeup_kswapd_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
    msg->set_zid(eventData.zid);
    msg->set_order(eventData.order);
    msg->set_gfp_flags(eventData.gfp_flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_wakeup_kswapd, ParseMmVmscanWakeupKswapd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_wakeup_kswapd,
    ParseMmVmscanWakeupKswapd<ProtoEncoder::FtraceEvent>);

struct MmVmscanWritepageData {
    uint64_t pfn {};
    int32_t reclaim_flags {};
};
__attribute__((noinline)) void ParseMmVmscanWritepageData(MmVmscanWritepageData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.reclaim_flags = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmVmscanWritepage(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmVmscanWritepageData eventData {};
    ParseMmVmscanWritepageData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_vmscan_writepage_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pfn(eventData.pfn);
    msg->set_reclaim_flags(eventData.reclaim_flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_writepage, ParseMmVmscanWritepage<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_writepage, ParseMmVmscanWritepage<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
