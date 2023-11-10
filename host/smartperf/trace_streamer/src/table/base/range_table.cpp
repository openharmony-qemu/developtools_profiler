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

#include "range_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { START_TS = 0, END_TS };
RangeTable::RangeTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("start_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_ts", "INTEGER"));
    tablePriKey_.push_back("start_ts");
}

RangeTable::~RangeTable() {}

std::unique_ptr<TableBase::Cursor> RangeTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

RangeTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table) : TableBase::Cursor(dataCache, table, 1)
{
}

RangeTable::Cursor::~Cursor() {}

int32_t RangeTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::START_TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(dataCache_->TraceStartTime()));
            break;
        case Index::END_TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(dataCache_->TraceEndTime()));
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
