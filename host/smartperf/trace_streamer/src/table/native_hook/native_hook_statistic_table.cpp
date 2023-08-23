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

#include "native_hook_statistic_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index {
    ID = 0,
    CALLCHAIN_ID,
    IPID,
    TS,
    MEMORY_TYPE,
    MEMORY_SUB_TYPE,
    APPLY_COUNT,
    RELEASE_COUNT,
    APPLY_SIZE,
    RELEASE_SIZE
};
NativeHookStatisticTable::NativeHookStatisticTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("callchain_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("sub_type_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("apply_count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("release_count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("apply_size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("release_size", "INTEGER"));
    tablePriKey_.push_back("id");
}

NativeHookStatisticTable::~NativeHookStatisticTable() {}

void NativeHookStatisticTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->GetConstNativeHookStatisticData().Size();
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

void NativeHookStatisticTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

std::unique_ptr<TableBase::Cursor> NativeHookStatisticTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

NativeHookStatisticTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstNativeHookStatisticData().Size())),
      nativeHookStatisticInfoObj_(dataCache->GetConstNativeHookStatisticData())
{
}

NativeHookStatisticTable::Cursor::~Cursor() {}

int32_t NativeHookStatisticTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
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
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int64(argv[i])),
                                    nativeHookStatisticInfoObj_.CallChainIds());
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

int32_t NativeHookStatisticTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(CurrentRow()));
            break;
        case CALLCHAIN_ID:
            if (nativeHookStatisticInfoObj_.CallChainIds()[CurrentRow()] != INVALID_UINT32) {
                sqlite3_result_int64(context_,
                                     static_cast<int64_t>(nativeHookStatisticInfoObj_.CallChainIds()[CurrentRow()]));
            } else {
                sqlite3_result_int64(context_, static_cast<int64_t>(INVALID_CALL_CHAIN_ID));
            }
            break;
        case IPID:
            if (nativeHookStatisticInfoObj_.Ipids()[CurrentRow()] != INVALID_UINT32) {
                sqlite3_result_int64(context_, static_cast<int64_t>(nativeHookStatisticInfoObj_.Ipids()[CurrentRow()]));
            }
            break;
        case TS:
            if (nativeHookStatisticInfoObj_.TimeStampData()[CurrentRow()] != INVALID_UINT64) {
                sqlite3_result_int64(context_,
                                     static_cast<int64_t>(nativeHookStatisticInfoObj_.TimeStampData()[CurrentRow()]));
            }
            break;
        case MEMORY_TYPE:
            sqlite3_result_int64(context_,
                                 static_cast<int64_t>(nativeHookStatisticInfoObj_.MemoryTypes()[CurrentRow()]));
            break;
        case MEMORY_SUB_TYPE:
            if (nativeHookStatisticInfoObj_.MemorySubTypes()[CurrentRow()] != INVALID_UINT64) {
                sqlite3_result_int64(context_,
                                     static_cast<int64_t>(nativeHookStatisticInfoObj_.MemorySubTypes()[CurrentRow()]));
            }
            break;
        case APPLY_COUNT:
            sqlite3_result_int64(context_,
                                 static_cast<int64_t>(nativeHookStatisticInfoObj_.ApplyCounts()[CurrentRow()]));
            break;
        case RELEASE_COUNT:
            sqlite3_result_int64(context_,
                                 static_cast<int64_t>(nativeHookStatisticInfoObj_.ReleaseCounts()[CurrentRow()]));
            break;
        case APPLY_SIZE: {
            sqlite3_result_int64(context_,
                                 static_cast<int64_t>(nativeHookStatisticInfoObj_.ApplySizes()[CurrentRow()]));
            break;
        }
        case RELEASE_SIZE: {
            sqlite3_result_int64(context_,
                                 static_cast<int64_t>(nativeHookStatisticInfoObj_.ReleaseSizes()[CurrentRow()]));
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
