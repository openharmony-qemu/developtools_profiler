/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "hook_manager_exception_test.h"
#include <sys/stat.h>
#include <unistd.h>
#include "command_poller.h"
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
void HookManagerExceptionTest::SetUpTestCase(void) {}
void HookManagerExceptionTest::TearDownTestCase(void) {}

void HookManagerExceptionTest::SetUp()
{
    hookManager_ = std::make_shared<HookManager>();
    hookConfig_.Clear();
}

void HookManagerExceptionTest::TearDown()
{
    hookManager_ = nullptr;
}

/*
 * @tc.name: CheckProcessWithNonExistPid
 * @tc.desc: test CheckProcess with non-existent PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, CheckProcessWithNonExistPid, TestSize.Level0)
{
    hookConfig_.set_pid(999999);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_FALSE(hookManager_->CheckProcess());
}

/*
 * @tc.name: CheckProcessWithZeroPid
 * @tc.desc: test CheckProcess with zero PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, CheckProcessWithZeroPid, TestSize.Level0)
{
    hookConfig_.set_pid(0);
    hookManager_->SetHookConfig(hookConfig_);
    bool result = hookManager_->CheckProcess();
    EXPECT_TRUE(hookManager_->hookCtx_.empty() || result);
}

/*
 * @tc.name: CheckProcessWithNegativePid
 * @tc.desc: test CheckProcess with negative PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, CheckProcessWithNegativePid, TestSize.Level0)
{
    hookConfig_.set_pid(-1);
    hookManager_->SetHookConfig(hookConfig_);
    bool result = hookManager_->CheckProcess();
    EXPECT_TRUE(hookManager_->hookCtx_.empty() || result);
}

/*
 * @tc.name: CheckProcessWithEmptyProcessName
 * @tc.desc: test CheckProcess with empty process name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, CheckProcessWithEmptyProcessName, TestSize.Level0)
{
    hookConfig_.set_process_name("");
    hookManager_->SetHookConfig(hookConfig_);
    bool result = hookManager_->CheckProcess();
    EXPECT_TRUE(hookManager_->hookCtx_.empty() || result);
}

/*
 * @tc.name: CheckProcessWithNonExistProcessName
 * @tc.desc: test CheckProcess with non-existent process name which will start later.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, CheckProcessWithNonExistProcessName, TestSize.Level0)
{
    hookConfig_.set_process_name("non_exist_process_12345");
    hookConfig_.set_startup_mode(false);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->CheckProcessName());
}

/*
 * @tc.name: CheckProcessExceedMaxPidCount
 * @tc.desc: test CheckProcess when PID count exceeds maximum in response mode.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, CheckProcessExceedMaxPidCount, TestSize.Level0)
{
    hookConfig_.set_response_library_mode(true);
    for (int i = 0; i < 10; ++i) {
        hookConfig_.add_expand_pids(getpid());
    }
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->CheckProcess());
}

/*
 * @tc.name: CreatePluginSessionWithEmptyConfig
 * @tc.desc: test CreatePluginSession with empty config vector.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, CreatePluginSessionWithEmptyConfig, TestSize.Level0)
{
    std::vector<ProfilerPluginConfig> emptyConfig;
    EXPECT_FALSE(hookManager_->CreatePluginSession(emptyConfig));
}

/*
 * @tc.name: CreatePluginSessionWithInvalidConfigData
 * @tc.desc: test CreatePluginSession with invalid config data.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, CreatePluginSessionWithInvalidConfigData, TestSize.Level0)
{
    ProfilerPluginConfig config;
    config.set_name("nativehook");
    config.set_config_data("invalid_protobuf_data");
    std::vector<ProfilerPluginConfig> configVec;
    configVec.push_back(config);
    EXPECT_FALSE(hookManager_->CreatePluginSession(configVec));
}

/*
 * @tc.name: StartPluginSessionWithEmptyContext
 * @tc.desc: test StartPluginSession when hookCtx_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, StartPluginSessionWithEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {1};
    std::vector<ProfilerPluginConfig> config;
    PluginResult result;
    EXPECT_FALSE(hookManager_->StartPluginSession(pluginIds, config, result));
}

/*
 * @tc.name: StopPluginSessionWithEmptyContext
 * @tc.desc: test StopPluginSession when hookCtx_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, StopPluginSessionWithEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {1};
    EXPECT_FALSE(hookManager_->StopPluginSession(pluginIds));
}

/*
 * @tc.name: HandleHookContextWithNullContext
 * @tc.desc: test HandleHookContext with nullptr context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, HandleHookContextWithNullContext, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->HandleHookContext(nullptr));
}

/*
 * @tc.name: HandleHookContextWithInvalidPidAndName
 * @tc.desc: test HandleHookContext with invalid pid and empty process name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, HandleHookContextWithInvalidPidAndName, TestSize.Level0)
{
    auto ctx = std::make_shared<HookManager::HookManagerCtx>(-1);
    ctx->processName = "";
    hookConfig_.set_smb_pages(16384);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_FALSE(hookManager_->HandleHookContext(ctx));
}

/*
 * @tc.name: DestroyPluginSessionWithEmptyContext
 * @tc.desc: test DestroyPluginSession when hookCtx_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, DestroyPluginSessionWithEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {1};
    EXPECT_TRUE(hookManager_->DestroyPluginSession(pluginIds));
}

/*
 * @tc.name: ReportPluginBasicDataWithEmptyContext
 * @tc.desc: test ReportPluginBasicData when hookCtx_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, ReportPluginBasicDataWithEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {};
    EXPECT_TRUE(hookManager_->ReportPluginBasicData(pluginIds));
}

/*
 * @tc.name: SetCommandPollerWithNullPoller
 * @tc.desc: test SetCommandPoller with null pointer.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, SetCommandPollerWithNullPoller, TestSize.Level0)
{
    hookManager_->SetCommandPoller(nullptr);
    EXPECT_EQ(hookManager_->commandPoller_, nullptr);
}

/*
 * @tc.name: RegisterAgentPluginWithEmptyName
 * @tc.desc: test RegisterAgentPlugin with empty plugin name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, RegisterAgentPluginWithEmptyName, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->RegisterAgentPlugin(""));
}

/*
 * @tc.name: UnregisterAgentPluginWithEmptyName
 * @tc.desc: test UnregisterAgentPlugin with empty plugin name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExceptionTest, UnregisterAgentPluginWithEmptyName, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->UnregisterAgentPlugin(""));
}
} // namespace OHOS::Developtools::NativeDaemon
