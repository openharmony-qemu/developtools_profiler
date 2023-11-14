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

#include "js_heap_info_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { FILE_ID = 0, KEY, TYPE, INT_VALUE, STR_VALUE };
JsHeapInfoTable::JsHeapInfoTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("key", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("int_value", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("str_value", "TEXT"));
    tablePriKey_.push_back("file_id");
}

JsHeapInfoTable::~JsHeapInfoTable() {}

std::unique_ptr<TableBase::Cursor> JsHeapInfoTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsHeapInfoTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsHeapInfoData().Size())),
      jsHeapInfo_(dataCache->GetConstJsHeapInfoData())
{
}

JsHeapInfoTable::Cursor::~Cursor() {}

int32_t JsHeapInfoTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::FILE_ID:
            sqlite3_result_int64(
                context_,
                static_cast<int64_t>(jsHeapInfo_.FileIds()[CurrentRow()])); // IdsData() will be optimized
            break;
        case Index::KEY:
            sqlite3_result_text(context_, jsHeapInfo_.Keys()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        case Index::TYPE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapInfo_.Types()[CurrentRow()]));
            break;
        case Index::INT_VALUE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapInfo_.IntValues()[CurrentRow()]));
            break;
        case Index::STR_VALUE:
            sqlite3_result_text(context_, jsHeapInfo_.StrValues()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}

} // namespace TraceStreamer
} // namespace SysTuning
