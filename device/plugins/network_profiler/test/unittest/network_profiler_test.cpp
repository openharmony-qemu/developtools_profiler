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
constexpr uint32_t SMB1_SIZE = 10 * 4096;
const std::string WRITER_NAME = "NetworkProfilerWriterTest";
int g_smbFd1 = 0;

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
    SystemSetParameter("hiviewdfx.hiprofiler.plugins.start", "1");
    auto networkProfiler = NetworkProfiler::GetInstance();
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    sleep(2);
    auto ret = client.SendNetworkProfilerData(nullptr, 0, nullptr, 0);
    ASSERT_FALSE(ret);
    NetworkConfig config;
    SocketContext ctx;
    ret = client.ProtocolProc(ctx, 0, reinterpret_cast<int8_t*>(&config), sizeof(config));
    ASSERT_TRUE(ret);
    SystemSetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
}

/**
 * @tc.name: plugin
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerWriterTest, TestSize.Level1)
{
    auto write = std::make_shared<NetworkProfilerWriter>(WRITER_NAME, SMB1_SIZE, g_smbFd1, -1, false);
    uint8_t data[] = {0x55, 0xAA, 0x55, 0xAA};

    auto myCallback = []() -> bool {
        return true;
    };
    auto ret = write->WriteTimeout(static_cast<void*>(data), sizeof(data), myCallback);
    EXPECT_TRUE(ret == 0);

    uint8_t payload[] = {0x11, 0x22, 0x33, 0x44};
    ret = write->WriteWithPayloadTimeout(static_cast<void*>(data), sizeof(data),
                                        static_cast<void*>(payload), sizeof(payload), myCallback);
    EXPECT_TRUE(ret == 0);
}
} // namespace
