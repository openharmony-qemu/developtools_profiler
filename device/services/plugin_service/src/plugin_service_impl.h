/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef PLUGIN_SERVICE_IMPL_H
#define PLUGIN_SERVICE_IMPL_H

#include "logging.h"
#include "plugin_service.ipc.h"
#include "socket_context.h"
class PluginService;

using GetCommandResponsePtr = STD_PTR(shared, ::GetCommandResponse);

class PluginServiceImpl final : public IPluginServiceServer {
public:
    explicit PluginServiceImpl(PluginService& p);
    ~PluginServiceImpl();
    bool RegisterPlugin(SocketContext& context, ::RegisterPluginRequest& request,
                        ::RegisterPluginResponse& response) override;
    bool UnregisterPlugin(SocketContext& context, ::UnregisterPluginRequest& request,
                          ::UnregisterPluginResponse& response) override;
    bool GetCommand(SocketContext& context, ::GetCommandRequest& request, ::GetCommandResponse& response) override;
    bool NotifyResult(SocketContext& context, ::NotifyResultRequest& request,
                      ::NotifyResultResponse& response) override;
    bool PushCommand(SocketContext& context, GetCommandResponsePtr command);
    bool SendHeartBeat(SocketContext& context);

private:
    PluginService* pluginService;
};

#endif // PLUGIN_SERVICE_IMPL_H
