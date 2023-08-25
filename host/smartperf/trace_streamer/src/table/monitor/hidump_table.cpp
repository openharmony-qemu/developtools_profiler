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

#include "hidump_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, TS, FPS };
HidumpTable::HidumpTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("fps", "INTEGER"));
    tablePriKey_.push_back("ts");
}

HidumpTable::~HidumpTable() {}
std::unique_ptr<TableBase::Cursor> HidumpTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

HidumpTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstHidumpData().Size())),
      hidumpObj_(dataCache->GetConstHidumpData())
{
}

HidumpTable::Cursor::~Cursor() {}

int32_t HidumpTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(CurrentRow()));
            break;
        case TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(hidumpObj_.TimeStampData()[CurrentRow()]));
            break;
        case FPS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(hidumpObj_.Fpss()[CurrentRow()]));
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
