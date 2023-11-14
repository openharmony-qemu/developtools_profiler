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

#include "thread_filter_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, TYPE, NAME, INTERNAL_TID };
ThreadFilterTable::ThreadFilterTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("itid", "INTEGER"));
    tablePriKey_.push_back("id");
}

ThreadFilterTable::~ThreadFilterTable() {}

std::unique_ptr<TableBase::Cursor> ThreadFilterTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

ThreadFilterTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstThreadFilterData().Size()))
{
}

ThreadFilterTable::Cursor::~Cursor() {}

int32_t ThreadFilterTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                               dataCache_->GetConstThreadFilterData().FilterIdData()[CurrentRow()]));
            break;
        case Index::TYPE:
            sqlite3_result_text(context_, "thread_filter", STR_DEFAULT_LEN, nullptr);
            break;
        case Index::NAME: {
            std::string str =
                dataCache_->GetDataFromDict(dataCache_->GetConstThreadFilterData().NameIndexData()[CurrentRow()]);
            sqlite3_result_text(context_, str.c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        }
        case Index::INTERNAL_TID:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                               dataCache_->GetConstThreadFilterData().InternalTidData()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
