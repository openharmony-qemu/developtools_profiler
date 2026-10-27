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
#include "htrace/htrace_processor.hpp"
#include "htrace/htrace_category.hpp"
#include "utils/platform.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/timer.hpp"
#include "shared/htrace_model/htrace_db_helper.hpp"
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

// ── Helper: 计算某 category（+是否过滤）下仍启用的 HtraceEventType ──
static std::vector<HtraceEventType> allowed_event_types(
        const std::string& category, bool filter_category) {
    if (!filter_category) {
        // parse-only：不按 category 过滤，全部启用的类型纳入，保持升序（与历史一致）。
        std::vector<HtraceEventType> allowed;
        for (HtraceEventType et : htrace_enabled_types()) allowed.push_back(et);
        std::sort(allowed.begin(), allowed.end(),
                  [](HtraceEventType a, HtraceEventType b) {
                      return static_cast<int>(a) < static_cast<int>(b);
                  });
        return allowed;
    }
    return htrace_category(category).event_types;  // 描述符表内已升序
}

// ── Build CompressedCallStack from frames ──
static CompressedCallStack build_compressed_call_stack(
    int callchain_id, long long total_size, long long total_count,
    HtraceEventType type_val, int sub_type_id,
    const std::unordered_map<int, std::vector<FrameRow>>& frames_by_chain)
{
    CompressedCallStack cs;
    cs.id = callchain_id;
    cs.type = type_val;
    cs.sub_type_id = sub_type_id;
    cs.existing_size = total_size;
    cs.existing_count = total_count;

    auto it = frames_by_chain.find(callchain_id);
    if (it != frames_by_chain.end()) {
        cs.frames.reserve(it->second.size());
        for (const auto& fr : it->second) {
            CompressedStackFrame sf;
            sf.depth = fr.depth;
            sf.symbol_id = fr.symbol_id;
            sf.file_id_null = fr.file_id_null;
            sf.file_id = fr.file_id;
            sf.vaddr_null = fr.vaddr_null;
            sf.vaddr = fr.vaddr;
            sf.build_id_null = fr.build_id_null;
            sf.build_id = fr.build_id;
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
    std::unordered_map<int, std::string> data_dict;

    HtraceParseContext() = default;
    HtraceParseContext(const HtraceParseContext&) = delete;
    HtraceParseContext& operator=(const HtraceParseContext&) = delete;
    ~HtraceParseContext() {
        if (db) sqlite3_close(db);
    }
};

// 拼 event type IN 列表（statistic 主 SQL 与帧子查询复用）。
std::string build_type_in_list(const std::vector<HtraceEventType>& types) {
    std::string type_in;
    for (HtraceEventType et : types) {
        if (!type_in.empty()) type_in += ",";
        type_in += std::to_string(static_cast<int>(et));
    }
    return type_in;
}

// 逗号分隔地址串 → 段列表（statistic 分支 b.data 的 existing_addrs）。
std::vector<std::string> split_comma_separated(const std::string& s) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= s.size()) {
        size_t e = s.find(',', start);
        if (e == std::string::npos) e = s.size();
        if (e > start) parts.emplace_back(s.substr(start, e - start));
        if (e == s.size()) break;
        start = e + 1;
    }
    return parts;
}

// 打开 db + 读 meta + data_dict（就地填充 ctx）。
void open_htrace_context(HtraceParseContext& ctx, const std::string& db_path, StageTimer& timer) {
    ctx.db = platform::open_sqlite3_db(db_path);
    ctx.meta = query_trace_meta(ctx.db);

    // Load data_dict
    timer.stage("Loading data_dict");
    std::string dsql = "SELECT id, data FROM data_dict";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(ctx.db, dsql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        StmtGuard guard(stmt);
        // Verify id column type is INTEGER before processing
        bool type_checked = false;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (!type_checked) {
                int col_type = sqlite3_column_type(stmt, 0);
                if (col_type != SQLITE_INTEGER) {
                    throw std::runtime_error(
                        "data_dict.id column type is not INTEGER (got type " +
                        std::to_string(col_type) + "), int overflow risk");
                }
                type_checked = true;
            }
            int id = sqlite3_column_int(stmt, 0);
            auto data = (const char*)sqlite3_column_text(stmt, 1);
            if (data) ctx.data_dict[id] = platform::sanitize_utf8(data);
        }
    }
    timer.tick("data_dict");
}

// statistic 模式：按 (callchain_id, type) 分区聚合，每条链在每个 type 下取最新一条。
void load_statistic_stacks(HtraceParseContext& ctx,
                           const std::vector<HtraceEventType>& allowed_types,
                           bool need_addrs,
                           CompressedCallStackList& stacks) {
    sqlite3* db = ctx.db;
    TraceMeta& meta = ctx.meta;

    std::string type_in = build_type_in_list(allowed_types);
    // 隔离：仅 -l(need_addrs) 才 JOIN data_dict 取地址（依赖 addr_id 列）；
    // 非 -l 用 NULL 占位列，不碰 addr_id，SQL 结构与 master 等价、不依赖该列存在。
    std::string sql =
        "SELECT ranked.callchain_id, ranked.type, ranked.ts, ranked.sub_type_id, "
        "(ranked.apply_size - ranked.release_size) as total_size, "
        "(ranked.apply_count - ranked.release_count) as total_count, ";
    sql += need_addrs ? "b.data as existing_addrs " : "NULL as existing_addrs ";
    sql += "FROM ("
        "  SELECT *, ROW_NUMBER() OVER (PARTITION BY callchain_id, type ORDER BY ts DESC) as rn "
        "  FROM native_hook_statistic WHERE type IN (";
    sql += type_in;
    sql += ") ";           // 闭合 IN (...)
    sql += ") ranked ";    // 闭合子查询 FROM (...) + 别名
    if (need_addrs) {
        sql += "LEFT JOIN data_dict b ON ranked.addr_id = b.id ";
    }
    sql += "WHERE ranked.rn = 1 AND (ranked.apply_size - ranked.release_size) != 0 "
        "ORDER BY total_size DESC";

    std::vector<std::tuple<int, int, int, int, long long, long long, std::string>> statistic_groups;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Statistic query prepare failed: " +
            std::string(sqlite3_errmsg(db)));
    }
    StmtGuard guard(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
            int ccid = sqlite3_column_int(stmt, 0);
            int type_val = sqlite3_column_int(stmt, 1);
            int max_ts = sqlite3_column_int(stmt, 2);
            int sub_type_id = -1;
            if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
                sub_type_id = sqlite3_column_int(stmt, 3);
            }
            long long total_size = sqlite3_column_int64(stmt, 4);
            long long total_count = sqlite3_column_int64(stmt, 5);
            auto addrs_str = (const char*)sqlite3_column_text(stmt, 6);
            statistic_groups.emplace_back(ccid, type_val, max_ts, sub_type_id, total_size, total_count,
                addrs_str ? addrs_str : "");
    }

    log::logi("  Statistic mode: ", statistic_groups.size(), " groups.");

    if (statistic_groups.empty()) {
        throw std::runtime_error("No records found in native_hook_statistic");
    }

    // Load frames for matching chains (reuse the same type filter in a subquery)
    std::string chain_select =
        "SELECT callchain_id FROM ("
        "  SELECT *, ROW_NUMBER() OVER (PARTITION BY callchain_id, type ORDER BY ts DESC) AS rn "
        "  FROM native_hook_statistic WHERE type IN (" + type_in + ") "
        ") WHERE rn = 1 AND (apply_size - release_size) != 0";
    auto frames_by_chain = read_frames(db, chain_select);

    for (auto& [ccid, type_val, max_ts, sub_type_id, total_size, total_count, addrs_str] : statistic_groups) {
        auto ev_type = static_cast<HtraceEventType>(type_val);
        if (htrace_is_count_size_type(ev_type)) {
            meta.type_sizes[ev_type] += total_size;
        }
        auto entry = build_compressed_call_stack(ccid, total_size, total_count,
            static_cast<HtraceEventType>(type_val), sub_type_id, frames_by_chain);
        if (!addrs_str.empty()) {
            // b.data 是逗号分隔的地址串，split 后填入 existing_addrs
            entry.existing_addrs = std::make_shared<std::vector<std::string>>(
                split_comma_separated(addrs_str));
        }
        stacks.stacks.push_back(std::move(entry));
    }
}

// detail 模式：AllocEvent 且 end_ts IS NULL/0 为未释放（outstanding）；
// 已释放(end_ts != 0)整体跳过，FreeEvent 无需处理。
void load_detail_stacks(HtraceParseContext& ctx,
                        const std::vector<HtraceEventType>& allowed_types,
                        bool need_addrs,
                        CompressedCallStackList& stacks) {
    sqlite3* db = ctx.db;
    TraceMeta& meta = ctx.meta;

    struct AllocEntry {
        long long heap_size;
        int count;
        int sub_type_id = -1;
    };

    // One alloc_map per event type mapping (filtered to allowed types)
    auto all_configs = default_event_mappings();
    std::vector<EventTypeMapping> configs;
    for (auto& c : all_configs) {
        if (std::find(allowed_types.begin(), allowed_types.end(), c.htrace_type) != allowed_types.end()) {
            configs.push_back(c);
        }
    }

    // 聚合键扩展为 (callchain_id, sub_type_id)：同一 ccid 理论上可能 mmap 多文件。
    std::vector<std::map<std::pair<int,int>, AllocEntry>> alloc_maps(configs.size());

    // Build lookup: alloc event_type string → list of mapping indices.
    // The same event_type ("MmapEvent") may map to multiple HtraceEventTypes,
    // disambiguated in-memory by sub_type_id (NULL → mmap-anon, NOT NULL → mmap-filepage).
    std::map<std::string, std::vector<int>> alloc_event_lookup;
    for (size_t i = 0; i < configs.size(); i++) {
        alloc_event_lookup[configs[i].alloc_event].push_back(static_cast<int>(i));
    }

    // Only process alloc events with end_ts IS NULL or 0 (outstanding, not freed).
    // Event_type whitelist is pushed down to SQL; sub_type dispatch is in-memory.
    std::set<std::string> event_types_seen;
    std::string event_type_in;
    for (auto& c : configs) {
        if (event_types_seen.count(c.alloc_event)) continue;
        event_types_seen.insert(c.alloc_event);
        if (!event_type_in.empty()) event_type_in += ",";
        event_type_in += "'" + c.alloc_event + "'";
    }
    std::string sql =
        "SELECT event_type, callchain_id, heap_size, sub_type_id FROM native_hook "
        "WHERE (end_ts IS NULL OR end_ts = 0) "
        "AND event_type IN (" + event_type_in + ")";
    sql += " ORDER BY start_ts ASC";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        StmtGuard guard(stmt);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto event_type_str = (const char*)sqlite3_column_text(stmt, 0);
            if (!event_type_str) continue;
            int ccid = sqlite3_column_int(stmt, 1);
            long long heap_size = sqlite3_column_int64(stmt, 2);
            int sub_type_id = -1;
            if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
                sub_type_id = sqlite3_column_int(stmt, 3);
            }

            auto lookup_it = alloc_event_lookup.find(event_type_str);
            if (lookup_it == alloc_event_lookup.end()) continue; // defensive

            for (int ci : lookup_it->second) {
                const auto& c = configs[ci];
                bool match = false;
                switch (c.sub_type_cond) {
                    case SubTypeCondition::Any:      match = true; break;
                    case SubTypeCondition::IsNull:   match = (sub_type_id == -1); break;
                    case SubTypeCondition::IsNotNull: match = (sub_type_id != -1); break;
                }
                if (match) {
                    auto key = std::make_pair(ccid, sub_type_id);
                    auto& entry = alloc_maps[ci][key];
                    entry.heap_size += heap_size;
                    entry.count++;
                    entry.sub_type_id = sub_type_id;
                }
            }
        }
    }

    // Collect results from all mappings
    struct GroupEntry {
        int callchain_id;
        long long heap_size;
        int count;
        HtraceEventType htrace_type;
        int sub_type_id = -1;
    };
    std::vector<GroupEntry> all_groups;
    for (size_t pi = 0; pi < configs.size(); pi++) {
        for (auto& [key, entry] : alloc_maps[pi]) {
            if (entry.heap_size != 0) {
                all_groups.push_back({key.first, entry.heap_size, entry.count,
                                      configs[pi].htrace_type, entry.sub_type_id});
            }
        }
    }

    if (all_groups.empty()) {
        throw std::runtime_error("Detail mode: no outstanding allocations");
    }

    // Sort by heap_size descending, then callchain_id
    std::sort(all_groups.begin(), all_groups.end(),
        [](const auto& a, const auto& b) {
            if (a.heap_size != b.heap_size) return a.heap_size > b.heap_size;
            return a.callchain_id < b.callchain_id;
        });

    log::logi("  Detail mode: ", all_groups.size(), " groups across ", configs.size(), " event type(s).");

    // Load frames for matching chains (reuse event_type/sub_type filter in a subquery)
    std::string chain_select =
        "SELECT callchain_id FROM native_hook "
        "WHERE (end_ts IS NULL OR end_ts = 0) AND event_type IN (" + event_type_in + ")";
    auto frames_by_chain = read_frames(db, chain_select);
    // 隔离：仅 -l(need_addrs) 才查 native_hook.addr 聚合地址；非 -l 空 map，循环 find 返回 end 不填
    std::unordered_map<int, std::vector<std::string>> addrs_by_chain;
    if (need_addrs) {
        addrs_by_chain = read_callchain_addrs(db, chain_select);
    }

    for (auto& g : all_groups) {
        if (htrace_is_count_size_type(g.htrace_type)) {
            meta.type_sizes[g.htrace_type] += g.heap_size;
        }
        auto cs = build_compressed_call_stack(g.callchain_id, g.heap_size, g.count,
            g.htrace_type, g.sub_type_id, frames_by_chain);
        auto a_it = addrs_by_chain.find(g.callchain_id);
        if (a_it != addrs_by_chain.end()) {
            cs.existing_addrs = std::make_shared<std::vector<std::string>>(a_it->second);
        }
        stacks.stacks.push_back(std::move(cs));
    }
}

} // namespace

// ── Main processor ──────────────────────────────────────────────────
TraceDB process_htrace_db(const std::string& db_path, const std::string& category,
                          bool filter_category, bool need_addrs) {
    StageTimer timer("DB");

    if (!platform::file_exists(db_path)) {
        throw std::runtime_error("DB file not found: " + db_path);
    }

    // Determine which types to include based on --category.
    // In parse-only mode (filter_category == false), all enabled types are included.
    auto allowed_types = allowed_event_types(category, filter_category);

    HtraceParseContext ctx;
    open_htrace_context(ctx, db_path, timer);

    TraceDB result;
    result.stacks = CompressedCallStackList{};
    auto& stacks = std::get<CompressedCallStackList>(result.stacks);

    if (ctx.meta.mode == "statistic") {
        load_statistic_stacks(ctx, allowed_types, need_addrs, stacks);
    } else if (ctx.meta.mode == "detail") {
        load_detail_stacks(ctx, allowed_types, need_addrs, stacks);
    } else {
        throw std::runtime_error(
            "No valid call stack data found (both or neither of native_hook_statistic/native_hook)");
    }

    stacks.dict = std::move(ctx.data_dict);
    result.meta = std::move(ctx.meta);
    timer.tick("call_stacks", std::to_string(stacks.stacks.size()) + " stacks");
    return result;
}

} // namespace mem_analyzer
