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

#include "memory_dma_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    ID = 0,
    TS,
    IPID,
    FD,
    SIZE,
    INO,
    EXP_PID,
    EXP_TASK_COMM,
    BUF_NAME_ID,
    EXP_NAME_ID,
    FLAG,
};
MemoryDmaTable::MemoryDmaTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("fd", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ino", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("exp_pid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("exp_task_comm_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("buf_name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("exp_name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("flag", "INTEGER"));
    tablePriKey_.push_back("id");
}

MemoryDmaTable::~MemoryDmaTable() {}

std::unique_ptr<TableBase::Cursor> MemoryDmaTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

MemoryDmaTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstDmaMemData().Size())),
      DmaMemDataObj_(dataCache->GetConstDmaMemData())
{
}

MemoryDmaTable::Cursor::~Cursor() {}

int32_t MemoryDmaTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, DmaMemDataObj_.IdsData()[CurrentRow()]);
            break;
        case Index::TS:
            sqlite3_result_int64(context_, DmaMemDataObj_.TimeStampData()[CurrentRow()]);
            break;
        case Index::IPID:
            sqlite3_result_int64(context_, DmaMemDataObj_.Ipids()[CurrentRow()]);
            break;
        case Index::FD:
            sqlite3_result_int(context_, DmaMemDataObj_.Fds()[CurrentRow()]);
            break;
        case Index::SIZE:
            sqlite3_result_int64(context_, DmaMemDataObj_.Sizes()[CurrentRow()]);
            break;
        case Index::INO:
            sqlite3_result_int(context_, DmaMemDataObj_.Inos()[CurrentRow()]);
            break;
        case Index::EXP_PID:
            sqlite3_result_int(context_, DmaMemDataObj_.ExpPids()[CurrentRow()]);
            break;
        case Index::EXP_TASK_COMM:
            sqlite3_result_int64(context_, DmaMemDataObj_.ExpTaskCommIds()[CurrentRow()]);
            break;
        case Index::BUF_NAME_ID:
            sqlite3_result_int64(context_, DmaMemDataObj_.BufNameIds()[CurrentRow()]);
            break;
        case Index::EXP_NAME_ID:
            sqlite3_result_int64(context_, DmaMemDataObj_.ExpNameIds()[CurrentRow()]);
            break;
        case Index::FLAG:
            sqlite3_result_int(context_, DmaMemDataObj_.Flags()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
