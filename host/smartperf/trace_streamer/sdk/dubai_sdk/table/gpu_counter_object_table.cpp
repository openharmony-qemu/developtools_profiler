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

#include "gpu_counter_object_table.h"
#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
namespace {
enum Index { COUNTER_ID = 0, COUNTER_NAME = 1 };
}
GpuCounterObjectTable::GpuCounterObjectTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("counter_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("counter_name", "REAL"));
    tablePriKey_.push_back("counter_id");
}

GpuCounterObjectTable::~GpuCounterObjectTable() {}

std::unique_ptr<TableBase::Cursor> GpuCounterObjectTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

GpuCounterObjectTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstGpuCounterObjectData().Size())),
      gpuCounterObjectDataObj_(dataCache->GetConstGpuCounterObjectData())
{
}

GpuCounterObjectTable::Cursor::~Cursor() {}

int32_t GpuCounterObjectTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case COUNTER_ID: {
            sqlite3_result_int64(context_, static_cast<int64_t>(gpuCounterObjectDataObj_.CounterId()[CurrentRow()]));
            break;
        }
        case COUNTER_NAME: {
            sqlite3_result_text(context_, gpuCounterObjectDataObj_.CounterName()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        }
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
