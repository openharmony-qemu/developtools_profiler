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
#include "shared/htrace_model/htrace_db_helper.hpp"
#include "utils/logger.hpp"
#include "utils/platform.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

namespace mem_analyzer {

// ── StmtGuard ──────────────────────────────────────────────────────
StmtGuard& StmtGuard::operator=(StmtGuard&& o) noexcept {
    if (this != &o) {
        reset(nullptr);
        stmt_ = o.stmt_;
        o.stmt_ = nullptr;
    }
    return *this;
}

void StmtGuard::reset(sqlite3_stmt* s) {
    if (stmt_) {
        sqlite3_finalize(stmt_);
    }
    stmt_ = s;
}

// ── Scalar query ──────────────────────────────────────────────────
namespace {
bool query_scalar(sqlite3* db, const char* sql,
                  void (*read)(sqlite3_stmt*, void*), void* out) {
    if (!db || !sql) return false;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    StmtGuard guard(stmt);
    if (sqlite3_step(stmt) != SQLITE_ROW) return false;
    read(stmt, out);
    return true;
}

void read_int64(sqlite3_stmt* s, void* out) {
    *static_cast<long long*>(out) = sqlite3_column_int64(s, 0);
}

bool query_int64(sqlite3* db, const char* sql, long long& out) {
    return query_scalar(db, sql, read_int64, &out);
}
} // namespace

// ── query_trace_meta ──────────────────────────────────────────────
TraceMeta query_trace_meta(sqlite3* db) {
    TraceMeta meta;
    if (!db) return meta;

    // Time range
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT start_ts, end_ts FROM trace_range LIMIT 1";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        StmtGuard guard(stmt);
        if (sqlite3_step(stmt) == SQLITE_ROW &&
            sqlite3_column_type(stmt, 0) != SQLITE_NULL &&
            sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
            const long long start_ts = sqlite3_column_int64(stmt, 0);
            const long long end_ts = sqlite3_column_int64(stmt, 1);
            if (start_ts >= 0 && end_ts >= start_ts) {
                meta.duration_ns = end_ts - start_ts;

                long long boot_ts = 0;
                long long real_ts = 0;
                if (query_int64(db,
                        "SELECT ts FROM clock_snapshot WHERE clock_name = 'boottime' LIMIT 1",
                        boot_ts) &&
                    query_int64(db,
                        "SELECT ts FROM clock_snapshot WHERE clock_name = 'realtime' LIMIT 1",
                        real_ts) &&
                    boot_ts != 0 && real_ts != 0) {
                    const long long offset_ns = real_ts - boot_ts;
                    meta.start_time_ms = (start_ts + offset_ns + 500000LL) / 1000000LL;
                    meta.end_time_ms = (end_ts + offset_ns + 500000LL) / 1000000LL;
                }
            }
        }
    }

    // Mode detection: statistic / detail / (empty for both or none)
    long long stat_count = 0;
    long long detail_count = 0;
    query_int64(db, "SELECT COUNT(*) FROM native_hook_statistic", stat_count);
    query_int64(db, "SELECT COUNT(*) FROM native_hook", detail_count);

    std::string hook_table;
    if (stat_count > 0 && detail_count == 0) {
        meta.mode = "statistic";
        hook_table = "native_hook_statistic";
    } else if (detail_count > 0 && stat_count == 0) {
        meta.mode = "detail";
        hook_table = "native_hook";
    } else {
        return meta;
    }

    // Process name (from hook table's ipid → process.name)
    const std::string ipid_sql =
        "SELECT DISTINCT ipid FROM " + hook_table + " WHERE ipid IS NOT NULL LIMIT 1";
    sqlite3_stmt* ipid_stmt = nullptr;
    if (sqlite3_prepare_v2(db, ipid_sql.c_str(), -1, &ipid_stmt, nullptr) == SQLITE_OK) {
        StmtGuard guard(ipid_stmt);
        if (sqlite3_step(ipid_stmt) == SQLITE_ROW) {
            const int ipid = sqlite3_column_int(ipid_stmt, 0);
            sqlite3_stmt* pstmt = nullptr;
            const char* psql = "SELECT name FROM process WHERE id = ?";
            if (sqlite3_prepare_v2(db, psql, -1, &pstmt, nullptr) == SQLITE_OK) {
                StmtGuard pguard(pstmt);
                sqlite3_bind_int(pstmt, 1, ipid);
                if (sqlite3_step(pstmt) == SQLITE_ROW) {
                    const auto* raw_name =
                        reinterpret_cast<const char*>(sqlite3_column_text(pstmt, 0));
                    if (raw_name) meta.process_name = platform::sanitize_utf8(raw_name);
                }
            }
        }
    }

    return meta;
}

// ── read_frames ───────────────────────────────────────────────────
namespace {

void query_frames(sqlite3* db, const std::string& chain_select,
                  std::unordered_map<int, std::vector<FrameRow>>& out) {
    const std::string sql =
        "SELECT depth, symbol_id, file_id, callchain_id, vaddr, build_id "
        "FROM native_hook_frame "
        "WHERE callchain_id IN (" + chain_select + ") "
        "ORDER BY callchain_id, depth ASC";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        // 各版本 DB 的 native_hook_frame 列集合不同（缺 vaddr/build_id 等列时
        // prepare 会失败）。此处曾静默返回空表，导致所有 call stack 无帧、
        // 报告显示 0 栈/0 符号却"成功"退出。
        log::logw("read_frames: prepare failed: ", sqlite3_errmsg(db));
        return;
    }
    StmtGuard guard(stmt);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FrameRow fr;
        fr.depth        = sqlite3_column_int(stmt, 0);
        fr.symbol_id    = sqlite3_column_int(stmt, 1);
        fr.file_id_null = (sqlite3_column_type(stmt, 2) == SQLITE_NULL);
        fr.file_id      = fr.file_id_null ? 0 : sqlite3_column_int(stmt, 2);
        fr.callchain_id = sqlite3_column_int(stmt, 3);
        fr.vaddr_null   = (sqlite3_column_type(stmt, 4) == SQLITE_NULL);
        fr.vaddr        = 0;
        if (!fr.vaddr_null) {
            const char* s = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            fr.vaddr = s ? static_cast<uint64_t>(strtoull(s, nullptr, 0)) : 0;
        }
        fr.build_id_null = (sqlite3_column_type(stmt, 5) == SQLITE_NULL);
        fr.build_id      = fr.build_id_null ? 0 : sqlite3_column_int(stmt, 5);

        out[fr.callchain_id].push_back(std::move(fr));
    }
}

} // namespace

std::unordered_map<int, std::vector<FrameRow>> read_frames(
        sqlite3* db, const std::string& chain_select) {
    std::unordered_map<int, std::vector<FrameRow>> result;
    if (!db || chain_select.empty()) return result;
    query_frames(db, chain_select, result);
    return result;
}

std::unordered_map<int, std::vector<std::string>> read_callchain_addrs(
        sqlite3* db, const std::string& chain_select) {
    std::unordered_map<int, std::vector<std::string>> result;
    if (!db || chain_select.empty()) return result;
    // detail 模式：直接 SELECT 单个地址（hex 串），C++ 按 callchain_id push_back，无需 GROUP_CONCAT + split。
    // chain_select 复用 read_frames 的同源子查询（已含 end_ts IS NULL + event_type 过滤）。
    std::string sql =
        "SELECT callchain_id, '0x'||printf('%x',addr) "
        "FROM native_hook WHERE callchain_id IN (" + chain_select + ")";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        log::logw("read_callchain_addrs: prepare failed: ", sqlite3_errmsg(db));
        return result;
    }
    StmtGuard guard(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int ccid = sqlite3_column_int(stmt, 0);
        auto addr_str = (const char*)sqlite3_column_text(stmt, 1);
        if (addr_str) {
            result[ccid].push_back(addr_str);
        }
    }
    return result;
}

} // namespace mem_analyzer