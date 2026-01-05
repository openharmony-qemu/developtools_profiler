/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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
#ifndef NETWORK_PROFILER_MANAGER_MOCK_H
#define NETWORK_PROFILER_MANAGER_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#define private public
#include "network_profiler_manager.h"
#include "command_poller.h"
#undef private

namespace OHOS::Developtools::Profiler {
class NetworkProfilerManagerMock : public NetworkProfilerManager {
public:
    NetworkProfilerManagerMock() : NetworkProfilerManager() {}
    ~NetworkProfilerManagerMock() = default;

    MOCK_METHOD0(CheckConfig, bool());
    MOCK_METHOD1(HandleNetworkProfilerContext, bool(const std::shared_ptr<NetworkProfilerCtx>& ctx));
};

class CommandPollerMock : public CommandPoller {
public:
    explicit CommandPollerMock(const ManagerInterfacePtr& p) : CommandPoller(p) {}
    ~CommandPollerMock() = default;

    MOCK_METHOD2(PushResult, void(const ProfilerPluginState& pluginState, uint32_t pluginId));
};
}
#endif // NETWORK_PROFILER_MANAGER_MOCK_H
