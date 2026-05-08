/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#ifndef HOOK_SERVICE_H
#define HOOK_SERVICE_H

#include <memory>
#include <string>

#include "hook_common.h"
#include "service_entry.h"

namespace OHOS::Developtools::NativeDaemon {
class HookManager;
class HookService : public ServiceBase {
public:
    HookService(const ClientConfig& clientConfig, std::shared_ptr<HookManager> hook, bool multipleProcesses = false);
    ~HookService();
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    void CloseSocketFd(pid_t pid);
    void AddPidInfo(pid_t pid, uid_t uid, gid_t gid);
    void RemovePidInfo(pid_t pid);
    void SendCommand(pid_t pid, uint32_t cmd);
    bool HasSocketFd(pid_t pid);
    void ClientDisconnectCallback(int socketFd);
private:
    bool StartService(const std::string& unixSocketName);
    bool HandlePeerMessage(SocketContext &context, const int8_t *buf, uint32_t size);
    bool ResolveProcessName(int32_t peerConfig, std::string& procName);
    bool CheckConnectionPermission(SocketContext &context);
    bool SendPeerFds(SocketContext &context, int32_t peerConfig, const std::string& procName);
    bool SendSharedMemoryFds(SocketContext &context, int32_t peerConfig, const std::string& procName);
    bool SendAsyncStackFd(SocketContext &context);
    bool SendMemLeakFd(SocketContext &context);
    void SaveSocketFd(SocketContext &context, pid_t pid);
    bool HandleControlCommand(uint32_t cmd);
    void ReceiveCacheFileMsg(SocketContext &context, const int8_t *buf);
    bool SendFileCacheFd(SocketContext &context, int32_t pid, uint16_t fileIndex);
    bool SendInitialFileCacheFds(SocketContext &context, int32_t pid, const std::string& name);

private:
    std::shared_ptr<ServiceEntry> serviceEntry_{nullptr};
    ClientConfig clientConfig_;
    std::shared_ptr<HookManager> hookMgr_{nullptr};
    std::mutex mtx_;
    std::mutex pidFdMtx_;
    std::mutex pidInfoMtx_;
    std::unordered_map<pid_t, int> pidFds_;
    bool firstProcess_ = true;
    bool multipleProcesses_ = false;
    std::unordered_map<pid_t, std::pair<uid_t, gid_t>> pidInfo_;
};
}
#endif // HOOK_SERVICE_H
