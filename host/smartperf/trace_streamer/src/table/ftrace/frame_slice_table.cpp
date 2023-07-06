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

#include "frame_slice_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, TS, VSYNC, IPID, ITID, CALLSTACK_ID, DUR, SRC, DST, TYPE, TYPE_DESC, FLAG, DEPTH, FRAME_NO };
FrameSliceTable::FrameSliceTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("vsync", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("itid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("callstack_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("src", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("dst", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type_desc", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("flag", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("depth", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("frame_no", "INTEGER"));
    tablePriKey_.push_back("id");
}

FrameSliceTable::~FrameSliceTable() {}

void FrameSliceTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
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

void FrameSliceTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

std::unique_ptr<TableBase::Cursor> FrameSliceTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

FrameSliceTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstFameSliceData().Size())),
      frameSliceObj_(dataCache->GetConstFameSliceData())
{
}

FrameSliceTable::Cursor::~Cursor() {}

int32_t FrameSliceTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
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
            case ITID:
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int(argv[i])),
                                    frameSliceObj_.InternalTidsData());
                break;
            case IPID:
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int(argv[i])), frameSliceObj_.Ipids());
                break;
            case VSYNC:
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int(argv[i])), frameSliceObj_.VsyncIds());
                break;
            case CALLSTACK_ID:
                indexMap_->MixRange(c.op, static_cast<uint64_t>(sqlite3_value_int(argv[i])),
                                    frameSliceObj_.CallStackIds());
                break;
            case DUR:
                indexMap_->MixRange(c.op, static_cast<uint64_t>(sqlite3_value_int(argv[i])), frameSliceObj_.Durs());
                break;
            case TYPE:
                indexMap_->MixRange(c.op, static_cast<uint8_t>(sqlite3_value_int(argv[i])), frameSliceObj_.Types());
                break;
            case FLAG:
                indexMap_->MixRange(c.op, static_cast<uint8_t>(sqlite3_value_int(argv[i])), frameSliceObj_.Flags());
                break;
            case DEPTH:
                indexMap_->MixRange(c.op, static_cast<uint8_t>(sqlite3_value_int(argv[i])), frameSliceObj_.Depths());
                break;
            case FRAME_NO:
                indexMap_->MixRange(c.op, static_cast<uint32_t>(sqlite3_value_int(argv[i])), frameSliceObj_.FrameNos());
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

int32_t FrameSliceTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(CurrentRow()));
            break;
        case TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(frameSliceObj_.TimeStampData()[CurrentRow()]));
            break;
        case VSYNC:
            sqlite3_result_int64(context_, static_cast<int32_t>(frameSliceObj_.VsyncIds()[CurrentRow()]));
            break;
        case IPID:
            sqlite3_result_int64(context_, static_cast<int32_t>(frameSliceObj_.Ipids()[CurrentRow()]));
            break;
        case ITID:
            sqlite3_result_int64(context_, static_cast<int32_t>(frameSliceObj_.InternalTidsData()[CurrentRow()]));
            break;
        case CALLSTACK_ID:
            sqlite3_result_int64(context_, static_cast<int64_t>(frameSliceObj_.CallStackIds()[CurrentRow()]));
            break;
        case DUR:
            if (frameSliceObj_.Durs()[CurrentRow()] != INVALID_UINT64) {
                sqlite3_result_int64(context_, static_cast<int64_t>(frameSliceObj_.Durs()[CurrentRow()]));
            }
            break;
        case SRC:
            sqlite3_result_text(context_, frameSliceObj_.Srcs()[CurrentRow()].c_str(),
                                frameSliceObj_.Srcs()[CurrentRow()].length(), nullptr);
            break;
        case DST:
            if (frameSliceObj_.Dsts()[CurrentRow()] != INVALID_UINT64) {
                sqlite3_result_int64(context_, static_cast<int64_t>(frameSliceObj_.Dsts()[CurrentRow()]));
            }
            break;
        case TYPE:
            sqlite3_result_int64(context_, static_cast<int64_t>(frameSliceObj_.Types()[CurrentRow()]));
            break;
        case TYPE_DESC:
            sqlite3_result_text(context_, frameSliceObj_.Types()[CurrentRow()] == 0 ? "actural" : "expect",
                                STR_DEFAULT_LEN, nullptr);
            break;
        case FLAG:
            if (frameSliceObj_.Flags()[CurrentRow()] != INVALID_UINT8) {
                sqlite3_result_int(context_, static_cast<int32_t>(frameSliceObj_.Flags()[CurrentRow()]));
            }
            break;
        case DEPTH:
            if (frameSliceObj_.Depths()[CurrentRow()] != INVALID_UINT8) {
                sqlite3_result_int(context_, static_cast<int32_t>(frameSliceObj_.Depths()[CurrentRow()]));
            }
            break;
        case FRAME_NO:
            if (frameSliceObj_.FrameNos()[CurrentRow()] != INVALID_UINT32) {
                sqlite3_result_int(context_, static_cast<int32_t>(frameSliceObj_.FrameNos()[CurrentRow()]));
            }
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
