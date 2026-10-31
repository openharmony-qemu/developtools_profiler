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
#include "shared/htrace_model/htrace_db_helper.h"
#include "utils/logger.h"
#include "utils/platform.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

namespace mem_analyzer {

// ── StmtGuard ──────────────────────────────────────────────────────
StmtGuard& StmtGuard::operator=(StmtGuard&& o) noexcept
{
    if (this != &o) {
        reset(nullptr);
        stmt_ = o.stmt_;
        o.stmt_ = nullptr;
    }
    return *this;
}

void StmtGuard::reset(sqlite3_stmt* s)
{
    if (stmt_) {
        sqlite3_finalize(stmt_);
    }
    stmt_ = s;
}

// ── Scalar query ──────────────────────────────────────────────────
namespace {
bool QueryScalar(sqlite3* db, const char* sql, void (*read)(sqlite3_stmt*, void*), void* out)
{
    if (!db || !sql) {
        return false;
    }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    StmtGuard guard(stmt);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        return false;
    }
    read(stmt, out);
    return true;
}

void ReadInt64(sqlite3_stmt* s, void* out)
{
    *static_cast<long long*>(out) = sqlite3_column_int64(s, 0);
}

bool QueryInt64(sqlite3* db, const char* sql, long long& out)
{
    return QueryScalar(db, sql, ReadInt64, &out);
}

void ResolveProcessName(sqlite3* db, int ipid, TraceMeta& meta)
{
    sqlite3_stmt* pstmt = nullptr;
    const char* psql = "SELECT name FROM process WHERE id = ?";
    if (sqlite3_prepare_v2(db, psql, -1, &pstmt, nullptr) == SQLITE_OK) {
        StmtGuard pguard(pstmt);
        sqlite3_bind_int(pstmt, 1, ipid);
        if (sqlite3_step(pstmt) == SQLITE_ROW) {
            const auto* rawName =
                reinterpret_cast<const char*>(sqlite3_column_text(pstmt, 0));
            if (rawName) {
                meta.processName = platform::SanitizeUtf8(rawName);
            }
        }
    }
}

void QueryTimeRange(sqlite3* db, TraceMeta& meta)
{
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT start_ts, end_ts FROM trace_range LIMIT 1";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }
    StmtGuard guard(stmt);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        return;
    }
    if (sqlite3_column_type(stmt, 0) == SQLITE_NULL ||
        sqlite3_column_type(stmt, 1) == SQLITE_NULL) {
        return;
    }
    const long long startTs = sqlite3_column_int64(stmt, 0);
    const long long endTs = sqlite3_column_int64(stmt, 1);
    if (startTs < 0 || endTs < startTs) {
        return;
    }
    meta.durationNs = endTs - startTs;

    long long bootTs = 0;
    long long realTs = 0;
    if (!QueryInt64(db, "SELECT ts FROM clock_snapshot WHERE clock_name = 'boottime' LIMIT 1", bootTs) ||
        !QueryInt64(db, "SELECT ts FROM clock_snapshot WHERE clock_name = 'realtime' LIMIT 1", realTs) ||
        bootTs == 0 || realTs == 0) {
        return;
    }
    const long long offsetNs = realTs - bootTs;
    meta.startTimeMs = (startTs + offsetNs + 500000LL) / 1000000LL;
    meta.endTimeMs = (endTs + offsetNs + 500000LL) / 1000000LL;
}
} // namespace

void QueryProcessName(sqlite3* db, const std::string& hookTable, TraceMeta& meta)
{
    // Process name (from hook table's ipid → process.name)
    const std::string ipidSql =
        "SELECT DISTINCT ipid FROM " + hookTable + " WHERE ipid IS NOT NULL LIMIT 1";
    sqlite3_stmt* ipidStmt = nullptr;
    if (sqlite3_prepare_v2(db, ipidSql.c_str(), -1, &ipidStmt, nullptr) == SQLITE_OK) {
        StmtGuard guard(ipidStmt);
        if (sqlite3_step(ipidStmt) == SQLITE_ROW) {
            const int ipid = sqlite3_column_int(ipidStmt, 0);
            ResolveProcessName(db, ipid, meta);
        }
    }
}

// ── query_trace_meta ──────────────────────────────────────────────
TraceMeta QueryTraceMeta(sqlite3* db)
{
    TraceMeta meta;
    if (!db) {
        return meta;
    }

    // Time range
    QueryTimeRange(db, meta);

    // Mode detection: statistic / detail / (empty for both or none)
    long long statCount = 0;
    long long detailCount = 0;
    QueryInt64(db, "SELECT COUNT(*) FROM native_hook_statistic", statCount);
    QueryInt64(db, "SELECT COUNT(*) FROM native_hook", detailCount);

    std::string hookTable;
    if (statCount > 0 && detailCount == 0) {
        meta.mode = "statistic";
        hookTable = "native_hook_statistic";
    } else if (detailCount > 0 && statCount == 0) {
        meta.mode = "detail";
        hookTable = "native_hook";
    } else {
        return meta;
    }

    QueryProcessName(db, hookTable, meta);

    return meta;
}

// ── read_frames ───────────────────────────────────────────────────
namespace {

constexpr int COL_FILE_ID = 2;
constexpr int COL_CALLCHAIN_ID = 3;
constexpr int COL_VADDR = 4;
constexpr int COL_BUILD_ID = 5;

void QueryFrames(sqlite3* db, const std::string& chainSelect, std::unordered_map<int, std::vector<FrameRow>>& out)
{
    const std::string sql =
        "SELECT depth, symbol_id, file_id, callchain_id, vaddr, build_id "
        "FROM native_hook_frame "
        "WHERE callchain_id IN (" + chainSelect + ") "
        "ORDER BY callchain_id, depth ASC";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        // 各版本 DB 的 native_hook_frame 列集合不同（缺 vaddr/build_id 等列时
        // prepare 会失败）。此处曾静默返回空表，导致所有 call stack 无帧、
        // 报告显示 0 栈/0 符号却"成功"退出。
        log::Logw("read_frames: prepare failed: ", sqlite3_errmsg(db));
        return;
    }
    StmtGuard guard(stmt);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FrameRow fr;
        fr.depth = sqlite3_column_int(stmt, 0);
        fr.symbolId = sqlite3_column_int(stmt, 1);
        fr.fileIdNull = (sqlite3_column_type(stmt, COL_FILE_ID) == SQLITE_NULL);
        fr.fileId = fr.fileIdNull ? 0 : sqlite3_column_int(stmt, COL_FILE_ID);
        fr.callchainId = sqlite3_column_int(stmt, COL_CALLCHAIN_ID);
        fr.vaddrNull = (sqlite3_column_type(stmt, COL_VADDR) == SQLITE_NULL);
        fr.vaddr = 0;
        if (!fr.vaddrNull) {
            const char* s = reinterpret_cast<const char*>(sqlite3_column_text(stmt, COL_VADDR));
            fr.vaddr = s ? static_cast<uint64_t>(strtoull(s, nullptr, 0)) : 0;
        }
        fr.buildIdNull = (sqlite3_column_type(stmt, COL_BUILD_ID) == SQLITE_NULL);
        fr.buildId = fr.buildIdNull ? 0 : sqlite3_column_int(stmt, COL_BUILD_ID);

        out[fr.callchainId].push_back(std::move(fr));
    }
}

} // namespace

std::unordered_map<int, std::vector<FrameRow>> ReadFrames(sqlite3* db, const std::string& chainSelect)
{
    std::unordered_map<int, std::vector<FrameRow>> result;
    if (!db || chainSelect.empty()) {
        return result;
    }
    QueryFrames(db, chainSelect, result);
    return result;
}

std::unordered_map<int, std::vector<std::string>> ReadCallchainAddrs(sqlite3* db, const std::string& chainSelect)
{
    std::unordered_map<int, std::vector<std::string>> result;
    if (!db || chainSelect.empty()) {
        return result;
    }
    // detail 模式：直接 SELECT 单个地址（hex 串），C++ 按 callchain_id push_back，无需 GROUP_CONCAT + split。
    // chain_select 复用 read_frames 的同源子查询（已含 end_ts IS NULL + event_type 过滤）。
    std::string sql =
        "SELECT callchain_id, '0x'||printf('%x',addr) "
        "FROM native_hook WHERE callchain_id IN (" + chainSelect + ")";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        log::Logw("read_callchain_addrs: prepare failed: ", sqlite3_errmsg(db));
        return result;
    }
    StmtGuard guard(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int ccid = sqlite3_column_int(stmt, 0);
        auto addrStr = (const char*)sqlite3_column_text(stmt, 1);
        if (addrStr) {
            result[ccid].push_back(addrStr);
        }
    }
    return result;
}

} // namespace mem_analyzer