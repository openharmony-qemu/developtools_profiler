/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NETWORK_PLUGIN_H
#define NETWORK_PLUGIN_H

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "logging.h"
#include "network_plugin_config.pb.h"
#include "network_plugin_result.pb.h"
#include "plugin_module_api.h"
#include "net_stats_client.h"

struct NetDetails {
    uint64_t tx;
    uint64_t rx;
    std::string type;
};

struct NetworkCell {
    int32_t pid;
    uint64_t tx;
    uint64_t rx;
    struct timespec ts;
    std::vector<NetDetails> details;
};

struct NetSystemDetails {
    std::string type;
    uint64_t rxBytes;
    uint64_t rxPackets;
    uint64_t txBytes;
    uint64_t txPackets;
};

struct NetSystemData {
    struct timespec ts;
    uint64_t rxBytes;
    uint64_t rxPackets;
    uint64_t txBytes;
    uint64_t txPackets;
    std::vector<NetSystemDetails> details;
};

enum NetworkNum {
    IFACE_INDEX = 2,
    UID_INDEX = 4,
    RX_BYTES_INDEX = 6,
    RX_PACKETS_INDEX = 7,
    TX_BYTES_INDEX = 8,
    TX_PACKETS_INDEX = 9,
    DEC_BASE = 10,
};

using TimeSpec = struct timespec;

struct NetFlowDetail {
    std::string type;
    uint64_t rxBytes;
    uint64_t rxPackets;
    uint64_t txBytes;
    uint64_t txPackets;
};

struct NetFlowData {
    TimeSpec ts;
    uint64_t rxBytes;
    uint64_t rxPackets;
    uint64_t txBytes;
    uint64_t txPackets;
    std::vector<NetFlowDetail> details;
};

class NetworkPlugin {
public:
    NetworkPlugin();
    ~NetworkPlugin() {};
    int Start(const uint8_t* configData, uint32_t configSize);
    int Report(uint8_t* data, uint32_t dataSize);
    int ReportOptimize(RandomWriteCtx* randomWrite);
    int Stop();
protected:
    std::string GetRateNodePath();
    int32_t GetUid(int32_t pid);
    bool ReadTxRxBytes(int32_t pid, NetworkCell &cell);
    bool ReadSystemTxRxBytes(NetSystemData &systemData);
    void AddNetDetails(NetworkCell& cell, NetDetails& data);
    void AddNetSystemDetails(NetSystemData& systemData, NetSystemDetails& data);
    std::string GetCmdArgs(const NetworkConfig& traceConfig);
    // for UT
    void setPathForTest(std::string path)
    {
        fileForTest_ = path;
    }
    std::string getPathForTest()
    {
        return fileForTest_;
    }

    template <typename T> bool WriteNetWorkData(T& networkDatasProto);
    //new version
    template <typename T> bool WriteNetFlowData(T& networkDatasProto);
    template <typename T> void WriteData(T &networkDatasProto, NetFlowData &netFlowData)
    {
        networkDatasProto.set_pid(singlePid_);
        networkDatasProto.set_tv_sec(netFlowData.ts.tv_sec);
        networkDatasProto.set_tv_nsec(netFlowData.ts.tv_nsec);
        networkDatasProto.set_rx_bytes(netFlowData.rxBytes);
        networkDatasProto.set_rx_packets(netFlowData.rxPackets);
        networkDatasProto.set_tx_bytes(netFlowData.txBytes);
        networkDatasProto.set_tx_packets(netFlowData.txPackets);
        for (auto& it:netFlowData.details) {
            auto* data = networkDatasProto.add_details();
            data->set_type(it.type);
            data->set_rx_bytes(it.rxBytes);
            data->set_rx_packets(it.rxPackets);
            data->set_tx_bytes(it.txBytes);
            data->set_tx_packets(it.txPackets);
        }
    }
    std::string GetBundleNameByPid(int32_t pid);
    int32_t GetUidByConfiguredBundleName(std::string bundleName);
    bool ScreenNetworkStatByUid(const std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos, NetFlowData &data);
    bool RetainAllNetworkStat(const std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos, NetFlowData &data);
    bool HandleData(NetFlowData present, NetFlowData &difference);
    void Record(NetFlowData &newData);
    // for UT
#ifdef NETWORK_PLUGIN_UNITTEST
    void setSingleUid(int32_t singleUid)
    {
        singleUid_ = singleUid;
    }
#endif

private:
    NetworkConfig protoConfig_;
    std::unique_ptr<uint8_t[]> buffer_;
    std::unique_ptr<FILE, int (*)(FILE*)> fp_;
    std::unordered_map<int32_t, int32_t> pidUid_;
    std::string fileForTest_;
    //new version
    int32_t singlePid_ = 0;
    int32_t singleUid_ = -1;
    NetFlowData previous_ = {{0, 0}, 0, 0, 0, 0, std::vector<NetFlowDetail>()};
    bool isFirst = true;
    bool isNewVersion = false;
    std::string bundleName_;
};
#endif