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

#include "perf_report_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    ID = 0,
    REPORT_TYPE,
    REPORT_VALUE,
};
PerfReportTable::PerfReportTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("report_type", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("report_value", "TEXT"));
    tablePriKey_.push_back("id");
}

PerfReportTable::~PerfReportTable() {}

std::unique_ptr<TableBase::Cursor> PerfReportTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

PerfReportTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstPerfReportData().Size())),
      perfReportObj_(dataCache->GetConstPerfReportData())
{
}

PerfReportTable::Cursor::~Cursor() {}

int32_t PerfReportTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(perfReportObj_.IdsData()[CurrentRow()]));
            break;
        case Index::REPORT_TYPE:
            if (perfReportObj_.Types()[CurrentRow()] != INVALID_UINT64) {
                auto typeIndex = static_cast<size_t>(perfReportObj_.Types()[CurrentRow()]);
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(typeIndex).c_str(), STR_DEFAULT_LEN, nullptr);
            }
            break;
        case Index::REPORT_VALUE:
            if (perfReportObj_.Values()[CurrentRow()] != INVALID_UINT64) {
                auto typeValueIndex = static_cast<size_t>(perfReportObj_.Values()[CurrentRow()]);
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(typeValueIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
