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

#include "sysevent_subkey_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, APP_NAME, APP_KEY };
SysEventSubkeyTable::SysEventSubkeyTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("app_name", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("app_key", "INTEGER"));
    tablePriKey_.push_back("id");
}

SysEventSubkeyTable::~SysEventSubkeyTable() {}

std::unique_ptr<TableBase::Cursor> SysEventSubkeyTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

SysEventSubkeyTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstHiSysEventSubkeysData().Size())),
      sysEventSubkeys_(dataCache->GetConstHiSysEventSubkeysData())
{
}

SysEventSubkeyTable::Cursor::~Cursor() {}

int32_t SysEventSubkeyTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, sysEventSubkeys_.IdsData()[CurrentRow()]);
            break;
        case Index::APP_NAME:
            sqlite3_result_int64(context_, sysEventSubkeys_.SysEventNameId()[CurrentRow()]);
            break;
        case Index::APP_KEY:
            sqlite3_result_int64(context_, sysEventSubkeys_.SysEventSubkeyId()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
