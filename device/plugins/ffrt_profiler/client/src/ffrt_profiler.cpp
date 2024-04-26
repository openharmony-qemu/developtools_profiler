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

#include "ffrt_profiler.h"

#include <charconv>
#include <fstream>
#include <unistd.h>
#include <sched.h>
#include <sys/prctl.h>

#include "logging.h"

namespace {
CachedHandle g_cachedHandle;
#define EXPECTANTLY(exp) (__builtin_expect(!!(exp), true))
#define UNEXPECTANTLY(exp) (__builtin_expect(!!(exp), false))

std::mutex g_mtx;
std::once_flag g_flag;
__thread int g_needReportThreadName = 1;
constexpr size_t ALIGNED_MASK = 7;
} // namespace

namespace OHOS::Developtools::Profiler {
FfrtProfiler* FfrtProfiler::instance_ = nullptr;

FfrtProfiler* FfrtProfiler::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mtx);
        if (instance_ == nullptr) {
            instance_ = new (std::nothrow) FfrtProfiler();
        }
    }
    return instance_;
}

FfrtProfiler::FfrtProfiler()
{
    pid_ = getprocpid();
    processName_ = GetProcessName(pid_);
}

FfrtProfiler::~FfrtProfiler() {}

bool FfrtProfiler::IsProfilerEnabled()
{
    CheckFfrtProfilerParam();
    return enable_;
}

void FfrtProfiler::CheckFfrtProfilerParam()
{
    if (UNEXPECTANTLY(g_cachedHandle == nullptr)) {
        std::unique_lock<std::mutex> lock(g_mtx);
        if (g_cachedHandle == nullptr) {
            g_cachedHandle = CachedParameterCreate(PARAM_KAY.c_str(), "");
        }
    }
    int changed = 0;
    const char *paramValue = CachedParameterGetChanged(g_cachedHandle, &changed);
    std::call_once(g_flag, [&]() { changed = 1; });

    if (UNEXPECTANTLY(changed == 1) && paramValue != nullptr) {
        if (!enable_ && strlen(paramValue) > 0) {
            std::vector<std::string> values;
            SplitString(paramValue, ",", values);
            for (auto& item : values) {
                int32_t pid = static_cast<int32_t>(strtoull(item.c_str(), nullptr, 0));
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

void FfrtProfiler::Enable()
{
    if (enable_) {
        return;
    }
    socketClent_ = std::make_shared<FfrtProfilerSocketClient>(pid_, this, nullptr);
    if (!socketClent_->ClientConnectState()) {
        PROFILER_LOG_ERROR(LOG_CORE, "ffrt profiler start failed");
        return;
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "ffrt profiler clent start, pid: %d, processName: %s", pid_, processName_.c_str());
}

void FfrtProfiler::Disable()
{
    if (!enable_) {
        return;
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "ffrt profiler clent stop, pid: %d, processName: %s", pid_, processName_.c_str());
    socketClent_ = nullptr;
    enable_ = false;
}

void FfrtProfiler::FfrtProfilerTrace(const uint8_t traceType, const std::string& lable, uint64_t cookie)
{
    if (EXPECTANTLY((!enable_))) {
        return;
    }
    FfrtTraceEvent trace;
    trace.type = TRACE_DATA;
    trace.traceType = traceType;
    trace.tid = getproctid();
    trace.cpu = static_cast<uint8_t>(sched_getcpu());
    trace.cookie = cookie;
    clock_gettime(clockType_, &trace.ts);
    GetThreadName(trace.threadName);

    if (lable.empty()) {
        FfrtProfilerSendData(reinterpret_cast<const void*>(&trace), sizeof(trace), nullptr, 0);
    } else {
        std::string strAligned = lable;
        size_t paddingCount = (strAligned.size() + 1 + ALIGNED_MASK) & (~ALIGNED_MASK);
        strAligned.append(paddingCount - strAligned.size() - 1, '\0');
        FfrtProfilerSendData(reinterpret_cast<const void*>(&trace), sizeof(trace), strAligned.c_str(),
            strAligned.size() + 1);
    }
}

void FfrtProfiler::FfrtProfiling(const EventType type, const char* payload, size_t payloadSize)
{
    if (EXPECTANTLY((!enable_)) || payload == nullptr || payloadSize == 0) {
        return;
    }
    FfrtResultBase base;
    base.type = static_cast<int32_t>(type);
    base.tid = getproctid();
    clock_gettime(clockType_, &base.ts);
    GetThreadName(base.threadName);
    FfrtProfilerSendData(&base, sizeof(base), payload, payloadSize);
}

void FfrtProfiler::FfrtProfilerSendData(const void* src, size_t size, const char* payload, size_t payloadSize)
{
    if (EXPECTANTLY((!enable_)) || src == nullptr) {
        return;
    }
    std::weak_ptr<FfrtProfilerSocketClient> weakClient = socketClent_;
    auto holder = weakClient.lock();
    if (holder == nullptr) {
        return;
    }

    if (!holder->SendFfrtProfilerData(src, size, payload, payloadSize)) {
        PROFILER_LOG_ERROR(LOG_CORE, "ffrt profiler SendFfrtProfilerData failed");
    }
}

void FfrtProfiler::GetThreadName(const void* src)
{
    if (!g_needReportThreadName) {
        return;
    }
    prctl(PR_GET_NAME, src);
    g_needReportThreadName = 0;
}
} // namespace OHOS::Developtools::Profiler