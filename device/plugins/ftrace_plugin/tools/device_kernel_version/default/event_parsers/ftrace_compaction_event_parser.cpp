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
void ParseMmCompactionBegin(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionBeginData eventData {};
    ParseMmCompactionBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_begin_format();
    msg->set_zone_start(eventData.zone_start);
    msg->set_migrate_pfn(eventData.migrate_pfn);
    msg->set_free_pfn(eventData.free_pfn);
    msg->set_zone_end(eventData.zone_end);
    msg->set_sync(eventData.sync);
}
void ParseMmCompactionBeginEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionBeginData eventData {};
    ParseMmCompactionBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_begin_format();
    msg->set_zone_start(eventData.zone_start);
    msg->set_migrate_pfn(eventData.migrate_pfn);
    msg->set_free_pfn(eventData.free_pfn);
    msg->set_zone_end(eventData.zone_end);
    msg->set_sync(eventData.sync);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_begin, ParseMmCompactionBegin);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_begin, ParseMmCompactionBeginEncoder);

struct MmCompactionDeferCompactionData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    uint32_t considered {};
    uint32_t defer_shift {};
    int32_t order_failed {};
};
__attribute__((noinline)) void ParseMmCompactionDeferCompactionData(MmCompactionDeferCompactionData& eventData, uint8_t data[], size_t size,
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
void ParseMmCompactionDeferCompaction(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionDeferCompactionData eventData {};
    ParseMmCompactionDeferCompactionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_defer_compaction_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
void ParseMmCompactionDeferCompactionEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionDeferCompactionData eventData {};
    ParseMmCompactionDeferCompactionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_defer_compaction_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_defer_compaction, ParseMmCompactionDeferCompaction);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_defer_compaction, ParseMmCompactionDeferCompactionEncoder);

struct MmCompactionDeferResetData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    uint32_t considered {};
    uint32_t defer_shift {};
    int32_t order_failed {};
};
__attribute__((noinline)) void ParseMmCompactionDeferResetData(MmCompactionDeferResetData& eventData, uint8_t data[], size_t size,
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
void ParseMmCompactionDeferReset(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionDeferResetData eventData {};
    ParseMmCompactionDeferResetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_defer_reset_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
void ParseMmCompactionDeferResetEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionDeferResetData eventData {};
    ParseMmCompactionDeferResetData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_defer_reset_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_defer_reset, ParseMmCompactionDeferReset);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_defer_reset, ParseMmCompactionDeferResetEncoder);

struct MmCompactionDeferredData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    uint32_t considered {};
    uint32_t defer_shift {};
    int32_t order_failed {};
};
__attribute__((noinline)) void ParseMmCompactionDeferredData(MmCompactionDeferredData& eventData, uint8_t data[], size_t size,
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
void ParseMmCompactionDeferred(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionDeferredData eventData {};
    ParseMmCompactionDeferredData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_deferred_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
void ParseMmCompactionDeferredEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionDeferredData eventData {};
    ParseMmCompactionDeferredData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_deferred_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_considered(eventData.considered);
    msg->set_defer_shift(eventData.defer_shift);
    msg->set_order_failed(eventData.order_failed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_deferred, ParseMmCompactionDeferred);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_deferred, ParseMmCompactionDeferredEncoder);

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
void ParseMmCompactionEnd(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionEndData eventData {};
    ParseMmCompactionEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_end_format();
    msg->set_zone_start(eventData.zone_start);
    msg->set_migrate_pfn(eventData.migrate_pfn);
    msg->set_free_pfn(eventData.free_pfn);
    msg->set_zone_end(eventData.zone_end);
    msg->set_sync(eventData.sync);
    msg->set_status(eventData.status);
}
void ParseMmCompactionEndEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionEndData eventData {};
    ParseMmCompactionEndData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_end_format();
    msg->set_zone_start(eventData.zone_start);
    msg->set_migrate_pfn(eventData.migrate_pfn);
    msg->set_free_pfn(eventData.free_pfn);
    msg->set_zone_end(eventData.zone_end);
    msg->set_sync(eventData.sync);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_end, ParseMmCompactionEnd);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_end, ParseMmCompactionEndEncoder);

struct MmCompactionFinishedData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseMmCompactionFinishedData(MmCompactionFinishedData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseMmCompactionFinished(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionFinishedData eventData {};
    ParseMmCompactionFinishedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_finished_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_ret(eventData.ret);
}
void ParseMmCompactionFinishedEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionFinishedData eventData {};
    ParseMmCompactionFinishedData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_finished_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_finished, ParseMmCompactionFinished);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_finished, ParseMmCompactionFinishedEncoder);

struct MmCompactionIsolateFreepagesData {
    uint64_t start_pfn {};
    uint64_t end_pfn {};
    uint64_t nr_scanned {};
    uint64_t nr_taken {};
};
__attribute__((noinline)) void ParseMmCompactionIsolateFreepagesData(MmCompactionIsolateFreepagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.start_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.end_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_scanned = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_taken = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseMmCompactionIsolateFreepages(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionIsolateFreepagesData eventData {};
    ParseMmCompactionIsolateFreepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_freepages_format();
    msg->set_start_pfn(eventData.start_pfn);
    msg->set_end_pfn(eventData.end_pfn);
    msg->set_nr_scanned(eventData.nr_scanned);
    msg->set_nr_taken(eventData.nr_taken);
}
void ParseMmCompactionIsolateFreepagesEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionIsolateFreepagesData eventData {};
    ParseMmCompactionIsolateFreepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_freepages_format();
    msg->set_start_pfn(eventData.start_pfn);
    msg->set_end_pfn(eventData.end_pfn);
    msg->set_nr_scanned(eventData.nr_scanned);
    msg->set_nr_taken(eventData.nr_taken);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_isolate_freepages, ParseMmCompactionIsolateFreepages);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_isolate_freepages, ParseMmCompactionIsolateFreepagesEncoder);

struct MmCompactionIsolateMigratepagesData {
    uint64_t start_pfn {};
    uint64_t end_pfn {};
    uint64_t nr_scanned {};
    uint64_t nr_taken {};
};
__attribute__((noinline)) void ParseMmCompactionIsolateMigratepagesData(MmCompactionIsolateMigratepagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.start_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.end_pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_scanned = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_taken = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseMmCompactionIsolateMigratepages(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionIsolateMigratepagesData eventData {};
    ParseMmCompactionIsolateMigratepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_migratepages_format();
    msg->set_start_pfn(eventData.start_pfn);
    msg->set_end_pfn(eventData.end_pfn);
    msg->set_nr_scanned(eventData.nr_scanned);
    msg->set_nr_taken(eventData.nr_taken);
}
void ParseMmCompactionIsolateMigratepagesEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionIsolateMigratepagesData eventData {};
    ParseMmCompactionIsolateMigratepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_migratepages_format();
    msg->set_start_pfn(eventData.start_pfn);
    msg->set_end_pfn(eventData.end_pfn);
    msg->set_nr_scanned(eventData.nr_scanned);
    msg->set_nr_taken(eventData.nr_taken);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_isolate_migratepages, ParseMmCompactionIsolateMigratepages);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_isolate_migratepages, ParseMmCompactionIsolateMigratepagesEncoder);

struct MmCompactionMigratepagesData {
    uint64_t nr_migrated {};
    uint64_t nr_failed {};
};
__attribute__((noinline)) void ParseMmCompactionMigratepagesData(MmCompactionMigratepagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nr_migrated = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.nr_failed = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseMmCompactionMigratepages(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionMigratepagesData eventData {};
    ParseMmCompactionMigratepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_migratepages_format();
    msg->set_nr_migrated(eventData.nr_migrated);
    msg->set_nr_failed(eventData.nr_failed);
}
void ParseMmCompactionMigratepagesEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionMigratepagesData eventData {};
    ParseMmCompactionMigratepagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_migratepages_format();
    msg->set_nr_migrated(eventData.nr_migrated);
    msg->set_nr_failed(eventData.nr_failed);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_migratepages, ParseMmCompactionMigratepages);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_migratepages, ParseMmCompactionMigratepagesEncoder);

struct MmCompactionSuitableData {
    int32_t nid {};
    uint32_t idx {};
    int32_t order {};
    int32_t ret {};
};
__attribute__((noinline)) void ParseMmCompactionSuitableData(MmCompactionSuitableData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.nid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.idx = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.ret = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseMmCompactionSuitable(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionSuitableData eventData {};
    ParseMmCompactionSuitableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_suitable_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_ret(eventData.ret);
}
void ParseMmCompactionSuitableEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionSuitableData eventData {};
    ParseMmCompactionSuitableData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_suitable_format();
    msg->set_nid(eventData.nid);
    msg->set_idx(eventData.idx);
    msg->set_order(eventData.order);
    msg->set_ret(eventData.ret);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_suitable, ParseMmCompactionSuitable);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_suitable, ParseMmCompactionSuitableEncoder);

struct MmCompactionTryToCompactPagesData {
    int32_t order {};
    uint32_t gfp_mask {};
    int32_t prio {};
};
__attribute__((noinline)) void ParseMmCompactionTryToCompactPagesData(MmCompactionTryToCompactPagesData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.order = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.gfp_mask = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.prio = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseMmCompactionTryToCompactPages(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionTryToCompactPagesData eventData {};
    ParseMmCompactionTryToCompactPagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_try_to_compact_pages_format();
    msg->set_order(eventData.order);
    msg->set_gfp_mask(eventData.gfp_mask);
    msg->set_prio(eventData.prio);
}
void ParseMmCompactionTryToCompactPagesEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    MmCompactionTryToCompactPagesData eventData {};
    ParseMmCompactionTryToCompactPagesData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_compaction_try_to_compact_pages_format();
    msg->set_order(eventData.order);
    msg->set_gfp_mask(eventData.gfp_mask);
    msg->set_prio(eventData.prio);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_try_to_compact_pages, ParseMmCompactionTryToCompactPages);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_try_to_compact_pages, ParseMmCompactionTryToCompactPagesEncoder);
} // namespace
FTRACE_NS_END
