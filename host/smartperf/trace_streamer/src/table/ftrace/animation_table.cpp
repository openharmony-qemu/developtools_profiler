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

#include "animation_table.h"

#include <cmath>

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { ID = 0, INPUT_TIME, START_POINT, END_POINT, FRAME_INFO, NAME };
AnimationTable::AnimationTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("input_time", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("start_point", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("end_point", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("frame_info", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("name", "TEXT"));
    tablePriKey_.push_back("id");
}

AnimationTable::~AnimationTable() {}

std::unique_ptr<TableBase::Cursor> AnimationTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

AnimationTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstAnimation().Size())),
      animationObj_(dataCache->GetConstAnimation())
{
}

AnimationTable::Cursor::~Cursor() {}

int32_t AnimationTable::Cursor::Column(int32_t col) const
{
    switch (static_cast<Index>(col)) {
        case Index::ID:
            sqlite3_result_int64(context_, static_cast<sqlite3_int64>(animationObj_.IdsData()[CurrentRow()]));
            break;
        case Index::INPUT_TIME: {
            if (animationObj_.InputTimes()[CurrentRow()] != INVALID_TIME) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(animationObj_.InputTimes()[CurrentRow()]));
            }
            break;
        }
        case Index::START_POINT: {
            if (animationObj_.StartPoints()[CurrentRow()] != INVALID_TIME) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(animationObj_.StartPoints()[CurrentRow()]));
            }
            break;
        }
        case Index::END_POINT:
            if (animationObj_.EndPoints()[CurrentRow()] != INVALID_TIME) {
                sqlite3_result_int64(context_, static_cast<sqlite3_int64>(animationObj_.EndPoints()[CurrentRow()]));
            }
            break;
        case Index::FRAME_INFO:
            sqlite3_result_text(context_, dataCache_->GetDataFromDict(animationObj_.FrameInfos()[CurrentRow()]).c_str(),
                                STR_DEFAULT_LEN, nullptr);
            break;
        case Index::NAME:
            sqlite3_result_text(context_, dataCache_->GetDataFromDict(animationObj_.Names()[CurrentRow()]).c_str(),
                                STR_DEFAULT_LEN, nullptr);
            break;
        default:
            TS_LOGF("Unregistered column : %d", col);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
