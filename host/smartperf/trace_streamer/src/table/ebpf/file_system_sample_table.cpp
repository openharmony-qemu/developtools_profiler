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

#include "file_system_sample_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index {
    ID = 0,
    CALLCHAIN_ID,
    TYPE,
    IPID,
    ITID,
    START_TS,
    END_TS,
    DUR,
    RETURN_VALUE,
    ERROR_VALUE,
    FD,
    FILE_ID,
    SIZE,
    FIRST_ARGUMENT,
    SECOND_ARGUMENT,
    THIRD_ARGUMENT,
    FOURTH_ARGUMENT,
};
FileSystemSampleTable::FileSystemSampleTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("callchain_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("itid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("return_value", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("error_code", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("fd", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("file_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("size", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("first_argument", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("second_argument", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("third_argument", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("fourth_argument", "TEXT"));
    tablePriKey_.push_back("id");
}

FileSystemSampleTable::~FileSystemSampleTable() {}

void FileSystemSampleTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
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

void FileSystemSampleTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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

std::unique_ptr<TableBase::Cursor> FileSystemSampleTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

FileSystemSampleTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstFileSystemSample().Size())),
      fileSystemSampleTableObj_(dataCache->GetConstFileSystemSample())
{
}

FileSystemSampleTable::Cursor::~Cursor() {}

int32_t FileSystemSampleTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
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
            case TYPE:
                indexMap_->MixRange(c.op, static_cast<uint16_t>(sqlite3_value_int(argv[i])),
                                    fileSystemSampleTableObj_.Types());
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

int32_t FileSystemSampleTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<int32_t>(fileSystemSampleTableObj_.IdsData()[CurrentRow()]));
            break;
        case CALLCHAIN_ID:
            if (fileSystemSampleTableObj_.CallChainIds()[CurrentRow()] != INVALID_UINT32) {
                sqlite3_result_int64(context_,
                                     static_cast<int64_t>(fileSystemSampleTableObj_.CallChainIds()[CurrentRow()]));
            } else {
                sqlite3_result_int64(context_, static_cast<int64_t>(INVALID_CALL_CHAIN_ID));
            }
            break;
        case TYPE:
            sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.Types()[CurrentRow()]));
            break;
        case IPID:
            sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.Ipids()[CurrentRow()]));
            break;
        case ITID:
            sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.Itids()[CurrentRow()]));
            break;
        case START_TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.StartTs()[CurrentRow()]));
            break;
        case END_TS:
            sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.EndTs()[CurrentRow()]));
            break;
        case DUR:
            sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.Durs()[CurrentRow()]));
            break;
        case RETURN_VALUE: {
            if (fileSystemSampleTableObj_.ReturnValues()[CurrentRow()] != INVALID_UINT64) {
                auto returnValueIndex = fileSystemSampleTableObj_.ReturnValues()[CurrentRow()];
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(returnValueIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case ERROR_VALUE: {
            if (fileSystemSampleTableObj_.ErrorCodes()[CurrentRow()] != INVALID_UINT64) {
                auto errorValueIndex = fileSystemSampleTableObj_.ErrorCodes()[CurrentRow()];
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(errorValueIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case FD: {
            if (fileSystemSampleTableObj_.Fds()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.Fds()[CurrentRow()]));
            }
            break;
        }
        case FILE_ID: {
            if (fileSystemSampleTableObj_.FileIds()[CurrentRow()] != INVALID_UINT64) {
                sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.FileIds()[CurrentRow()]));
            }
            break;
        }
        case SIZE: {
            if (fileSystemSampleTableObj_.Sizes()[CurrentRow()] != MAX_SIZE_T) {
                sqlite3_result_int64(context_, static_cast<int64_t>(fileSystemSampleTableObj_.Sizes()[CurrentRow()]));
            }
            break;
        }
        case FIRST_ARGUMENT: {
            if (fileSystemSampleTableObj_.FirstArguments()[CurrentRow()] != INVALID_UINT64) {
                auto firstArgIndex = fileSystemSampleTableObj_.FirstArguments()[CurrentRow()];
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(firstArgIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case SECOND_ARGUMENT: {
            if (fileSystemSampleTableObj_.SecondArguments()[CurrentRow()] != INVALID_UINT64) {
                auto secondArgIndex = fileSystemSampleTableObj_.SecondArguments()[CurrentRow()];
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(secondArgIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case THIRD_ARGUMENT: {
            if (fileSystemSampleTableObj_.ThirdArguments()[CurrentRow()] != INVALID_UINT64) {
                auto thirdArgIndex = fileSystemSampleTableObj_.ThirdArguments()[CurrentRow()];
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(thirdArgIndex).c_str(), STR_DEFAULT_LEN,
                                    nullptr);
            }
            break;
        }
        case FOURTH_ARGUMENT: {
            if (fileSystemSampleTableObj_.FourthArguments()[CurrentRow()] != INVALID_UINT64) {
                auto fourthArgIndex = fileSystemSampleTableObj_.FourthArguments()[CurrentRow()];
                sqlite3_result_text(context_, dataCache_->GetDataFromDict(fourthArgIndex).c_str(), STR_DEFAULT_LEN,
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
