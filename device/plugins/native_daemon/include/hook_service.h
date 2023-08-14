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
class HookService : public ServiceBase {
public:
    HookService(int smbFd, int eventFd, int pid, const std::string& processName, const ClientConfig& clientConfig);
    ~HookService();
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    inline int GetPid()
    {
        return pid_;
    }
private:
    bool StartService(const std::string& unixSocketName);

    std::shared_ptr<ServiceEntry> serviceEntry_;
    int smbFd_;
    int eventFd_;
    int pid_;
    std::string processName_;
    ClientConfig clientConfig_;
};
}
#endif // HOOK_SERVICE_H