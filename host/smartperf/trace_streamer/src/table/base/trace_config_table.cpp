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

#include "trace_config_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, TRACE_SOURCE, KEY, VALUE };
TraceConfigTable::TraceConfigTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("trace_source", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("key", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("value", "TEXT"));
    tablePriKey_.push_back("id");
}

TraceConfigTable::~TraceConfigTable() {}

std::unique_ptr<TableBase::Cursor> TraceConfigTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

TraceConfigTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstTraceConfigData().Size())),
      TraceConfigData_(dataCache->GetConstTraceConfigData())
{
}

TraceConfigTable::Cursor::~Cursor() {}

int32_t TraceConfigTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(CurrentRow()));
            break;
        case Index::TRACE_SOURCE:
            sqlite3_result_text(context_, dataCache_->GetConstTraceConfigData().TraceSource()[CurrentRow()].c_str(),
                                STR_DEFAULT_LEN, nullptr);
            break;
        case Index::KEY:
            sqlite3_result_text(context_, dataCache_->GetConstTraceConfigData().Key()[CurrentRow()].c_str(),
                                STR_DEFAULT_LEN, nullptr);
            break;
        case Index::VALUE:
            sqlite3_result_text(context_, dataCache_->GetConstTraceConfigData().Value()[CurrentRow()].c_str(),
                                STR_DEFAULT_LEN, nullptr);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
