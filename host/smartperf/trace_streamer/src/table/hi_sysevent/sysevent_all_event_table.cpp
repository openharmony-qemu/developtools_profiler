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

#include "sysevent_all_event_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    ID = 0,
    DOMAIN_ID,
    EVENT_NAME_ID,
    TS,
    TYPE,
    TIME_ZONE,
    PID,
    TID,
    UID,
    LEVEL,
    TAG,
    EVENT_ID,
    SEQ,
    INFO,
    CONTEXT
};
SysEventAllEventTable::SysEventAllEventTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("domain_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("event_name_id", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("type", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("time_zone", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("pid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("tid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("uid", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("level", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("tag", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("event_id", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("seq", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("info", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("contents", "TEXT"));
    tablePriKey_.push_back("id");
}

SysEventAllEventTable::~SysEventAllEventTable() {}

std::unique_ptr<TableBase::Cursor> SysEventAllEventTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

SysEventAllEventTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstHiSysEventAllEventData().Size())),
      hiSysEventAllEventObj_(dataCache->GetConstHiSysEventAllEventData())
{
}

SysEventAllEventTable::Cursor::~Cursor() {}
int32_t SysEventAllEventTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::ID:
            sqlite3_result_int64(context_, hiSysEventAllEventObj_.IdsData()[CurrentRow()]);
            break;
        case Index::DOMAIN_ID:
            sqlite3_result_int64(context_, hiSysEventAllEventObj_.DomainIds()[CurrentRow()]);
            break;
        case Index::EVENT_NAME_ID:
            sqlite3_result_int64(context_, hiSysEventAllEventObj_.EventNameIds()[CurrentRow()]);
            break;
        case Index::TS:
            sqlite3_result_int64(context_, hiSysEventAllEventObj_.TimeStampData()[CurrentRow()]);
            break;
        case Index::TYPE:
            sqlite3_result_int(context_, hiSysEventAllEventObj_.Types()[CurrentRow()]);
            break;
        case Index::TIME_ZONE:
            sqlite3_result_text(context_, hiSysEventAllEventObj_.TimeZones()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::PID:
            sqlite3_result_int(context_, hiSysEventAllEventObj_.Pids()[CurrentRow()]);
            break;
        case Index::TID:
            sqlite3_result_int(context_, hiSysEventAllEventObj_.Tids()[CurrentRow()]);
            break;
        case Index::UID:
            sqlite3_result_int(context_, hiSysEventAllEventObj_.Uids()[CurrentRow()]);
            break;
        case Index::LEVEL:
            sqlite3_result_text(context_, hiSysEventAllEventObj_.Levels()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::TAG:
            sqlite3_result_text(context_, hiSysEventAllEventObj_.Tags()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::EVENT_ID:
            sqlite3_result_text(context_, hiSysEventAllEventObj_.EventIds()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::SEQ:
            sqlite3_result_int(context_, hiSysEventAllEventObj_.Seqs()[CurrentRow()]);
            break;
        case Index::INFO:
            sqlite3_result_text(context_, hiSysEventAllEventObj_.Infos()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::CONTEXT:
            sqlite3_result_text(context_, hiSysEventAllEventObj_.Contents()[CurrentRow()].c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
