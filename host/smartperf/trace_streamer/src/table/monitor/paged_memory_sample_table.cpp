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

#include "paged_memory_sample_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, CALLCHAIN_ID, TYPE, IPID, START_TS, END_TS, DUR, SIZE, ADDR, ITID };
PagedMemorySampleTable::PagedMemorySampleTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("callchain_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("addr", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("itid", "INTEGER"));
    tablePriKey_.push_back("id");
}

PagedMemorySampleTable::~PagedMemorySampleTable() {}

void PagedMemorySampleTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
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

void PagedMemorySampleTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

std::unique_ptr<TableBase::Cursor> PagedMemorySampleTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

PagedMemorySampleTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstPagedMemorySampleData().Size())),
      PagedMemorySampleDataObj_(dataCache->GetConstPagedMemorySampleData())
{
}

PagedMemorySampleTable::Cursor::~Cursor() {}

int32_t PagedMemorySampleTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
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

int32_t PagedMemorySampleTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(PagedMemorySampleDataObj_.IdsData()[CurrentRow()]));
            break;
        case CALLCHAIN_ID:
            if (PagedMemorySampleDataObj_.CallChainIds()[CurrentRow()] != INVALID_UINT32) {
                sqlite3_result_int64(context_,
                                     static_cast<int64_t>(PagedMemorySampleDataObj_.CallChainIds()[CurrentRow()]));
            } else {
                sqlite3_result_int64(context_, static_cast<int64_t>(INVALID_CALL_CHAIN_ID));
            }
            break;
        case TYPE:
            sqlite3_result_int64(context_, static_cast<int64_t>(PagedMemorySampleDataObj_.Types()[CurrentRow()]));
            break;
        case IPID:
            sqlite3_result_int64(context_, static_cast<int64_t>(PagedMemorySampleDataObj_.Ipids()[CurrentRow()]));
            break;
        case ITID:
            sqlite3_result_int64(context_, static_cast<int64_t>(PagedMemorySampleDataObj_.Itids()[CurrentRow()]));
            break;
        case START_TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(PagedMemorySampleDataObj_.StartTs()[CurrentRow()]));
            break;
        case END_TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(PagedMemorySampleDataObj_.EndTs()[CurrentRow()]));
            break;
        case DUR:
            sqlite3_result_int64(context_, static_cast<int64_t>(PagedMemorySampleDataObj_.Durs()[CurrentRow()]));
            break;
        case SIZE: {
            if (PagedMemorySampleDataObj_.Sizes()[CurrentRow()] != MAX_SIZE_T) {
                sqlite3_result_int64(context_, static_cast<int64_t>(PagedMemorySampleDataObj_.Sizes()[CurrentRow()]));
            }
            break;
        }
        case ADDR: {
            if (PagedMemorySampleDataObj_.Addr()[CurrentRow()] != INVALID_UINT64) {
                auto firstArgIndex = PagedMemorySampleDataObj_.Addr()[CurrentRow()];
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(firstArgIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
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
