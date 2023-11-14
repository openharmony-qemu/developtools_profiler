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

#include "js_heap_sample_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { FILE_ID = 0, TIMESTAMP_US, LAST_ASSIGNED_ID };
JsHeapSampleTable::JsHeapSampleTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("timestamp_us", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("last_assigned_id", "INTEGER"));
    tablePriKey_.push_back("file_id");
}

JsHeapSampleTable::~JsHeapSampleTable() {}

std::unique_ptr<TableBase::Cursor> JsHeapSampleTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsHeapSampleTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsHeapSampleData().Size())),
      jsHeapSample_(dataCache->GetConstJsHeapSampleData())
{
}

JsHeapSampleTable::Cursor::~Cursor() {}

int32_t JsHeapSampleTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::FILE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapSample_.FileIds()[CurrentRow()]));
            break;
        case Index::TIMESTAMP_US:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapSample_.TimeStampUs()[CurrentRow()]));
            break;
        case Index::LAST_ASSIGNED_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapSample_.LastAssignedIds()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
