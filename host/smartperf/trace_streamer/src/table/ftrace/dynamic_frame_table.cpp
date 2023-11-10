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

#include "dynamic_frame_table.h"

#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, X, Y, WIDTH, HEIGHT, ALPHA, NAME, END_TIME };
DynamicFrameTable::DynamicFrameTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("x", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("y", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("width", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("height", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("alpha", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_time", "INTEGER"));
    tablePriKey_.push_back("id");
}

DynamicFrameTable::~DynamicFrameTable() {}

std::unique_ptr<TableBase::Cursor> DynamicFrameTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

DynamicFrameTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstDynamicFrame().Size())),
      dynamicFrameObj_(dataCache->GetConstDynamicFrame())
{
}

DynamicFrameTable::Cursor::~Cursor() {}
int32_t DynamicFrameTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::ID:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(dynamicFrameObj_.IdsData()[CurrentRow()]));
            break;
        case Index::X: {
            sqlite3_result_int(context_, static_cast<int32_t>(dynamicFrameObj_.Xs()[CurrentRow()]));
            break;
        }
        case Index::Y: {
            sqlite3_result_int(context_, static_cast<int32_t>(dynamicFrameObj_.Ys()[CurrentRow()]));
            break;
        }
        case Index::WIDTH: {
            sqlite3_result_int(context_, static_cast<int32_t>(dynamicFrameObj_.Widths()[CurrentRow()]));
            break;
        }
        case Index::HEIGHT: {
            sqlite3_result_int(context_, static_cast<int32_t>(dynamicFrameObj_.Heights()[CurrentRow()]));
            break;
        }
        case Index::ALPHA: {
            if (dynamicFrameObj_.Alphas()[CurrentRow()] != INVALID_UINT64) {
                const std::string& str =
                    dataCache_->GetDataFromDict(static_cast<size_t>(dynamicFrameObj_.Alphas()[CurrentRow()]));
                sqlite3_result_text(context_, str.c_str(), STR_DEFAULT_LEN, nullptr);
            }
            break;
        }
        case Index::NAME: {
            if (dynamicFrameObj_.Names()[CurrentRow()] != INVALID_UINT64) {
                const std::string& str =
                    dataCache_->GetDataFromDict(static_cast<size_t>(dynamicFrameObj_.Names()[CurrentRow()]));
                sqlite3_result_text(context_, str.c_str(), STR_DEFAULT_LEN, nullptr);
            }
            break;
        }
        case Index::END_TIME:
            if (dynamicFrameObj_.EndTimes()[CurrentRow()] != INVALID_TIME) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(dynamicFrameObj_.EndTimes()[CurrentRow()]));
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
