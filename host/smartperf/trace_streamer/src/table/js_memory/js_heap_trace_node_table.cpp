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

#include "js_heap_trace_node_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { FILE_ID = 0, TRACE_NODE_ID, FUNCTION_INFO_INDEX, COUNT, SIZE, PARENT_ID };
JsHeapTraceNodeTable::JsHeapTraceNodeTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("function_info_index", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("parent_id", "INTEGER"));
    tablePriKey_.push_back("file_id");
}

JsHeapTraceNodeTable::~JsHeapTraceNodeTable() {}

std::unique_ptr<TableBase::Cursor> JsHeapTraceNodeTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsHeapTraceNodeTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsHeapTraceNodeData().Size())),
      jsHeapTraceNode_(dataCache->GetConstJsHeapTraceNodeData())
{
}

JsHeapTraceNodeTable::Cursor::~Cursor() {}

int32_t JsHeapTraceNodeTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::FILE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceNode_.FileIds()[CurrentRow()]));
            break;
        case Index::TRACE_NODE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceNode_.TraceNodeIDs()[CurrentRow()]));
            break;
        case Index::FUNCTION_INFO_INDEX:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceNode_.FunctionInfoIndexs()[CurrentRow()]));
            break;
        case Index::COUNT:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceNode_.Counts()[CurrentRow()]));
            break;
        case Index::SIZE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceNode_.NodeSizes()[CurrentRow()]));
            break;
        case Index::PARENT_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapTraceNode_.ParentIds()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
