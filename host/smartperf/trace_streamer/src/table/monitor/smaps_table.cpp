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

#include "smaps_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index {
    ID = 0,
    TIME_STAMP,
    START_ADDRESS,
    END_ADDRESS,
    DIRTY,
    SWAPPER,
    RSS,
    PSS,
    SIZE,
    RESIDE,
    PROTECTION,
    PATH,
    SHARED_CLEAN,
    SHARED_DIRTY,
    PRIVATE_CLEAN,
    PRIVATE_DIRTY,
    SWAP,
    SWAP_PSS,
    TYPE
};
SmapsTable::SmapsTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("timeStamp", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_addr", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_addr", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("dirty", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("swapper", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("resident_size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("pss", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("virtaul_size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("reside", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("protection_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("path_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("shared_clean", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("shared_dirty", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("private_clean", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("private_dirty", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("swap", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("swap_pss", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tablePriKey_.push_back("id");
}

SmapsTable::~SmapsTable() {}

std::unique_ptr<TableBase::Cursor> SmapsTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

SmapsTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstSmapsData().Size())),
      smapsObj_(dataCache->GetConstSmapsData())
{
}

SmapsTable::Cursor::~Cursor() {}

int32_t SmapsTable::Cursor::Column(int32_t col) const
{
    switch (col) {
        case ID:
            sqlite3_result_int64(context_, smapsObj_.IdsData()[CurrentRow()]);
            break;
        case TIME_STAMP:
            sqlite3_result_int64(context_, smapsObj_.TimeStamps()[CurrentRow()]);
            break;
        case START_ADDRESS:
            sqlite3_result_text(context_, smapsObj_.StartAddrs()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        case END_ADDRESS:
            sqlite3_result_text(context_, smapsObj_.EndAddrs()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        case DIRTY:
            sqlite3_result_int64(context_, smapsObj_.Dirtys()[CurrentRow()]);
            break;
        case SWAPPER:
            sqlite3_result_int64(context_, smapsObj_.Swappers()[CurrentRow()]);
            break;
        case RSS:
            sqlite3_result_int64(context_, smapsObj_.Rss()[CurrentRow()]);
            break;
        case PSS:
            sqlite3_result_int64(context_, smapsObj_.Pss()[CurrentRow()]);
            break;
        case SIZE:
            sqlite3_result_int64(context_, smapsObj_.Sizes()[CurrentRow()]);
            break;
        case RESIDE:
            sqlite3_result_double(context_, smapsObj_.Resides()[CurrentRow()]);
            break;
        case PROTECTION:
            sqlite3_result_int64(context_, smapsObj_.ProtectionIds()[CurrentRow()]);
            break;
        case PATH:
            sqlite3_result_int64(context_, smapsObj_.PathIds()[CurrentRow()]);
            break;
        case SHARED_CLEAN:
            sqlite3_result_int64(context_, smapsObj_.SharedClean()[CurrentRow()]);
            break;
        case SHARED_DIRTY:
            sqlite3_result_int64(context_, smapsObj_.SharedDirty()[CurrentRow()]);
            break;
        case PRIVATE_CLEAN:
            sqlite3_result_int64(context_, smapsObj_.PrivateClean()[CurrentRow()]);
            break;
        case PRIVATE_DIRTY:
            sqlite3_result_int64(context_, smapsObj_.PrivateDirty()[CurrentRow()]);
            break;
        case SWAP:
            sqlite3_result_int64(context_, smapsObj_.Swap()[CurrentRow()]);
            break;
        case SWAP_PSS:
            sqlite3_result_int64(context_, smapsObj_.SwapPss()[CurrentRow()]);
            break;
        case TYPE:
            sqlite3_result_int64(context_, smapsObj_.Type()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}

} // namespace TraceStreamer
} // namespace SysTuning
