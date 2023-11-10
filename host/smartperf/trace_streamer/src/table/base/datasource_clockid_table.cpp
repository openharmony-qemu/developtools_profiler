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

#include "datasource_clockid_table.h"
#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, NAME, CLOCK_ID };
DataSourceClockIdTableTable::DataSourceClockIdTableTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("data_source_name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("clock_id", "INTEGER"));
    tablePriKey_.push_back("id");
}

DataSourceClockIdTableTable::~DataSourceClockIdTableTable() {}

std::unique_ptr<TableBase::Cursor> DataSourceClockIdTableTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

DataSourceClockIdTableTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstDataSourceClockIdData().Size())),
      dataSourceClockIdData_(dataCache->GetConstDataSourceClockIdData())
{
}

DataSourceClockIdTableTable::Cursor::~Cursor() {}

int32_t DataSourceClockIdTableTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, CurrentRow());
            break;
        case Index::NAME:
            sqlite3_result_text(context_, dataSourceClockIdData_.Names()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::CLOCK_ID:
            sqlite3_result_int(context_, static_cast<int64_t>(dataSourceClockIdData_.ClockIds()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
