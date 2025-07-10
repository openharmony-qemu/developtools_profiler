/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include "network_profiler.h"

#include <charconv>
#include <fstream>
#include <unistd.h>
#include <sched.h>
#include <sys/prctl.h>

#include "logging.h"
#include "parameters.h"

namespace {
CachedHandle g_cachedHandle;
#define EXPECTANTLY(exp) (__builtin_expect(!!(exp), true))
#define UNEXPECTANTLY(exp) (__builtin_expect(!!(exp), false))

std::mutex g_cachedMtx;
std::mutex g_mtx;
std::once_flag g_flag;
thread_local std::string g_threadName;
const std::string PARAM_KAY = "hiviewdfx.hiprofiler.networkprofiler.target";
static std::atomic<bool> g_serverClosing{false};
static std::atomic<bool> waitingConnect_{false};
static std::vector<OHOS::Developtools::Profiler::NetworkProfiler::CachedData> cachedData_;
static std::atomic<bool> enable_{false};
static std::shared_ptr<OHOS::Developtools::Profiler::NetworkProfilerSocketClient> socketClent_{nullptr};
static std::string processName_;
static int32_t pid_{0};
static int g_changed{0};
} // namespace

namespace OHOS::Developtools::Profiler {
NetworkProfiler* NetworkProfiler::instance_ = nullptr;

NetworkProfiler* NetworkProfiler::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mtx);
        if (instance_ == nullptr) {
            instance_ = new (std::nothrow) NetworkProfiler();
        }
    }
    return instance_;
}

NetworkProfiler::NetworkProfiler()
{
    pid_ = getprocpid();
    processName_ = GetProcessNameByPid(pid_);
    isDeveloperMode_ = OHOS::system::GetBoolParameter("const.security.developermode.state", false);
}

NetworkProfiler::~NetworkProfiler() {}

void NetworkProfiler::SetEnableFlag(bool flag)
{
    enable_ = flag;
}

void NetworkProfiler::SetWaitingFlag(bool flag)
{
    waitingConnect_ = flag;
}

bool NetworkProfiler::IsProfilerEnable()
{
    if (g_serverClosing) {
        PROFILER_LOG_ERROR(LOG_CORE, "network profiler server is closing");
        return false;
    }
    CheckNetworkProfilerParam();
    return enable_ || waitingConnect_;
}

void NetworkProfiler::CheckNetworkProfilerParam()
{
    if (!isDeveloperMode_) {
        return;
    }
    if (UNEXPECTANTLY(g_cachedHandle == nullptr)) {
        std::unique_lock<std::mutex> lock(g_mtx);
        if (g_cachedHandle == nullptr) {
            g_cachedHandle = CachedParameterCreate(PARAM_KAY.c_str(), "");
        }
    }
    int changed = 0;
    const char *paramValue = CachedParameterGetChanged(g_cachedHandle, &changed);
    std::call_once(g_flag, [&]() { changed = 1; });

    if ((UNEXPECTANTLY(changed == 1) || g_changed) && paramValue != nullptr) {
        if (!enable_ && strlen(paramValue) > 0) {
            std::vector<std::string> values;
            SplitParamValue(paramValue, ",", values);
            for (auto& item : values) {
                int32_t pid = static_cast<int32_t>(strtoull(item.c_str(), nullptr, 0));
                if (((pid > 0) && (pid == pid_)) || ((pid == 0) && (item == processName_))) {
                    Enable();
                    break;
                }
            }
        }
    }
}

void NetworkProfiler::Enable()
{
    if (enable_) {
        return;
    }
    waitingConnect_ = true;
    socketClent_ = std::make_shared<NetworkProfilerSocketClient>(pid_, this,
    reinterpret_cast<void (*)()>(&ServiceCloseCallback));
    if (!socketClent_->ClientConnectState()) {
        waitingConnect_ = false;
        PROFILER_LOG_ERROR(LOG_CORE, "network profiler start failed");
        return;
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "network profiler clent start, pid: %d, processName: %s", pid_, processName_.c_str());
}

void NetworkProfiler::ServiceCloseCallback()
{
    bool expected = false;
    if (g_serverClosing.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_relaxed)) {
        g_changed = 1;
        Disable();
        g_serverClosing = false;
        g_changed = 0;
        waitingConnect_ = false;
        cachedData_.clear();
    }
}

void NetworkProfiler::Disable()
{
    if (!enable_) {
        return;
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "network profiler clent stop, pid: %d, processName: %s", pid_, processName_.c_str());
    if (socketClent_) {
        socketClent_->Reset();
    }
    enable_ = false;
}

void NetworkProfiler::SendCachedData()
{
    std::unique_lock<std::mutex> lock(g_cachedMtx);
    for (size_t index = 0; index < cachedData_.size(); ++index) {
        NetworkProfiling(cachedData_[index].cachedType, cachedData_[index].cachedData,
                         cachedData_[index].cachedDataSize);
    }
    cachedData_.clear();
}

void NetworkProfiler::NetworkProfiling(const uint8_t type, const char* data, size_t dataSize)
{
    if (EXPECTANTLY((!enable_))) {
        if (waitingConnect_) {
            std::unique_lock<std::mutex> lock(g_cachedMtx);
            cachedData_.push_back({type, data, dataSize});
        }
        return;
    }
    NetworkEvent event;
    event.type = type;
    event.tid = getproctid();
    clock_gettime(clockType_, &event.ts);
    GetThreadName(event.threadName);
    if (g_threadName.size() == 0) {
        GetThreadName(event.threadName);
    } else {
        if (sprintf_s(event.threadName, sizeof(event.threadName), "%s", g_threadName.c_str()) < 0) {
            PROFILER_LOG_DEBUG(LOG_CORE, "sprintf_s event.threadName failed");
            return;
        }
    }
    NetworkProfilerSendData(reinterpret_cast<const void*>(&event), sizeof(event), data, dataSize);
}

void NetworkProfiler::NetworkProfilerSendData(const void* src, size_t size, const char* payload, size_t payloadSize)
{
    if (EXPECTANTLY((!enable_)) || src == nullptr) {
        return;
    }
    std::weak_ptr<NetworkProfilerSocketClient> weakClient = socketClent_;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return;
    }

    if (!holder->SendNetworkProfilerData(src, size, payload, payloadSize)) {
        PROFILER_LOG_ERROR(LOG_CORE, "network profiler SendNetworkProfilerData failed");
    }
}

void NetworkProfiler::GetThreadName(const char* src)
{
    prctl(PR_GET_NAME, src);
    std::string tmp(src);
    g_threadName = tmp;
}
} // namespace OHOS::Developtools::Profiler
