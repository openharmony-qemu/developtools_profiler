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
#include "network_plugin.h"

#include <sstream>
#include "network_plugin_result.pbencoder.h"
#include "securec.h"
#include "application_info.h"
#include "bundle_mgr_proxy.h"
#include "file_ex.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "os_account_info.h"
#include "os_account_manager.h"
#include "common.h"

using namespace OHOS::Developtools::Profiler;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

int NetworkPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    if (protoConfig_.ParseFromArray(configData, configSize) <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:NetworkPlugin, parseFromArray failed!", __func__);
        return -1;
    }
    pid_ = protoConfig_.pid();
    CHECK_TRUE(pid_ >= 0, -1, "%s:invalid pid", __func__);
    if (pid_>0) {
        uid_= GetUid(pid_);
        CHECK_TRUE(uid_ != -1, -1, "%s:get uid failed", __func__);
    }
    int ret = COMMON::PluginWriteToHisysevent("network_plugin", "sh", GetCmdArgs(protoConfig_),
                                              COMMON::ErrorType::RET_SUCC, "success");
    PROFILER_LOG_INFO(LOG_CORE, "%s: NetworkPlugin success! hisysevent report result:%d", __func__, ret);
    return 0;
}

std::string NetworkPlugin::GetCmdArgs(const NetworkConfig &traceConfig)
{
    std::stringstream args;
    args << "pid: " << traceConfig.pid();
    return args.str();
}

int NetworkPlugin::ReportOptimize(RandomWriteCtx* randomWrite)
{
    ProtoEncoder::NetworkFlowData dataProto(randomWrite);
    CHECK_TRUE(WriteNetWorkData(dataProto), -1, "%s:write network data failed", __func__);

    int msgSize = dataProto.Finish();
    return msgSize;
}

int NetworkPlugin::Report(uint8_t* data, uint32_t dataSize)
{
    NetworkFlowData dataProto;
    CHECK_TRUE(WriteNetWorkData(dataProto), -1, "%s:write network data failed", __func__);

    uint32_t length = dataProto.ByteSizeLong();
    if (length > dataSize) {
        return -length;
    }
    if (dataProto.SerializeToArray(data, length) > 0) {
        return length;
    }
    return 0;
}

int NetworkPlugin::Stop()
{
    PROFILER_LOG_INFO(LOG_CORE, "%s:NetworkPlugin, stop success!", __func__);
    return 0;
}

template <typename T> bool NetworkPlugin::WriteNetWorkData(T& networkDatasProto)
{
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos;
    if (OHOS::NetManagerStandard::NetStatsClient::GetInstance().GetAllStatsInfo(infos) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetAllStatsInfo failed");
    }
    NetFlowData netFlowData = {{0, 0}, 0, 0, 0, 0, std::vector<NetFlowDetail>()};
    if (pid_ > 0) {
        ScreenNetworkStatByUid(infos, netFlowData);
    } else {
        RetainAllNetworkStat(infos, netFlowData);
    }
    WriteData(networkDatasProto, netFlowData);
    return true;
}

template <typename T> void NetworkPlugin::WriteData(T &networkDatasProto, NetFlowData &netFlowData)
{
    networkDatasProto.set_pid(pid_);
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

bool NetworkPlugin::RetainAllNetworkStat(std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos, NetFlowData &data)
{
    NetFlowData present = {{0, 0}, 0, 0, 0, 0, std::vector<NetFlowDetail>()};
    std::map<std::string, NetFlowDetail> detailMap;
    for (auto info : infos) {
        present.rxBytes += info.rxBytes_;
        present.rxPackets += info.rxPackets_;
        present.txBytes += info.txBytes_;
        present.txPackets += info.txPackets_;
        std::string type = info.iface_;
        if (detailMap.find(type) != detailMap.end()) {
            detailMap[type].rxBytes += info.rxBytes_;
            detailMap[type].rxPackets += info.rxPackets_;
            detailMap[type].txBytes += info.txBytes_;
            detailMap[type].txPackets += info.txPackets_;
        } else {
            NetFlowDetail detail;
            detail.type = info.iface_;
            detail.rxBytes = info.rxBytes_;
            detail.rxPackets = info.rxPackets_;
            detail.txBytes = info.txBytes_;
            detail.txPackets = info.txPackets_;
            detailMap[type] = detail;
        }
    }
    for (auto it : detailMap) {
        present.details.push_back(it.second);
    }
    HandleData(present, data);
    return true;
}

bool NetworkPlugin::ScreenNetworkStatByUid(std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos, NetFlowData &data)
{
    NetFlowData present = {{0, 0}, 0, 0, 0, 0, std::vector<NetFlowDetail>()};
    for (auto info : infos) {
        if (info.uid_ == uid_) {
            NetFlowDetail detail;
            detail.type = info.iface_;
            detail.rxBytes = info.rxBytes_;
            detail.rxPackets = info.rxPackets_;
            detail.txBytes = info.txBytes_;
            detail.txPackets = info.txPackets_;
            present.details.push_back(detail);
            present.rxBytes += info.rxBytes_;
            present.rxPackets += info.rxPackets_;
            present.txBytes += info.txBytes_;
            present.txPackets += info.txPackets_;
        }
    }
    HandleData(present, data);
    return true;
}

bool NetworkPlugin::HandleData(NetFlowData present, NetFlowData &difference)
{
    if (isFirst) {
        (void)clock_gettime(CLOCK_REALTIME, &(difference.ts));
        difference.rxBytes = 0;
        difference.rxPackets = 0;
        difference.txBytes = 0;
        difference.txPackets = 0;
        isFirst = false;
        Record(present);
        return true;
    }

    (void)clock_gettime(CLOCK_REALTIME, &(difference.ts));
    difference.rxBytes = present.rxBytes - previous_.rxBytes;
    difference.rxPackets = present.rxPackets - previous_.rxPackets;
    difference.txBytes = present.txBytes - previous_.txBytes;
    difference.txPackets = present.txPackets - previous_.txPackets;
    for (auto presentDetail : present.details) {
        NetFlowDetail detail;
        bool havePrevious = false;
        for (auto previousDetail : previous_.details) {
            if (previousDetail.type == presentDetail.type) {
                detail.type = presentDetail.type;
                detail.rxBytes = presentDetail.rxBytes - previousDetail.rxBytes;
                detail.rxPackets = presentDetail.rxPackets - previousDetail.rxPackets;
                detail.txBytes = presentDetail.txBytes - previousDetail.txBytes;
                detail.txPackets = presentDetail.txPackets - previousDetail.txPackets;
                havePrevious = true;
                break;
            }
        }
        if (!havePrevious) {
            detail.type = presentDetail.type;
            detail.rxBytes = presentDetail.rxBytes;
            detail.rxPackets = presentDetail.rxPackets;
            detail.txBytes = presentDetail.txBytes;
            detail.txPackets = presentDetail.txPackets;
        }
        difference.details.push_back(detail);
    }
    Record(present);
    return true;
}

void NetworkPlugin::Record(NetFlowData &newData)
{
    previous_.rxBytes = newData.rxBytes;
    previous_.rxPackets = newData.rxPackets;
    previous_.txBytes = newData.txBytes;
    previous_.txPackets = newData.txPackets;
    previous_.details.clear();
    for (auto detail : newData.details) {
        previous_.details.push_back(detail);
    }
}

int32_t NetworkPlugin::GetUid(int32_t pid)
{
    std::string bundleName;
    std::string filePath = "/proc/" + std::to_string(pid) + "/cmdline";
    LoadStringFromFile(filePath, bundleName);
    bundleName.resize(strlen(bundleName.c_str()));
    int32_t userId = 0;
    std::vector<int32_t> activeIds;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "QueryActiveOsAccountIds failed");
        return -1;
    }
    if (activeIds.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "active id is empty");
        return -1;
    }
    userId = activeIds[0];
    auto manager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (manager == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "SystemAbilityManager is nullptr");
        return -1;
    }
    sptr<IRemoteObject> remoteObject = manager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "SystemAbility is nullptr");
        return -1;
    }
    sptr<AppExecFwk::IBundleMgr> mgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (mgr == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "mgr is nullptr");
        return -1;
    }
    int32_t uid = mgr->GetUidByBundleName(bundleName, userId);
    return uid;
}