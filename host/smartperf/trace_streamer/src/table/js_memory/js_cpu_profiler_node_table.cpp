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

#include "js_cpu_profiler_node_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    FUNCTION_ID = 0,
    FUNCTION_NAME,
    SCRIPT_ID,
    URL,
    LINE_NUMBER,
    COLUMN_NUMBER,
    HIT_COUNT,
    CHILDREN,
    PARENT_ID
};
JsCpuProfilerNodeTable::JsCpuProfilerNodeTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("function_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("function_index", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("script_id", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("url_index", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("line_number", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("column_number", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("hit_count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("children", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("parent_id", "INTEGER"));
    tablePriKey_.push_back("function_id");
}

JsCpuProfilerNodeTable::~JsCpuProfilerNodeTable() {}

std::unique_ptr<TableBase::Cursor> JsCpuProfilerNodeTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsCpuProfilerNodeTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsCpuProfilerNodeData().Size())),
      jsCpuProfilerNodes_(dataCache->GetConstJsCpuProfilerNodeData())
{
}

JsCpuProfilerNodeTable::Cursor::~Cursor() {}
int32_t JsCpuProfilerNodeTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::FUNCTION_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerNodes_.FunctionIds()[CurrentRow()]));
            break;
        case Index::FUNCTION_NAME:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerNodes_.FunctionNames()[CurrentRow()]));
            break;
        case Index::SCRIPT_ID:
            sqlite3_result_text(context_, jsCpuProfilerNodes_.ScriptIds()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::URL:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerNodes_.Urls()[CurrentRow()]));
            break;
        case Index::LINE_NUMBER:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerNodes_.LineNumbers()[CurrentRow()]));
            break;
        case Index::COLUMN_NUMBER:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerNodes_.ColumnNumbers()[CurrentRow()]));
            break;
        case Index::HIT_COUNT:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerNodes_.HitCounts()[CurrentRow()]));
            break;
        case Index::CHILDREN:
            sqlite3_result_text(context_, jsCpuProfilerNodes_.Children()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::PARENT_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerNodes_.Parents()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
