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

#include "cpu_measure_filter_table.h"

#include <cmath>

#include "log.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, TYPE, NAME, CPU };
CpuMeasureFilterTable::CpuMeasureFilterTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("cpu", "INTEGER"));
    tablePriKey_.push_back("id");
}

CpuMeasureFilterTable::~CpuMeasureFilterTable() {}

void CpuMeasureFilterTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->GetConstCpuMeasureData().Size();
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
            case Index::ID:
                break;
            default: // other columns can be sorted by SQLite
                ei.isOrdered = false;
                break;
        }
    }
}

void CpuMeasureFilterTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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
            case Index::ID: {
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

bool CpuMeasureFilterTable::CanFilterSorted(const char op, size_t& rowCount) const
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

std::unique_ptr<TableBase::Cursor> CpuMeasureFilterTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

CpuMeasureFilterTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstCpuMeasureData().Size())),
      cpuMeasureObj_(dataCache->GetConstCpuMeasureData())
{
}

CpuMeasureFilterTable::Cursor::~Cursor() {}

int32_t CpuMeasureFilterTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
{
    // reset indexMap_
    indexMap_ = std::make_unique<IndexMap>(0, rowCount_);

    if (rowCount_ <= 0) {
        return SQLITE_OK;
    }

    auto& cs = fc.GetConstraints();
    for (size_t i = 0; i < cs.size(); i++) {
        const auto& c = cs[i];
        switch (static_cast<Index>(c.col)) {
            case Index::ID:
                FilterSorted(c.col, c.op, argv[i]);
                break;
            case Index::CPU:
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int(argv[i])), cpuMeasureObj_.CpuData());
                break;
            default:
                break;
        }
    }

    auto orderbys = fc.GetOrderBys();
    for (auto i = orderbys.size(); i > 0;) {
        i--;
        switch (static_cast<Index>(orderbys[i].iColumn)) {
            case Index::ID:
                indexMap_->SortBy(orderbys[i].desc);
                break;
            default:
                break;
        }
    }

    return SQLITE_OK;
}

int32_t CpuMeasureFilterTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(cpuMeasureObj_.IdsData()[CurrentRow()]));
            break;
        case Index::TYPE:
            sqlite3_result_text(context_, "cpu_measure_filter", STR_DEFAULT_LEN, nullptr);
            break;
        case Index::NAME: {
            const std::string& str =
                dataCache_->GetDataFromDict(static_cast<size_t>(cpuMeasureObj_.NameData()[CurrentRow()]));
            sqlite3_result_text(context_, str.c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        }
        case Index::CPU:
            sqlite3_result_int64(context_, static_cast<int32_t>(cpuMeasureObj_.CpuData()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}

void CpuMeasureFilterTable::Cursor::FilterSorted(int32_t col, unsigned char op, sqlite3_value* argv)
{
    auto type = sqlite3_value_type(argv);
    if (type != SQLITE_INTEGER) {
        // other type consider it NULL, filter out nothing
        indexMap_->Intersect(0, 0);
        return;
    }

    switch (static_cast<Index>(col)) {
        case Index::ID: {
            auto v = static_cast<uint64_t>(sqlite3_value_int64(argv));
            auto getValue = [](const uint32_t& row) { return row; };
            switch (op) {
                case SQLITE_INDEX_CONSTRAINT_EQ:
                    indexMap_->IntersectabcEqual(cpuMeasureObj_.IdsData(), v, getValue);
                    break;
                case SQLITE_INDEX_CONSTRAINT_GT:
                    v++;
                case SQLITE_INDEX_CONSTRAINT_GE: {
                    indexMap_->IntersectGreaterEqual(cpuMeasureObj_.IdsData(), v, getValue);
                    break;
                }
                case SQLITE_INDEX_CONSTRAINT_LE:
                    v++;
                case SQLITE_INDEX_CONSTRAINT_LT: {
                    indexMap_->IntersectLessEqual(cpuMeasureObj_.IdsData(), v, getValue);
                    break;
                }
                default:
                    break;
            } // end of switch (op)
        }     // end of case TS
        default:
            // can't filter, all rows
            break;
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
