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

#include "js_heap_string_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { FILE_ID = 0, FILE_INDEX, STRING };
JsHeapStringTable::JsHeapStringTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("file_index", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("string", "TEXT"));
    tablePriKey_.push_back("file_id");
}

JsHeapStringTable::~JsHeapStringTable() {}

std::unique_ptr<TableBase::Cursor> JsHeapStringTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsHeapStringTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsHeapStringData().Size())),
      jsHeapString_(dataCache->GetConstJsHeapStringData())
{
}

JsHeapStringTable::Cursor::~Cursor() {}

int32_t JsHeapStringTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::FILE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapString_.FileIds()[CurrentRow()]));
            break;
        case Index::FILE_INDEX:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapString_.FileIndexs()[CurrentRow()]));
            break;
        case Index::STRING:
            sqlite3_result_text(context_, jsHeapString_.Strings()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}

} // namespace TraceStreamer
} // namespace SysTuning
