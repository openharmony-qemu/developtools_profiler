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

#ifndef FFRT_PROFILER_SOCKET_CLIENT
#define FFRT_PROFILER_SOCKET_CLIENT

#include "service_base.h"
#include "ffrt_profiler_write.h"
#include "unix_socket_client.h"

namespace OHOS::Developtools::Profiler {
class FfrtProfiler;
class FfrtProfilerSocketClient : public ServiceBase {
public:
    FfrtProfilerSocketClient(int pid, FfrtProfiler* profiler, void (*disableHookCallback)());
    ~FfrtProfilerSocketClient();
    bool Connect(const std::string addrname);
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    int GetSmbFd()
    {
        return smbFd_;
    }
    int GetEventFd()
    {
        return eventFd_;
    }
    void Flush();
    bool SendFfrtProfilerData(const void* data, size_t size, const void* payload, size_t payloadSize);
    void DisableHook();
    bool PeerIsConnected();
    bool ClientConnectState()
    {
        return unixSocketClient_ == nullptr ? false : true;
    }

private:
    std::shared_ptr<UnixSocketClient> unixSocketClient_{nullptr};
    int smbFd_{0};
    int eventFd_{0};
    int pid_{0};
    std::shared_ptr<FfrtProfilerWriter> writer_{nullptr};
    void (*disableHookCallback_)(){nullptr};
    uint32_t flushInterval_{0};
    FfrtProfiler* profiler_{nullptr};
    bool block_{false};
};
} // namespace OHOS::Developtools::Profiler

#endif // FFRT_PROFILER_SOCKET_CLIENT