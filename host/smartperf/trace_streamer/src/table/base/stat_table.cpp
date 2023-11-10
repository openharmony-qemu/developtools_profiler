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

#include "stat_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t { EVENT_NAME = 0, STAT_EVENT_TYPE = 1, COUNT = 2, SEVERITY = 3, SOURCE = 4 };
StatTable::StatTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("event_name", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("stat_type", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("count", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("serverity", "TEXT"));
    tableColumn_.push_back(TableBase::ColumnInfo("source", "TEXT"));
    tablePriKey_.push_back("event_name");
    tablePriKey_.push_back("stat_type");
}

StatTable::~StatTable() {}

std::unique_ptr<TableBase::Cursor> StatTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

StatTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, STAT_EVENT_MAX * TRACE_EVENT_MAX)
{
}

StatTable::Cursor::~Cursor() {}

int32_t StatTable::Cursor::Column(int32_t column) const
{
    const StatAndInfo stat = dataCache_->GetConstStatAndInfo();
    SupportedTraceEventType eventType = static_cast<SupportedTraceEventType>(CurrentRow() / STAT_EVENT_MAX);
    StatType statType = static_cast<StatType>(CurrentRow() % STAT_EVENT_MAX);
    switch (static_cast<Index>(column)) {
        case Index::EVENT_NAME:
            sqlite3_result_text(context_, dataCache_->GetConstStatAndInfo().GetEvent(eventType).c_str(),
                                STR_DEFAULT_LEN, nullptr);
            break;
        case Index::STAT_EVENT_TYPE:
            sqlite3_result_text(context_, dataCache_->GetConstStatAndInfo().GetStat(statType).c_str(), STR_DEFAULT_LEN,
                                nullptr);
            break;
        case Index::COUNT:
            sqlite3_result_int64(context_,
                                 static_cast<int64_t>(dataCache_->GetConstStatAndInfo().GetValue(eventType, statType)));
            break;
        case Index::SEVERITY:
            sqlite3_result_text(context_,
                                dataCache_->GetConstStatAndInfo().GetSeverityDesc(eventType, statType).c_str(),
                                STR_DEFAULT_LEN, nullptr);
            break;
        case Index::SOURCE:
            sqlite3_result_text(context_, "trace", STR_DEFAULT_LEN, nullptr);
            break;
        default:
            TS_LOGF("Unregistered column : %d", column);
            break;
    }
    return SQLITE_OK;
}
} // namespace TraceStreamer
} // namespace SysTuning
