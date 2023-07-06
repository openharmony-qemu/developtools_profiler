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

#include "raw_table.h"
namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, TYPE, TS, NAME, CPU, INTERNAL_TID };
enum RawType { RAW_CPU_IDLE = 1, RAW_SCHED_WAKEUP = 2, RAW_SCHED_WAKING = 3 };
uint32_t GetNameIndex(const std::string& name)
{
    if (name == "cpu_idle") {
        return RAW_CPU_IDLE;
    } else if (name == "sched_wakeup") {
        return RAW_SCHED_WAKEUP;
    } else if (name == "sched_waking") {
        return RAW_SCHED_WAKING;
    } else {
        return INVALID_UINT32;
    }
}
RawTable::RawTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("cpu", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("itid", "INTEGER"));
    tablePriKey_.push_back("id");
}

RawTable::~RawTable() {}

void RawTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->GetConstRawTableData().Size();
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

void RawTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

std::unique_ptr<TableBase::Cursor> RawTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

RawTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstRawTableData().Size())),
      rawObj_(dataCache->GetConstRawTableData())
{
}

RawTable::Cursor::~Cursor() {}
int32_t RawTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
{
    // reset indexMap_
    indexMap_ = std::make_unique<IndexMap>(0, rowCount_);

    if (rowCount_ <= 0) {
        return SQLITE_OK;
    }

    auto& cs = fc.GetConstraints();
    for (size_t i = 0; i < cs.size(); i++) {
        const auto& c = cs[i];
        switch (c.col) {
            case ID:
                FilterId(c.op, argv[i]);
                break;
            case NAME:
                indexMap_->MixRange(
                    c.op, GetNameIndex(std::string(reinterpret_cast<const char*>(sqlite3_value_text(argv[i])))),
                    rawObj_.NameData());
                break;
            case TS:
                FilterTS(c.op, argv[i], rawObj_.TimeStampData());
                break;
            case INTERNAL_TID:
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int(argv[i])),
                                    rawObj_.InternalTidsData());
                break;
            default:
                break;
        }
    }

    auto orderbys = fc.GetOrderBys();
    for (auto i = orderbys.size(); i > 0;) {
        i--;
        switch (orderbys[i].iColumn) {
            case ID:
                indexMap_->SortBy(orderbys[i].desc);
                break;
            default:
                break;
        }
    }

    return SQLITE_OK;
}

int32_t RawTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(CurrentRow()));
            break;
        case TYPE:
            sqlite3_result_text(context_, "raw", STR_DEFAULT_LEN, nullptr);
            break;
        case TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(rawObj_.TimeStampData()[CurrentRow()]));
            break;
        case NAME: {
            if (rawObj_.NameData()[CurrentRow()] == RAW_CPU_IDLE) {
                sqlite3_result_text(context_, "cpu_idle", STR_DEFAULT_LEN, nullptr);
            } else if (rawObj_.NameData()[CurrentRow()] == RAW_SCHED_WAKEUP) {
                sqlite3_result_text(context_, "sched_wakeup", STR_DEFAULT_LEN, nullptr);
            } else if (rawObj_.NameData()[CurrentRow()] == RAW_SCHED_WAKING) {
                sqlite3_result_text(context_, "sched_waking", STR_DEFAULT_LEN, nullptr);
            }
            break;
        }
        case CPU:
            sqlite3_result_int64(context_, static_cast<int32_t>(rawObj_.CpuData()[CurrentRow()]));
            break;
        case INTERNAL_TID:
            sqlite3_result_int64(context_, static_cast<int32_t>(rawObj_.InternalTidData()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
