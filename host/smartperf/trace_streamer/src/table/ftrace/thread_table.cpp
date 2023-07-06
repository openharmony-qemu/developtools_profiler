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

#include "thread_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { ID = 0, ITID, TYPE, TID, NAME, START_TS, END_TS, INTERNAL_PID, IS_MAIN_THREAD, SWITCH_COUNT };
ThreadTable::ThreadTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("itid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("tid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ipid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("is_main_thread", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("switch_count", "INTEGER"));
    tablePriKey_.push_back("id");
}

ThreadTable::~ThreadTable() {}

void ThreadTable::EstimateFilterCost(FilterConstraints& fc, EstimatedIndexInfo& ei)
{
    constexpr double filterBaseCost = 1000.0; // set-up and tear-down
    constexpr double indexCost = 2.0;
    ei.estimatedCost = filterBaseCost;

    auto rowCount = dataCache_->ThreadSize();
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
            case ITID:
            case ID:
                break;
            default: // other columns can be sorted by SQLite
                ei.isOrdered = false;
                break;
        }
    }
}

void ThreadTable::FilterByConstraint(FilterConstraints& fc, double& filterCost, size_t rowCount)
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
            case ITID:
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

std::unique_ptr<TableBase::Cursor> ThreadTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

ThreadTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, dataCache->ThreadSize())
{
}

ThreadTable::Cursor::~Cursor() {}
void ThreadTable::Cursor::FilterTid(unsigned char op, uint64_t value)
{
    bool remove = false;
    if (indexMapBack_->HasData()) {
        indexMapBack_->CovertToIndexMap();
        remove = true;
    }
    const auto& threadQueue = dataCache_->GetConstThreadData();
    auto size = threadQueue.size();
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            if (remove) {
                for (auto i = indexMapBack_->rowIndex_.begin(); i != indexMapBack_->rowIndex_.end();) {
                    if (threadQueue[*i].tid_ != value) {
                        i = indexMapBack_->rowIndex_.erase(i);
                    } else {
                        i++;
                    }
                }
            } else {
                for (auto i = 0; i < size; i++) {
                    if (threadQueue[i].tid_ == value) {
                        indexMapBack_->rowIndex_.push_back(i);
                    }
                }
            }
            indexMapBack_->FixSize();
            break;
        case SQLITE_INDEX_CONSTRAINT_ISNOT:
        case SQLITE_INDEX_CONSTRAINT_NE:
            if (remove) {
                for (auto i = indexMapBack_->rowIndex_.begin(); i != indexMapBack_->rowIndex_.end();) {
                    if (threadQueue[*i].tid_ == value) {
                        i = indexMapBack_->rowIndex_.erase(i);
                    } else {
                        i++;
                    }
                }
            } else {
                for (auto i = 0; i < size; i++) {
                    if (threadQueue[i].tid_ != value) {
                        indexMapBack_->rowIndex_.push_back(i);
                    }
                }
            }
            indexMapBack_->FixSize();
            break;
        default:
            break;
    } // end of switch (op)
}
void ThreadTable::Cursor::FilterIpid(unsigned char op, uint64_t value)
{
    bool remove = false;
    if (indexMapBack_->HasData()) {
        indexMapBack_->CovertToIndexMap();
        remove = true;
    }
    const auto& threadQueue = dataCache_->GetConstThreadData();
    auto size = threadQueue.size();
    rowIndexBak_.clear();
    bool changed = false;
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            if (remove) {
                for (auto i = indexMapBack_->rowIndex_.begin(); i != indexMapBack_->rowIndex_.end();) {
                    if (threadQueue[*i].internalPid_ != value) {
                        i++;
                    } else {
                        changed = true;
                        rowIndexBak_.push_back(*i);
                        i++;
                    }
                }
                if (changed) {
                    indexMapBack_->rowIndex_ = rowIndexBak_;
                }
            } else {
                for (auto i = 0; i < size; i++) {
                    if (threadQueue[i].internalPid_ == value) {
                        indexMapBack_->rowIndex_.push_back(i);
                    }
                }
            }
            indexMapBack_->FixSize();
            break;
        case SQLITE_INDEX_CONSTRAINT_ISNULL:
            if (remove) {
                for (auto i = indexMapBack_->rowIndex_.begin(); i != indexMapBack_->rowIndex_.end();) {
                    if (threadQueue[*i].internalPid_ != INVALID_UINT32) {
                        i++;
                    } else {
                        changed = true;
                        rowIndexBak_.push_back(*i);
                        i++;
                    }
                }
                if (changed) {
                    indexMapBack_->rowIndex_ = rowIndexBak_;
                }
            } else {
                for (auto i = 0; i < size; i++) {
                    if (threadQueue[i].internalPid_ == INVALID_UINT32) {
                        indexMapBack_->rowIndex_.push_back(i);
                    }
                }
            }
            indexMapBack_->FixSize();
            break;
        case SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
            if (remove) {
                for (auto i = indexMapBack_->rowIndex_.begin(); i != indexMapBack_->rowIndex_.end();) {
                    if (threadQueue[*i].internalPid_ == INVALID_UINT32) {
                        i++;
                    } else {
                        changed = true;
                        rowIndexBak_.push_back(*i);
                        i++;
                    }
                }
                if (changed) {
                    indexMapBack_->rowIndex_ = rowIndexBak_;
                }
            } else {
                for (auto i = 0; i < size; i++) {
                    if (threadQueue[i].internalPid_ != INVALID_UINT32) {
                        indexMapBack_->rowIndex_.push_back(i);
                    }
                }
            }
            indexMapBack_->FixSize();
            break;
        default:
            break;
    } // end of switch (op)
}
void ThreadTable::Cursor::FilterSwitchCount(unsigned char op, uint64_t value)
{
    bool remove = false;
    if (indexMapBack_->HasData()) {
        indexMapBack_->CovertToIndexMap();
        remove = true;
    }
    const auto& threadQueue = dataCache_->GetConstThreadData();
    auto size = threadQueue.size();
    rowIndexBak_.clear();
    bool changed = false;
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            if (remove) {
                for (auto i = indexMapBack_->rowIndex_.begin(); i != indexMapBack_->rowIndex_.end();) {
                    if (threadQueue[*i].switchCount_ != value) {
                        i++;
                    } else {
                        changed = true;
                        rowIndexBak_.push_back(*i);
                        i++;
                    }
                }
                if (changed) {
                    indexMapBack_->rowIndex_ = rowIndexBak_;
                }
            } else {
                for (auto i = 0; i < size; i++) {
                    if (threadQueue[i].switchCount_ == value) {
                        indexMapBack_->rowIndex_.push_back(i);
                    }
                }
            }
            indexMapBack_->FixSize();
            break;
        case SQLITE_INDEX_CONSTRAINT_ISNULL:
            if (remove) {
                for (auto i = indexMapBack_->rowIndex_.begin(); i != indexMapBack_->rowIndex_.end();) {
                    if (threadQueue[*i].switchCount_ != INVALID_UINT32) {
                        i++;
                    } else {
                        changed = true;
                        rowIndexBak_.push_back(*i);
                        i++;
                    }
                }
                if (changed) {
                    indexMapBack_->rowIndex_ = rowIndexBak_;
                }
            } else {
                for (auto i = 0; i < size; i++) {
                    if (threadQueue[i].switchCount_ == INVALID_UINT32) {
                        indexMapBack_->rowIndex_.push_back(i);
                    }
                }
            }
            indexMapBack_->FixSize();
            break;
        case SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
            if (remove) {
                for (auto i = indexMapBack_->rowIndex_.begin(); i != indexMapBack_->rowIndex_.end();) {
                    if (threadQueue[*i].switchCount_ == INVALID_UINT32) {
                        i++;
                    } else {
                        changed = true;
                        rowIndexBak_.push_back(*i);
                        i++;
                    }
                }
                if (changed) {
                    indexMapBack_->rowIndex_ = rowIndexBak_;
                }
            } else {
                for (auto i = 0; i < size; i++) {
                    if (threadQueue[i].switchCount_ != INVALID_UINT32) {
                        indexMapBack_->rowIndex_.push_back(i);
                    }
                }
            }
            indexMapBack_->FixSize();
            break;
        default:
            break;
    } // end of switch (op)
}
void ThreadTable::Cursor::FilterIndex(int32_t col, unsigned char op, sqlite3_value* argv)
{
    switch (col) {
        case INTERNAL_PID:
            FilterIpid(op, static_cast<uint64_t>(sqlite3_value_int64(argv)));
            break;
        case TID:
            FilterTid(op, static_cast<uint64_t>(sqlite3_value_int64(argv)));
            break;
        case SWITCH_COUNT:
            FilterSwitchCount(op, static_cast<uint64_t>(sqlite3_value_int64(argv)));
            break;
        default:
            // we can't filter all rows
            break;
    }
}
int32_t ThreadTable::Cursor::Filter(const FilterConstraints& fc, sqlite3_value** argv)
{
    // reset indexMapBack_
    if (rowCount_ <= 0) {
        return SQLITE_OK;
    }
    indexMapBack_ = indexMap_.get();
    if (indexMap_->HasData()) {
        indexMapBack_ = std::make_unique<IndexMap>(0, rowCount_).get();
    }
    auto& cs = fc.GetConstraints();
    for (size_t i = 0; i < cs.size(); i++) {
        const auto& c = cs[i];
        switch (c.col) {
            case ID:
            case ITID:
                FilterId(c.op, argv[i]);
                break;
            case TID:
            case INTERNAL_PID:
            case SWITCH_COUNT:
                FilterIndex(c.col, c.op, argv[i]);
                break;
            default:
                break;
        }
    }
    if (indexMap_->HasData()) {
        indexMap_->Merge(indexMapBack_);
    }

    auto orderbys = fc.GetOrderBys();
    for (auto i = orderbys.size(); i > 0;) {
        i--;
        switch (orderbys[i].iColumn) {
            case ID:
            case ITID:
                indexMap_->SortBy(orderbys[i].desc);
                break;
            default:
                break;
        }
    }

    return SQLITE_OK;
}

int32_t ThreadTable::Cursor::Column(int32_t col) const
{
    const auto& thread = dataCache_->GetConstThreadData(CurrentRow());
    switch (col) {
        case ID:
        case ITID: {
            sqlite3_result_int64(context_, CurrentRow());
            break;
        }
        case TYPE: {
            sqlite3_result_text(context_, "thread", strlen("thread"), nullptr);
            break;
        }
        case TID: {
            sqlite3_result_int64(context_, static_cast<int32_t>(thread.tid_));
            break;
        }
        case NAME: {
            const auto& name = dataCache_->GetDataFromDict(thread.nameIndex_);
            if (name.size()) {
                sqlite3_result_text(context_, name.c_str(), static_cast<int32_t>(name.length()), nullptr);
            }
            break;
        }
        case START_TS: {
            if (thread.startT_) {
                sqlite3_result_int64(context_, static_cast<int64_t>(thread.startT_));
            }
            break;
        }
        case END_TS: {
            if (thread.endT_) {
                sqlite3_result_int64(context_, static_cast<int64_t>(thread.endT_));
            }
            break;
        }
        case INTERNAL_PID: {
            if (thread.internalPid_ != INVALID_UINT32) {
                sqlite3_result_int(context_, static_cast<int32_t>(thread.internalPid_));
            }
            break;
        }
        case IS_MAIN_THREAD: {
            // When it is not clear which process the thread belongs to, is_main_thread should be set to null
            if (thread.internalPid_ == INVALID_UINT32) {
                break;
            }
            const auto& process = dataCache_->GetConstProcessData(thread.internalPid_);
            sqlite3_result_int(context_, thread.tid_ == process.pid_);
            break;
        }
        case SWITCH_COUNT: {
            // When it is not clear which process the thread belongs to, is_main_thread should be set to null
            sqlite3_result_int(context_, thread.switchCount_);
            break;
        }

        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}

int32_t ThreadTable::Update(int32_t argc, sqlite3_value** argv, sqlite3_int64* pRowid)
{
    if (argc <= 1) {
        return SQLITE_READONLY;
    }
    if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
        return SQLITE_READONLY;
    }
    auto id = sqlite3_value_int64(argv[0]);
    auto thread = wdataCache_->GetThreadData(static_cast<InternalPid>(id));
    constexpr int32_t colOffset = 2;
    for (auto i = colOffset; i < argc; i++) {
        auto col = i - colOffset;
        if (col != INTERNAL_PID) {
            continue;
        }
        auto ipid = static_cast<uint32_t>(sqlite3_value_int(argv[i]));
        if (ipid) {
            thread->internalPid_ = ipid;
        }
        break;
    }
    return SQLITE_OK;
}
void ThreadTable::Cursor::FilterId(unsigned char op, sqlite3_value* argv)
{
    auto type = sqlite3_value_type(argv);
    if (type != SQLITE_INTEGER) {
        // other type consider it NULL
        indexMapBack_->Intersect(0, 0);
        return;
    }

    auto v = static_cast<TableRowId>(sqlite3_value_int64(argv));
    switch (op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            indexMapBack_->Intersect(v, v + 1);
            break;
        case SQLITE_INDEX_CONSTRAINT_GE:
            indexMapBack_->Intersect(v, rowCount_);
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
            v++;
            indexMapBack_->Intersect(v, rowCount_);
            break;
        case SQLITE_INDEX_CONSTRAINT_LE:
            v++;
            indexMapBack_->Intersect(0, v);
            break;
        case SQLITE_INDEX_CONSTRAINT_LT:
            indexMapBack_->Intersect(0, v);
            break;
        default:
            // can't filter, all rows
            break;
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
