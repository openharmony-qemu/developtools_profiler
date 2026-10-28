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
struct MmCompactionBeginData {
    uint64_t zone_start {};
    uint64_t migrate_pfn {};
    uint64_t free_pfn {};
    uint64_t zone_end {};
    uint32_t sync {};
};
__attribute__((noinline)) void ParseMmCompactionBeginData(MmCompactionBeginData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.zone_start = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.migrate_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.free_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.zone_end = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionBegin(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionBeginData eventData {};
    ParseMmCompactionBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_begin_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_zone_start(eventData.zone_start);
    msg->set_migrate_pfn(eventData.migrate_pfn);
    msg->set_free_pfn(eventData.free_pfn);
    msg->set_zone_end(eventData.zone_end);
    msg->set_sync(eventData.sync);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_begin, ParseMmCompactionBegin<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_begin, ParseMmCompactionBegin<ProtoEncoder::FtraceEvent>);

struct MmCompactionDeferCompactionData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    uint32_t considered {};
    uint32_t defer_shift {};
    int32_t order_failed {};
};
__attribute__((noinline)) void ParseMmCompactionDeferCompactionData(
    MmCompactionDeferCompactionData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.considered = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.defer_shift = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order_failed = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionDeferCompaction(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionDeferCompactionData eventData {};
    ParseMmCompactionDeferCompactionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_defer_compaction_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_defer_compaction, ParseMmCompactionDeferCompaction<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_defer_compaction,
    ParseMmCompactionDeferCompaction<ProtoEncoder::FtraceEvent>);

struct MmCompactionDeferResetData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    uint32_t considered {};
    uint32_t defer_shift {};
    int32_t order_failed {};
};
__attribute__((noinline)) void ParseMmCompactionDeferResetData(
    MmCompactionDeferResetData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.considered = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.defer_shift = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order_failed = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionDeferReset(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionDeferResetData eventData {};
    ParseMmCompactionDeferResetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_defer_reset_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_defer_reset, ParseMmCompactionDeferReset<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_defer_reset,
    ParseMmCompactionDeferReset<ProtoEncoder::FtraceEvent>);

struct MmCompactionDeferredData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    uint32_t considered {};
    uint32_t defer_shift {};
    int32_t order_failed {};
};
__attribute__((noinline)) void ParseMmCompactionDeferredData(
    MmCompactionDeferredData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.considered = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.defer_shift = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order_failed = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionDeferred(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionDeferredData eventData {};
    ParseMmCompactionDeferredData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_deferred_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_deferred, ParseMmCompactionDeferred<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_deferred,
    ParseMmCompactionDeferred<ProtoEncoder::FtraceEvent>);

struct MmCompactionEndData {
    uint64_t zone_start {};
    uint64_t migrate_pfn {};
    uint64_t free_pfn {};
    uint64_t zone_end {};
    uint32_t sync {};
    int32_t status {};
};
__attribute__((noinline)) void ParseMmCompactionEndData(MmCompactionEndData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.zone_start = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.migrate_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.free_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.zone_end = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.sync = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionEnd(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionEndData eventData {};
    ParseMmCompactionEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_end_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_zone_start(eventData.zone_start);
    msg->set_migrate_pfn(eventData.migrate_pfn);
    msg->set_free_pfn(eventData.free_pfn);
    msg->set_zone_end(eventData.zone_end);
    msg->set_sync(eventData.sync);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_end, ParseMmCompactionEnd<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_end, ParseMmCompactionEnd<ProtoEncoder::FtraceEvent>);

struct MmCompactionFinishedData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseMmCompactionFinishedData(
    MmCompactionFinishedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionFinished(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionFinishedData eventData {};
    ParseMmCompactionFinishedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_finished_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_finished, ParseMmCompactionFinished<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_finished,
    ParseMmCompactionFinished<ProtoEncoder::FtraceEvent>);

struct MmCompactionIsolateFreepagesData {
    uint64_t start_pfn {};
    uint64_t end_pfn {};
    uint64_t nr_scanned {};
    uint64_t nr_taken {};
};
__attribute__((noinline)) void ParseMmCompactionIsolateFreepagesData(
    MmCompactionIsolateFreepagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.start_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.end_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_scanned = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_taken = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionIsolateFreepages(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionIsolateFreepagesData eventData {};
    ParseMmCompactionIsolateFreepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_freepages_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_start_pfn(eventData.start_pfn);
    msg->set_end_pfn(eventData.end_pfn);
    msg->set_nr_scanned(eventData.nr_scanned);
    msg->set_nr_taken(eventData.nr_taken);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_isolate_freepages, ParseMmCompactionIsolateFreepages<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_isolate_freepages,
    ParseMmCompactionIsolateFreepages<ProtoEncoder::FtraceEvent>);

struct MmCompactionIsolateMigratepagesData {
    uint64_t start_pfn {};
    uint64_t end_pfn {};
    uint64_t nr_scanned {};
    uint64_t nr_taken {};
};
__attribute__((noinline)) void ParseMmCompactionIsolateMigratepagesData(
    MmCompactionIsolateMigratepagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.start_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.end_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_scanned = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_taken = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionIsolateMigratepages(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionIsolateMigratepagesData eventData {};
    ParseMmCompactionIsolateMigratepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_migratepages_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_start_pfn(eventData.start_pfn);
    msg->set_end_pfn(eventData.end_pfn);
    msg->set_nr_scanned(eventData.nr_scanned);
    msg->set_nr_taken(eventData.nr_taken);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_isolate_migratepages,
    ParseMmCompactionIsolateMigratepages<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_isolate_migratepages,
    ParseMmCompactionIsolateMigratepages<ProtoEncoder::FtraceEvent>);

struct MmCompactionMigratepagesData {
    uint64_t nr_migrated {};
    uint64_t nr_failed {};
};
__attribute__((noinline)) void ParseMmCompactionMigratepagesData(
    MmCompactionMigratepagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nr_migrated = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_failed = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionMigratepages(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionMigratepagesData eventData {};
    ParseMmCompactionMigratepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_migratepages_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nr_migrated(eventData.nr_migrated);
    msg->set_nr_failed(eventData.nr_failed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_migratepages, ParseMmCompactionMigratepages<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_migratepages,
    ParseMmCompactionMigratepages<ProtoEncoder::FtraceEvent>);

struct MmCompactionSuitableData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseMmCompactionSuitableData(
    MmCompactionSuitableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionSuitable(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionSuitableData eventData {};
    ParseMmCompactionSuitableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_suitable_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_suitable, ParseMmCompactionSuitable<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_suitable,
    ParseMmCompactionSuitable<ProtoEncoder::FtraceEvent>);

struct MmCompactionTryToCompactPagesData {
    int32_t order {};
    uint32_t gfp_mask {};
    int32_t prio {};
};
__attribute__((noinline)) void ParseMmCompactionTryToCompactPagesData(
    MmCompactionTryToCompactPagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.gfp_mask = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmCompactionTryToCompactPages(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    MmCompactionTryToCompactPagesData eventData {};
    ParseMmCompactionTryToCompactPagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_try_to_compact_pages_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_order(eventData.order);
    msg->set_gfp_mask(eventData.gfp_mask);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_try_to_compact_pages,
    ParseMmCompactionTryToCompactPages<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_try_to_compact_pages,
    ParseMmCompactionTryToCompactPages<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
