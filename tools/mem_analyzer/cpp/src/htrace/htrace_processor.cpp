/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include "htrace/htrace_processor.h"
#include "htrace/htrace_category.h"
#include "utils/platform.h"
#include "utils/file_utils.h"
#include "utils/logger.h"
#include "utils/timer.h"
#include "shared/htrace_model/htrace_db_helper.h"
#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <set>
#include <cstdlib>
#include <algorithm>
#include <tuple>

namespace mem_analyzer {

constexpr int COL_SUB_TYPE_ID = 3;

std::vector<EventTypeMapping> DefaultEventMappings()
{
    return {
        {"AllocEvent", HtraceEventType::MALLOC},
        {"MmapEvent", HtraceEventType::MMAP_ANON, SubTypeCondition::IS_NULL},      // MAP_ANONYMOUS: sub_type_id IS NULL
        // file/page mapping: sub_type_id IS NOT NULL
        {"MmapEvent", HtraceEventType::FILE_PAGE_MSG, SubTypeCondition::IS_NOT_NULL},
        {"FD_Open_Event", HtraceEventType::HP_FD},
        {"THREAD_Create_Event", HtraceEventType::HP_THREAD},
        {"ASHMEM_Alloc_Event", HtraceEventType::ASHMEM},
        {"ION_Alloc_Event", HtraceEventType::HP_DMA},
        {"ARK_GLOBAL_HANDLE_Alloc_Event", HtraceEventType::ARK_GLOBAL_HANDLE},
        {"ARK_LOCAL_HANDLE_Alloc_Event", HtraceEventType::ARK_LOCAL_HANDLE},
        // Future: {"MapAllocEvent", HtraceEventType::MAP_ALLOC}, etc.
    };
}

// ── Helper: 计算某 category（+是否过滤）下仍启用的 HtraceEventType ──
static std::vector<HtraceEventType> AllowedEventTypes(const std::string& category, bool filterCategory)
{
    if (!filterCategory) {
        // parse-only：不按 category 过滤，全部启用的类型纳入，保持升序（与历史一致）。
        std::vector<HtraceEventType> allowed;
        for (HtraceEventType et : HtraceEnabledTypes()) {
            allowed.push_back(et);
        }
        std::sort(allowed.begin(), allowed.end(),
            [](HtraceEventType a, HtraceEventType b) {
                return static_cast<int>(a) < static_cast<int>(b);
            });
        return allowed;
    }
    return HtraceCategory(category).eventTypes;  // 描述符表内已升序
}

// ── Build CompressedCallStack from frames ──
static CompressedCallStack BuildCompressedCallStack(
    int callchainId, long long totalSize, long long totalCount,
    HtraceEventType typeVal, int subTypeId,
    const std::unordered_map<int, std::vector<FrameRow>>& framesByChain)
{
    CompressedCallStack cs;
    cs.id = callchainId;
    cs.type = typeVal;
    cs.subTypeId = subTypeId;
    cs.existingSize = totalSize;
    cs.existingCount = totalCount;

    auto it = framesByChain.find(callchainId);
    if (it != framesByChain.end()) {
        cs.frames.reserve(it->second.size());
        for (const auto& fr : it->second) {
            CompressedStackFrame sf;
            sf.depth = fr.depth;
            sf.symbolId = fr.symbolId;
            sf.fileIdNull = fr.fileIdNull;
            sf.fileId = fr.fileId;
            sf.vaddrNull = fr.vaddrNull;
            sf.vaddr = fr.vaddr;
            sf.buildIdNull = fr.buildIdNull;
            sf.buildId = fr.buildId;
            cs.frames.push_back(std::move(sf));
        }
    }
    return cs;
}

// ── 拆分后的 processor 内部件（文件局部）──────────────────────────────
namespace {

// 解析上下文：db 句柄 + meta + data_dict；析构关闭 db（RAII，去掉原两处 sqlite3_close）。
struct HtraceParseContext {
    sqlite3* db = nullptr;
    TraceMeta meta;
    std::unordered_map<int, std::string> dataDict;

    HtraceParseContext() = default;
    HtraceParseContext(const HtraceParseContext&) = delete;
    HtraceParseContext& operator=(const HtraceParseContext&) = delete;
    ~HtraceParseContext()
    {
        if (db) {
            sqlite3_close(db);
        }
    }
};

// 拼 event type IN 列表（statistic 主 SQL 与帧子查询复用）。
std::string BuildTypeInList(const std::vector<HtraceEventType>& types)
{
    std::string typeIn;
    for (HtraceEventType et : types) {
        if (!typeIn.empty()) {
            typeIn += ",";
        }
        typeIn += std::to_string(static_cast<int>(et));
    }
    return typeIn;
}

// 逗号分隔地址串 → 段列表（statistic 分支 b.data 的 existing_addrs）。
std::vector<std::string> splitCommaSeparated(const std::string& s)
{
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= s.size()) {
        size_t e = s.find(',', start);
        if (e == std::string::npos) {
            e = s.size();
        }
        if (e > start) {
            parts.emplace_back(s.substr(start, e - start));
        }
        if (e == s.size()) {
            break;
        }
        start = e + 1;
    }
    return parts;
}

void VerifyDataDictColumnType(sqlite3_stmt* stmt, bool& typeChecked)
{
    if (typeChecked) {
        return;
    }
    // Verify id column type is INTEGER before processing
    int colType = sqlite3_column_type(stmt, 0);
    if (colType != SQLITE_INTEGER) {
        throw std::runtime_error(
            "data_dict.id column type is not INTEGER (got type " +
            std::to_string(colType) + "), int overflow risk");
    }
    typeChecked = true;
}

// 打开 db + 读 meta + data_dict（就地填充 ctx）。
void OpenHtraceContext(HtraceParseContext& ctx, const std::string& dbPath, StageTimer& timer)
{
    ctx.db = platform::OpenSqlite3Db(dbPath);
    ctx.meta = QueryTraceMeta(ctx.db);

    // Load data_dict
    timer.Stage("Loading data_dict");
    std::string dsql = "SELECT id, data FROM data_dict";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(ctx.db, dsql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        StmtGuard guard(stmt);
        bool typeChecked = false;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            VerifyDataDictColumnType(stmt, typeChecked);
            int id = sqlite3_column_int(stmt, 0);
            auto data = (const char*)sqlite3_column_text(stmt, 1);
            if (data) {
                ctx.dataDict[id] = platform::SanitizeUtf8(data);
            }
        }
    }
    timer.Tick("data_dict");
}

std::string BuildStatisticSql(const std::vector<HtraceEventType>& allowedTypes, bool needAddrs)
{
    std::string typeIn = BuildTypeInList(allowedTypes);
    std::string sql =
        "SELECT ranked.callchain_id, ranked.type, ranked.ts, ranked.sub_type_id, "
        "(ranked.apply_size - ranked.release_size) as total_size, "
        "(ranked.apply_count - ranked.release_count) as total_count, ";
    // 隔离：仅 -l(need_addrs) 才 JOIN data_dict 取地址（依赖 addr_id 列）；
    // 非 -l 用 NULL 占位列，不碰 addr_id，SQL 结构与 master 等价、不依赖该列存在。
    sql += needAddrs ? "b.data as existing_addrs " : "NULL as existing_addrs ";
    sql += "FROM ("
        "  SELECT *, ROW_NUMBER() OVER (PARTITION BY callchain_id, type ORDER BY ts DESC) as rn "
        "  FROM native_hook_statistic WHERE type IN (";
    sql += typeIn;
    sql += ") ";  // 闭合 IN (...)
    sql += ") ranked ";  // 闭合子查询 FROM (...) + 别名
    if (needAddrs) {
        sql += "LEFT JOIN data_dict b ON ranked.addr_id = b.id ";
    }
    sql += "WHERE ranked.rn = 1 AND (ranked.apply_size - ranked.release_size) != 0 "
        "ORDER BY total_size DESC";
    return sql;
}

// statistic 模式：按 (callchain_id, type) 分区聚合，每条链在每个 type 下取最新一条。
void LoadStatisticStacks(HtraceParseContext& ctx,
    const std::vector<HtraceEventType>& allowedTypes,
    bool needAddrs,
    CompressedCallStackList& stacks)
{
    sqlite3* db = ctx.db;
    TraceMeta& meta = ctx.meta;

    std::string sql = BuildStatisticSql(allowedTypes, needAddrs);

    std::vector<std::tuple<int, int, int, int, long long, long long, std::string>> statisticGroups;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Statistic query prepare failed: " +
            std::string(sqlite3_errmsg(db)));
    }
    StmtGuard guard(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
            int ccid = sqlite3_column_int(stmt, 0);
            int typeVal = sqlite3_column_int(stmt, 1);
            int maxTs = sqlite3_column_int(stmt, 2);
            int subTypeId = -1;
            if (sqlite3_column_type(stmt, COL_SUB_TYPE_ID) != SQLITE_NULL) {
                subTypeId = sqlite3_column_int(stmt, COL_SUB_TYPE_ID);
            }
            long long totalSize = sqlite3_column_int64(stmt, 4);
            long long totalCount = sqlite3_column_int64(stmt, 5);
            auto addrsStr = (const char*)sqlite3_column_text(stmt, 6);
            statisticGroups.emplace_back(ccid, typeVal, maxTs, subTypeId, totalSize, totalCount,
                addrsStr ? addrsStr : "");
    }

    log::Logi("  Statistic mode: ", statisticGroups.size(), " groups.");

    if (statisticGroups.empty()) {
        throw std::runtime_error("No records found in native_hook_statistic");
    }

    // Load frames for matching chains (reuse the same type filter in a subquery)
    std::string chainSelect =
        "SELECT callchain_id FROM ("
        "  SELECT *, ROW_NUMBER() OVER (PARTITION BY callchain_id, type ORDER BY ts DESC) AS rn "
        "  FROM native_hook_statistic WHERE type IN (" + BuildTypeInList(allowedTypes) + ") "
        ") WHERE rn = 1 AND (apply_size - release_size) != 0";
    auto framesByChain = ReadFrames(db, chainSelect);

    for (auto& [ccid, typeVal, maxTs, subTypeId, totalSize, totalCount, addrsStr] : statisticGroups) {
        auto evType = static_cast<HtraceEventType>(typeVal);
        if (HtraceIsCountSizeType(evType)) {
            meta.typeSizes[evType] += totalSize;
        }
        auto entry = BuildCompressedCallStack(ccid, totalSize, totalCount,
            static_cast<HtraceEventType>(typeVal), subTypeId, framesByChain);
        if (!addrsStr.empty()) {
            // b.data 是逗号分隔的地址串，split 后填入 existing_addrs
            entry.existingAddrs = std::make_shared<std::vector<std::string>>(
                splitCommaSeparated(addrsStr));
        }
        stacks.stacks.push_back(std::move(entry));
    }
}

struct AllocEntry {
    long long heapSize;
    int count;
    int subTypeId = -1;
};

struct GroupEntry {
    int callchainId;
    long long heapSize;
    int count;
    HtraceEventType htraceType;
    int subTypeId = -1;
};

std::pair<std::string, std::string> BuildDetailSql(const std::vector<EventTypeMapping>& configs)
{
    // Only process alloc events with end_ts IS NULL or 0 (outstanding, not freed).
    // Event_type whitelist is pushed down to SQL; sub_type dispatch is in-memory.
    std::set<std::string> eventTypesSeen;
    std::string eventTypeIn;
    for (auto& c : configs) {
        if (eventTypesSeen.count(c.allocEvent)) {
            continue;
        }
        eventTypesSeen.insert(c.allocEvent);
        if (!eventTypeIn.empty()) {
            eventTypeIn += ",";
        }
        eventTypeIn += "'" + c.allocEvent + "'";
    }
    std::string sql =
        "SELECT event_type, callchain_id, heap_size, sub_type_id FROM native_hook "
        "WHERE (end_ts IS NULL OR end_ts = 0) "
        "AND event_type IN (" + eventTypeIn + ")";
    sql += " ORDER BY start_ts ASC";
    return {eventTypeIn, sql};
}

static bool MatchSubTypeCond(SubTypeCondition cond, int subTypeId)
{
    switch (cond) {
        case SubTypeCondition::ANY:
            return true;
        case SubTypeCondition::IS_NULL:
            return subTypeId == -1;
        case SubTypeCondition::IS_NOT_NULL:
            return subTypeId != -1;
        default:
            return false;
    }
}

void QueryAndAggregateAllocs(sqlite3* db, const std::string& sql,
    const std::vector<EventTypeMapping>& configs,
    const std::map<std::string, std::vector<int>>& allocEventLookup,
    std::vector<std::map<std::pair<int, int>, AllocEntry>>& allocMaps)
{
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }
    StmtGuard guard(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto eventTypeStr = (const char*)sqlite3_column_text(stmt, 0);
        if (!eventTypeStr) {
            continue;
        }
        int ccid = sqlite3_column_int(stmt, 1);
        long long heapSize = sqlite3_column_int64(stmt, 2);
        int subTypeId = -1;
        if (sqlite3_column_type(stmt, COL_SUB_TYPE_ID) != SQLITE_NULL) {
            subTypeId = sqlite3_column_int(stmt, COL_SUB_TYPE_ID);
        }

        auto lookupIt = allocEventLookup.find(eventTypeStr);
        if (lookupIt == allocEventLookup.end()) { // defensive
            continue;
        }

        for (int ci : lookupIt->second) {
            if (!MatchSubTypeCond(configs[ci].subTypeCond, subTypeId)) {
                continue;
            }
            auto key = std::make_pair(ccid, subTypeId);
            auto& entry = allocMaps[ci][key];
            entry.heapSize += heapSize;
            entry.count++;
            entry.subTypeId = subTypeId;
        }
    }
}

void ResolveDetailFrames(HtraceParseContext& ctx, const std::string& eventTypeIn,
    std::vector<GroupEntry>& allGroups, bool needAddrs, CompressedCallStackList& stacks)
{
    sqlite3* db = ctx.db;
    TraceMeta& meta = ctx.meta;

    // Load frames for matching chains (reuse event_type/sub_type filter in a subquery)
    std::string chainSelect =
        "SELECT callchain_id FROM native_hook "
        "WHERE (end_ts IS NULL OR end_ts = 0) AND event_type IN (" + eventTypeIn + ")";
    auto framesByChain = ReadFrames(db, chainSelect);
    // 隔离：仅 -l(need_addrs) 才查 native_hook.addr 聚合地址；非 -l 空 map，循环 find 返回 end 不填
    std::unordered_map<int, std::vector<std::string>> addrsByChain;
    if (needAddrs) {
        addrsByChain = ReadCallchainAddrs(db, chainSelect);
    }

    for (auto& g : allGroups) {
        if (HtraceIsCountSizeType(g.htraceType)) {
            meta.typeSizes[g.htraceType] += g.heapSize;
        }
        auto cs = BuildCompressedCallStack(g.callchainId, g.heapSize, g.count,
            g.htraceType, g.subTypeId, framesByChain);
        auto aIt = addrsByChain.find(g.callchainId);
        if (aIt != addrsByChain.end()) {
            cs.existingAddrs = std::make_shared<std::vector<std::string>>(aIt->second);
        }
        stacks.stacks.push_back(std::move(cs));
    }
}

// detail 模式：AllocEvent 且 end_ts IS NULL/0 为未释放（outstanding）；
// 已释放(end_ts != 0)整体跳过，FreeEvent 无需处理。
void LoadDetailStacks(HtraceParseContext& ctx,
    const std::vector<HtraceEventType>& allowedTypes,
    bool needAddrs,
    CompressedCallStackList& stacks)
{
    sqlite3* db = ctx.db;

    // One alloc_map per event type mapping (filtered to allowed types)
    auto allConfigs = DefaultEventMappings();
    std::vector<EventTypeMapping> configs;
    for (auto& c : allConfigs) {
        if (std::find(allowedTypes.begin(), allowedTypes.end(), c.htraceType) != allowedTypes.end()) {
            configs.push_back(c);
        }
    }

    // 聚合键扩展为 (callchain_id, sub_type_id)：同一 ccid 理论上可能 mmap 多文件。
    std::vector<std::map<std::pair<int, int>, AllocEntry>> allocMaps(configs.size());

    // Build lookup: alloc event_type string → list of mapping indices.
    // The same event_type ("MmapEvent") may map to multiple HtraceEventTypes,
    // disambiguated in-memory by sub_type_id (NULL → mmap-anon, NOT NULL → mmap-filepage).
    std::map<std::string, std::vector<int>> allocEventLookup;
    for (size_t i = 0; i < configs.size(); i++) {
        allocEventLookup[configs[i].allocEvent].push_back(static_cast<int>(i));
    }

    auto [eventTypeIn, sql] = BuildDetailSql(configs);
    QueryAndAggregateAllocs(db, sql, configs, allocEventLookup, allocMaps);

    // Collect results from all mappings
    std::vector<GroupEntry> allGroups;
    for (size_t pi = 0; pi < configs.size(); pi++) {
        for (auto& [key, entry] : allocMaps[pi]) {
            if (entry.heapSize != 0) {
                allGroups.push_back({key.first, entry.heapSize, entry.count,
                    configs[pi].htraceType, entry.subTypeId});
            }
        }
    }

    if (allGroups.empty()) {
        throw std::runtime_error("Detail mode: no outstanding allocations");
    }

    // Sort by heap_size descending, then callchain_id
    std::sort(allGroups.begin(), allGroups.end(),
        [](const auto& a, const auto& b) {
            if (a.heapSize != b.heapSize) {
                return a.heapSize > b.heapSize;
            }
            return a.callchainId < b.callchainId;
        });

    log::Logi("  Detail mode: ", allGroups.size(), " groups across ", configs.size(), " event type(s).");

    ResolveDetailFrames(ctx, eventTypeIn, allGroups, needAddrs, stacks);
}

} // namespace

// ── Main processor ──────────────────────────────────────────────────
TraceDB ProcessHtraceDb(const std::string& dbPath, const std::string& category, bool filterCategory, bool needAddrs)
{
    StageTimer timer("DB");

    if (!platform::FileExists(dbPath)) {
        throw std::runtime_error("DB file not found: " + dbPath);
    }

    // Determine which types to include based on --category.
    // In parse-only mode (filter_category == false), all enabled types are included.
    auto allowedTypes = AllowedEventTypes(category, filterCategory);

    HtraceParseContext ctx;
    OpenHtraceContext(ctx, dbPath, timer);

    TraceDB result;
    result.stacks = CompressedCallStackList{};
    auto& stacks = std::get<CompressedCallStackList>(result.stacks);

    if (ctx.meta.mode == "statistic") {
        LoadStatisticStacks(ctx, allowedTypes, needAddrs, stacks);
    } else if (ctx.meta.mode == "detail") {
        LoadDetailStacks(ctx, allowedTypes, needAddrs, stacks);
    } else {
        throw std::runtime_error(
            "No valid call stack data found (both or neither of native_hook_statistic/native_hook)");
    }

    stacks.dict = std::move(ctx.dataDict);
    result.meta = std::move(ctx.meta);
    timer.Tick("call_stacks", std::to_string(stacks.stacks.size()) + " stacks");
    return result;
}

} // namespace mem_analyzer
