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

#include "memory_process_gpu_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    ID = 0,
    TS,
    GPU_NAME_ID,
    ALL_GPU_SIZE,
    ADDR,
    IPID,
    ITID,
    USED_GPU_SIZE,
};
MemoryProcessGpuTable::MemoryProcessGpuTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("gpu_name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("all_gpu_size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("addr", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("itid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("used_gpu_size", "INTEGER"));
    tablePriKey_.push_back("id");
}

MemoryProcessGpuTable::~MemoryProcessGpuTable() {}

std::unique_ptr<TableBase::Cursor> MemoryProcessGpuTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

MemoryProcessGpuTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstGpuProcessMemData().Size())),
      GpuProcessMemDataObj_(dataCache->GetConstGpuProcessMemData())
{
}

MemoryProcessGpuTable::Cursor::~Cursor() {}

int32_t MemoryProcessGpuTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, GpuProcessMemDataObj_.IdsData()[CurrentRow()]);
            break;
        case Index::TS:
            sqlite3_result_int64(context_, GpuProcessMemDataObj_.TimeStampData()[CurrentRow()]);
            break;
        case Index::GPU_NAME_ID:
            sqlite3_result_int64(context_, GpuProcessMemDataObj_.GpuNameIds()[CurrentRow()]);
            break;
        case Index::ALL_GPU_SIZE:
            sqlite3_result_int64(context_, GpuProcessMemDataObj_.AllGpuSizes()[CurrentRow()]);
            break;
        case Index::ADDR:
            sqlite3_result_text(context_, GpuProcessMemDataObj_.Addrs()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::IPID:
            sqlite3_result_int64(context_, GpuProcessMemDataObj_.Ipids()[CurrentRow()]);
            break;
        case Index::ITID:
            sqlite3_result_int64(context_, GpuProcessMemDataObj_.Itids()[CurrentRow()]);
            break;
        case Index::USED_GPU_SIZE:
            sqlite3_result_int64(context_, GpuProcessMemDataObj_.UsedGpuSizes()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
