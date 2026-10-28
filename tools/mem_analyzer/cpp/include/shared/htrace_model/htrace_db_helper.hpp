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
#pragma once
// ============================================================================
//  htrace_db_helper.hpp
//
//  htrace / tshandle SQLite 读取的统一底层工具：
//   - StmtGuard          RAII finalize
//   - query_trace_meta   时间区间 + 模式(statistic/detail) + 进程名
//   - read_frames        native_hook_frame 帧读取（过滤子查询下推）
// ============================================================================
#include "shared/htrace_model/htrace_data.hpp"
#include <sqlite3.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mem_analyzer {

// RAII 守卫：作用域结束时自动 finalize sqlite3_stmt
class StmtGuard {
public:
    explicit StmtGuard(sqlite3_stmt* s = nullptr) : stmt_(s) {}
    ~StmtGuard() { reset(nullptr); }
    StmtGuard(const StmtGuard&) = delete;
    StmtGuard& operator=(const StmtGuard&) = delete;
    StmtGuard(StmtGuard&& o) noexcept : stmt_(o.stmt_) { o.stmt_ = nullptr; }
    StmtGuard& operator=(StmtGuard&& o) noexcept;

    sqlite3_stmt* get() const { return stmt_; }
    sqlite3_stmt* operator->() const { return stmt_; }

    // 释放当前 stmt 并接管新的（传 nullptr 即仅释放）
    void reset(sqlite3_stmt* s);

private:
    sqlite3_stmt* stmt_ = nullptr;
};

// 读时间区间（trace_range + clock_snapshot）、模式（statistic/detail）与进程名
TraceMeta QueryTraceMeta(sqlite3* db);

// native_hook_frame 的统一原始行（可空列统一为 flag + 默认值）
struct FrameRow {
    int       callchainId = 0;
    int       depth = 0;
    int       symbolId = 0;
    bool      fileIdNull = false;
    int       fileId = 0;
    bool      vaddrNull = false;
    uint64_t  vaddr = 0;
    bool      buildIdNull = false;
    int       buildId = 0;
};

// 按 callchain_id 分组读取帧。
// chain_select 是返回单列 callchain_id 的 SELECT（作为 IN 子查询），
// 过滤逻辑由调用方下推到 SQL，避免 C++ 收集 chain_ids 或拼超长 IN。
// 返回 map：callchain_id -> (按 depth 升序的帧)。
std::unordered_map<int, std::vector<FrameRow>> ReadFrames(sqlite3* db, const std::string& chainSelect);

// 按 callchain_id 读取分配地址（detail 模式：native_hook.addr，GROUP_CONCAT hex）。
// chain_select 是返回 callchain_id 的 SELECT（与 read_frames 同源，已含 end_ts IS NULL + event_type 过滤）。
// 返回 map：callchain_id -> (地址 hex 串列表，如 "0x5c843f8b00")。
// statistic 模式的地址在主 SQL 里经 addr_id→data_dict JOIN 获取，不走此函数。
std::unordered_map<int, std::vector<std::string>> ReadCallchainAddrs(sqlite3* db, const std::string& chainSelect);

} // namespace mem_analyzer
