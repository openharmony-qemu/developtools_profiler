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

#ifndef NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_COMMON_H
#define NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_COMMON_H

#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include "test_common.h"
#include "init_param.h"
#include "ipc_skeleton.h"
#include "native_memory_profiler_sa_callback_proxy.h"
#include "native_memory_profiler_sa_callback_stub.h"
#include "native_memory_profiler_sa_proxy.h"
#include "native_memory_profiler_sa_stub.h"
#include "native_memory_profiler_sa_service.h"
#undef private
#include "common.h"
#include "hook_common.h"
#include "parameters.h"
#include "socket_context.h"
#include "utilities.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "singleton.h"
inline const std::string TEST_PROC_NAME = "hiview";
inline const std::string TEST_NATIVE_NAME = "foundation";
inline const std::string NATIVE_PARAM = "hiviewdfx.hiprofiler.native_memoryd.start";
inline constexpr uint32_t TEST_DURATION = 10;
inline constexpr uint32_t TEST_SHARE_MEMORY_SIZE = 4096;
inline constexpr uint32_t START_TYPE_API = 1;
inline constexpr uint32_t TEST_START_TYPE_HIVIEW = 0;
inline constexpr uint32_t TEST_START_TYPE_API = 1;
inline OHOS::Security::AccessToken::AccessTokenID g_selfTokenId;
inline TEST_COMMON::MockNativeToken* g_mock = nullptr;

inline std::shared_ptr<OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaConfig> MakeSaConfig()
{
    auto config = std::make_shared<OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaConfig>();
    if (config == nullptr) {
        return nullptr;
    }
    config->processName_ = "sa_coverage_test";
    config->filePath_ = "/data/local/tmp/sa_coverage_test.htrace";
    config->duration_ = TEST_DURATION;
    config->shareMemorySize_ = TEST_SHARE_MEMORY_SIZE;
    config->typeFileName_ = "test";
    return config;
}

inline std::string MakeTracePath(const std::string& tag)
{
    return "/data/local/tmp/" + tag + "_" + std::to_string(getpid()) + ".htrace";
}

inline std::string GetProtocolProcessName()
{
    std::string processName = COMMON::GetBundleNameByPid(getpid());
    constexpr size_t prefixLen = 2; // length of "./" prefix
    if (processName.substr(0, prefixLen) == "./") {
        processName = processName.substr(prefixLen);
    }
    return processName;
}

class NativeMemoryProfilerSaServiceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        g_selfTokenId = GetSelfTokenID();
        TEST_COMMON::SetTestEvironment(g_selfTokenId);
        g_mock = new (std::nothrow) TEST_COMMON::MockNativeToken(TEST_PROC_NAME);
    }

    static void TearDownTestCase()
    {
        if (g_mock != nullptr) {
            delete g_mock;
            g_mock = nullptr;
        }
        SetSelfTokenID(g_selfTokenId);
        TEST_COMMON::ResetTestEvironment();
        SystemSetParameter(NATIVE_PARAM.c_str(), "0");
    }
    void SetUp() {}
    void TearDown() {}
};

namespace OHOS::Developtools::NativeDaemon {
class TestNativeMemoryProfilerSaStub : public NativeMemoryProfilerSaStub {
public:
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return RET_OK;
    }
    int32_t StartPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
        const sptr<IRemoteObject>& callback) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        callbackIsNull_ = callback == nullptr;
        return RET_OK;
    }
    int32_t Stop(uint32_t pid) override
    {
        lastPid_ = static_cast<int32_t>(pid);
        return RET_OK;
    }
    int32_t Stop(const std::string& name) override
    {
        lastName_ = name;
        return RET_OK;
    }
    int32_t Stop() override
    {
        stoppedVoid_ = true;
        return RET_OK;
    }
    int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override
    {
        lastFd_ = fd;
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return RET_OK;
    }
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, std::string& replyStats) override
    {
        return RET_ERR;
    }
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, MessageParcel &reply) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return RET_OK;
    }
    int32_t StopSync() override
    {
        return RET_OK;
    }
    int32_t DumpDataAsync(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
        const sptr<IRemoteObject>& callback) override
    {
        lastFd_ = fd;
        lastPid_ = config == nullptr ? -1 : config->pid_;
        callbackIsNull_ = callback == nullptr;
        return RET_OK;
    }
    virtual int32_t Update(std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override
    {
        return RET_OK;
    }

    int32_t lastPid_ = -1;
    uint32_t lastFd_ = 0;
    std::string lastName_;
    bool callbackIsNull_ = false;
    bool stoppedVoid_ = false;
};

class FailingNativeMemoryProfilerSaStub : public TestNativeMemoryProfilerSaStub {
public:
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return RET_ERR;
    }
    int32_t StartPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
        const sptr<IRemoteObject>& callback) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        callbackIsNull_ = callback == nullptr;
        return RET_ERR;
    }
    int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override
    {
        lastFd_ = fd;
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return RET_ERR;
    }
    int32_t Stop(uint32_t pid) override
    {
        lastPid_ = static_cast<int32_t>(pid);
        return RET_ERR;
    }
    int32_t Stop(const std::string& name) override
    {
        lastName_ = name;
        return RET_ERR;
    }
    int32_t Stop() override
    {
        stoppedVoid_ = true;
        return RET_ERR;
    }
};

class ReplyNativeMemoryProfilerSaStub : public TestNativeMemoryProfilerSaStub {
public:
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, MessageParcel &reply) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return reply.WriteString("reply_stats_ok") ? RET_OK : RET_ERR;
    }
};

class TestNativeMemoryProfilerSaCallbackStub : public NativeMemoryProfilerSaCallbackStub {
public:
    void OnProfilingResult(int32_t pid, int32_t errorCode, const std::string& filePath) override
    {
        pid_ = pid;
        errorCode_ = errorCode;
        filePath_ = filePath;
    }

    int32_t pid_ = 0;
    int32_t errorCode_ = 0;
    std::string filePath_;
};

class TestSocketContext : public SocketContext {
public:
    ~TestSocketContext() override
    {
        socketHandle_ = -1;
    }

    void SetSocketHandleForTest(int socketHandle)
    {
        socketHandle_ = socketHandle;
    }
};

class ScopedSocketPair {
public:
    ScopedSocketPair()
    {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds_) != 0) {
            fds_[0] = -1;
            fds_[1] = -1;
        }
    }

    ~ScopedSocketPair()
    {
        CloseFd(fds_[0]);
        CloseFd(fds_[1]);
    }

    bool IsValid() const
    {
        return fds_[0] >= 0 && fds_[1] >= 0;
    }

    int First() const
    {
        return fds_[0];
    }

    int Second() const
    {
        return fds_[1];
    }

private:
    static void CloseFd(int fd)
    {
        if (fd >= 0 && fcntl(fd, F_GETFD) != -1) {
            close(fd);
        }
    }

    int fds_[2] {-1, -1};
};

} // namespace OHOS::Developtools::NativeDaemon

inline bool WriteStubConfig(OHOS::MessageParcel& data, int32_t pid)
{
    auto config = MakeSaConfig();
    if (config == nullptr) {
        return false;
    }
    config->pid_ = pid;
    return config->Marshalling(data);
}

#endif // NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_COMMON_H
