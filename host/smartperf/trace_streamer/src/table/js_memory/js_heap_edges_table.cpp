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

#include "js_heap_edges_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { FILE_ID = 0, EDGE_INDEX, TYPE, NAME_OR_INDEX, TO_NODE, FROM_NODE_ID, TO_NODE_ID };
JsHeapEdgesTable::JsHeapEdgesTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("edge_index", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("name_or_index", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("to_node", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("from_node_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("to_node_id", "INTEGER"));
    tablePriKey_.push_back("file_id");
}

JsHeapEdgesTable::~JsHeapEdgesTable() {}

std::unique_ptr<TableBase::Cursor> JsHeapEdgesTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsHeapEdgesTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsHeapEdgesData().Size())),
      jsHeapEdges_(dataCache->GetConstJsHeapEdgesData())
{
}

JsHeapEdgesTable::Cursor::~Cursor() {}

int32_t JsHeapEdgesTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::FILE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapEdges_.FileIds()[CurrentRow()]));
            break;
        case Index::EDGE_INDEX:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapEdges_.EdgeIndexs()[CurrentRow()]));
            break;
        case Index::TYPE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapEdges_.Types()[CurrentRow()]));
            break;
        case Index::NAME_OR_INDEX:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapEdges_.NameOrIndexs()[CurrentRow()]));
            break;
        case Index::TO_NODE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapEdges_.ToNodes()[CurrentRow()]));
            break;
        case Index::FROM_NODE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapEdges_.FromNodeIds()[CurrentRow()]));
            break;
        case Index::TO_NODE_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsHeapEdges_.ToNodeIds()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
