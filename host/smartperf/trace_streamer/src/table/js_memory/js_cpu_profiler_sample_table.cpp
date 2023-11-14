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

#include "js_cpu_profiler_sample_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, FUNCTION_ID, START_TIME, END_TIME, DUR };
JsCpuProfilerSampleTable::JsCpuProfilerSampleTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("function_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tablePriKey_.push_back("id");
}

JsCpuProfilerSampleTable::~JsCpuProfilerSampleTable() {}

std::unique_ptr<TableBase::Cursor> JsCpuProfilerSampleTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsCpuProfilerSampleTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsCpuProfilerSampleData().Size())),
      jsCpuProfilerSample_(dataCache->GetConstJsCpuProfilerSampleData())
{
}

JsCpuProfilerSampleTable::Cursor::~Cursor() {}

int32_t JsCpuProfilerSampleTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerSample_.IdsData()[CurrentRow()]));
            break;
        case Index::FUNCTION_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerSample_.FunctionIds()[CurrentRow()]));
            break;
        case Index::START_TIME:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerSample_.StartTimes()[CurrentRow()]));
            break;
        case Index::END_TIME:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerSample_.EndTimes()[CurrentRow()]));
            break;
        case Index::DUR:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsCpuProfilerSample_.Durs()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
