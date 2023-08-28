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

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
template <typename T> void mm_shrink_slab_end_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_shr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_shrink(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_unused_scan(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_new_scan(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_retval(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_total_scan(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_shrink_slab_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_shrink_slab_end_format();
        mm_shrink_slab_end_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_shrink_slab_end,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_shrink_slab_end_format();
        mm_shrink_slab_end_func(msg, data, size, format);
    });

template <typename T> void mm_shrink_slab_start_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_shr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_shrink(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_nr_objects_to_shrink(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_cache_items(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_delta(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_total_scan(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_priority(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_shrink_slab_start,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_shrink_slab_start_format();
        mm_shrink_slab_start_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_shrink_slab_start,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_shrink_slab_start_format();
        mm_shrink_slab_start_func(msg, data, size, format);
    });

template <typename T>
void mm_vmscan_direct_reclaim_begin_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_may_writepage(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_classzone_idx(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_direct_reclaim_begin,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_direct_reclaim_begin_format();
        mm_vmscan_direct_reclaim_begin_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_direct_reclaim_begin,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_direct_reclaim_begin_format();
        mm_vmscan_direct_reclaim_begin_func(msg, data, size, format);
    });

template <typename T>
void mm_vmscan_direct_reclaim_end_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nr_reclaimed(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_direct_reclaim_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_direct_reclaim_end_format();
        mm_vmscan_direct_reclaim_end_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_direct_reclaim_end,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_direct_reclaim_end_format();
        mm_vmscan_direct_reclaim_end_func(msg, data, size, format);
    });

template <typename T> void mm_vmscan_kswapd_sleep_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_kswapd_sleep,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_kswapd_sleep_format();
        mm_vmscan_kswapd_sleep_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_kswapd_sleep,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_kswapd_sleep_format();
        mm_vmscan_kswapd_sleep_func(msg, data, size, format);
    });

template <typename T> void mm_vmscan_kswapd_wake_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_zid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_kswapd_wake,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_kswapd_wake_format();
        mm_vmscan_kswapd_wake_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_kswapd_wake,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_kswapd_wake_format();
        mm_vmscan_kswapd_wake_func(msg, data, size, format);
    });

template <typename T> void mm_vmscan_lru_isolate_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_classzone_idx(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_nr_requested(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_scanned(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_skipped(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_taken(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_isolate_mode(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_lru(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_lru_isolate,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_lru_isolate_format();
        mm_vmscan_lru_isolate_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_lru_isolate,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_lru_isolate_format();
        mm_vmscan_lru_isolate_func(msg, data, size, format);
    });

template <typename T>
void mm_vmscan_lru_shrink_inactive_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_nr_scanned(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_reclaimed(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_dirty(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_writeback(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_congested(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_immediate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_activate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_ref_keep(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_unmap_fail(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_priority(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_reclaim_flags(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_lru_shrink_inactive,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_lru_shrink_inactive_format();
        mm_vmscan_lru_shrink_inactive_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_lru_shrink_inactive,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_lru_shrink_inactive_format();
        mm_vmscan_lru_shrink_inactive_func(msg, data, size, format);
    });

template <typename T> void mm_vmscan_wakeup_kswapd_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_zid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_gfp_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_wakeup_kswapd,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_wakeup_kswapd_format();
        mm_vmscan_wakeup_kswapd_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_wakeup_kswapd,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_wakeup_kswapd_format();
        mm_vmscan_wakeup_kswapd_func(msg, data, size, format);
    });

template <typename T> void mm_vmscan_writepage_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_reclaim_flags(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_vmscan_writepage,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_writepage_format();
        mm_vmscan_writepage_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_vmscan_writepage,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_vmscan_writepage_format();
        mm_vmscan_writepage_func(msg, data, size, format);
    });
} // namespace
FTRACE_NS_END
