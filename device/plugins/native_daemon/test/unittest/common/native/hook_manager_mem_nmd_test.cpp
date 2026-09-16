/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include <gtest/gtest.h>
#include <cstring>
#include <functional>
#include <unistd.h>
#include <sys/stat.h>
#include "hook_manager.h"
#include "native_memory_profiler_sa_config.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;
namespace {
const std::string SIMP_NMD_TEMP_FILE_DIR = "/data/log/hiprofiler/";
constexpr unsigned SIMP_NMD_TEMP_FILE_DIR_MODE = 0755;

class HookManagerMemNmdTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        if (access(SIMP_NMD_TEMP_FILE_DIR.c_str(), F_OK) != 0) {
            mkdir(SIMP_NMD_TEMP_FILE_DIR.c_str(), SIMP_NMD_TEMP_FILE_DIR_MODE);
        }
    }

    static void TearDownTestCase() {}

    void SetUp() override
    {
        hookManager_ = std::make_shared<HookManager>();
        ASSERT_TRUE(hookManager_ != nullptr);
    }

    void TearDown() override
    {
        hookManager_->CloseResource();
        hookManager_ = nullptr;
    }

protected:
    std::shared_ptr<HookManager> hookManager_ = nullptr;
};

/*
 * @tc.name: SetSimplifiedMemConfig_Basic
 * @tc.desc: test SetSimplifiedMemConfig sets maxGrowthSize_ from config.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, SetSimplifiedMemConfig_Basic, TestSize.Level0)
{
    auto config = std::make_shared<SimplifiedMemConfig>();
    config->maxGrowthSize = 4096;
    hookManager_->SetSimplifiedMemConfig(config);
    auto result = std::make_shared<SimplifiedMemConfig>();
    hookManager_->GetSimplifiedMemConfig(result);
    EXPECT_EQ(result->maxGrowthSize, 4096u);
}

/*
 * @tc.name: SetSimplifiedMemConfig_Nullptr
 * @tc.desc: test SetSimplifiedMemConfig with nullptr does nothing.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, SetSimplifiedMemConfig_Nullptr, TestSize.Level0)
{
    auto initConfig = std::make_shared<SimplifiedMemConfig>();
    initConfig->maxGrowthSize = 1024;
    hookManager_->SetSimplifiedMemConfig(initConfig);
    hookManager_->SetSimplifiedMemConfig(nullptr);
    auto result = std::make_shared<SimplifiedMemConfig>();
    hookManager_->GetSimplifiedMemConfig(result);
    EXPECT_EQ(result->maxGrowthSize, 1024u);
}

/*
 * @tc.name: GetSimplifiedMemConfig_Basic
 * @tc.desc: test GetSimplifiedMemConfig reads internal state into config.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, GetSimplifiedMemConfig_Basic, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->largestSize_ = 100;
    saConfig->secondLargestSize_ = 50;
    saConfig->maxGrowthSize_ = 200;
    hookManager_->SetHookConfig(saConfig);
    NativeHookConfig hookConfig;
    hookConfig.set_sample_interval(8192);
    hookManager_->SetHookConfig(hookConfig);

    auto config = std::make_shared<SimplifiedMemConfig>();
    hookManager_->GetSimplifiedMemConfig(config);
    EXPECT_EQ(config->largestSize, 100u);
    EXPECT_EQ(config->secondLargestSize, 50u);
    EXPECT_EQ(config->maxGrowthSize, 200u);
    EXPECT_EQ(config->sampleSize, 8192u);
}

/*
 * @tc.name: GetSimplifiedMemConfig_Nullptr
 * @tc.desc: test GetSimplifiedMemConfig with nullptr does nothing.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, GetSimplifiedMemConfig_Nullptr, TestSize.Level0)
{
    std::shared_ptr<SimplifiedMemConfig> config = nullptr;
    hookManager_->GetSimplifiedMemConfig(config);
    EXPECT_EQ(config, nullptr);
}

/*
 * @tc.name: SimplifiedMemConfig_RoundTrip
 * @tc.desc: test Set then Get of SimplifiedMemConfig round-trips maxGrowthSize.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, SimplifiedMemConfig_RoundTrip, TestSize.Level0)
{
    auto setConfig = std::make_shared<SimplifiedMemConfig>();
    setConfig->maxGrowthSize = 7777;
    hookManager_->SetSimplifiedMemConfig(setConfig);

    auto getConfig = std::make_shared<SimplifiedMemConfig>();
    hookManager_->GetSimplifiedMemConfig(getConfig);
    EXPECT_EQ(getConfig->maxGrowthSize, 7777u);
}

/*
 * @tc.name: CreateInHookSimpNmdFd_Basic
 * @tc.desc: test CreateInHookSimpNmdFd returns a valid writable fd.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, CreateInHookSimpNmdFd_Basic, TestSize.Level0)
{
    int fd = hookManager_->CreateInHookSimpNmdFd();
    ASSERT_GE(fd, 0);
    const char* content = "hello_simp_nmd";
    EXPECT_EQ(write(fd, content, strlen(content)), static_cast<ssize_t>(strlen(content)));
    EXPECT_EQ(close(fd), 0);
}

/*
 * @tc.name: StartInHookSimpNmdDump_NullPostProcessor
 * @tc.desc: test StartInHookSimpNmdDump with null postProcessor returns false.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, StartInHookSimpNmdDump_NullPostProcessor, TestSize.Level0)
{
    std::function<bool(int)> nullProc = nullptr;
    EXPECT_FALSE(hookManager_->StartInHookSimpNmdDump(nullProc));
    EXPECT_FALSE(hookManager_->inHookSimpNmdStarted_.load());
}

/*
 * @tc.name: StartInHookSimpNmdDump_PostProcessorFails
 * @tc.desc: test StartInHookSimpNmdDump resets state when postProcessor fails.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, StartInHookSimpNmdDump_PostProcessorFails, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->StartInHookSimpNmdDump([](int) { return false; }));
    EXPECT_FALSE(hookManager_->inHookSimpNmdStarted_.load());
    EXPECT_TRUE(hookManager_->inHookSimpNmdSem_ == nullptr);
    EXPECT_EQ(hookManager_->inHookSimpNmdFd_, -1);
}

/*
 * @tc.name: StartInHookSimpNmdDump_AlreadyStarted
 * @tc.desc: test StartInHookSimpNmdDump returns false when dump already started.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, StartInHookSimpNmdDump_AlreadyStarted, TestSize.Level0)
{
    EXPECT_TRUE(hookManager_->StartInHookSimpNmdDump([](int) { return true; }));
    EXPECT_FALSE(hookManager_->StartInHookSimpNmdDump([](int) { return true; }));
    // cleanup: signal and drain result to reset state
    EXPECT_TRUE(hookManager_->HandleInHookSimpNmdDone());
    hookManager_->GetInHookSimpNmdResult(1000);
    EXPECT_FALSE(hookManager_->inHookSimpNmdStarted_.load());
}

/*
 * @tc.name: HandleInHookSimpNmdDone_NotStarted
 * @tc.desc: test HandleInHookSimpNmdDone returns false when not started.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, HandleInHookSimpNmdDone_NotStarted, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->HandleInHookSimpNmdDone());
}

/*
 * @tc.name: GetInHookSimpNmdResult_NotStarted
 * @tc.desc: test GetInHookSimpNmdResult returns empty when not started.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, GetInHookSimpNmdResult_NotStarted, TestSize.Level0)
{
    EXPECT_TRUE(hookManager_->GetInHookSimpNmdResult(100).empty());
}

/*
 * @tc.name: GetInHookSimpNmdResult_Timeout
 * @tc.desc: test GetInHookSimpNmdResult returns empty on timeout and resets state.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, GetInHookSimpNmdResult_Timeout, TestSize.Level0)
{
    EXPECT_TRUE(hookManager_->StartInHookSimpNmdDump([](int) { return true; }));
    std::string result = hookManager_->GetInHookSimpNmdResult(100);
    EXPECT_TRUE(result.empty());
    EXPECT_FALSE(hookManager_->inHookSimpNmdStarted_.load());
    EXPECT_EQ(hookManager_->inHookSimpNmdFd_, -1);
}

/*
 * @tc.name: InHookSimpNmdDump_FullFlow
 * @tc.desc: test full in-hook simp nmd dump flow: start, signal, get result.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemNmdTest, InHookSimpNmdDump_FullFlow, TestSize.Level0)
{
    const std::string expected = "simp_nmd_dump_content";
    EXPECT_TRUE(hookManager_->StartInHookSimpNmdDump([&expected](int fd) {
        EXPECT_GE(fd, 0);
        EXPECT_EQ(write(fd, expected.data(), expected.size()),
                  static_cast<ssize_t>(expected.size()));
        return true;
    }));
    EXPECT_TRUE(hookManager_->HandleInHookSimpNmdDone());
    std::string result = hookManager_->GetInHookSimpNmdResult(1000);
    EXPECT_EQ(result, expected);
    EXPECT_FALSE(hookManager_->inHookSimpNmdStarted_.load());
    EXPECT_EQ(hookManager_->inHookSimpNmdFd_, -1);
}
} // namespace
