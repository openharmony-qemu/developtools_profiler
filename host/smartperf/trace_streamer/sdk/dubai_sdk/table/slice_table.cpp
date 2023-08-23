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

#include "slice_table.h"
#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
namespace {
enum Index { TS = 0, ENDTS = 1, ST = 2, ET = 3, VALUE = 4, SLICE_ID = 5 };
}
SliceTable::SliceTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("start_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("end", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("value", "DOUBLE"));
    tableColumn_.push_back(TableBase::ColumnInfo("slice_id", "INTEGER"));
    tablePriKey_.push_back("start_ts");
    tablePriKey_.push_back("slice_id");
}

SliceTable::~SliceTable() {}

std::unique_ptr<TableBase::Cursor> SliceTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

SliceTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstSliceData().Size())),
      sliceDataObj_(dataCache->GetConstSliceData())
{
}

SliceTable::Cursor::~Cursor() {}

int32_t SliceTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case TS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(sliceDataObj_.TimeStamp()[CurrentRow()]));
            break;
        }
        case ENDTS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(sliceDataObj_.EndTs()[CurrentRow()]));
            break;
        }
        case ST: {
            sqlite3_result_text(context_, sliceDataObj_.StartTime()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        }
        case ET: {
            sqlite3_result_text(context_, sliceDataObj_.EndTime()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        }
        case VALUE: {
            sqlite3_result_double(context_, static_cast<double>(sliceDataObj_.Value()[CurrentRow()]));
            break;
        }
        case SLICE_ID: {
            sqlite3_result_int64(context_, static_cast<int64_t>(sliceDataObj_.SliceId()[CurrentRow()]));
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
