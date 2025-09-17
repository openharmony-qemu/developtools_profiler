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

#include <gtest/gtest.h>
#include "hook_service.h"
#include "hook_manager.h"
#include "hook_socket_client.h"
#include "socket_context.h"
#include "share_memory_allocator.h"
#include "event_notifier.h"
#include "command_poller.h"
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
class HookServiceTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.name: ProtocolProc
 * @tc.desc: test HookService::ProtocolProc with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceTest, ProtocolProc, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);

    ProfilerPluginConfig config;
    config.set_name("nativehook");
    config.set_plugin_sha256("");
    config.set_sample_interval(20);

    NativeHookConfig hookConfig;
    hookConfig.set_pid(1);
    hookConfig.set_smb_pages(16384);
    hookConfig.set_startup_mode(true);
    std::vector<uint8_t> buffer(hookConfig.ByteSizeLong());
    hookConfig.SerializeToArray(buffer.data(), hookConfig.ByteSizeLong());
    config.set_config_data(buffer.data(), buffer.size());

    PluginResult result;
    std::vector<ProfilerPluginConfig> configVec;
    configVec.push_back(config);
    EXPECT_TRUE(hookManager->CreatePluginSession(configVec));

    SocketContext socketContext;
    int pid = 1;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    ClientConfig clientConfig;
    auto hookService = std::make_shared<HookService>(clientConfig, hookManager);
    ASSERT_TRUE(hookService != nullptr);
    ASSERT_TRUE(hookService->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}
} // namespace
