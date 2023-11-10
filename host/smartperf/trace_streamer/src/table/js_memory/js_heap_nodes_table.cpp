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

#include "js_heap_nodes_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    FILE_ID = 0,
    NODE_INDEX,
    TYPE,
    NAME,
    NODE_ID,
    SELF_SIZE,
    EDGE_COUNT,
    TRACE_NODE_ID,
    DETACHEDNESS
};
JsHeapNodesTable::JsHeapNodesTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("node_index", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("self_size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("edge_count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("trace_node_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("detachedness", "INTEGER"));
    tablePriKey_.push_back("file_id");
}

JsHeapNodesTable::~JsHeapNodesTable() {}

std::unique_ptr<TableBase::Cursor> JsHeapNodesTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsHeapNodesTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsHeapNodesData().Size())),
      jsHeapNodes_(dataCache->GetConstJsHeapNodesData())
{
}

JsHeapNodesTable::Cursor::~Cursor() {}

int32_t JsHeapNodesTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::FILE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.FileIds()[CurrentRow()]));
            break;
        case Index::NODE_INDEX:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.NodeIndexs()[CurrentRow()]));
            break;
        case Index::TYPE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.Types()[CurrentRow()]));
            break;
        case Index::NAME:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.Names()[CurrentRow()]));
            break;
        case Index::NODE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.NodeIds()[CurrentRow()]));
            break;
        case Index::SELF_SIZE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.SelfSizes()[CurrentRow()]));
            break;
        case Index::EDGE_COUNT:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.EdgeCounts()[CurrentRow()]));
            break;
        case Index::TRACE_NODE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.TraceNodeIds()[CurrentRow()]));
            break;
        case Index::DETACHEDNESS:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapNodes_.DetachedNess()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
