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

#include "disk_io_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    TS = 0,
    DUR,
    RD,
    WR,
    RD_SPEED,
    WR_SPEED,
    RD_COUNT,
    WR_COUNT,
    RD_COUNT_SPEED,
    WR_COUNT_SPEED,
};
DiskIOTable::DiskIOTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("rd", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("wr", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("rd_speed", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("wr_speed", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("rd_count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("wr_count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("rd_count_speed", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("wr_count_speed", "REAL"));
    tablePriKey_.push_back("ts");
}

DiskIOTable::~DiskIOTable() {}

std::unique_ptr<TableBase::Cursor> DiskIOTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

DiskIOTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstDiskIOData().Size())),
      diskIODataObj_(dataCache->GetConstDiskIOData())
{
}

DiskIOTable::Cursor::~Cursor() {}

int32_t DiskIOTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::TS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(diskIODataObj_.TimeStampData()[CurrentRow()]));
            break;
        }
        case Index::DUR: {
            sqlite3_result_int64(context_, static_cast<int64_t>(diskIODataObj_.Durs()[CurrentRow()]));
            break;
        }
        case Index::RD: {
            sqlite3_result_int64(context_, static_cast<int64_t>(diskIODataObj_.RdDatas()[CurrentRow()]));
            break;
        }
        case Index::WR: {
            sqlite3_result_int64(context_, static_cast<int64_t>(diskIODataObj_.WrDatas()[CurrentRow()]));
            break;
        }
        case Index::RD_SPEED: {
            sqlite3_result_double(context_, static_cast<double>(diskIODataObj_.RdSpeedDatas()[CurrentRow()]));
            break;
        }
        case Index::WR_SPEED: {
            sqlite3_result_double(context_, static_cast<double>(diskIODataObj_.WrSpeedDatas()[CurrentRow()]));
            break;
        }
        case Index::RD_COUNT: {
            sqlite3_result_int64(context_, static_cast<int64_t>(diskIODataObj_.RdCountDatas()[CurrentRow()]));
            break;
        }
        case Index::WR_COUNT: {
            sqlite3_result_int64(context_, static_cast<int64_t>(diskIODataObj_.WrCountDatas()[CurrentRow()]));
            break;
        }
        case Index::RD_COUNT_SPEED: {
            sqlite3_result_double(context_, static_cast<double>(diskIODataObj_.RdCountPerSecDatas()[CurrentRow()]));
            break;
        }
        case Index::WR_COUNT_SPEED: {
            sqlite3_result_double(context_, static_cast<double>(diskIODataObj_.WrCountPerSecDatas()[CurrentRow()]));
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
