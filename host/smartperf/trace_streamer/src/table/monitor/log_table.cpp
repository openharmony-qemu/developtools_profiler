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

#include "log_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { SEQ = 0, TS, PID, TID, LEVEL, TAG, CONTEXT, ORIGINTS };
LogTable::LogTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("seq", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("pid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("tid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("level", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("tag", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("context", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("origints", "INTEGER"));
    tablePriKey_.push_back("ts");
}

LogTable::~LogTable() {}

std::unique_ptr<TableBase::Cursor> LogTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

LogTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstHilogData().Size())),
      logInfoObj_(dataCache->GetConstHilogData())
{
}

LogTable::Cursor::~Cursor() {}

int32_t LogTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::SEQ:
            sqlite3_result_int64(context_, static_cast<int64_t>(logInfoObj_.HilogLineSeqs()[CurrentRow()]));
            break;
        case Index::TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(logInfoObj_.TimeStampData()[CurrentRow()]));
            break;
        case Index::PID: {
            sqlite3_result_int64(context_, static_cast<int64_t>(logInfoObj_.Pids()[CurrentRow()]));
            break;
        }
        case Index::TID:
            sqlite3_result_int64(context_, static_cast<int64_t>(logInfoObj_.Tids()[CurrentRow()]));
            break;
        case Index::LEVEL: {
            if (logInfoObj_.Levels()[CurrentRow()] != INVALID_UINT64) {
                auto levelDataIndex = static_cast<size_t>(logInfoObj_.Levels()[CurrentRow()]);
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(levelDataIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case Index::TAG: {
            if (logInfoObj_.Tags()[CurrentRow()] != INVALID_UINT64) {
                auto tagDataIndex = static_cast<size_t>(logInfoObj_.Tags()[CurrentRow()]);
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(tagDataIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case Index::CONTEXT: {
            if (logInfoObj_.Contexts()[CurrentRow()] != INVALID_UINT64) {
                auto contextDataIndex = static_cast<size_t>(logInfoObj_.Contexts()[CurrentRow()]);
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(contextDataIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case Index::ORIGINTS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(logInfoObj_.OriginTimeStamData()[CurrentRow()]));
            break;
        }
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
