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

#include "so_static_initalization_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, IPID, TID, CALL_ID, START_TIME, END_TIME, SO_NAME, DEPTH };
SoStaticInitalizationTable::SoStaticInitalizationTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("tid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("call_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("so_name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("depth", "INTEGER"));
    tablePriKey_.push_back("id");
}

SoStaticInitalizationTable::~SoStaticInitalizationTable() {}

std::unique_ptr<TableBase::Cursor> SoStaticInitalizationTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

SoStaticInitalizationTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstStaticInitalizationData().Size())),
      staticInitalizationObj_(dataCache->GetConstStaticInitalizationData())
{
}

SoStaticInitalizationTable::Cursor::~Cursor() {}

int32_t SoStaticInitalizationTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(CurrentRow()));
            break;
        case IPID:
            sqlite3_result_int64(context_, static_cast<int64_t>(staticInitalizationObj_.Pids()[CurrentRow()]));
            break;
        case TID:
            sqlite3_result_int64(context_, static_cast<int64_t>(staticInitalizationObj_.Tids()[CurrentRow()]));
            break;
        case CALL_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(staticInitalizationObj_.CallIds()[CurrentRow()]));
            break;
        case START_TIME:
            sqlite3_result_int64(context_, static_cast<int64_t>(staticInitalizationObj_.StartTimes()[CurrentRow()]));
            break;
        case END_TIME:
            sqlite3_result_int64(context_, static_cast<int64_t>(staticInitalizationObj_.EndTimes()[CurrentRow()]));
            break;
        case SO_NAME:
            sqlite3_result_text(context_,
                                dataCache_->GetDataFromDict(staticInitalizationObj_.SoNames()[CurrentRow()]).c_str(),
                                STR_DEFAULT_LEN, nullptr);
            break;
        case DEPTH:
            sqlite3_result_int64(context_, static_cast<int64_t>(staticInitalizationObj_.Depths()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
