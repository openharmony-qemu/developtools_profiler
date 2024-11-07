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

#include <dlfcn.h>
#include <iostream>
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <memory>

#include "init_param.h"
#include "network_profiler_socker_client.h"
#include "network_profiler.h"
#include "network_profiler_manager.h"
#include "grpc/impl/codegen/log.h"
#include "plugin_service.h"
#include "plugin_service.ipc.h"
#include "socket_context.h"
#include "network_profiler_config.pb.h"

using namespace testing::ext;
using namespace OHOS::Developtools::Profiler;

namespace {
const std::string PARAM_KAY = "hiviewdfx.hiprofiler.networkprofiler.target";
class NetworkProfilerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
    };

    static void TearDownTestCase()
    {
    }
    void SetUp() {}
    void TearDown() {}
};

void CallBackFunc()
{
    return;
}

/**
 * @tc.name: network profiler test
 * @tc.desc: Test NetworkProfilerSocketClient IsProfilerEnable without SystemSetParameter
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerTest001, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    auto networkProfilerService = std::make_unique<NetworkProfilerSocketService>(networkProfilerMgr);
    ASSERT_TRUE(networkProfilerService != nullptr);
    networkProfilerMgr->Init();
    auto networkProfiler = NetworkProfiler::GetInstance();
    auto ret = networkProfiler->IsProfilerEnable();
    ASSERT_FALSE(ret);
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    client.SendNetworkProfilerData(nullptr, 0, nullptr, 0);
}

/**
 * @tc.name: network profiler test
 * @tc.desc: Test NetworkProfilerSocketClient send empty data to server
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerTest002, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    auto networkProfilerService = std::make_unique<NetworkProfilerSocketService>(networkProfilerMgr);
    ASSERT_TRUE(networkProfilerService != nullptr);
    networkProfilerMgr->Init();
    SystemSetParameter(PARAM_KAY.c_str(), std::to_string(getpid()).c_str());
    auto networkProfiler = NetworkProfiler::GetInstance();
    auto ret = networkProfiler->IsProfilerEnable();
    ASSERT_TRUE(ret);
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    sleep(2);
    ret = client.SendNetworkProfilerData(nullptr, 0, nullptr, 0);
    ASSERT_FALSE(ret);
    SocketContext ctx;
    ret = client.ProtocolProc(ctx, 0, nullptr, 0);
    ASSERT_TRUE(ret);
}
} // namespace
