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

#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <memory>

#include "htrace_network_parser.h"
#include "network_plugin_result.pb.h"
#include "network_plugin_result.pbreader.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "parser/common_types.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
class HtraceNetworkParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
};

/**
 * @tc.name: ParseHtraceNetworkWithoutNetworkData
 * @tc.desc: Parse a Process that does not contain any ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceNetworkParserTest, ParseHtraceNetworkWithoutNetworkData, TestSize.Level1)
{
    TS_LOGI("test17-1");
    uint64_t ts = 100;
    auto networkInfo = std::make_unique<NetworkDatas>();
    std::string networkData = "";
    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());

    HtraceNetworkParser htraceNetworkParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceNetworkParser.Parse(networkInfoData, ts);
    auto size = stream_.traceDataCache_->GetConstNetworkData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHtraceNetworkWithNetworkData
 * @tc.desc: Parse a Process with ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceNetworkParserTest, ParseHtraceNetworkWithNetworkData, TestSize.Level1)
{
    TS_LOGI("test17-2");
    uint64_t ts = 100;
    const uint64_t DURS = 1999632780;
    const uint64_t TX = 712924;
    const uint64_t RX = 13535014;
    const uint64_t PACKETIN = 11431;
    const uint64_t PACKETOUT = 7373;

    auto networkInfo = std::make_unique<NetworkDatas>();
    NetworkSystemData* networkSystemData = new NetworkSystemData();
    networkSystemData->set_rx_bytes(RX);
    networkSystemData->set_tx_bytes(TX);
    networkSystemData->set_rx_packets(PACKETIN);
    networkSystemData->set_tx_packets(PACKETOUT);
    networkInfo->set_allocated_network_system_info(networkSystemData);

    std::string networkData = "";
    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());

    HtraceNetworkParser htraceNetworkParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceNetworkParser.Parse(networkInfoData, ts);
    htraceNetworkParser.Finish();
    auto size = stream_.traceDataCache_->GetConstNetworkData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseHtraceNetworkWithTwoNetworkData
 * @tc.desc: Parse a Process with ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceNetworkParserTest, ParseHtraceNetworkWithTwoNetworkData, TestSize.Level1)
{
    TS_LOGI("test17-3");
    uint64_t ts = 100;
    auto networkInfo = std::make_unique<NetworkDatas>();

    const uint64_t DURS_01 = 1999632781;
    const uint64_t TX_01 = 712921;
    const uint64_t RX_01 = 13535011;
    const uint64_t PACKETIN_01 = 11431;
    const uint64_t PACKETOUT_01 = 7371;
    NetworkSystemData* networkSystemDataFirst = new NetworkSystemData();
    networkSystemDataFirst->set_rx_bytes(RX_01);
    networkSystemDataFirst->set_tx_bytes(TX_01);
    networkSystemDataFirst->set_rx_packets(PACKETIN_01);
    networkSystemDataFirst->set_tx_packets(PACKETOUT_01);
    networkInfo->set_allocated_network_system_info(networkSystemDataFirst);

    std::string networkData = "";
    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData01(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());

    HtraceNetworkParser htraceNetworkParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceNetworkParser.Parse(networkInfoData01, ts);

    const uint64_t DURS_02 = 1999632782;
    const uint64_t TX_02 = 712922;
    const uint64_t RX_02 = 13535012;
    const uint64_t PACKETIN_02 = 11432;
    const uint64_t PACKETOUT_02 = 7372;
    NetworkSystemData* networkSystemDataSecond = new NetworkSystemData();
    networkSystemDataSecond->set_rx_bytes(RX_02);
    networkSystemDataSecond->set_tx_bytes(TX_02);
    networkSystemDataSecond->set_rx_packets(PACKETIN_02);
    networkSystemDataSecond->set_tx_packets(PACKETOUT_02);
    networkInfo->set_allocated_network_system_info(networkSystemDataSecond);

    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData02(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());

    htraceNetworkParser.Parse(networkInfoData02, ts);
    htraceNetworkParser.Finish();

    auto tx = stream_.traceDataCache_->GetConstNetworkData().TxDatas()[0];
    EXPECT_EQ(tx, TX_02);
    auto rx = stream_.traceDataCache_->GetConstNetworkData().RxDatas()[0];
    EXPECT_EQ(rx, RX_02);
    auto packetIn = stream_.traceDataCache_->GetConstNetworkData().PacketIn()[0];
    EXPECT_EQ(packetIn, PACKETIN_02);
    auto packetOut = stream_.traceDataCache_->GetConstNetworkData().PacketOut()[0];
    EXPECT_EQ(packetOut, PACKETOUT_02);
}

/**
 * @tc.name: ParseHtraceNetworkWithThreeNetworkData
 * @tc.desc: Parse a Process with ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceNetworkParserTest, ParseHtraceNetworkWithThreeNetworkData, TestSize.Level1)
{
    TS_LOGI("test17-4");
    uint64_t ts = 100;
    auto networkInfo = std::make_unique<NetworkDatas>();

    const uint64_t DURS_01 = 1999632781;
    const uint64_t TX_01 = 712921;
    const uint64_t RX_01 = 13535011;
    const uint64_t PACKETIN_01 = 11431;
    const uint64_t PACKETOUT_01 = 7371;
    NetworkSystemData* networkSystemDataFirst = new NetworkSystemData();
    networkSystemDataFirst->set_rx_bytes(RX_01);
    networkSystemDataFirst->set_tx_bytes(TX_01);
    networkSystemDataFirst->set_rx_packets(PACKETIN_01);
    networkSystemDataFirst->set_tx_packets(PACKETOUT_01);
    networkInfo->set_allocated_network_system_info(networkSystemDataFirst);

    std::string networkData = "";
    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData01(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());

    HtraceNetworkParser htraceNetworkParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceNetworkParser.Parse(networkInfoData01, ts);

    const uint64_t DURS_02 = 1999632782;
    const uint64_t TX_02 = 712922;
    const uint64_t RX_02 = 13535012;
    const uint64_t PACKETIN_02 = 11432;
    const uint64_t PACKETOUT_02 = 7372;
    NetworkSystemData* networkSystemDataSecond = new NetworkSystemData();
    networkSystemDataSecond->set_rx_bytes(RX_02);
    networkSystemDataSecond->set_tx_bytes(TX_02);
    networkSystemDataSecond->set_rx_packets(PACKETIN_02);
    networkSystemDataSecond->set_tx_packets(PACKETOUT_02);
    networkInfo->set_allocated_network_system_info(networkSystemDataSecond);

    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData02(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());
    htraceNetworkParser.Parse(networkInfoData02, ts);

    const uint64_t DURS_03 = 1999632783;
    const uint64_t TX_03 = 712923;
    const uint64_t RX_03 = 13535013;
    const uint64_t PACKETIN_03 = 11433;
    const uint64_t PACKETOUT_03 = 7373;
    NetworkSystemData* networkSystemDataThird = new NetworkSystemData();
    networkSystemDataThird->set_rx_bytes(RX_03);
    networkSystemDataThird->set_tx_bytes(TX_03);
    networkSystemDataThird->set_rx_packets(PACKETIN_03);
    networkSystemDataThird->set_tx_packets(PACKETOUT_03);
    networkInfo->set_allocated_network_system_info(networkSystemDataThird);

    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData03(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());
    htraceNetworkParser.Parse(networkInfoData03, ts);
    htraceNetworkParser.Finish();

    auto txFirst = stream_.traceDataCache_->GetConstNetworkData().TxDatas()[0];
    auto txSecond = stream_.traceDataCache_->GetConstNetworkData().TxDatas()[1];
    EXPECT_EQ(txFirst, TX_02);
    EXPECT_EQ(txSecond, TX_03);
    auto rxFirst = stream_.traceDataCache_->GetConstNetworkData().RxDatas()[0];
    auto rxSecond = stream_.traceDataCache_->GetConstNetworkData().RxDatas()[1];
    EXPECT_EQ(rxFirst, RX_02);
    EXPECT_EQ(rxSecond, RX_03);
    auto packetInFirst = stream_.traceDataCache_->GetConstNetworkData().PacketIn()[0];
    auto packetInSecond = stream_.traceDataCache_->GetConstNetworkData().PacketIn()[1];
    EXPECT_EQ(packetInFirst, PACKETIN_02);
    EXPECT_EQ(packetInSecond, PACKETIN_03);
    auto packetOutFirst = stream_.traceDataCache_->GetConstNetworkData().PacketOut()[0];
    auto packetOutSecond = stream_.traceDataCache_->GetConstNetworkData().PacketOut()[1];
    EXPECT_EQ(packetOutFirst, PACKETOUT_02);
    EXPECT_EQ(packetOutSecond, PACKETOUT_03);
}

/**
 * @tc.name: ParseHtraceNetworkWithMultipleNetworkData
 * @tc.desc: Parse a Process with ProcessData
 * @tc.type: FUNC
 */
HWTEST_F(HtraceNetworkParserTest, ParseHtraceNetworkWithMultipleNetworkData, TestSize.Level1)
{
    TS_LOGI("test17-5");
    uint64_t ts = 100;
    auto networkInfo = std::make_unique<NetworkDatas>();

    const uint64_t DURS_01 = 1999632781;
    const uint64_t TX_01 = 712921;
    const uint64_t RX_01 = 13535011;
    const uint64_t PACKETIN_01 = 11431;
    const uint64_t PACKETOUT_01 = 7371;
    NetworkSystemData* networkSystemDataFirst = new NetworkSystemData();
    networkSystemDataFirst->set_rx_bytes(RX_01);
    networkSystemDataFirst->set_tx_bytes(TX_01);
    networkSystemDataFirst->set_rx_packets(PACKETIN_01);
    networkSystemDataFirst->set_tx_packets(PACKETOUT_01);
    networkInfo->set_allocated_network_system_info(networkSystemDataFirst);

    std::string networkData = "";
    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData01(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());

    HtraceNetworkParser htraceNetworkParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    htraceNetworkParser.Parse(networkInfoData01, ts);

    const uint64_t DURS_02 = 1999632782;
    const uint64_t TX_02 = 712922;
    const uint64_t RX_02 = 13535012;
    const uint64_t PACKETIN_02 = 11432;
    const uint64_t PACKETOUT_02 = 7372;
    NetworkSystemData* networkSystemDataSecond = new NetworkSystemData();
    networkSystemDataSecond->set_rx_bytes(RX_02);
    networkSystemDataSecond->set_tx_bytes(TX_02);
    networkSystemDataSecond->set_rx_packets(PACKETIN_02);
    networkSystemDataSecond->set_tx_packets(PACKETOUT_02);
    networkInfo->set_allocated_network_system_info(networkSystemDataSecond);

    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData02(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());
    htraceNetworkParser.Parse(networkInfoData02, ts);

    const uint64_t DURS_03 = 1999632783;
    const uint64_t TX_03 = 712923;
    const uint64_t RX_03 = 13535013;
    const uint64_t PACKETIN_03 = 11433;
    const uint64_t PACKETOUT_03 = 7373;
    NetworkSystemData* networkSystemDataThird = new NetworkSystemData();
    networkSystemDataThird->set_rx_bytes(RX_03);
    networkSystemDataThird->set_tx_bytes(TX_03);
    networkSystemDataThird->set_rx_packets(PACKETIN_03);
    networkSystemDataThird->set_tx_packets(PACKETOUT_03);
    networkInfo->set_allocated_network_system_info(networkSystemDataThird);

    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData03(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());
    htraceNetworkParser.Parse(networkInfoData03, ts);

    const uint64_t DURS_04 = 1999632784;
    const uint64_t TX_04 = 712924;
    const uint64_t RX_04 = 13535014;
    const uint64_t PACKETIN_04 = 11434;
    const uint64_t PACKETOUT_04 = 7374;
    NetworkSystemData* networkSystemDataForth = new NetworkSystemData();
    networkSystemDataForth->set_rx_bytes(RX_04);
    networkSystemDataForth->set_tx_bytes(TX_04);
    networkSystemDataForth->set_rx_packets(PACKETIN_04);
    networkSystemDataForth->set_tx_packets(PACKETOUT_04);
    networkInfo->set_allocated_network_system_info(networkSystemDataForth);

    networkInfo->SerializeToString(&networkData);
    ProtoReader::BytesView networkInfoData04(reinterpret_cast<const uint8_t*>(networkData.data()), networkData.size());
    htraceNetworkParser.Parse(networkInfoData04, ts);
    htraceNetworkParser.Finish();

    auto txFirst = stream_.traceDataCache_->GetConstNetworkData().TxDatas()[0];
    auto txSecond = stream_.traceDataCache_->GetConstNetworkData().TxDatas()[1];
    auto txThird = stream_.traceDataCache_->GetConstNetworkData().TxDatas()[2];
    EXPECT_EQ(txFirst, TX_02);
    EXPECT_EQ(txSecond, TX_03);
    EXPECT_EQ(txThird, TX_04);
    auto rxFirst = stream_.traceDataCache_->GetConstNetworkData().RxDatas()[0];
    auto rxSecond = stream_.traceDataCache_->GetConstNetworkData().RxDatas()[1];
    auto rxThird = stream_.traceDataCache_->GetConstNetworkData().RxDatas()[2];
    EXPECT_EQ(rxFirst, RX_02);
    EXPECT_EQ(rxSecond, RX_03);
    EXPECT_EQ(rxThird, RX_04);
    auto packetInFirst = stream_.traceDataCache_->GetConstNetworkData().PacketIn()[0];
    auto packetInSecond = stream_.traceDataCache_->GetConstNetworkData().PacketIn()[1];
    auto packetInThird = stream_.traceDataCache_->GetConstNetworkData().PacketIn()[2];
    EXPECT_EQ(packetInFirst, PACKETIN_02);
    EXPECT_EQ(packetInSecond, PACKETIN_03);
    EXPECT_EQ(packetInThird, PACKETIN_04);
    auto packetOutFirst = stream_.traceDataCache_->GetConstNetworkData().PacketOut()[0];
    auto packetOutSecond = stream_.traceDataCache_->GetConstNetworkData().PacketOut()[1];
    auto packetOutThird = stream_.traceDataCache_->GetConstNetworkData().PacketOut()[2];
    EXPECT_EQ(packetOutFirst, PACKETOUT_02);
    EXPECT_EQ(packetOutSecond, PACKETOUT_03);
    EXPECT_EQ(packetOutThird, PACKETOUT_04);
}
} // namespace TraceStreamer
} // namespace SysTuning