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

#include "memory_rs_image_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    ID = 0,
    IPID,
    TS,
    MEM_SIZE,
    TYPE_INDEX,
    SURFACE_NAME_INDEX,
};
MemoryRSImageTable::MemoryRSImageTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("mem_size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("surface_name_id", "INTEGER"));
    tablePriKey_.push_back("id");
}

MemoryRSImageTable::~MemoryRSImageTable() {}

std::unique_ptr<TableBase::Cursor> MemoryRSImageTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

MemoryRSImageTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstRSImageDumpInfo().Size())),
      rsImageDumpInfoObj_(dataCache->GetConstRSImageDumpInfo())
{
}

MemoryRSImageTable::Cursor::~Cursor() {}

int32_t MemoryRSImageTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, rsImageDumpInfoObj_.IdsData()[CurrentRow()]);
            break;
        case Index::IPID:
            if (rsImageDumpInfoObj_.Ipids()[CurrentRow()] != INVALID_IPID) {
                sqlite3_result_int64(context_, rsImageDumpInfoObj_.Ipids()[CurrentRow()]);
            }
            break;
        case Index::TS:
            sqlite3_result_int64(context_, rsImageDumpInfoObj_.TimeStampData()[CurrentRow()]);
            break;
        case Index::MEM_SIZE:
            sqlite3_result_int64(context_, rsImageDumpInfoObj_.MemSizes()[CurrentRow()]);
            break;
        case Index::TYPE_INDEX:
            sqlite3_result_int64(context_, rsImageDumpInfoObj_.TypeIndexs()[CurrentRow()]);
            break;
        case Index::SURFACE_NAME_INDEX:
            sqlite3_result_int64(context_, rsImageDumpInfoObj_.SurfaceNameIndexs()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
