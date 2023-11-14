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

#include "js_heap_trace_function_info_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { FILE_ID = 0, FUNCTION_INDEX, FUNCTION_ID, NAME, SCRIPT_NAME, SCRIPT_ID, LINE, COLUMN };
JsHeapTraceFunctionInfoTable::JsHeapTraceFunctionInfoTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("function_index", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("function_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("script_name", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("script_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("line", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("column", "INTEGER"));
    tablePriKey_.push_back("file_id");
}

JsHeapTraceFunctionInfoTable::~JsHeapTraceFunctionInfoTable() {}

std::unique_ptr<TableBase::Cursor> JsHeapTraceFunctionInfoTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsHeapTraceFunctionInfoTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsHeapTraceFuncInfoData().Size())),
      jsHeapTraceFuncInfo_(dataCache->GetConstJsHeapTraceFuncInfoData())
{
}

JsHeapTraceFunctionInfoTable::Cursor::~Cursor() {}

int32_t JsHeapTraceFunctionInfoTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::FILE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceFuncInfo_.FileIds()[CurrentRow()]));
            break;
        case Index::FUNCTION_INDEX:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceFuncInfo_.FunctionIndexs()[CurrentRow()]));
            break;
        case Index::FUNCTION_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceFuncInfo_.FunctionIds()[CurrentRow()]));
            break;
        case Index::NAME:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceFuncInfo_.Names()[CurrentRow()]));
            break;
        case Index::SCRIPT_NAME:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceFuncInfo_.ScriptNames()[CurrentRow()]));
            break;
        case Index::SCRIPT_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceFuncInfo_.ScriptIds()[CurrentRow()]));
            break;
        case Index::LINE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceFuncInfo_.Lines()[CurrentRow()]));
            break;
        case Index::COLUMN:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceFuncInfo_.Columns()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
