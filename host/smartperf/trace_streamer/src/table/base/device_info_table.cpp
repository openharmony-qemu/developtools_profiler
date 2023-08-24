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

#include "device_info_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum Index { PHYSICAL_WIDTH = 0, PHYSICAL_HEIGHT, PHYSICAL_FRAME_RATE };
DeviceInfoTable::DeviceInfoTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("physical_width", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("physical_height", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("physical_frame_rate", "INTEGER"));
    tablePriKey_.push_back("physical_width");
}

DeviceInfoTable::~DeviceInfoTable() {}

std::unique_ptr<TableBase::Cursor> DeviceInfoTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

DeviceInfoTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, 1), deviceInfoObj_(dataCache->GetConstDeviceInfo())
{
}

DeviceInfoTable::Cursor::~Cursor() {}

int32_t DeviceInfoTable::Cursor::Column(int32_t column) const
{
    switch (column) {
        case PHYSICAL_WIDTH:
            if (deviceInfoObj_.PhysicalWidth() != INVALID_UINT32) {
                sqlite3_result_int(context_, static_cast<int32_t>(deviceInfoObj_.PhysicalWidth()));
            }
            break;
        case PHYSICAL_HEIGHT:
            if (deviceInfoObj_.PhysicalHeight() != INVALID_UINT32) {
                sqlite3_result_int(context_, static_cast<int32_t>(deviceInfoObj_.PhysicalHeight()));
            }
            break;
        case PHYSICAL_FRAME_RATE:
            if (deviceInfoObj_.PhysicalFrameRate() != INVALID_UINT32) {
                sqlite3_result_int(context_, static_cast<int32_t>(deviceInfoObj_.PhysicalFrameRate()));
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
