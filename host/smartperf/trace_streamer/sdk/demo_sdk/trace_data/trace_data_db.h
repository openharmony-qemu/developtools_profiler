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
#include <string>
#include "sqlite3.h"

namespace SysTuning {
namespace TraceStreamer {
const int32_t SEND_CONTINUE = 0;
const int32_t SEND_FINISH = 1;
class TraceDataDB {
public:
    TraceDataDB();
    TraceDataDB(const TraceDataDB&) = delete;
    TraceDataDB& operator=(const TraceDataDB&) = delete;
    virtual ~TraceDataDB();
    virtual void InitDB() = 0;
    void Prepare();

public:
    int32_t ExportDatabase(const std::string& outputName);
    int32_t SearchData();
    int32_t OperateDatabase(const std::string& sql);
    using ResultCallBack = std::function<void(const std::string /* json result */, int32_t, int32_t)>;
    int32_t SearchDatabase(const std::string& sql, ResultCallBack resultCallBack);
    int32_t SearchDatabase(const std::string& sql, uint8_t* out, int32_t outLen);
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
    static void GetRowString(sqlite3_stmt* stmt, int32_t colCount, std::string& rowStr);
    int32_t SearchDatabase(const std::string& sql, bool print);
    std::list<std::string> internalTables_ = {};
    bool exportMetaTable_ = true;
    bool pared_ = false;
    bool cancelQuery_ = false;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif
