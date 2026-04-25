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

#include "hook_service.h"

#include "hook_manager.h"
#include <cinttypes>
#include <unistd.h>
#include <sys/socket.h>
#include "common.h"
#include "logging.h"
#include "parameter.h"
#include "socket_context.h"

namespace OHOS::Developtools::NativeDaemon {
const int VEC_SHIFT = 2;
HookService* g_hookServiceInstance = nullptr;

void HookServiceDisconnectCallback(int socketFd)
{
    if (g_hookServiceInstance != nullptr) {
        g_hookServiceInstance->ClientDisconnectCallback(socketFd);
    }
}

bool HookService::SendFileCacheFd(SocketContext &context, int32_t pid, uint16_t fileIndex)
{
    if (hookMgr_ == nullptr) {
        return false;
    }
    int cacheFd = hookMgr_->CreateFileCacheFd();
    if (cacheFd < 0) {
        return false;
    }
    HookCacheFileMsg cacheFileMsg(pid, fileIndex, CACHE_FILE_CREATED);
    bool ret = context.SendHookConfig(reinterpret_cast<uint8_t*>(&cacheFileMsg), sizeof(cacheFileMsg)) &&
        context.SendFileDescriptor(cacheFd);
    close(cacheFd);
    return ret;
}

bool HookService::SendInitialFileCacheFds(SocketContext &context, int32_t pid, const std::string& name)
{
    if (hookMgr_ == nullptr || !hookMgr_->PrepareFileCacheContext(pid, name)) {
        return false;
    }
    for (uint16_t fileIndex = 0; fileIndex < FILE_CACHE_FD_COUNT; ++fileIndex) {
        int cacheFd = hookMgr_->CreateFileCacheFd();
        if (cacheFd < 0) {
            return false;
        }
        bool ret = context.SendFileDescriptor(cacheFd);
        close(cacheFd);
        if (!ret) {
            PROFILER_LOG_ERROR(LOG_CORE, "SendInitialFileCacheFds failed, pid:%d index:%u", pid, fileIndex);
            return false;
        }
    }
    return true;
}

HookService::HookService(const ClientConfig& clientConfig, std::shared_ptr<HookManager> hook, bool multipleProcesses)
    : clientConfig_(clientConfig), hookMgr_(hook), multipleProcesses_(multipleProcesses)
{
    serviceName_ = "HookService";
    StartService(DEFAULT_UNIX_SOCKET_HOOK_PATH);
}

HookService::~HookService()
{
    if (g_hookServiceInstance == this) {
        g_hookServiceInstance = nullptr;
    }
    serviceEntry_ = nullptr;
}

bool HookService::StartService(const std::string& unixSocketName)
{
    serviceEntry_ = std::make_shared<ServiceEntry>();
    g_hookServiceInstance = this;
    if (!serviceEntry_->StartServer(unixSocketName, HookServiceDisconnectCallback)) {
        g_hookServiceInstance = nullptr;
        serviceEntry_ = nullptr;
        PROFILER_LOG_DEBUG(LOG_CORE, "Start IPC Service FAIL");
        return false;
    }
    serviceEntry_->RegisterService(*this);
    return true;
}

bool HookService::HandlePeerMessage(SocketContext &context, const int8_t *buf, uint32_t size)
{
    if (clientConfig_.enableFileCacheMode && size == sizeof(HookCacheFileMsg)) {
        ReceiveCacheFileMsg(context, buf);
        return true;
    }
    std::string path = hookMgr_->GetArktsDiagPath(buf, size);
    if (path.empty()) {
        return false;
    }
    context.SendHookConfig(reinterpret_cast<const uint8_t *>(path.c_str()), path.size());
    PROFILER_LOG_INFO(LOG_CORE, "ProtocolProc, receive message does not from hook client, just return");
    return false;
}

bool HookService::ResolveProcessName(int32_t peerConfig, std::string& procName)
{
    hookMgr_->SetPid(peerConfig);
    std::string filePath = "/proc/" + std::to_string(peerConfig) + "/cmdline";
    std::string bundleName;
    if (!LoadStringFromFile(filePath, bundleName)) {
        PROFILER_LOG_ERROR(LOG_CORE, "Get process name by pid failed!, pid: %d", peerConfig);
        return false;
    }
    bundleName.resize(strlen(bundleName.c_str()));
    if (bundleName.size() >= 2 && bundleName.substr(0, 2) == "./") {
        bundleName = bundleName.substr(2);
    }
    size_t found = bundleName.rfind("/");
    procName = (found != std::string::npos) ? bundleName.substr(found + 1) : bundleName;
    return true;
}

bool HookService::CheckConnectionPermission(SocketContext &context)
{
    if ((!firstProcess_) && (!multipleProcesses_)) {
        return false;
    }
    firstProcess_ = false;
    if (hookMgr_->GetHookConfig().startup_mode()) {
        return true;
    }
    int socketHandle = context.GetSocketHandle();
    struct ucred cred;
    socklen_t len = sizeof(struct ucred);
    if (getsockopt(socketHandle, SOL_SOCKET, SO_PEERCRED, &cred, &len) == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookService ProtocolProc getsockopt failed");
        return false;
    }
    auto it = pidInfo_.find(cred.pid);
    if (it == pidInfo_.end() || it->second.first != cred.uid || it->second.second != cred.gid) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookService ProtocolProc process uid gid check failed");
        return false;
    }
    return true;
}

bool HookService::SendSharedMemoryFds(SocketContext &context, int32_t peerConfig, const std::string& procName)
{
    int sharedMemCount = clientConfig_.offlineSymbolization ? SHARED_MEMORY_NUM : 1;
    std::vector<int> fdVec = hookMgr_->GetFds(peerConfig, procName, sharedMemCount);
    for (int i = 0; i < sharedMemCount; ++i) {
        int eventFd = fdVec[i * VEC_SHIFT];
        int smbFd = fdVec[i * VEC_SHIFT + 1];
        if (eventFd == smbFd) {
            PROFILER_LOG_ERROR(LOG_CORE, "Get fd failed!, name: %s, pid: %d", procName.c_str(), peerConfig);
            return false;
        }
        context.SendFileDescriptor(smbFd);
        context.SendFileDescriptor(eventFd);
    }
    return true;
}

bool HookService::SendPeerFds(SocketContext &context, int32_t peerConfig, const std::string& procName)
{
    if (clientConfig_.enableFileCacheMode) {
        if (!SendInitialFileCacheFds(context, peerConfig, procName)) {
            PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc send initial file cache fds failed");
            return false;
        }
        return true;
    }
    return SendSharedMemoryFds(context, peerConfig, procName);
}

bool HookService::SendAsyncStackFd(SocketContext &context)
{
    if (!clientConfig_.asyncStackEnable) {
        return true;
    }
    if (hookMgr_->uniqueTableBlock_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc uniqueTableBlock_ nullptr");
        return false;
    }
    int fd = hookMgr_->uniqueTableBlock_->GetfileDescriptor();
    context.SendFileDescriptor(fd);
    return true;
}

bool HookService::SendMemLeakFd(SocketContext &context)
{
    if (clientConfig_.memLeakFormat == MemLeakFormat::TYPE_DISABLE) {
        return true;
    }
    int fd = hookMgr_->CreateMemLeakFile();
    if (fd <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "SendMemLeakFd invalid fd");
        return false;
    }
    context.SendFileDescriptor(fd);
    PROFILER_LOG_INFO(LOG_CORE, "SendMemLeakFd sent fd %d", fd);
    return true;
}

void HookService::SaveSocketFd(SocketContext &context, pid_t pid)
{
    std::unique_lock<std::mutex> socketLock(pidFdMtx_);
    pidFds_[pid] = context.GetSocketHandle();
}

void HookService::AddPidInfo(pid_t pid, uid_t uid, gid_t gid)
{
    std::unique_lock<std::mutex> lock(pidInfoMtx_);
    pidInfo_[pid] = std::make_pair(uid, gid);
}

void HookService::RemovePidInfo(pid_t pid)
{
    std::unique_lock<std::mutex> lock(pidInfoMtx_);
    pidInfo_.erase(pid);
}

void HookService::CloseSocketFd(pid_t pid)
{
    std::unique_lock<std::mutex> lock(pidFdMtx_);
    if ((pidFds_.find(pid) != pidFds_.end()) && (pidFds_[pid] != 0)) {
        int socketFd = pidFds_[pid];
        pidFds_.erase(pid);
        lock.unlock();
        serviceEntry_->RemoveContext(socketFd);
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaService::CloseSocketFd pid invalid");
    }
}

bool HookService::HasSocketFd(pid_t pid)
{
    std::lock_guard<std::mutex> lock(pidFdMtx_);
    auto it = pidFds_.find(pid);
    return it != pidFds_.end() && it->second > 0;
}

void HookService::ClientDisconnectCallback(int socketFd)
{
    pid_t pid = 0;
    {
        std::lock_guard<std::mutex> lock(pidFdMtx_);
        for (const auto& [key, value] : pidFds_) {
            if (value == socketFd) {
                pid = key;
                break;
            }
        }
    }
    if (pid > 0 && hookMgr_ != nullptr && clientConfig_.enableFileCacheMode) {
        (void)hookMgr_->HandleCacheFileStopByPid(pid);
    }
}

bool HookService::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    if (hookMgr_ == nullptr) {
        return false;
    }
    PROFILER_LOG_INFO(LOG_CORE, "ProtocolProc recv size[%u]", size);
    if (size != sizeof(int)) {
        return HandlePeerMessage(context, buf, size);
    }
    int peerConfig = *const_cast<int *>(reinterpret_cast<const int *>(buf));
    if (peerConfig == -1) {
        return false;
    }
    std::string procName;
    if (!ResolveProcessName(peerConfig, procName)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mtx_);
    if (!CheckConnectionPermission(context)) {
        return false;
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "ProtocolProc, receive message from hook client, and send hook config to process %d",
                       peerConfig);
    if (!context.SendHookConfig(reinterpret_cast<uint8_t *>(&clientConfig_), sizeof(clientConfig_))) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProtocolProc send client config failed");
        return false;
    }
    if (!SendPeerFds(context, peerConfig, procName) || !SendAsyncStackFd(context) || !SendMemLeakFd(context)) {
        return false;
    }
    hookMgr_->ResetStartupParam();
    SaveSocketFd(context, static_cast<pid_t>(peerConfig));
    return true;
}

void HookService::ReceiveCacheFileMsg(SocketContext &context, const int8_t *buf)
{
    if (buf == nullptr) {
        return;
    }

    HookCacheFileMsg cacheFileMsg = *reinterpret_cast<HookCacheFileMsg *>(const_cast<int8_t*>(buf));
    // in file cache mode, receive cached file fd from native_hook
    std::lock_guard<std::mutex> lock(mtx_);
    if (cacheFileMsg.msgType == CACHE_FILE_READY) {
        int cacheFd = context.ReceiveFileDiscriptor();
        if (!hookMgr_->HandleCachedFileFdByPid(cacheFileMsg.hookPid, cacheFd, cacheFileMsg.fileIndex)) {
            close(cacheFd);
            return;
        }
        PROFILER_LOG_INFO(LOG_CORE, "ReceiveCacheFileMsg pid:%d ready index %u",
            cacheFileMsg.hookPid, cacheFileMsg.fileIndex);
        if (!hookMgr_->IsCacheFileStopRequested(cacheFileMsg.hookPid) &&
            !SendFileCacheFd(context, cacheFileMsg.hookPid, cacheFileMsg.fileIndex)) {
            PROFILER_LOG_ERROR(LOG_CORE, "ReceiveCacheFileMsg replenish fd failed, pid:%d index:%u",
                cacheFileMsg.hookPid, cacheFileMsg.fileIndex);
        }
        return;
    }
    if (cacheFileMsg.msgType == CACHE_FILE_STOP) {
        if (!hookMgr_->HandleCacheFileStopByPid(cacheFileMsg.hookPid)) {
            PROFILER_LOG_ERROR(LOG_CORE, "ReceiveCacheFileMsg stop failed, pid:%d", cacheFileMsg.hookPid);
        }
    }
}

void HookService::SendCommand(pid_t pid, uint32_t cmd)
{
    std::unique_lock<std::mutex> lock(pidFdMtx_);
    auto it = pidFds_.find(pid);
    if (it == pidFds_.end() || it->second <= 0) {
        return;
    }
    int sockFd = it->second;
    lock.unlock();

    struct ProtocolHead phead;
    phead.protoType = PROTOCOL_TYPE_PROTOBUF;
    phead.protoSize = sizeof(cmd) + sizeof(struct ProtocolHead);
    if (send(sockFd, reinterpret_cast<int8_t*>(&phead),
             sizeof(phead), 0) == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "SendCommand send head failed, cmd: %u", cmd);
        return;
    }
    if (send(sockFd, reinterpret_cast<int8_t*>(&cmd),
             sizeof(cmd), 0) == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "SendCommand send cmd failed, cmd: %u", cmd);
    }
}
}
