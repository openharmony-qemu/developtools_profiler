/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include <cstring>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include "hook_manager.h"
#include "command_poller.h"
#include "native_memory_profiler_sa_config.h"
#include "parameters.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
const int ASYNC_TEST_PID = 999999;

class AsyncStackConfigTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
        sleep(1);
    }

    static void TearDownTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
    }

    void SetUp() override
    {
        hookManager_ = std::make_shared<HookManager>();
        ASSERT_TRUE(hookManager_ != nullptr);

        commandPoller_ = std::make_shared<CommandPoller>(hookManager_);
        ASSERT_TRUE(commandPoller_ != nullptr);
        ASSERT_TRUE(commandPoller_->OnConnect());

        hookManager_->SetCommandPoller(commandPoller_);
    }

    void TearDown() override
    {
        hookManager_ = nullptr;
        commandPoller_ = nullptr;
    }

protected:
    std::shared_ptr<HookManager> hookManager_;
    std::shared_ptr<CommandPoller> commandPoller_;
};

/*
* @tc.name: SetHookConfigAsyncStackDepth
* @tc.desc: test SetHookConfig with async nesting depth and task stack depth from SaConfig.
* @tc.type: FUNC
*/
HWTEST_F(AsyncStackConfigTest, SetHookConfigAsyncStackDepth, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = ASYNC_TEST_PID;
    saConfig->asyncNestingDepth_ = 8;
    saConfig->asyncTaskStackDepth_ = 16;

    hookManager_->SetHookConfig(saConfig);

    const auto& config = hookManager_->GetHookConfig();
    EXPECT_EQ(config.async_nesting_depth(), 8);
    EXPECT_EQ(config.async_task_stack_depth(), 16);
}

/*
* @tc.name: GetClientConfigAsyncStackDepth
* @tc.desc: test GetClientConfig includes async nesting depth and task stack depth.
* @tc.type: FUNC
*/
HWTEST_F(AsyncStackConfigTest, GetClientConfigAsyncStackDepth, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = ASYNC_TEST_PID;
    saConfig->asyncNestingDepth_ = 10;
    saConfig->asyncTaskStackDepth_ = 20;

    hookManager_->SetHookConfig(saConfig);

    ClientConfig clientConfig;
    hookManager_->GetClientConfig(clientConfig);

    EXPECT_EQ(clientConfig.asyncNestingDepth, 10);
    EXPECT_EQ(clientConfig.asyncTaskStackDepth, 20);
}

/*
* @tc.name: SetHookConfigAsyncStackDepthBoundary
* @tc.desc: test SetHookConfig handles boundary values for async nesting depth and task stack depth.
* @tc.type: FUNC
*/
HWTEST_F(AsyncStackConfigTest, SetHookConfigAsyncStackDepthBoundary, TestSize.Level0)
{
    // Test minimum boundary
    auto saConfigMin = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfigMin->pid_ = ASYNC_TEST_PID;
    saConfigMin->asyncNestingDepth_ = 1;
    saConfigMin->asyncTaskStackDepth_ = 1;

    hookManager_->SetHookConfig(saConfigMin);
    const auto& configMin = hookManager_->GetHookConfig();
    EXPECT_EQ(configMin.async_nesting_depth(), 1u);
    EXPECT_EQ(configMin.async_task_stack_depth(), 1u);

    // Test maximum boundary
    auto saConfigMax = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfigMax->pid_ = ASYNC_TEST_PID;
    saConfigMax->asyncNestingDepth_ = 16;
    saConfigMax->asyncTaskStackDepth_ = 256;

    hookManager_->SetHookConfig(saConfigMax);
    const auto& configMax = hookManager_->GetHookConfig();
    EXPECT_EQ(configMax.async_nesting_depth(), 16u);
    EXPECT_EQ(configMax.async_task_stack_depth(), 256u);
}
} // namespace
