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

#include "callstack_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index {
    ID = 0,
    TS,
    DUR,
    CALL_ID,
    CAT,
    IDENTIFY,
    NAME,
    DEPTH,
    COOKIE_ID,
    PARENT_ID,
    ARGSET,
    CHAIN_ID,
    SPAN_ID,
    PARENT_SPAN_ID,
    FLAG,
    ARGS
};
CallStackTable::CallStackTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("callid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("cat", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("identify", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("depth", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("cookie", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("parent_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("argsetid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("chainId", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("spanId", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("parentSpanId", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("flag", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("args", "TEXT"));
    tablePriKey_.push_back("callid");
    tablePriKey_.push_back("ts");
    tablePriKey_.push_back("depth");
}

CallStackTable::~CallStackTable() {}

void CallStackTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->GetConstInternalSlicesData().Size();
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

void CallStackTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

std::unique_ptr<TableBase::Cursor> CallStackTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

CallStackTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstInternalSlicesData().Size())),
      slicesObj_(dataCache->GetConstInternalSlicesData())
{
}

CallStackTable::Cursor::~Cursor() {}

int32_t CallStackTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
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
            case TS:
                FilterTS(c.op, argv[i], slicesObj_.TimeStampData());
                break;
            case CALL_ID:
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int(argv[i])), slicesObj_.CallIds());
                break;
            case COOKIE_ID:
                indexMap_->MixRange(c.op, static_cast<uint64_t>(sqlite3_value_int64(argv[i])), slicesObj_.Cookies());
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

int32_t CallStackTable::Cursor::Column(int32_t col) const
{
    switch (col) {
        case ID:
            sqlite3_result_int64(context_, CurrentRow());
            break;
        case TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(slicesObj_.TimeStampData()[CurrentRow()]));
            break;
        case DUR:
            sqlite3_result_int64(context_, static_cast<int64_t>(slicesObj_.DursData()[CurrentRow()]));
            break;
        case CALL_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(slicesObj_.CallIds()[CurrentRow()]));
            break;
        case CAT: {
            if (slicesObj_.CatsData()[CurrentRow()] != INVALID_UINT64) {
                auto catsDataIndex = static_cast<size_t>(slicesObj_.CatsData()[CurrentRow()]);
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(catsDataIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case IDENTIFY:
            sqlite3_result_int(context_, slicesObj_.IdentifysData()[CurrentRow()]);
            break;
        case NAME: {
            if (slicesObj_.NamesData()[CurrentRow()] != INVALID_UINT64) {
                auto nameDataIndex = static_cast<size_t>(slicesObj_.NamesData()[CurrentRow()]);
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(nameDataIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case DEPTH:
            sqlite3_result_int64(context_, static_cast<int64_t>(slicesObj_.Depths()[CurrentRow()]));
            break;
        case COOKIE_ID:
            if (slicesObj_.Cookies()[CurrentRow()] != INVALID_UINT64) {
                sqlite3_result_int64(context_, static_cast<int64_t>(slicesObj_.Cookies()[CurrentRow()]));
            }
            break;
        case PARENT_ID: {
            if (slicesObj_.ParentIdData()[CurrentRow()].has_value()) {
                sqlite3_result_int64(context_, static_cast<int64_t>(slicesObj_.ParentIdData()[CurrentRow()].value()));
            }
            break;
        }
        case ARGSET:
            if (slicesObj_.ArgSetIdsData()[CurrentRow()] != INVALID_UINT32) {
                sqlite3_result_int64(context_, static_cast<int64_t>(slicesObj_.ArgSetIdsData()[CurrentRow()]));
            }
            break;
        case CHAIN_ID:
            if (!slicesObj_.ChainIds()[CurrentRow()].empty()) {
                sqlite3_result_text(context_, slicesObj_.ChainIds()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            }
            break;
        case SPAN_ID:
            if (!slicesObj_.SpanIds()[CurrentRow()].empty()) {
                sqlite3_result_text(context_, slicesObj_.SpanIds()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            }
            break;
        case PARENT_SPAN_ID:
            if (!slicesObj_.ParentSpanIds()[CurrentRow()].empty()) {
                sqlite3_result_text(context_, slicesObj_.ParentSpanIds()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        case FLAG:
            if (!slicesObj_.Flags()[CurrentRow()].empty()) {
                sqlite3_result_text(context_, slicesObj_.Flags()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            }
            break;
        case ARGS:
            if (!slicesObj_.ArgsData()[CurrentRow()].empty()) {
                sqlite3_result_text(context_, slicesObj_.ArgsData()[CurrentRow()].c_str(), STR_DEFAULT_LEN, nullptr);
            }
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
