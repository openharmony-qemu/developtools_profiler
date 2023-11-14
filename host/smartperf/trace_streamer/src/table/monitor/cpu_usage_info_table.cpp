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

#include "cpu_usage_info_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { TS = 0, DUR, TOTAL_LOAD, USER_LOAD, SYSTEM_LOAD, THREADS };
CpuUsageInfoTable::CpuUsageInfoTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("total_load", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("user_load", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("system_load", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("process_num", "INTEGER"));
    tablePriKey_.push_back("ts");
}

CpuUsageInfoTable::~CpuUsageInfoTable() {}

std::unique_ptr<TableBase::Cursor> CpuUsageInfoTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

CpuUsageInfoTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstCpuUsageInfoData().Size())),
      cpuUsageInfoObj_(dataCache->GetConstCpuUsageInfoData())
{
}

CpuUsageInfoTable::Cursor::~Cursor() {}

int32_t CpuUsageInfoTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::TS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(cpuUsageInfoObj_.TimeStampData()[CurrentRow()]));
            break;
        }
        case Index::DUR: {
            sqlite3_result_int64(context_, static_cast<int64_t>(cpuUsageInfoObj_.Durs()[CurrentRow()]));
            break;
        }
        case Index::TOTAL_LOAD: {
            sqlite3_result_int64(context_, static_cast<int64_t>(cpuUsageInfoObj_.TotalLoad()[CurrentRow()]));
            break;
        }
        case Index::USER_LOAD: {
            sqlite3_result_double(context_, cpuUsageInfoObj_.UserLoad()[CurrentRow()]);
            break;
        }
        case Index::SYSTEM_LOAD: {
            sqlite3_result_double(context_, cpuUsageInfoObj_.SystemLoad()[CurrentRow()]);
            break;
        }
        case Index::THREADS: {
            sqlite3_result_int(context_, static_cast<int32_t>(cpuUsageInfoObj_.Threads()[CurrentRow()]));
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
