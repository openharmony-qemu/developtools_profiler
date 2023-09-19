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
template <typename T> void mm_compaction_begin_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_zone_start(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_migrate_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_free_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_zone_end(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_sync(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_begin,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_begin_format();
        mm_compaction_begin_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_begin,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_begin_format();
        mm_compaction_begin_func(msg, data, size, format);
    });

template <typename T>
void mm_compaction_defer_compaction_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_idx(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_considered(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_defer_shift(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_order_failed(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_defer_compaction,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_defer_compaction_format();
        mm_compaction_defer_compaction_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_defer_compaction,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_defer_compaction_format();
        mm_compaction_defer_compaction_func(msg, data, size, format);
    });

template <typename T>
void mm_compaction_defer_reset_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_idx(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_considered(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_defer_shift(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_order_failed(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_defer_reset,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_defer_reset_format();
        mm_compaction_defer_reset_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_defer_reset,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_defer_reset_format();
        mm_compaction_defer_reset_func(msg, data, size, format);
    });

template <typename T> void mm_compaction_deferred_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_idx(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_considered(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_defer_shift(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_order_failed(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_deferred,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_deferred_format();
        mm_compaction_deferred_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_deferred,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_deferred_format();
        mm_compaction_deferred_func(msg, data, size, format);
    });

template <typename T> void mm_compaction_end_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_zone_start(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_migrate_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_free_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_zone_end(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_sync(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_end,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_end_format();
        mm_compaction_end_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_end,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_end_format();
        mm_compaction_end_func(msg, data, size, format);
    });

template <typename T> void mm_compaction_finished_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_idx(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_finished,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_finished_format();
        mm_compaction_finished_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_finished,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_finished_format();
        mm_compaction_finished_func(msg, data, size, format);
    });

template <typename T>
void mm_compaction_isolate_freepages_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_start_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_end_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_scanned(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_taken(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_isolate_freepages,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_isolate_freepages_format();
        mm_compaction_isolate_freepages_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_isolate_freepages,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_isolate_freepages_format();
        mm_compaction_isolate_freepages_func(msg, data, size, format);
    });

template <typename T>
void mm_compaction_isolate_migratepages_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_start_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_end_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_scanned(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_taken(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_isolate_migratepages,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_isolate_migratepages_format();
        mm_compaction_isolate_migratepages_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_isolate_migratepages,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_isolate_migratepages_format();
        mm_compaction_isolate_migratepages_func(msg, data, size, format);
    });

template <typename T>
void mm_compaction_migratepages_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nr_migrated(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_failed(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_migratepages,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_migratepages_format();
        mm_compaction_migratepages_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_migratepages,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_migratepages_format();
        mm_compaction_migratepages_func(msg, data, size, format);
    });

template <typename T> void mm_compaction_suitable_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_nid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_idx(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_ret(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_suitable,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_suitable_format();
        mm_compaction_suitable_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_suitable,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_suitable_format();
        mm_compaction_suitable_func(msg, data, size, format);
    });

template <typename T>
void mm_compaction_try_to_compact_pages_func(T& msg, uint8_t data[], size_t size, const EventFormat& format)
{
    int i = 0;
    msg->set_order(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_gfp_mask(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_prio(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_try_to_compact_pages,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_try_to_compact_pages_format();
        mm_compaction_try_to_compact_pages_func(msg, data, size, format);
    });
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_try_to_compact_pages,
    [](ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        auto msg = ftraceEvent.mutable_mm_compaction_try_to_compact_pages_format();
        mm_compaction_try_to_compact_pages_func(msg, data, size, format);
    });
} // namespace
FTRACE_NS_END
