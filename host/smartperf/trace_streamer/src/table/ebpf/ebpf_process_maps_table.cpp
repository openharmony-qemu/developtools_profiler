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
#include "ebpf_process_maps_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index {
    ID = 0,
    START_ADDR,
    END_ADDR,
    OFFSETS,
    PID,
    FILE_NAME_LEN,
    FILE_PATH_ID,
};
EbpfProcessMapsTable::EbpfProcessMapsTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_addr", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_addr", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("offset", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("pid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("file_path_len", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("file_path_id", "INTEGER"));
    tablePriKey_.push_back("id");
}

EbpfProcessMapsTable::~EbpfProcessMapsTable() {}

void EbpfProcessMapsTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->GetConstHidumpData().Size();
    if (rowCount == 0 || rowCount == 1) {
        ei.estimatedRows = rowCount;
        ei.estimatedCost += indexCost * rowCount;
        return;
    }

    double filterCost = 0.0;
    auto constraints = fc.GetConstraints();
    if (constraints.empty()) { // scan all rows
        filterCost = rowCount;
    } else {
        FilterByConstraint(fc, filterCost, rowCount);
    }
    ei.estimatedCost += filterCost;
    ei.estimatedRows = rowCount;
    ei.estimatedCost += rowCount * indexCost;

    ei.isOrdered = true;
    auto orderbys = fc.GetOrderBys();
    for (auto i = 0; i < orderbys.size(); i++) {
        switch (orderbys[i].iColumn) {
            case ID:
                break;
            default: // other columns can be sorted by SQLite
                ei.isOrdered = false;
                break;
        }
    }
}

void EbpfProcessMapsTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
{
    auto fcConstraints = fc.GetConstraints();
    for (int32_t i = 0; i < static_cast<int32_t>(fcConstraints.size()); i++) {
        if (rowCount <= 1) {
            // only one row or nothing, needn't filter by constraint
            filterCost += rowCount;
            break;
        }
        const auto& c = fcConstraints[i];
        switch (c.col) {
            case ID: {
                if (CanFilterId(c.op, rowCount)) {
                    fc.UpdateConstraint(i, true);
                    filterCost += 1; // id can position by 1 step
                } else {
                    filterCost += rowCount; // scan all rows
                }
                break;
            }
            default:                    // other column
                filterCost += rowCount; // scan all rows
                break;
        }
    }
}

std::unique_ptr<TableBase::Cursor> EbpfProcessMapsTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

EbpfProcessMapsTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstEbpfProcessMaps().Size())),
      ebpfProcessMapsObj_(dataCache->GetConstEbpfProcessMaps())
{
}

EbpfProcessMapsTable::Cursor::~Cursor() {}

int32_t EbpfProcessMapsTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(ebpfProcessMapsObj_.IdsData()[CurrentRow()]));
            break;
        case START_ADDR:
            sqlite3_result_int64(context_, static_cast<int64_t>(ebpfProcessMapsObj_.Starts()[CurrentRow()]));
            break;
        case END_ADDR:
            sqlite3_result_int64(context_, static_cast<int64_t>(ebpfProcessMapsObj_.Ends()[CurrentRow()]));
            break;
        case OFFSETS:
            sqlite3_result_int64(context_, static_cast<int64_t>(ebpfProcessMapsObj_.Offsets()[CurrentRow()]));
            break;
        case PID: {
            if (ebpfProcessMapsObj_.Pids()[CurrentRow()] != INVALID_UINT32) {
                sqlite3_result_int64(context_, static_cast<int64_t>(ebpfProcessMapsObj_.Pids()[CurrentRow()]));
            }
            break;
        }
        case FILE_NAME_LEN: {
            if (ebpfProcessMapsObj_.FileNameLens()[CurrentRow()] != INVALID_UINT32) {
                sqlite3_result_int64(context_, static_cast<int64_t>(ebpfProcessMapsObj_.FileNameLens()[CurrentRow()]));
            }
            break;
        }
        case FILE_PATH_ID: {
            if (ebpfProcessMapsObj_.FileNameIndexs()[CurrentRow()] != INVALID_UINT64) {
                sqlite3_result_int64(context_,
                                     static_cast<int64_t>(ebpfProcessMapsObj_.FileNameIndexs()[CurrentRow()]));
            }
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
