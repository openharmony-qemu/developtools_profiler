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

#include "logging.h"
#include "network_plugin_config.pb.h"
#include "network_plugin_result.pb.h"
#include "plugin_module_api.h"
#include "net_stats_client.h"

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
    NetworkPlugin() {};
    ~NetworkPlugin() {};
    int Start(const uint8_t* configData, uint32_t configSize);
    int Report(uint8_t* data, uint32_t dataSize);
    int ReportOptimize(RandomWriteCtx* randomWrite);
    int Stop();

    template <typename T> bool WriteNetWorkData(T& networkDatasProto);
    int32_t GetUid(int32_t pid);
    bool ScreenNetworkStatByUid(std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos, NetFlowData &data);
    bool RetainAllNetworkStat(std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos, NetFlowData &data);
    bool HandleData(NetFlowData present, NetFlowData &difference);

private:
    NetworkConfig protoConfig_;
    int32_t pid_ = 0;
    int32_t uid_ = -1;
    NetFlowData previous_ = {{0, 0}, 0, 0, 0, 0, std::vector<NetFlowDetail>()};
    bool isFirst = true;
};
#endif