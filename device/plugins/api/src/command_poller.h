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

#ifndef COMMAND_POLLER_H
#define COMMAND_POLLER_H

#include <memory>

class ManagerInterface;
class CreateSessionCmd;
class DestroySessionCmd;
class StartSessionCmd;
class StopSessionCmd;

#include "logging.h"
#include "plugin_service.ipc.h"

using ManagerInterfacePtr = STD_PTR(shared, ManagerInterface);

class CommandPoller final : public IPluginServiceClient {
public:
    explicit CommandPoller(const ManagerInterfacePtr& p);
    ~CommandPoller();

    bool OnCreateSessionCmd(const CreateSessionCmd& cmd, SocketContext& context) const;
    bool OnDestroySessionCmd(const DestroySessionCmd& cmd) const;
    bool OnStartSessionCmd(const StartSessionCmd& cmd, PluginResult& result) const;
    bool OnStopSessionCmd(const StopSessionCmd& cmd) const;
    bool OnReportBasicDataCmd(const RefreshSessionCmd& cmd) const;

    uint32_t GetRequestId();
    bool OnConnect();

protected:
    bool OnGetCommandResponse(SocketContext& context, ::GetCommandResponse& response) override;

private:
    uint32_t requestIdAutoIncrease_;
    std::weak_ptr<ManagerInterface> pluginManager_;
};

#endif // !COMMAND_POLLER_H