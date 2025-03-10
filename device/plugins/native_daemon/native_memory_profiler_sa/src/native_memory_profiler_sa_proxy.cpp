/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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


#include "native_memory_profiler_sa_proxy.h"
#include "parameters.h"
#include <sys/time.h>
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include "logging.h"
#include "common.h"
#include "dfx_dump_catcher.h"
#include <iostream>
#include <sstream>

namespace OHOS::Developtools::NativeDaemon {
constexpr int32_t TIME_OUT = 15;
constexpr int32_t MAX_FRAME_NUM = 100;
constexpr uint64_t S_TO_NS = 1000 * 1000 * 1000;
NativeMemoryProfilerSaProxy::NativeMemoryProfilerSaProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INativeMemoryProfilerSa>(impl) {}

int32_t NativeMemoryProfilerSaProxy::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed to write descriptor");
        return RET_ERR;
    }
    CHECK_TRUE(config->Marshalling(data), RET_ERR, "NativeMemoryProfilerSaConfig marshalling failed");

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::START),
                                      data, reply, option);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed");
        return ret;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaProxy::DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpData failed to write descriptor");
        return RET_ERR;
    }

    CHECK_TRUE(config->Marshalling(data), RET_ERR, "NativeMemoryProfilerSaConfig marshalling failed");
    if (!data.WriteFileDescriptor(fd)) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpData failed to write fd");
        return RET_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_DATA),
                                      data, reply, option);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpData failed");
        return ret;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaProxy::Stop(uint32_t pid)
{
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
    XCollieCallback callbackFunc = [](void *) {
        int pidVal = 0;
        if (!COMMON::IsProcessExist("native_daemon", pidVal)) {
            PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaProxy::Stop native_daemon doesn't exist!");
        } else {
            HiviewDFX::DfxDumpCatcher dumplog;
            std::string msg = "";
            if (!dumplog.DumpCatch(pidVal, 0, msg, MAX_FRAME_NUM)) {
                PROFILER_LOG_ERROR(LOG_CORE, "NativeMemoryProfilerSaProxy::Stop DumpCatch failed!");
            } else {
                std::istringstream iss(msg);
                std::string line;
                while (std::getline(iss, line, '\n')) {
                    PROFILER_LOG_ERROR(LOG_CORE, "%s", line.c_str());
                }
            }
        }
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.memprofiler.start", "0");
    };
    int setTimerRet = HiviewDFX::XCollie::GetInstance().SetTimer("Hiprofiler_Timeout", TIME_OUT, callbackFunc, nullptr,
                                                                 HiviewDFX::XCOLLIE_FLAG_LOG);
    if (setTimerRet == HiviewDFX::INVALID_ID) {
        PROFILER_LOG_ERROR(LOG_CORE, "add hicollie failed for native daemon sa Stop");
        return RET_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop failed to write descriptor");
        HiviewDFX::XCollie::GetInstance().CancelTimer(setTimerRet);
        return RET_ERR;
    }
    WRITEUINT32(data, pid, RET_ERR);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_PID),
                                      data, reply, option);
    HiviewDFX::XCollie::GetInstance().CancelTimer(setTimerRet);
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t timeCost = static_cast<uint64_t>((end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec));
    PROFILER_LOG_INFO(LOG_CORE, "NativeMemoryProfilerSaProxy::Stop cost time: %llu",
                      static_cast<unsigned long long>(timeCost));
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop failed");
        return ret;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaProxy::Stop(const std::string& name)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop failed to write descriptor");
        return RET_ERR;
    }
    WRITESTRING(data, name, RET_ERR);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_NAME),
                                      data, reply, option);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Stop failed");
        return ret;
    }
    return RET_OK;
}

int32_t NativeMemoryProfilerSaProxy::Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
                                           std::string& replyStats)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NativeMemoryProfilerSaProxy::GetDescriptor())) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed to write descriptor");
        return RET_ERR;
    }
    CHECK_TRUE(config->Marshalling(data), RET_ERR, "NativeMemoryProfilerSaConfig marshalling failed");

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHECK_NOTNULL(remote, RET_ERR, "remote is nullptr");
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_SIMP_DATA),
                                      data, reply, option);
    if (ret != RET_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "Start failed");
        return ret;
    }
    READSTRING(reply, replyStats, RET_ERR);
    return RET_OK;
}
} // namespace OHOS::Developtools::NativeDaemon