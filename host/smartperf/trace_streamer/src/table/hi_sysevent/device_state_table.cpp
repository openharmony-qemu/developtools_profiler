
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

#include "device_state_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    ID = 0,
    BRIGHTNESS,
    BT_STATE,
    LOCATION,
    WIFI,
    STREAM_DEFAULT,
    VOICE_CALL,
    MUSIC,
    STREAM_RING,
    MEDIA,
    VOICE_ASSISTANT,
    SYSTEM,
    ALARM,
    NOTIFICATION,
    BT_SCO,
    ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    ACCESSIBILITY,
    RECORDING,
    STREAM_ALL
};
DeviceStateTable::DeviceStateTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("brightness", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("bt_state", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("location", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("wifi", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("stream_default", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("voice_call", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("music", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("stream_ring", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("media", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("voice_assistant", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("system", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("alarm", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("notification", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("bt_sco", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("enforced_audible", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("stream_dtmf", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("stream_tts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("accessibility", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("recording", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("stream_all", "INTEGER"));
    tablePriKey_.push_back("id");
}

DeviceStateTable::~DeviceStateTable() {}

std::unique_ptr<TableBase::Cursor> DeviceStateTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

DeviceStateTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstHiSysEventDeviceStateData().Size())),
      deviceStateData_(dataCache->GetConstHiSysEventDeviceStateData())
{
}

DeviceStateTable::Cursor::~Cursor() {}

int32_t DeviceStateTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, dataCache_->GetConstHiSysEventDeviceStateData().IdsData()[CurrentRow()]);
            break;
        case Index::BRIGHTNESS:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Brightness()[CurrentRow()]);
            break;
        case Index::BT_STATE:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().BtState()[CurrentRow()]);
            break;
        case Index::LOCATION:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Location()[CurrentRow()]);
            break;
        case Index::WIFI:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Wifi()[CurrentRow()]);
            break;
        case Index::STREAM_DEFAULT:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().StreamDefault()[CurrentRow()]);
            break;
        case Index::VOICE_CALL:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().VoiceCall()[CurrentRow()]);
            break;
        case Index::MUSIC:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Music()[CurrentRow()]);
            break;
        case Index::STREAM_RING:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().StreamRing()[CurrentRow()]);
            break;
        case Index::MEDIA:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Media()[CurrentRow()]);
            break;
        case Index::VOICE_ASSISTANT:
            sqlite3_result_int(context_,
                               dataCache_->GetConstHiSysEventDeviceStateData().VoiceAssistant()[CurrentRow()]);
            break;
        case Index::SYSTEM:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().System()[CurrentRow()]);
            break;
        case Index::ALARM:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Alarm()[CurrentRow()]);
            break;
        case Index::NOTIFICATION:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Notification()[CurrentRow()]);
            break;
        case Index::BT_SCO:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().BtSco()[CurrentRow()]);
            break;
        case Index::ENFORCED_AUDIBLE:
            sqlite3_result_int(context_,
                               dataCache_->GetConstHiSysEventDeviceStateData().EnforcedAudible()[CurrentRow()]);
            break;
        case Index::STREAM_DTMF:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().StreamDtmf()[CurrentRow()]);
            break;
        case Index::STREAM_TTS:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().StreamTts()[CurrentRow()]);
            break;
        case Index::ACCESSIBILITY:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Accessibility()[CurrentRow()]);
            break;
        case Index::RECORDING:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().Recording()[CurrentRow()]);
            break;
        case Index::STREAM_ALL:
            sqlite3_result_int(context_, dataCache_->GetConstHiSysEventDeviceStateData().StreamAll()[CurrentRow()]);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
