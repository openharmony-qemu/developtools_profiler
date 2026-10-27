#include "tshandle/tshandle_processor.hpp"
#include "utils/platform.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include "shared/htrace_model/htrace_db_helper.hpp"
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

// ── Helper: split string by delimiter ─────────────────────────────
static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::istringstream iss(s);
    std::string token;
    while (std::getline(iss, token, delim)) {
        if (!token.empty()) parts.push_back(token);
    }
    return parts;
}

// ── Main processor ─────────────────────────────────────────────────
TraceDB process_tshandle_db(const std::string& db_path) {
    if (!platform::file_exists(db_path)) {
        throw std::runtime_error("DB file not found: " + db_path);
    }

    sqlite3* db = nullptr;
    {
        db = platform::open_sqlite3_db(db_path);
    }

    try {
        StageTimer timer("TS DB");
        TraceDB result;
        result.stacks = ResolvedCallStackList{};
        auto& call_stacks = std::get<ResolvedCallStackList>(result.stacks).stacks;
        auto& meta = result.meta;
        bool is_statistics = false;

        // 1. Detect mode + meta（时间/模式/进程名）由 query_trace_meta 统一读取
        timer.stage("Detecting meta");
        meta = query_trace_meta(db);
        is_statistics = (meta.mode == "statistic");
        timer.tick("detect_meta", meta.mode);

        // 2. Load data_dict for symbol/file resolution
        timer.stage("Loading data_dict");
        std::unordered_map<int, std::string> data_dict;
        {
            sqlite3_stmt* stmt = nullptr;
            const char* sql = "SELECT id, data FROM data_dict";
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                StmtGuard guard(stmt);
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    int id = sqlite3_column_int(stmt, 0);
                    auto data = (const char*)sqlite3_column_text(stmt, 1);
                    if (data) data_dict[id] = platform::sanitize_utf8(data);
                }
            } else {
                log::loge("Error: prepare data dict failed:sql=", sql, ", err=", sqlite3_errmsg(db));
            }
        }
        timer.tick("data_dict", std::to_string(data_dict.size()) + " entries");

        // 3. Query call stacks
        timer.stage("Querying call stacks");
        struct ChainGroup {
            int callchain_id;
            int type;                // 17 or 18
            std::vector<std::string> existing_addrs;
            long long existing_size;
        };
        std::vector<ChainGroup> groups;

        if (is_statistics) {
            // ── Statistic mode: native_hook_statistic WHERE type IN (17, 18) ──
            const char* stack_sql =
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
            if (sqlite3_prepare_v2(db, stack_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                StmtGuard guard(stmt);
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    ChainGroup g;
                    g.callchain_id = sqlite3_column_int(stmt, 0);
                    g.type = sqlite3_column_int(stmt, 1);
                    auto existing_addrs_str = (const char*)sqlite3_column_text(stmt, 2);
                    if (existing_addrs_str) {
                        g.existing_addrs = split(existing_addrs_str, ',');
                    }
                    g.existing_size = sqlite3_column_int64(stmt, 3);
                    groups.push_back(g);
                }
            } else {
                log::loge("Error: prepare stastics failed:sql=", stack_sql, ", err=", sqlite3_errmsg(db));
            }
        } else {
            // ── Detail mode: native_hook with ARK handle events ──
            const char* stack_sql =
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
            if (sqlite3_prepare_v2(db, stack_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                StmtGuard guard(stmt);
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    ChainGroup g;
                    g.callchain_id = sqlite3_column_int(stmt, 0);
                    g.type = sqlite3_column_int(stmt, 1);
                    auto existing_addrs_str = (const char*)sqlite3_column_text(stmt, 2);
                    if (existing_addrs_str) {
                        g.existing_addrs = split(existing_addrs_str, ',');
                    }
                    g.existing_size = sqlite3_column_int64(stmt, 3);
                    groups.push_back(g);
                }
            } else {
                log::loge("Error: prepare detail failed:sql=", stack_sql, ", err=", sqlite3_errmsg(db));
            }
        }

        if (groups.empty()) {
            throw std::runtime_error("No ARK handle call stacks found");
        }
        timer.tick("call_stacks", std::to_string(groups.size()) + " groups");

        // 4. Query frames (shared reader; chain filter pushed down via subquery)
        timer.stage("Querying frames");
        std::string chain_select;
        if (is_statistics) {
            chain_select =
                "SELECT callchain_id FROM native_hook_statistic WHERE type IN (17, 18)";
        } else {
            chain_select =
                "SELECT callchain_id FROM native_hook "
                "WHERE event_type = 'ARK_LOCAL_HANDLE_Alloc_Event' "
                "OR (event_type = 'ARK_GLOBAL_HANDLE_Alloc_Event' AND end_ts IS NULL)";
        }
        auto frames_by_chain = read_frames(db, chain_select);

        if (frames_by_chain.empty()) {
            throw std::runtime_error("Failed to query frames");
        }
        timer.tick("frames", std::to_string(frames_by_chain.size()) + " chains");

        // 5. Build call stack entries
        timer.stage("Building call stack entries");
        for (const auto& g : groups) {
            ResolvedCallStack cs;
            cs.id = g.callchain_id;
            if (g.type == 17) {
                cs.type = HtraceEventType::ARK_GLOBAL_HANDLE;
            } else if (g.type == 18) {
                cs.type = HtraceEventType::ARK_LOCAL_HANDLE;
            } else {
                cs.type = HtraceEventType::UNKNOWN;
            }
            cs.existing_size = g.existing_size;
            cs.existing_addrs = std::make_shared<std::vector<std::string>>(std::move(g.existing_addrs));

            // Look up frames for this chain
            auto it = frames_by_chain.find(g.callchain_id);
            if (it != frames_by_chain.end()) {
                for (const auto& fr : it->second) {
                    ResolvedStackFrame tf;
                    tf.depth = fr.depth;

                    auto sym_it = data_dict.find(fr.symbol_id);
                    tf.symbol = (sym_it != data_dict.end()) ? sym_it->second : "unknown";

                    if (fr.file_id_null) {
                        tf.lib = "unknown";
                    } else {
                        auto lib_it = data_dict.find(fr.file_id);
                        tf.lib = (lib_it != data_dict.end()) ? lib_it->second : "unknown";
                    }

                    if (fr.build_id_null) {
                        tf.build_id = "";
                    } else {
                        auto build_it = data_dict.find(fr.build_id);
                        tf.build_id = (build_it != data_dict.end()) ? build_it->second : "";
                    }

                    // TS: frame.vaddr != null ? `0x${Number(frame.vaddr).toString(16)}` : ''
                    if (!fr.vaddr_null) {
                        std::stringstream ss;
                        ss << "0x" << std::hex << fr.vaddr;
                        tf.lib_offset = ss.str();
                    }

                    cs.frames.push_back(std::move(tf));
                }
            }

            call_stacks.push_back(std::move(cs));
        }

        sqlite3_close(db);
        timer.tick("build_entries", std::to_string(call_stacks.size()) + " stacks");
        timer.done();
        return result;
    } catch (...) {
        sqlite3_close(db);
        throw;
    }
}

} // namespace mem_analyzer
