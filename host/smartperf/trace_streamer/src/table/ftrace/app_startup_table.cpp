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

#include "app_startup_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, CALL_ID, IPID, TID, START_TIME, END_TIME, START_NAME, PACKED_NAME };
AppStartupTable::AppStartupTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("call_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("tid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_name", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("packed_name", "TEXT"));
    tablePriKey_.push_back("id");
}

AppStartupTable::~AppStartupTable() {}

std::unique_ptr<TableBase::Cursor> AppStartupTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

AppStartupTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstAppStartupData().Size())),
      appStartupObj_(dataCache->GetConstAppStartupData())
{
}

AppStartupTable::Cursor::~Cursor() {}

int32_t AppStartupTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(CurrentRow()));
            break;
        case IPID:
            sqlite3_result_int64(context_, static_cast<int64_t>(appStartupObj_.Pids()[CurrentRow()]));
            break;
        case TID:
            sqlite3_result_int64(context_, static_cast<int64_t>(appStartupObj_.Tids()[CurrentRow()]));
            break;
        case CALL_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(appStartupObj_.CallIds()[CurrentRow()]));
            break;
        case START_TIME:
            sqlite3_result_int64(context_, static_cast<int64_t>(appStartupObj_.StartTimes()[CurrentRow()]));
            break;
        case END_TIME:
            sqlite3_result_int64(context_, static_cast<int64_t>(appStartupObj_.EndTimes()[CurrentRow()]));
            break;
        case START_NAME:
            sqlite3_result_int64(context_, static_cast<int64_t>(appStartupObj_.StartNames()[CurrentRow()]));
            break;
        case PACKED_NAME:
            sqlite3_result_text(context_,
                                dataCache_->GetDataFromDict(appStartupObj_.PackedNames()[CurrentRow()]).c_str(),
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
