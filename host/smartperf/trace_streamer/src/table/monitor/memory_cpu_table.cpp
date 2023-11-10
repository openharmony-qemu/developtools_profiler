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

#include "memory_cpu_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, TS, TOTAL_SIZE };
MemoryCpuTable::MemoryCpuTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("total_size", "INTEGER"));
    tablePriKey_.push_back("id");
}

MemoryCpuTable::~MemoryCpuTable() {}

std::unique_ptr<TableBase::Cursor> MemoryCpuTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

MemoryCpuTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstCpuDumpInfo().Size())),
      cpuDumpInfoObj_(dataCache->GetConstCpuDumpInfo())
{
}

MemoryCpuTable::Cursor::~Cursor() {}

int32_t MemoryCpuTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, cpuDumpInfoObj_.IdsData()[CurrentRow()]);
            break;
        case Index::TS:
            sqlite3_result_int64(context_, cpuDumpInfoObj_.TimeStampData()[CurrentRow()]);
            break;
        case Index::TOTAL_SIZE:
            sqlite3_result_int64(context_, cpuDumpInfoObj_.TotalSizes()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
