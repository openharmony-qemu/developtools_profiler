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

#ifndef HOOK_SOCKET_CLIENT
#define HOOK_SOCKET_CLIENT

#include "hook_common.h"
#include "service_base.h"
#include "stack_writer.h"

class UnixSocketClient;
class Sampling;

class HookSocketClient : public ServiceBase {
public:
    HookSocketClient(int pid, ClientConfig *config, Sampling *sampler,
                     std::atomic<Range>* targetedRange = nullptr, std::atomic<int>* memCount = nullptr,
                     void (*disableHookCallback)() = nullptr);
    ~HookSocketClient();
    bool Connect(const std::string addrname);
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    std::vector<int> GetSmbFds()
    {
        return smbFds_;
    }
    std::vector<int>  GetEventFds()
    {
        return eventFds_;
    }
    void Flush();
    bool SendStack(const void* data, size_t size);
    bool SendStackWithPayload(const void* data, size_t size, const void* payload,
            size_t payloadSize, int smbIndex = 0);
    void DisableHook();
    bool PeerIsConnected();
    bool SendNmdInfo();
    bool SendSimplifiedNmdInfo();
    bool SendEndMsg();
    int GetNmdType()
    {
        return nmdType_;
    }

private:
    std::shared_ptr<UnixSocketClient> unixSocketClient_;
    std::vector<int> smbFds_;
    std::vector<int> eventFds_;
    int pid_;
    int nmdType_ = -1;
    ClientConfig *config_ = nullptr;
    Sampling *sampler_ = nullptr;
    std::atomic<Range>* targetedRange_ = nullptr;
    std::atomic<int>* sharedMemCount_ = nullptr;
    std::vector<std::shared_ptr<StackWriter>> stackWriterList_;
    void (*disableHookCallback_)(){nullptr};
    uint32_t largestSize_ = 0;
    uint32_t secondLargestSize_ = 0;
    uint32_t maxGrowthSize_ = 0;
    uint32_t sampleInterval_ = 0;
};

#endif // HOOK_SOCKET_CLIENT