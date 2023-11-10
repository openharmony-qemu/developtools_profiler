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

#include "memory_ashmem_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    ID = 0,
    TS,
    IPID,
    ADJ,
    FD,
    ASHMEM_NAME_ID,
    SIZE,
    PSS,
    ASHMEM_ID,
    TIME,
    REF_COUNT,
    PURGED,
    FLAG,
};
MemoryAshMemTable::MemoryAshMemTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("adj", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("fd", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ashmem_name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("pss", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ashmem_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ref_count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("purged", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("flag", "INTEGER"));
    tablePriKey_.push_back("id");
}

MemoryAshMemTable::~MemoryAshMemTable() {}

std::unique_ptr<TableBase::Cursor> MemoryAshMemTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

MemoryAshMemTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstAshMemData().Size())),
      AshMemDataObj_(dataCache->GetConstAshMemData())
{
}

MemoryAshMemTable::Cursor::~Cursor() {}

int32_t MemoryAshMemTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, AshMemDataObj_.IdsData()[CurrentRow()]);
            break;
        case Index::TS:
            sqlite3_result_int64(context_, AshMemDataObj_.TimeStampData()[CurrentRow()]);
            break;
        case Index::IPID:
            sqlite3_result_int64(context_, AshMemDataObj_.Ipids()[CurrentRow()]);
            break;
        case Index::ADJ:
            sqlite3_result_int(context_, AshMemDataObj_.Adjs()[CurrentRow()]);
            break;
        case Index::FD:
            sqlite3_result_int(context_, AshMemDataObj_.Fds()[CurrentRow()]);
            break;
        case Index::ASHMEM_NAME_ID:
            sqlite3_result_int(context_, AshMemDataObj_.AshmemNameIds()[CurrentRow()]);
            break;
        case Index::SIZE:
            sqlite3_result_int64(context_, AshMemDataObj_.Sizes()[CurrentRow()]);
            break;
        case Index::PSS:
            sqlite3_result_int64(context_, AshMemDataObj_.Psss()[CurrentRow()]);
            break;
        case Index::ASHMEM_ID:
            sqlite3_result_int64(context_, AshMemDataObj_.AshmemIds()[CurrentRow()]);
            break;
        case Index::TIME:
            sqlite3_result_int64(context_, AshMemDataObj_.Times()[CurrentRow()]);
            break;
        case Index::REF_COUNT:
            sqlite3_result_int64(context_, AshMemDataObj_.RefCounts()[CurrentRow()]);
            break;
        case Index::PURGED:
            sqlite3_result_int64(context_, AshMemDataObj_.Purgeds()[CurrentRow()]);
            break;
        case Index::FLAG:
            sqlite3_result_int(context_, AshMemDataObj_.Flags()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
