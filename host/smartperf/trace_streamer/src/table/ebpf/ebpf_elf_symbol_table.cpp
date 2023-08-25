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
#include "ebpf_elf_symbol_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index {
    ID = 0,
    ELF_ID,
    ST_NAME,
    ST_VALUE,
    ST_SIZE,
};
EbpfElfSymbolTable::EbpfElfSymbolTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("elf_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("st_name", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("st_value", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("st_size", "INTEGER"));
    tablePriKey_.push_back("id");
}

EbpfElfSymbolTable::~EbpfElfSymbolTable() {}

void EbpfElfSymbolTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
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

void EbpfElfSymbolTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

std::unique_ptr<TableBase::Cursor> EbpfElfSymbolTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

EbpfElfSymbolTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstEbpfElfSymbol().Size())),
      ebpfElfSymbolObj_(dataCache->GetConstEbpfElfSymbol())
{
}

EbpfElfSymbolTable::Cursor::~Cursor() {}

int32_t EbpfElfSymbolTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(ebpfElfSymbolObj_.IdsData()[CurrentRow()]));
            break;
        case ELF_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(ebpfElfSymbolObj_.ElfIds()[CurrentRow()]));
            break;
        case ST_NAME:
            sqlite3_result_int64(context_, static_cast<int64_t>(ebpfElfSymbolObj_.StNames()[CurrentRow()]));
            break;
        case ST_VALUE:
            sqlite3_result_int64(context_, static_cast<int64_t>(ebpfElfSymbolObj_.StValues()[CurrentRow()]));
            break;
        case ST_SIZE:
            sqlite3_result_int64(context_, static_cast<int64_t>(ebpfElfSymbolObj_.StSizes()[CurrentRow()]));
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
