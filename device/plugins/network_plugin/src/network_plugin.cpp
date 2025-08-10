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

#include <string>
#include <sys/stat.h>

#include "buffer_splitter.h"
#include "common.h"
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

namespace {
using namespace OHOS::Developtools::Profiler;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
constexpr size_t READ_BUFFER_SIZE = 1024 * 16;
const std::string DEFAULT_NET_PATH("/proc/net/xt_qtaguid/stats");
} // namespace

NetworkPlugin::NetworkPlugin() : fp_(nullptr, nullptr)
{
    pidUid_.clear();
    buffer_ = std::make_unique<uint8_t[]>(READ_BUFFER_SIZE);
}

int NetworkPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    CHECK_NOTNULL(buffer_, -1, "%s:NetworkPlugin, buffer_ is null", __func__);

    if (protoConfig_.ParseFromArray(configData, configSize) <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:NetworkPlugin, parseFromArray failed!", __func__);
        return -1;
    }

    for (int i = 0; i < protoConfig_.pid().size(); i++) {
        int32_t pid = protoConfig_.pid(i);
        pidUid_.emplace(pid, GetUid(pid));
    }
    if (protoConfig_.pid().empty() && protoConfig_.test_file() == "") {
        PROFILER_LOG_INFO(LOG_CORE, "NetworkPlugin New Version!");
        isNewVersion = true;
        singlePid_ = protoConfig_.single_pid();
        CHECK_TRUE(singlePid_ >= 0, -1, "%s:invalid pid", __func__);
        if (protoConfig_.startup_process_name() != "") {
            bundleName_ = protoConfig_.startup_process_name();
            singleUid_ = GetUidByConfiguredBundleName(bundleName_);
            CHECK_TRUE(singleUid_ != -1, -1, "%s:get uid failed", __func__);
        } else if (protoConfig_.restart_process_name() != "") {
            bundleName_ = protoConfig_.restart_process_name();
            singleUid_ = GetUidByConfiguredBundleName(bundleName_);
            CHECK_TRUE(singleUid_ != -1, -1, "%s:get uid failed", __func__);
        } else if (singlePid_ > 0) {
            std::string name = GetBundleNameByPid(singlePid_);
            CHECK_TRUE(name != "", -1, "%s:get bundle name failed", __func__);
            bundleName_ = name;
            singleUid_ = GetUidByConfiguredBundleName(name);
            CHECK_TRUE(singleUid_ != -1, -1, "%s:get uid failed", __func__);
        }
    }

    int ret = COMMON::PluginWriteToHisysevent("network_plugin", "sh", GetCmdArgs(protoConfig_),
                                              COMMON::ErrorType::RET_SUCC, "success");
    PROFILER_LOG_INFO(LOG_CORE, "%s: NetworkPlugin success! hisysevent report result:%d", __func__, ret);
    return 0;
}

std::string NetworkPlugin::GetCmdArgs(const NetworkConfig& traceConfig)
{
    std::stringstream args;
    for (const auto& p : traceConfig.pid()) {
        args << "pid: " << COMMON::GetProcessNameByPid(p) << ", ";
    }
    args << "test_file: " << traceConfig.test_file();
    return args.str();
}

template <typename T> bool NetworkPlugin::WriteNetWorkData(T& networkDatasProto)
{
    std::string file = GetRateNodePath();
    if (protoConfig_.test_file() != "") {
        file = protoConfig_.test_file();
    }

    struct stat s;
    lstat(file.c_str(), &s);
    CHECK_TRUE(!S_ISDIR(s.st_mode), false, "%s:path(%s) is directory, no data to report", __func__, file.c_str());

    char realPath[PATH_MAX + 1] = {0};
    CHECK_TRUE((file.length() < PATH_MAX) && (realpath(file.c_str(), realPath) != nullptr), false,
               "%s:path is invalid: %s, errno=%d", __func__, file.c_str(), errno);
    fp_ = std::unique_ptr<FILE, int (*)(FILE*)>(fopen(realPath, "r"), fclose);
    CHECK_NOTNULL(fp_, false, "%s:NetworkPlugin, open(%s) Failed, errno(%d)", __func__, file.c_str(), errno);

    if (protoConfig_.pid().size() > 0) {
        for (int i = 0; i < protoConfig_.pid().size(); i++) {
            auto* info = networkDatasProto.add_networkinfo();
            int32_t pid = protoConfig_.pid(i);
            NetworkCell dataCell = {0};
            ReadTxRxBytes(pid, dataCell);
            // set proto
            for (auto& it : dataCell.details) {
                auto* data = info->add_details();
                data->set_tx_bytes(it.tx);
                data->set_rx_bytes(it.rx);
                data->set_type(it.type);
            }
            info->set_pid(pid);
            info->set_tx_bytes(dataCell.tx);
            info->set_rx_bytes(dataCell.rx);
            info->set_tv_sec(dataCell.ts.tv_sec);
            info->set_tv_nsec(dataCell.ts.tv_nsec);
        }
    } else if (protoConfig_.test_file() != "") { // test data
        NetSystemData systemData = {};
        ReadSystemTxRxBytes(systemData);
        static int randNum = 0;
        randNum++;
        auto* systemInfo = networkDatasProto.mutable_network_system_info();
        for (auto& it : systemData.details) {
            auto* data = systemInfo->add_details();
            data->set_rx_bytes(it.rxBytes + randNum * RX_BYTES_INDEX);
            data->set_rx_packets(it.rxPackets + randNum * RX_PACKETS_INDEX);
            data->set_tx_bytes(it.txBytes + randNum * TX_BYTES_INDEX);
            data->set_tx_packets(it.txPackets + randNum * TX_PACKETS_INDEX);
            data->set_type(it.type);
        }
        systemInfo->set_tv_sec(systemData.ts.tv_sec);
        systemInfo->set_tv_nsec(systemData.ts.tv_nsec);
        systemInfo->set_rx_bytes(systemData.rxBytes + (randNum * RX_BYTES_INDEX * systemData.details.size()));
        systemInfo->set_rx_packets(systemData.rxPackets + (randNum * RX_PACKETS_INDEX * systemData.details.size()));
        systemInfo->set_tx_bytes(systemData.txBytes + (randNum * TX_BYTES_INDEX * systemData.details.size()));
        systemInfo->set_tx_packets(systemData.txPackets + (randNum * TX_PACKETS_INDEX * systemData.details.size()));
    } else { // real data
        NetSystemData systemData = {};
        ReadSystemTxRxBytes(systemData);
        auto* systemInfo = networkDatasProto.mutable_network_system_info();
        for (auto& it : systemData.details) {
            auto* data = systemInfo->add_details();
            data->set_rx_bytes(it.rxBytes);
            data->set_rx_packets(it.rxPackets);
            data->set_tx_bytes(it.txBytes);
            data->set_tx_packets(it.txPackets);
            data->set_type(it.type);
        }
        systemInfo->set_tv_sec(systemData.ts.tv_sec);
        systemInfo->set_tv_nsec(systemData.ts.tv_nsec);
        systemInfo->set_rx_bytes(systemData.rxBytes);
        systemInfo->set_rx_packets(systemData.rxPackets);
        systemInfo->set_tx_bytes(systemData.txBytes);
        systemInfo->set_tx_packets(systemData.txPackets);
    }

    return true;
}

int NetworkPlugin::ReportOptimize(RandomWriteCtx* randomWrite)
{
    if (isNewVersion) {
        ProtoEncoder::NetworkFlowData dataProto(randomWrite);
        CHECK_TRUE(WriteNetFlowData(dataProto), -1, "%s:write network data failed", __func__);

        int msgSize = dataProto.Finish();
        return msgSize;
    } else {
        ProtoEncoder::NetworkDatas dataProto(randomWrite);
        CHECK_TRUE(WriteNetWorkData(dataProto), -1, "%s:write network data failed", __func__);

        int msgSize = dataProto.Finish();
        return msgSize;
    }
}

int NetworkPlugin::Report(uint8_t* data, uint32_t dataSize)
{
    if (isNewVersion) {
        NetworkFlowData dataProto;
        CHECK_TRUE(WriteNetFlowData(dataProto), -1, "%s:write network data failed", __func__);

        uint32_t length = dataProto.ByteSizeLong();
        if (length > dataSize) {
            return -length;
        }
        if (dataProto.SerializeToArray(data, length) > 0) {
            return length;
        }
        return 0;
    } else {
        NetworkDatas dataProto;
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
}

int NetworkPlugin::Stop()
{
    buffer_ = nullptr;
    fp_ = nullptr;
    pidUid_.clear();

    PROFILER_LOG_INFO(LOG_CORE, "%s:NetworkPlugin, stop success!", __func__);
    return 0;
}

std::string NetworkPlugin::GetRateNodePath()
{
    std::string name = "";

    if (!fileForTest_.empty()) {
        name = fileForTest_ + DEFAULT_NET_PATH;
        return name;
    }
    if (access(DEFAULT_NET_PATH.c_str(), F_OK) == 0) {
        name = DEFAULT_NET_PATH;
    }
    return name;
}

int32_t NetworkPlugin::GetUid(int32_t pid)
{
    CHECK_TRUE(pid > 0, -1, "%s:NetworkPlugin, check param fail, pid less than 0!", __func__);

    char* end = nullptr;
    std::string path = std::string("/proc/") + std::to_string(pid) + std::string("/status");
    if (!fileForTest_.empty()) {
        path = fileForTest_ + std::string("/proc/") + std::to_string(pid) + std::string("/status");
    }
    std::ifstream input(path, std::ios::in);
    if (input.fail()) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "%s:NetworkPlugin, open %s failed, errno(%s)", __func__, path.c_str(), buf);
        return -1;
    }
    do {
        if (!input.good()) {
            return -1;
        }
        std::string line;
        getline(input, line);
        if (!strncmp(line.c_str(), "Uid:", strlen("Uid:"))) {
            std::string str = line.substr(strlen("Uid:\t"));
            PROFILER_LOG_INFO(LOG_CORE, "%s:NetworkPlugin, line(%s), str(%s)", __func__, line.c_str(), str.c_str());
            return strtol(str.c_str(), &end, DEC_BASE);
        }
    } while (!input.eof());
    input.close();

    return -1;
}

bool NetworkPlugin::ReadTxRxBytes(int32_t pid, NetworkCell &cell)
{
    int32_t uid = pidUid_.at(pid);
    CHECK_NOTNULL(fp_.get(), false, "%s:NetworkPlugin, fp_ is null", __func__);
    int ret = fseek(fp_.get(), 0, SEEK_SET);
    CHECK_TRUE(ret == 0, false, "%s:NetworkPlugin, fseek failed, error(%d)!", __func__, errno);
    size_t rsize = static_cast<size_t>(fread(buffer_.get(), sizeof(char), READ_BUFFER_SIZE - 1, fp_.get()));
    buffer_.get()[rsize] = '\0';
    CHECK_TRUE(rsize >= 0, false, "%s:NetworkPlugin, read failed, errno(%d)", __func__, errno);
    char* end = nullptr;
    BufferSplitter totalbuffer((const char*)buffer_.get(), rsize + 1);
    do {
        int index = 0;
        NetDetails cache = {0};
        char tmp[TX_BYTES_INDEX + 1] = {0};
        while (totalbuffer.NextWord(' ')) {
            index++;
            if (totalbuffer.CurWord() == nullptr) {
                continue;
            }
            if (index == IFACE_INDEX && !strncmp(totalbuffer.CurWord(), "lo", strlen("lo"))) {
                break;
            }
            if (index == IFACE_INDEX &&
                strncpy_s(tmp, sizeof(tmp), totalbuffer.CurWord(), totalbuffer.CurWordSize()) == EOK) {
                cache.type = tmp;
            }
            uint64_t value = static_cast<uint64_t>(strtoull(totalbuffer.CurWord(), &end, DEC_BASE));
            CHECK_TRUE(value >= 0, false, "%s:NetworkPlugin, strtoull value failed", __func__);
            if ((index == UID_INDEX) && (uid != static_cast<int32_t>(value))) {
                break;
            }
            if (index == RX_BYTES_INDEX) {
                uint64_t rxBytes = value;
                cache.rx = rxBytes;
                cell.rx += rxBytes;
            }
            if (index == TX_BYTES_INDEX) {
                uint64_t txBytes = value;
                cache.tx = txBytes;
                cell.tx += txBytes;
                AddNetDetails(cell, cache);
            }
        }
    } while (totalbuffer.NextLine());

    clock_gettime(CLOCK_REALTIME, &cell.ts);

    return true;
}

void NetworkPlugin::AddNetDetails(NetworkCell& cell, NetDetails& data)
{
    bool finded = false;

    // 处理重复数据
    for (auto it = cell.details.begin(); it != cell.details.end(); it++) {
        if (it->type == data.type) {
            it->tx += data.tx;
            it->rx += data.rx;
            finded = true;
        }
    }

    if (!finded) {
        cell.details.push_back(data);
    }
}

bool NetworkPlugin::ReadSystemTxRxBytes(NetSystemData &systemData)
{
    CHECK_NOTNULL(fp_.get(), false, "%s:NetworkPlugin, fp_ is null", __func__);
    int ret = fseek(fp_.get(), 0, SEEK_SET);
    CHECK_TRUE(ret == 0, false, "%s:NetworkPlugin, fseek failed, error(%d)!", __func__, errno);
    size_t rsize = static_cast<size_t>(fread(buffer_.get(), sizeof(char), READ_BUFFER_SIZE - 1, fp_.get()));
    buffer_.get()[rsize] = '\0';
    CHECK_TRUE(rsize >= 0, false, "%s:NetworkPlugin, read failed, errno(%d)", __func__, errno);
    char* end = nullptr;
    BufferSplitter totalbuffer((const char*)buffer_.get(), rsize + 1);
    do {
        int index = 0;
        NetSystemDetails systemCache = {};
        char tmp[TX_BYTES_INDEX + 1] = "";
        while (totalbuffer.NextWord(' ')) {
            index++;
            if (totalbuffer.CurWord() == nullptr) {
                continue;
            }
            if (index == IFACE_INDEX && !strncmp(totalbuffer.CurWord(), "lo", strlen("lo"))) {
                break;
            }
            if (index == IFACE_INDEX &&
                strncpy_s(tmp, sizeof(tmp), totalbuffer.CurWord(), totalbuffer.CurWordSize()) == EOK) {
                systemCache.type = tmp;
            }
            if (strcmp(systemCache.type.c_str(), "iface") == 0) {
                break;
            }
            uint64_t value = static_cast<uint64_t>(strtoull(totalbuffer.CurWord(), &end, DEC_BASE));
            CHECK_TRUE(value >= 0, false, "%s:NetworkPlugin, strtoull value failed", __func__);
            if (index == RX_BYTES_INDEX) {
                uint64_t rxBytes = value;
                systemCache.rxBytes = rxBytes;
                systemData.rxBytes += rxBytes;
            } else if (index == RX_PACKETS_INDEX) {
                uint64_t rxPackets = value;
                systemCache.rxPackets = rxPackets;
                systemData.rxPackets += rxPackets;
            } else if (index == TX_BYTES_INDEX) {
                uint64_t txBytes = value;
                systemCache.txBytes = txBytes;
                systemData.txBytes += txBytes;
            } else if (index == TX_PACKETS_INDEX) {
                uint64_t txPackets = value;
                systemCache.txPackets = txPackets;
                systemData.txPackets += txPackets;
                AddNetSystemDetails(systemData, systemCache);
            }
        }
    } while (totalbuffer.NextLine());

    clock_gettime(CLOCK_REALTIME, &systemData.ts);

    return true;
}

void NetworkPlugin::AddNetSystemDetails(NetSystemData& systemData, NetSystemDetails& data)
{
    bool finded = false;

    // 处理重复数据
    for (auto it = systemData.details.begin(); it != systemData.details.end(); it++) {
        if (it->type == data.type) {
            it->rxBytes += data.rxBytes;
            it->rxPackets += data.rxPackets;
            it->txBytes += data.txBytes;
            it->txPackets += data.txPackets;
            finded = true;
        }
    }

    if (!finded) {
        systemData.details.push_back(data);
    }
}

template <typename T> bool NetworkPlugin::WriteNetFlowData(T& networkDatasProto)
{
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos;
    if (OHOS::NetManagerStandard::NetStatsClient::GetInstance().GetAllStatsInfo(infos) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetAllStatsInfo failed");
        return false;
    }
    NetFlowData netFlowData = {{0, 0}, 0, 0, 0, 0, std::vector<NetFlowDetail>()};
    if (bundleName_ != "") {
        ScreenNetworkStatByUid(infos, netFlowData);
    } else {
        RetainAllNetworkStat(infos, netFlowData);
    }
    WriteData(networkDatasProto, netFlowData);
    return true;
}

bool NetworkPlugin::RetainAllNetworkStat(const std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos,
                                         NetFlowData &data)
{
    NetFlowData present = {{0, 0}, 0, 0, 0, 0, std::vector<NetFlowDetail>()};
    std::map<std::string, NetFlowDetail> detailMap;
    for (auto& info : infos) {
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

bool NetworkPlugin::ScreenNetworkStatByUid(const std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos,
                                           NetFlowData &data)
{
    NetFlowData present = {{0, 0}, 0, 0, 0, 0, std::vector<NetFlowDetail>()};
    for (auto& info : infos) {
        if (static_cast<int32_t>(info.uid_) == singleUid_) {
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
    for (auto& presentDetail : present.details) {
        NetFlowDetail detail;
        bool havePrevious = false;
        for (auto& previousDetail : previous_.details) {
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

std::string NetworkPlugin::GetBundleNameByPid(int32_t pid)
{
    std::string bundleName;
    std::string filePath = "/proc/" + std::to_string(pid) + "/cmdline";
    LoadStringFromFile(filePath, bundleName);
    bundleName.resize(strlen(bundleName.c_str()));
    return bundleName;
}

int32_t NetworkPlugin::GetUidByConfiguredBundleName(std::string bundleName)
{
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