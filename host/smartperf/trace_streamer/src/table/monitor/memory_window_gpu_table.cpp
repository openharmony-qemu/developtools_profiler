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

#include "memory_window_gpu_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, TS, WINDOW_NAME_ID, WINDOW_ID, MODULE_NAME_ID, CATEGORY_NAME_ID, SIZE, COUNT, PURGEABLE_SIZE };
MemoryWindowGpuTable::MemoryWindowGpuTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("window_name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("window_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("module_name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("category_name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("purgeable_size", "INTEGER"));
    tablePriKey_.push_back("id");
}

MemoryWindowGpuTable::~MemoryWindowGpuTable() {}

std::unique_ptr<TableBase::Cursor> MemoryWindowGpuTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

MemoryWindowGpuTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstGpuWindowMemData().Size())),
      GpuWindowMemDataObj_(dataCache->GetConstGpuWindowMemData())
{
}

MemoryWindowGpuTable::Cursor::~Cursor() {}

int32_t MemoryWindowGpuTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, GpuWindowMemDataObj_.IdsData()[CurrentRow()]);
            break;
        case TS:
            sqlite3_result_int64(context_, GpuWindowMemDataObj_.TimeStampData()[CurrentRow()]);
            break;
        case WINDOW_NAME_ID:
            sqlite3_result_int64(context_, GpuWindowMemDataObj_.WindowNameIds()[CurrentRow()]);
            break;
        case WINDOW_ID:
            sqlite3_result_int64(context_, GpuWindowMemDataObj_.WindowIds()[CurrentRow()]);
            break;
        case MODULE_NAME_ID:
            sqlite3_result_int64(context_, GpuWindowMemDataObj_.ModuleNameIds()[CurrentRow()]);
            break;
        case CATEGORY_NAME_ID:
            sqlite3_result_int64(context_, GpuWindowMemDataObj_.CategoryNameIds()[CurrentRow()]);
            break;
        case SIZE:
            sqlite3_result_int64(context_, GpuWindowMemDataObj_.Sizes()[CurrentRow()]);
            break;
        case COUNT:
            sqlite3_result_int(context_, GpuWindowMemDataObj_.Counts()[CurrentRow()]);
            break;
        case PURGEABLE_SIZE:
            sqlite3_result_int64(context_, GpuWindowMemDataObj_.PurgeableSizes()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
