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

#include "logging.h"
#include "network_profiler_common.h"

namespace {
CachedHandle g_cachedHandle;
#define EXPECTANTLY(exp) (__builtin_expect(!!(exp), true))
#define UNEXPECTANTLY(exp) (__builtin_expect(!!(exp), false))

std::mutex g_mtx;
std::once_flag flag;
const std::string THREAD_ID_HEAD = "/proc/self/task/";
const std::string THREAD_ID_TAIL = "/comm";
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
    processName_ = GetProcessName(pid_);
}

NetworkProfiler::~NetworkProfiler() {}

void NetworkProfiler::CheckNetworkProfilerParam()
{
    if (UNEXPECTANTLY(g_cachedHandle == nullptr)) {
        std::unique_lock<std::mutex> lock(g_mtx);
        if (g_cachedHandle == nullptr) {
            g_cachedHandle = CachedParameterCreate(PARAM_KAY.c_str(), "");
        }
    }
    int changed = 0;
    const char *paramValue = CachedParameterGetChanged(g_cachedHandle, &changed);
    std::call_once(flag, [&]() { changed = 1; });

    if (UNEXPECTANTLY(changed == 1) && paramValue != nullptr) {
        if (!enable_ && strlen(paramValue) > 0) {
            std::vector<std::string> values;
            SplitString(paramValue, ",", values);
            for (auto& item : values) {
                uint64_t pid = strtoull(item.c_str(), nullptr, 0);
                if (((pid > 0) && (pid == pid_)) || ((pid == 0) && (item == processName_))) {
                    Enable();
                    break;
                }
            }
        } else if (enable_ && strlen(paramValue) == 0) {
            Disable();
        }
    }
}

void NetworkProfiler::Enable()
{
    if (enable_) {
        return;
    }
    socketClent_ = std::make_shared<NetworkProfilerSocketClient>(pid_, this, nullptr);
    if (!socketClent_->ClientConnectState()) {
        PROFILER_LOG_ERROR(LOG_CORE, "network profiler start failed");
        return;
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "network profiler clent start, pid: %d, processName: %s", pid_, processName_.c_str());
}

void NetworkProfiler::Disable()
{
    if (!enable_) {
        return;
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "network profiler clent stop, pid: %d, processName: %s", pid_, processName_.c_str());
    socketClent_ = nullptr;
    enable_ = false;
}

void NetworkProfiler::NetworkProfilerTrace(const int32_t traceType, char* data, uint64_t dataLength)
{
    if (EXPECTANTLY((!enable_))) {
        return;
    }
    NetworkData networkData;
    networkData.type = traceType;
    networkData.tid = getproctid();
    clock_gettime(CLOCK_REALTIME, &networkData.ts);
    GetThreadName(networkData.threadName, MAX_COMM_LEN);
    NetworkProfilerBase(reinterpret_cast<const void*>(&networkData), sizeof(NetworkResultBase), data, dataLength);
}

void NetworkProfiler::NetworkProfilerBase(const void* src, size_t size, const char* payload, size_t payloadSize)
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
        PROFILER_LOG_DEBUG(LOG_CORE, "network profiler SendNetworkProfilerData failed");
    }
}

void NetworkProfiler::GetThreadName(uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    std::string threadNameStr = THREAD_ID_HEAD + std::to_string(getproctid()) + THREAD_ID_TAIL;
    FILE* fp = fopen(threadNameStr.c_str(), "r");
    if (fp == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "network profiler fopen %s failed", threadNameStr.c_str());
        return;
    }

    if (fgets(reinterpret_cast<char*>(data), size, fp) == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "network profiler fgets failed");
    }

    (void)fclose(fp);
}
} // namespace OHOS::Developtools::Profiler