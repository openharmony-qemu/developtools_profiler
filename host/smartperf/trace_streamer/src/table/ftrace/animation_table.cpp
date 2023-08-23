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

#include "animation_table.h"

#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, INPUT_TIME, START_POINT, END_POINT };
AnimationTable::AnimationTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("input_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_point", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_point", "INTEGER"));
    tablePriKey_.push_back("id");
}

AnimationTable::~AnimationTable() {}

void AnimationTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->GetConstAnimation().Size();
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

void AnimationTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

bool AnimationTable::CanFilterSorted(const char op, size_t& rowCount) const
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

std::unique_ptr<TableBase::Cursor> AnimationTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

AnimationTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstAnimation().Size())),
      animationObj_(dataCache->GetConstAnimation())
{
}

AnimationTable::Cursor::~Cursor() {}

int32_t AnimationTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
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
                FilterSorted(c.col, c.op, argv[i]);
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

int32_t AnimationTable::Cursor::Column(int32_t col) const
{
    switch (col) {
        case ID:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(animationObj_.IdsData()[CurrentRow()]));
            break;
        case INPUT_TIME: {
            if (animationObj_.InputTimes()[CurrentRow()] != INVALID_TIME) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(animationObj_.InputTimes()[CurrentRow()]));
            }
            break;
        }
        case START_POINT: {
            if (animationObj_.StartPoints()[CurrentRow()] != INVALID_TIME) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(animationObj_.StartPoints()[CurrentRow()]));
            }
            break;
        }
        case END_POINT:
            if (animationObj_.EndPoints()[CurrentRow()] != INVALID_TIME) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(animationObj_.EndPoints()[CurrentRow()]));
            }
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}

void AnimationTable::Cursor::FilterSorted(int32_t col, unsigned char op, sqlite3_value* argv)
{
    auto type = sqlite3_value_type(argv);
    if (type != SQLITE_INTEGER) {
        // other type consider it NULL, filter out nothing
        indexMap_->Intersect(0, 0);
        return;
    }

    switch (col) {
        case ID: {
            auto v = static_cast<uint64_t>(sqlite3_value_int64(argv));
            auto getValue = [](const uint32_t& row) { return row; };
            switch (op) {
                case SQLITE_INDEX_CONSTRAINT_EQ:
                    indexMap_->IntersectabcEqual(animationObj_.IdsData(), v, getValue);
                    break;
                case SQLITE_INDEX_CONSTRAINT_GT:
                    v++;
                case SQLITE_INDEX_CONSTRAINT_GE: {
                    indexMap_->IntersectGreaterEqual(animationObj_.IdsData(), v, getValue);
                    break;
                }
                case SQLITE_INDEX_CONSTRAINT_LE:
                    v++;
                case SQLITE_INDEX_CONSTRAINT_LT: {
                    indexMap_->IntersectLessEqual(animationObj_.IdsData(), v, getValue);
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
