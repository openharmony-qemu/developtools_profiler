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

#include "clock_event_filter_table.h"

#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, TYPE, NAME, CPU };
ClockEventFilterTable::ClockEventFilterTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("cpu", "INTEGER"));
    tablePriKey_.push_back("id");
}

ClockEventFilterTable::~ClockEventFilterTable() {}

std::unique_ptr<TableBase::Cursor> ClockEventFilterTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

ClockEventFilterTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstClockEventFilterData().Size()))
{
}

ClockEventFilterTable::Cursor::~Cursor() {}

int32_t ClockEventFilterTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::ID:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                               dataCache_->GetConstClockEventFilterData().IdsData()[CurrentRow()]));
            break;
        case Index::TYPE: {
            size_t typeId = static_cast<size_t>(dataCache_->GetConstClockEventFilterData().TypesData()[CurrentRow()]);
            sqlite3_result_text(context_, dataCache_->GetDataFromDict(typeId).c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        }
        case Index::NAME: {
            size_t strId = static_cast<size_t>(dataCache_->GetConstClockEventFilterData().NamesData()[CurrentRow()]);
            sqlite3_result_text(context_, dataCache_->GetDataFromDict(strId).c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        }
        case Index::CPU:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                               dataCache_->GetConstClockEventFilterData().CpusData()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
