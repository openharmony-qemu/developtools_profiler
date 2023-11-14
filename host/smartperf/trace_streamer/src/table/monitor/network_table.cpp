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

#include "network_table.h"

namespace SysTuning {
namespace TraceStreamer {
enum class Index : int32_t {
    TS = 0,
    DUR,
    TX,
    RX,
    TX_SPEED,
    RX_SPEED,
    PACKET_IN,
    PACKET_IN_SEC,
    PACKET_OUT,
    PACKET_OUT_SEC,
    NET_TYPE,
};
NetworkTable::NetworkTable(const TraceDataCache* dataCache) : TableBase(dataCache)
{
    tableColumn_.push_back(TableBase::ColumnInfo("ts", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("dur", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("tx", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("rx", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("tx_speed", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("rx_speed", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("packet_in", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("packet_in_sec", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("packet_out", "INTEGER"));
    tableColumn_.push_back(TableBase::ColumnInfo("packet_out_sec", "REAL"));
    tableColumn_.push_back(TableBase::ColumnInfo("net_type", "TEXT"));
    tablePriKey_.push_back("ts");
}

NetworkTable::~NetworkTable() {}

std::unique_ptr<TableBase::Cursor> NetworkTable::CreateCursor()
{
    return std::make_unique<Cursor>(dataCache_, this);
}

NetworkTable::Cursor::Cursor(const TraceDataCache* dataCache, TableBase* table)
    : TableBase::Cursor(dataCache, table, static_cast<uint32_t>(dataCache->GetConstNetworkData().Size())),
      networkDataObj_(dataCache->GetConstNetworkData())
{
}

NetworkTable::Cursor::~Cursor() {}

int32_t NetworkTable::Cursor::Column(int32_t column) const
{
    switch (static_cast<Index>(column)) {
        case Index::TS: {
            sqlite3_result_int64(context_, static_cast<int64_t>(networkDataObj_.TimeStampData()[CurrentRow()]));
            break;
        }
        case Index::TX: {
            sqlite3_result_int64(context_, static_cast<int64_t>(networkDataObj_.TxDatas()[CurrentRow()]));
            break;
        }
        case Index::RX: {
            sqlite3_result_int64(context_, static_cast<int64_t>(networkDataObj_.RxDatas()[CurrentRow()]));
            break;
        }
        case Index::DUR: {
            sqlite3_result_int64(context_, static_cast<int64_t>(networkDataObj_.Durs()[CurrentRow()]));
            break;
        }
        case Index::TX_SPEED: {
            sqlite3_result_double(context_, static_cast<double>(networkDataObj_.TxSpeed()[CurrentRow()]));
            break;
        }
        case Index::RX_SPEED: {
            sqlite3_result_double(context_,
                                  static_cast<double>(dataCache_->GetConstNetworkData().RxSpeed()[CurrentRow()]));
            break;
        }
        case Index::PACKET_IN: {
            sqlite3_result_int64(context_,
                                 static_cast<int64_t>(dataCache_->GetConstNetworkData().PacketIn()[CurrentRow()]));
            break;
        }
        case Index::PACKET_IN_SEC: {
            sqlite3_result_double(context_,
                                  static_cast<double>(dataCache_->GetConstNetworkData().PacketInSec()[CurrentRow()]));
            break;
        }
        case Index::PACKET_OUT: {
            sqlite3_result_int64(context_,
                                 static_cast<double>(dataCache_->GetConstNetworkData().PacketOut()[CurrentRow()]));
            break;
        }
        case Index::PACKET_OUT_SEC: {
            sqlite3_result_double(context_,
                                  static_cast<double>(dataCache_->GetConstNetworkData().PacketOutSec()[CurrentRow()]));
            break;
        }
        case Index::NET_TYPE: {
            sqlite3_result_text(context_, dataCache_->GetConstNetworkData().NetTypes()[CurrentRow()].c_str(),
                                STR_DEFAULT_LEN, nullptr);
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
