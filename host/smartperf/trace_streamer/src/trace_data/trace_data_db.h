/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef TRACE_DATA_DB_H
#define TRACE_DATA_DB_H

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include "sqlite3.h"

struct ElfSymbolTable {
    uint64_t filePathIndex;
    uint64_t textVaddr;
    uint32_t textOffset;
    uint32_t symEntSize;
    std::string strTable;
    std::string symTable;
};

namespace SysTuning {
namespace TraceStreamer {
const int32_t SEND_CONTINUE = 0;
const int32_t SEND_FINISH = 1;
constexpr int32_t DATABASE_BASE = (1U << 20);
class TraceDataDB {
public:
    TraceDataDB();
    TraceDataDB(const TraceDataDB&) = delete;
    TraceDataDB& operator=(const TraceDataDB&) = delete;
    virtual ~TraceDataDB();
    virtual void InitDB() = 0;
    void Prepare();

public:
    using ResultCallBack = std::function<void(const std::string /* json result */, int32_t)>;
    int32_t ExportDatabase(const std::string& outputName, ResultCallBack resultCallBack = nullptr);
    std::vector<std::string> SearchData();
    int32_t OperateDatabase(const std::string& sql);
    int32_t SearchDatabase(const std::string& sql, ResultCallBack resultCallBack);
    int32_t SearchDatabase(const std::string& sql, uint8_t* out, int32_t outLen);
    int32_t SearchDatabase(const std::string& sql, bool print);
    void SetCancel(bool cancel);
    void AppendNewTable(std::string tableName);
    void EnableMetaTable(bool enabled);
    bool Cancel() const
    {
        return cancelQuery_;
    }

public:
    sqlite3* db_;

private:
    void ExecuteSql(const std::string_view& sql);
    void SendDatabase(ResultCallBack resultCallBack);
    static void GetRowString(sqlite3_stmt* stmt, int32_t colCount, std::string& rowStr);
    std::list<std::string> internalTables_ = {};
    bool exportMetaTable_ = true;
    bool pared_ = false;
    bool cancelQuery_ = false;
    std::string wasmDBName_;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif
