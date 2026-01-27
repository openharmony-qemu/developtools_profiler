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

#include "hook_boundary_test.h"
#include <climits>
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
constexpr uint32_t MAX_MATCH_INTERVAL = 3600;
constexpr uint32_t MAX_MATCH_CNT = 1000;

void HookBoundaryTest::SetUpTestCase(void) {}
void HookBoundaryTest::TearDownTestCase(void) {}

void HookBoundaryTest::SetUp()
{
    hookManager_ = std::make_shared<HookManager>();
    hookConfig_.Clear();
}

void HookBoundaryTest::TearDown()
{
    hookManager_ = nullptr;
}

/*
 * @tc.name: FilterSizeZeroBoundary
 * @tc.desc: test filter_size = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, FilterSizeZeroBoundary, TestSize.Level0)
{
    hookConfig_.set_filter_size(0);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.filter_size(), 0);
}

/*
 * @tc.name: FilterSizeMaxBoundary
 * @tc.desc: test filter_size = USHRT_MAX boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, FilterSizeMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_filter_size(USHRT_MAX);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.filter_size(), USHRT_MAX);
}

/*
 * @tc.name: SmbPagesZeroBoundary
 * @tc.desc: test smb_pages = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, SmbPagesZeroBoundary, TestSize.Level0)
{
    hookConfig_.set_smb_pages(0);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.smb_pages(), 0);
}

/*
 * @tc.name: SmbPagesMinBoundary
 * @tc.desc: test smb_pages = 1 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, SmbPagesMinBoundary, TestSize.Level0)
{
    hookConfig_.set_smb_pages(1);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.smb_pages(), 1);
}

/*
 * @tc.name: MaxStackDepthZeroBoundary
 * @tc.desc: test max_stack_depth = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, MaxStackDepthZeroBoundary, TestSize.Level0)
{
    hookConfig_.set_max_stack_depth(0);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), 0);
}

/*
 * @tc.name: MaxStackDepthMinBoundary
 * @tc.desc: test max_stack_depth = 1 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, MaxStackDepthMinBoundary, TestSize.Level0)
{
    hookConfig_.set_max_stack_depth(1);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), 1);
}

/*
 * @tc.name: MaxJsStackDepthZeroBoundary
 * @tc.desc: test max_js_stack_depth = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, MaxJsStackDepthZeroBoundary, TestSize.Level0)
{
    hookConfig_.set_max_js_stack_depth(0);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_js_stack_depth(), 0);
}

/*
 * @tc.name: StatisticsIntervalZeroBoundary
 * @tc.desc: test statistics_interval = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, StatisticsIntervalZeroBoundary, TestSize.Level0)
{
    hookConfig_.set_statistics_interval(0);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.statistics_interval(), 0);
}

/*
 * @tc.name: StatisticsIntervalMinBoundary
 * @tc.desc: test statistics_interval = 1 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, StatisticsIntervalMinBoundary, TestSize.Level0)
{
    hookConfig_.set_statistics_interval(1);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.statistics_interval(), 1);
}

/*
 * @tc.name: SampleIntervalZeroBoundary
 * @tc.desc: test sample_interval = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, SampleIntervalZeroBoundary, TestSize.Level0)
{
    hookConfig_.set_sample_interval(0);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.sample_interval(), 0);
}

/*
 * @tc.name: MallocFreeMatchingIntervalMaxBoundary
 * @tc.desc: test malloc_free_matching_interval = MAX_MATCH_INTERVAL boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, MallocFreeMatchingIntervalMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_malloc_free_matching_interval(MAX_MATCH_INTERVAL);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.malloc_free_matching_interval(), MAX_MATCH_INTERVAL);
}

/*
 * @tc.name: MallocFreeMatchingCntMaxBoundary
 * @tc.desc: test malloc_free_matching_cnt = MAX_MATCH_CNT boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, MallocFreeMatchingCntMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_malloc_free_matching_cnt(MAX_MATCH_CNT);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.malloc_free_matching_cnt(), MAX_MATCH_CNT);
}

/*
 * @tc.name: PidZeroBoundary
 * @tc.desc: test pid = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, PidZeroBoundary, TestSize.Level0)
{
    hookConfig_.set_pid(0);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.pid(), 0);
}

/*
 * @tc.name: PidMaxBoundary
 * @tc.desc: test pid = INT_MAX boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, PidMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_pid(INT_MAX);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.pid(), INT_MAX);
}

/*
 * @tc.name: ExpandPidsEmptyBoundary
 * @tc.desc: test expand_pids empty boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, ExpandPidsEmptyBoundary, TestSize.Level0)
{
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.expand_pids_size(), 0);
}

/*
 * @tc.name: ExpandPidsSingleBoundary
 * @tc.desc: test expand_pids with single element.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, ExpandPidsSingleBoundary, TestSize.Level0)
{
    hookConfig_.add_expand_pids(12345);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.expand_pids_size(), 1);
    EXPECT_EQ(hookManager_->hookConfig_.expand_pids(0), 12345);
}

/*
 * @tc.name: ExpandPidsMultipleBoundary
 * @tc.desc: test expand_pids with multiple elements.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, ExpandPidsMultipleBoundary, TestSize.Level0)
{
    hookConfig_.add_expand_pids(1001);
    hookConfig_.add_expand_pids(1002);
    hookConfig_.add_expand_pids(1003);
    hookConfig_.add_expand_pids(1004);
    hookConfig_.add_expand_pids(1005);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.expand_pids_size(), 5);
}

/*
 * @tc.name: MaxJsStackDepthMinBoundary
 * @tc.desc: test max_js_stack_depth = 1 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, MaxJsStackDepthMinBoundary, TestSize.Level0)
{
    hookConfig_.set_max_js_stack_depth(1);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_js_stack_depth(), 1);
}

/*
 * @tc.name: MaxJsStackDepthMaxBoundary
 * @tc.desc: test max_js_stack_depth = 100 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, MaxJsStackDepthMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_max_js_stack_depth(100);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_js_stack_depth(), 100);
}

/*
 * @tc.name: SmbPagesMaxBoundary
 * @tc.desc: test smb_pages = 16384 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, SmbPagesMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_smb_pages(16384);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.smb_pages(), 16384);
}

/*
 * @tc.name: MaxStackDepthMaxBoundary
 * @tc.desc: test max_stack_depth = 1024 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, MaxStackDepthMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_max_stack_depth(1024);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), 1024);
}

/*
 * @tc.name: StatisticsIntervalMaxBoundary
 * @tc.desc: test statistics_interval = 3600 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, StatisticsIntervalMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_statistics_interval(3600);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.statistics_interval(), 3600);
}

/*
 * @tc.name: SampleIntervalMinBoundary
 * @tc.desc: test sample_interval = 1 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, SampleIntervalMinBoundary, TestSize.Level0)
{
    hookConfig_.set_sample_interval(1);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.sample_interval(), 1);
}

/*
 * @tc.name: SampleIntervalMaxBoundary
 * @tc.desc: test sample_interval = UINT32_MAX boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookBoundaryTest, SampleIntervalMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_sample_interval(UINT_MAX);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.sample_interval(), UINT_MAX);
}
} // namespace OHOS::Developtools::NativeDaemon
