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

#include "task_pool_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index {
    ID = 0,
    ALLOCATION_TASK_ROW,
    EXECUTE_TASK_ROW,
    RETURN_TASK_ROW,
    ALLOCATION_ITID,
    EXECUTE_ITID,
    RETURN_ITID,
    EXECUTE_ID,
    PRIORITY,
    EXECUTE_STATE,
    RETURN_STATE,
    TIMEOUT_ROW
};
TaskPoolTable::TaskPoolTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("allocation_task_row", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("execute_task_row", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("return_task_row", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("allocation_itid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("execute_itid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("return_itid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("execute_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("priority", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("execute_state", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("return_state", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("timeout_row", "INTEGER"));
    tablePriKey_.push_back("id");
}

TaskPoolTable::~TaskPoolTable() {}

std::unique_ptr<TableBase::Cursor> TaskPoolTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

TaskPoolTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstTaskPoolData().Size())),
      taskPoolObj_(dataCache->GetConstTaskPoolData())
{
}

TaskPoolTable::Cursor::~Cursor() {}

int32_t TaskPoolTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case ID:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(CurrentRow()));
            break;
        case ALLOCATION_TASK_ROW:
            if (taskPoolObj_.AllocationTaskRows()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(
                    context_,
                    static_cast<sqlite3_int64>(dataCache_->GetConstTaskPoolData().AllocationTaskRows()[CurrentRow()]));
            }
            break;
        case EXECUTE_TASK_ROW:
            if (taskPoolObj_.ExecuteTaskRows()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().ExecuteTaskRows()[CurrentRow()]));
            }
            break;
        case RETURN_TASK_ROW:
            if (taskPoolObj_.ReturnTaskRows()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().ReturnTaskRows()[CurrentRow()]));
            }
            break;
        case ALLOCATION_ITID:
            if (taskPoolObj_.AllocationItids()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().AllocationItids()[CurrentRow()]));
            }
            break;
        case EXECUTE_ITID:
            if (taskPoolObj_.ExecuteItids()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().ExecuteItids()[CurrentRow()]));
            }
            break;
        case RETURN_ITID:
            if (taskPoolObj_.ReturnItids()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().ReturnItids()[CurrentRow()]));
            }
            break;
        case EXECUTE_ID:
            if (taskPoolObj_.ExecuteIds()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().ExecuteIds()[CurrentRow()]));
            }
            break;
        case PRIORITY:
            if (taskPoolObj_.Prioritys()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(
                    context_, static_cast<sqlite3_int64>(dataCache_->GetConstTaskPoolData().Prioritys()[CurrentRow()]));
            }
            break;
        case EXECUTE_STATE:
            if (taskPoolObj_.ExecuteStates()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().ExecuteStates()[CurrentRow()]));
            }
            break;
        case RETURN_STATE:
            if (taskPoolObj_.ReturnStates()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().ReturnStates()[CurrentRow()]));
            }
            break;
        case TIMEOUT_ROW:
            if (taskPoolObj_.TimeoutRows()[CurrentRow()] != INVALID_INT32) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(
                                                   dataCache_->GetConstTaskPoolData().TimeoutRows()[CurrentRow()]));
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
