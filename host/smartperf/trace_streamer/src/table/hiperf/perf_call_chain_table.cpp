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

#include "perf_call_chain_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, CALLCHAIN_ID, DEPTH, VADDR_IN_FILE, FILE_ID, SYMBOL_ID, NAME };
PerfCallChainTable::PerfCallChainTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("callchain_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("depth", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("vaddr_in_file", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("symbol_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tablePriKey_.push_back("id");
}

PerfCallChainTable::~PerfCallChainTable() {}

void PerfCallChainTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->GetConstPerfCallChainData().Size();
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

void PerfCallChainTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

std::unique_ptr<TableBase::Cursor> PerfCallChainTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

PerfCallChainTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstPerfCallChainData().Size())),
      perfCallChainObj_(dataCache->GetConstPerfCallChainData())
{
}

PerfCallChainTable::Cursor::~Cursor() {}

int32_t PerfCallChainTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
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
            case CALLCHAIN_ID:
                indexMap_->MixRange(c.op, static_cast<uint64_t>(sqlite3_value_int64(argv[i])),
                                    perfCallChainObj_.SampleIds());
                break;
            case FILE_ID:
                indexMap_->MixRange(c.op, static_cast<uint64_t>(sqlite3_value_int64(argv[i])),
                                    perfCallChainObj_.FileIds());
                break;
            case SYMBOL_ID:
                indexMap_->MixRange(c.op, static_cast<uint64_t>(sqlite3_value_int64(argv[i])),
                                    perfCallChainObj_.SymbolIds());
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

int32_t PerfCallChainTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<uint64_t>(perfCallChainObj_.IdsData()[CurrentRow()]));
            break;
        case CALLCHAIN_ID:
            sqlite3_result_int64(context_, static_cast<uint64_t>(perfCallChainObj_.SampleIds()[CurrentRow()]));
            break;
        case DEPTH:
            sqlite3_result_int64(context_, static_cast<uint64_t>(perfCallChainObj_.CallChainIds()[CurrentRow()]));
            break;
        case VADDR_IN_FILE:
            sqlite3_result_int64(context_, static_cast<uint64_t>(perfCallChainObj_.VaddrInFiles()[CurrentRow()]));
            break;
        case FILE_ID:
            sqlite3_result_int64(context_, static_cast<uint64_t>(perfCallChainObj_.FileIds()[CurrentRow()]));
            break;
        case SYMBOL_ID:
            sqlite3_result_int64(context_, static_cast<uint64_t>(perfCallChainObj_.SymbolIds()[CurrentRow()]));
            break;
        case NAME:
            sqlite3_result_text(context_, perfCallChainObj_.Names()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
