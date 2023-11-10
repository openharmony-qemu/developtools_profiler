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

#include "sysevent_measure_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, SERIAL, TS, NAME_ID, KEY_ID, TYPE, INT_VALUE, STRING_VALUE };
SysEventMeasureTable::SysEventMeasureTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("serial", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("key_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("int_value", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("string_value", "TEXT"));
    tablePriKey_.push_back("id");
}

SysEventMeasureTable::~SysEventMeasureTable() {}

std::unique_ptr<TableBase::Cursor> SysEventMeasureTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

SysEventMeasureTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstSyseventMeasureData().Size())),
      sysEventMeasure_(dataCache->GetConstSyseventMeasureData())
{
}

SysEventMeasureTable::Cursor::~Cursor() {}

int32_t SysEventMeasureTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, dataCache_->GetConstSyseventMeasureData().IdsData()[CurrentRow()]);
            break;
        case Index::SERIAL:
            sqlite3_result_int64(context_, dataCache_->GetConstSyseventMeasureData().Serial()[CurrentRow()]);
            break;
        case Index::TS:
            sqlite3_result_int64(context_, dataCache_->GetConstSyseventMeasureData().Ts()[CurrentRow()]);
            break;
        case Index::NAME_ID:
            sqlite3_result_int(context_, dataCache_->GetConstSyseventMeasureData().NameFilterId()[CurrentRow()]);
            break;
        case Index::KEY_ID:
            sqlite3_result_int(context_, dataCache_->GetConstSyseventMeasureData().AppKeyFilterId()[CurrentRow()]);
            break;
        case Index::TYPE:
            sqlite3_result_int(context_, dataCache_->GetConstSyseventMeasureData().Type()[CurrentRow()]);
            break;
        case Index::INT_VALUE:
            sqlite3_result_double(context_, dataCache_->GetConstSyseventMeasureData().NumValue()[CurrentRow()]);
            break;
        case Index::STRING_VALUE:
            sqlite3_result_text(context_,
                                dataCache_->GetDataFromDict(sysEventMeasure_.StringValue()[CurrentRow()]).c_str(),
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
