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

#include "measure_table.h"
#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { TYPE = 0, TS, DUR, VALUE, FILTER_ID };
MeasureTable::MeasureTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("type", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("value", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("filter_id", "INTEGER"));
    tablePriKey_.push_back("ts");
    tablePriKey_.push_back("filter_id");
}

MeasureTable::~MeasureTable() {}

std::unique_ptr<TableBase::Cursor> MeasureTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

MeasureTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(
          dataCache,
          table,
          static_cast<uint32_t>(table->name_ == "measure" || table->name_ == "_measure"
                                    ? dataCache->GetConstMeasureData().Size()
                                    : (table->name_ == "process_measure" || table->name_ == "_process_measure"
                                           ? dataCache->GetConstProcessMeasureData().Size()
                                           : dataCache->GetConstSysMemMeasureData().Size()))),
      measureObj(table->name_ == "measure" || table->name_ == "_measure"
                     ? dataCache->GetConstMeasureData()
                     : (table->name_ == "process_measure" || table->name_ == "_process_measure"
                            ? dataCache->GetConstProcessMeasureData()
                            : dataCache->GetConstSysMemMeasureData()))
{
}

MeasureTable::Cursor::~Cursor() {}

void MeasureTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->GetConstMeasureData().Size();
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
        switch (static_cast<Index>(orderbys[i].iColumn)) {
            case Index::TS:
                break;
            default: // other columns can be sorted by SQLite
                ei.isOrdered = false;
                break;
        }
    }
}

void MeasureTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
{
    auto fcConstraints = fc.GetConstraints();
    for (int32_t i = 0; i < static_cast<int32_t>(fcConstraints.size()); i++) {
        if (rowCount <= 1) {
            // only one row or nothing, needn't filter by constraint
            filterCost += rowCount;
            break;
        }
        const auto& c = fcConstraints[i];
        switch (static_cast<Index>(c.col)) {
            case Index::TS: {
                auto oldRowCount = rowCount;
                if (CanFilterSorted(c.op, rowCount)) {
                    fc.UpdateConstraint(i, true);
                    filterCost += log2(oldRowCount); // binary search
                } else {
                    filterCost += oldRowCount;
                }
                break;
            }
            default:                    // other column
                filterCost += rowCount; // scan all rows
                break;
        }
    }
}

bool MeasureTable::CanFilterSorted(const char op, size_t& rowCount) const
{
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            rowCount = rowCount / log2(rowCount);
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
        case SQLITE_INDEX_CONSTRAINT_GE:
        case SQLITE_INDEX_CONSTRAINT_LE:
        case SQLITE_INDEX_CONSTRAINT_LT:
            rowCount = (rowCount >> 1);
            break;
        default:
            return false;
    }
    return true;
}

int32_t MeasureTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
{
    // reset
    indexMap_ = std::make_unique<IndexMap>(0, rowCount_);

    if (rowCount_ <= 0) {
        return SQLITE_OK;
    }
    auto& cs = fc.GetConstraints();
    for (size_t i = 0; i < cs.size(); i++) {
        const auto& c = cs[i];
        switch (static_cast<Index>(c.col)) {
            case Index::TS:
                FilterTS(c.op, argv[i], measureObj.TimeStampData());
                break;
            case Index::FILTER_ID:
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int(argv[i])), measureObj.FilterIdData());
                break;
            default:
                break;
        }
    }

    auto orderbys = fc.GetOrderBys();
    for (auto i = orderbys.size(); i > 0;) {
        i--;
        switch (static_cast<Index>(orderbys[i].iColumn)) {
            case Index::TS:
                indexMap_->SortBy(orderbys[i].desc);
                break;
            case Index::FILTER_ID:
                indexMap_->SortBy(orderbys[i].desc);
                break;
            default:
                break;
        }
    }

    return SQLITE_OK;
}

int32_t MeasureTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::TYPE:
            sqlite3_result_text(context_, "measure", STR_DEFAULT_LEN, nullptr);
            break;
        case Index::TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(measureObj.TimeStampData()[CurrentRow()]));
            break;
        case Index::DUR:
            if (measureObj.DursData()[CurrentRow()] != INVALID_UINT64) {
                sqlite3_result_int64(context_, static_cast<int64_t>(measureObj.DursData()[CurrentRow()]));
            }
            break;
        case Index::VALUE:
            sqlite3_result_int64(context_, static_cast<int64_t>(measureObj.ValuesData()[CurrentRow()]));
            break;
        case Index::FILTER_ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(measureObj.FilterIdData()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
