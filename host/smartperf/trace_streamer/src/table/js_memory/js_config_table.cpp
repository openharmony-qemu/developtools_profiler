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

#include "js_config_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    PID = 0,
    TYPE,
    INTERVAL,
    CAPTURE_NUMERIC_VALUE,
    TRACK_ALLOCATION,
    CPU_PROFILER,
    CPU_PROFILER_INTERVAL
};
JsConfigTable::JsConfigTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("pid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("interval", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("capture_numeric_value", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("trace_allocation", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("enable_cpu_profiler ", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("cpu_profiler_interval ", "INTEGER"));
    tablePriKey_.push_back("pid");
}

JsConfigTable::~JsConfigTable() {}

std::unique_ptr<TableBase::Cursor> JsConfigTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

JsConfigTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstJsConfigData().Size())),
      jsConfig_(dataCache->GetConstJsConfigData())
{
}

JsConfigTable::Cursor::~Cursor() {}

int32_t JsConfigTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::PID:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsConfig_.Pids()[CurrentRow()]));
            break;
        case Index::TYPE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsConfig_.Types()[CurrentRow()]));
            break;
        case Index::INTERVAL:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsConfig_.Intervals()[CurrentRow()]));
            break;
        case Index::CAPTURE_NUMERIC_VALUE:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsConfig_.CaptureNumericValue()[CurrentRow()]));
            break;
        case Index::TRACK_ALLOCATION:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsConfig_.TrackAllocations()[CurrentRow()]));
            break;
        case Index::CPU_PROFILER:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsConfig_.CpuProfiler()[CurrentRow()]));
            break;
        case Index::CPU_PROFILER_INTERVAL:
            sqlite3_result_int64(context_, static_cast<int64_t>(jsConfig_.CpuProfilerInterval()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
