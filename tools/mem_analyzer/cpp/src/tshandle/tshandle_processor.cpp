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
#include "tshandle/tshandle_processor.h"
#include "utils/platform.h"
#include "utils/file_utils.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"
#include "utils/timer.h"
#include "shared/htrace_model/htrace_db_helper.h"
#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>

namespace mem_analyzer {

constexpr int ARK_GLOBAL_HANDLE_TYPE = 17;
constexpr int ARK_LOCAL_HANDLE_TYPE = 18;
constexpr int COL_EXISTING_SIZE = 3;

// ── Helper: split string by delimiter ─────────────────────────────
static std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> parts;
    std::istringstream iss(s);
    std::string token;
    while (std::getline(iss, token, delim)) {
        if (!token.empty()) {
            parts.push_back(token);
        }
    }
    return parts;
}

struct ChainGroup {
    int callchainId;
    int type;                // 17 or 18
    std::vector<std::string> existingAddrs;
    long long existingSize;
};

static std::unordered_map<int, std::string> LoadDataDict(sqlite3* db)
{
    std::unordered_map<int, std::string> dataDict;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, data FROM data_dict";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        StmtGuard guard(stmt);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            auto data = (const char*)sqlite3_column_text(stmt, 1);
            if (data) {
                dataDict[id] = platform::SanitizeUtf8(data);
            }
        }
    } else {
        log::Loge("Error: prepare data dict failed:sql=", sql, ", err=", sqlite3_errmsg(db));
    }
    return dataDict;
}

static std::vector<ChainGroup> QueryCallStackGroups(sqlite3* db, bool isStatistics)
{
    std::vector<ChainGroup> groups;

    if (isStatistics) {
        // ── Statistic mode: native_hook_statistic WHERE type IN (17, 18) ──
        const char* stackSql =
            "SELECT a.callchain_id, a.type, b.data AS existing_addrs,"
            "  (a.apply_size - a.release_size) AS existing_size"
            "  FROM ("
            "    SELECT ranked.*, ROW_NUMBER() OVER ("
            "      PARTITION BY ranked.callchain_id ORDER BY ranked.ts DESC"
            "    ) AS rn"
            "    FROM native_hook_statistic ranked"
            "  ) a"
            "  INNER JOIN data_dict b ON a.addr_id = b.id"
            "  WHERE a.rn = 1 AND a.type IN (17, 18)"
            "    AND (a.apply_size - a.release_size) > 0"
            "  ORDER BY existing_size DESC";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, stackSql, -1, &stmt, nullptr) == SQLITE_OK) {
            StmtGuard guard(stmt);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                ChainGroup g;
                g.callchainId = sqlite3_column_int(stmt, 0);
                g.type = sqlite3_column_int(stmt, 1);
                auto existingAddrsStr = (const char*)sqlite3_column_text(stmt, 2);
                if (existingAddrsStr) {
                    g.existingAddrs = split(existingAddrsStr, ',');
                }
                g.existingSize = sqlite3_column_int64(stmt, COL_EXISTING_SIZE);
                groups.push_back(g);
            }
        } else {
            log::Loge("Error: prepare stastics failed:sql=", stackSql, ", err=", sqlite3_errmsg(db));
        }
    } else {
        // ── Detail mode: native_hook with ARK handle events ──
        const char* stackSql =
            "SELECT a.callchain_id,"
            "  CASE a.event_type"
            "    WHEN 'ARK_GLOBAL_HANDLE_Alloc_Event' THEN 17"
            "    WHEN 'ARK_LOCAL_HANDLE_Alloc_Event' THEN 18"
            "    ELSE 0"
            "  END AS type,"
            "  GROUP_CONCAT('0x' || printf('%x', a.addr), ',') AS existing_addrs,"
            "  SUM(a.heap_size) AS existing_size"
            "  FROM native_hook a"
            "  WHERE a.event_type = 'ARK_LOCAL_HANDLE_Alloc_Event'"
            "     OR (a.event_type = 'ARK_GLOBAL_HANDLE_Alloc_Event' AND a.end_ts IS NULL)"
            "  GROUP BY a.callchain_id"
            "  ORDER BY existing_size DESC";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, stackSql, -1, &stmt, nullptr) == SQLITE_OK) {
            StmtGuard guard(stmt);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                ChainGroup g;
                g.callchainId = sqlite3_column_int(stmt, 0);
                g.type = sqlite3_column_int(stmt, 1);
                auto existingAddrsStr = (const char*)sqlite3_column_text(stmt, 2);
                g.existingAddrs = existingAddrsStr ? split(existingAddrsStr, ',') : std::vector<std::string>{};
                g.existingSize = sqlite3_column_int64(stmt, COL_EXISTING_SIZE);
                groups.push_back(g);
            }
        } else {
            log::Loge("Error: prepare detail failed:sql=", stackSql, ", err=", sqlite3_errmsg(db));
        }
    }

    return groups;
}

static std::vector<ResolvedCallStack> BuildCallStackEntries(
    const std::vector<ChainGroup>& groups,
    const std::unordered_map<int, std::string>& dataDict,
    const std::unordered_map<int, std::vector<FrameRow>>& framesByChain)
{
    std::vector<ResolvedCallStack> callStacks;
    for (const auto& g : groups) {
        ResolvedCallStack cs;
        cs.id = g.callchainId;
        if (g.type == ARK_GLOBAL_HANDLE_TYPE) {
            cs.type = HtraceEventType::ARK_GLOBAL_HANDLE;
        } else if (g.type == ARK_LOCAL_HANDLE_TYPE) {
            cs.type = HtraceEventType::ARK_LOCAL_HANDLE;
        } else {
            cs.type = HtraceEventType::UNKNOWN;
        }
        cs.existingSize = g.existingSize;
        cs.existingAddrs = std::make_shared<std::vector<std::string>>(std::move(g.existingAddrs));

        // Look up frames for this chain
        auto it = framesByChain.find(g.callchainId);
        if (it != framesByChain.end()) {
            for (const auto& fr : it->second) {
                ResolvedStackFrame tf;
                tf.depth = fr.depth;

                auto symIt = dataDict.find(fr.symbolId);
                tf.symbol = (symIt != dataDict.end()) ? symIt->second : "unknown";

                if (fr.fileIdNull) {
                    tf.lib = "unknown";
                } else {
                    auto libIt = dataDict.find(fr.fileId);
                    tf.lib = (libIt != dataDict.end()) ? libIt->second : "unknown";
                }

                if (fr.buildIdNull) {
                    tf.buildId = "";
                } else {
                    auto buildIt = dataDict.find(fr.buildId);
                    tf.buildId = (buildIt != dataDict.end()) ? buildIt->second : "";
                }

                // TS: frame.vaddr != null ? `0x${Number(frame.vaddr).toString(16)}` : ''
                if (!fr.vaddrNull) {
                    std::stringstream ss;
                    ss << "0x" << std::hex << fr.vaddr;
                    tf.libOffset = ss.str();
                }

                cs.frames.push_back(std::move(tf));
            }
        }

        callStacks.push_back(std::move(cs));
    }
    return callStacks;
}

// ── Main processor ─────────────────────────────────────────────────
TraceDB ProcessTshandleDb(const std::string& dbPath)
{
    if (!platform::FileExists(dbPath)) {
        throw std::runtime_error("DB file not found: " + dbPath);
    }

    sqlite3* db = nullptr;
    {
        db = platform::OpenSqlite3Db(dbPath);
    }

    try {
        StageTimer timer("TS DB");
        TraceDB result;
        result.stacks = ResolvedCallStackList{};
        auto& callStacks = std::get<ResolvedCallStackList>(result.stacks).stacks;
        auto& meta = result.meta;
        bool isStatistics = false;

        // 1. Detect mode + meta（时间/模式/进程名）由 query_trace_meta 统一读取
        timer.Stage("Detecting meta");
        meta = QueryTraceMeta(db);
        isStatistics = (meta.mode == "statistic");
        timer.Tick("detect_meta", meta.mode);

        // 2. Load data_dict for symbol/file resolution
        timer.Stage("Loading data_dict");
        auto dataDict = LoadDataDict(db);
        timer.Tick("data_dict", std::to_string(dataDict.size()) + " entries");

        // 3. Query call stacks
        timer.Stage("Querying call stacks");
        auto groups = QueryCallStackGroups(db, isStatistics);

        if (groups.empty()) {
            throw std::runtime_error("No ARK handle call stacks found");
        }
        timer.Tick("call_stacks", std::to_string(groups.size()) + " groups");

        // 4. Query frames (shared reader; chain filter pushed down via subquery)
        timer.Stage("Querying frames");
        std::string chainSelect;
        if (isStatistics) {
            chainSelect =
                "SELECT callchain_id FROM native_hook_statistic WHERE type IN (17, 18)";
        } else {
            chainSelect =
                "SELECT callchain_id FROM native_hook "
                "WHERE event_type = 'ARK_LOCAL_HANDLE_Alloc_Event' "
                "OR (event_type = 'ARK_GLOBAL_HANDLE_Alloc_Event' AND end_ts IS NULL)";
        }
        auto framesByChain = ReadFrames(db, chainSelect);
        if (framesByChain.empty()) {
            throw std::runtime_error("Failed to query frames");
        }
        timer.Tick("frames", std::to_string(framesByChain.size()) + " chains");

        // 5. Build call stack entries
        timer.Stage("Building call stack entries");
        callStacks = BuildCallStackEntries(groups, dataDict, framesByChain);

        sqlite3_close(db);
        timer.Tick("build_entries", std::to_string(callStacks.size()) + " stacks");
        timer.Done();
        return result;
    } catch (...) {
        sqlite3_close(db);
        throw;
    }
}

} // namespace mem_analyzer
