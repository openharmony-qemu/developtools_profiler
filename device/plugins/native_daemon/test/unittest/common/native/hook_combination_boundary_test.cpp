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

#include "hook_combination_boundary_test.h"
#include <climits>
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
void HookCombinationBoundaryTest::SetUpTestCase(void) {}
void HookCombinationBoundaryTest::TearDownTestCase(void) {}

void HookCombinationBoundaryTest::SetUp()
{
    hookManager_ = std::make_shared<HookManager>();
    hookConfig_.Clear();
}

void HookCombinationBoundaryTest::TearDown()
{
    hookManager_ = nullptr;
}

/*
 * @tc.name: MallocDisableAndStatisticsInterval
 * @tc.desc: test malloc_disable + statistics_interval combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, MallocDisableAndStatisticsInterval, TestSize.Level0)
{
    hookConfig_.set_malloc_disable(true);
    hookConfig_.set_statistics_interval(10);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.malloc_disable());
    EXPECT_EQ(hookManager_->hookConfig_.statistics_interval(), 10);
}

/*
 * @tc.name: MmapDisableAndOfflineSymbolization
 * @tc.desc: test mmap_disable + offline_symbolization combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, MmapDisableAndOfflineSymbolization, TestSize.Level0)
{
    hookConfig_.set_mmap_disable(true);
    hookConfig_.set_offline_symbolization(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.mmap_disable());
    EXPECT_TRUE(hookManager_->hookConfig_.offline_symbolization());
}

/*
 * @tc.name: FpUnwindAndMaxStackDepth
 * @tc.desc: test fp_unwind + max_stack_depth combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, FpUnwindAndMaxStackDepth, TestSize.Level0)
{
    hookConfig_.set_fp_unwind(true);
    hookConfig_.set_max_stack_depth(128);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.fp_unwind());
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), 128);
}

/*
 * @tc.name: ResponseLibraryModeAndMultiProcess
 * @tc.desc: test response_library_mode + multiple expand_pids combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, ResponseLibraryModeAndMultiProcess, TestSize.Level0)
{
    hookConfig_.set_response_library_mode(true);
    hookConfig_.add_expand_pids(1001);
    hookConfig_.add_expand_pids(1002);
    hookConfig_.add_expand_pids(1003);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.response_library_mode());
    EXPECT_EQ(hookManager_->hookConfig_.expand_pids_size(), 3);
}

/*
 * @tc.name: CallframeCompressAndStringCompressed
 * @tc.desc: test callframe_compress + string_compressed combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, CallframeCompressAndStringCompressed, TestSize.Level0)
{
    hookConfig_.set_callframe_compress(true);
    hookConfig_.set_string_compressed(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.callframe_compress());
    EXPECT_TRUE(hookManager_->hookConfig_.string_compressed());
}

/*
 * @tc.name: AllFeaturesEnabled
 * @tc.desc: test all features enabled combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, AllFeaturesEnabled, TestSize.Level0)
{
    hookConfig_.set_save_file(true);
    hookConfig_.set_offline_symbolization(true);
    hookConfig_.set_fp_unwind(true);
    hookConfig_.set_callframe_compress(true);
    hookConfig_.set_string_compressed(true);
    hookConfig_.set_statistics_interval(10);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.save_file());
    EXPECT_TRUE(hookManager_->hookConfig_.offline_symbolization());
    EXPECT_TRUE(hookManager_->hookConfig_.fp_unwind());
}

/*
 * @tc.name: AllFeaturesDisabled
 * @tc.desc: test all features disabled combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, AllFeaturesDisabled, TestSize.Level0)
{
    hookConfig_.set_malloc_disable(false);
    hookConfig_.set_mmap_disable(false);
    hookConfig_.set_free_stack_report(false);
    hookConfig_.set_munmap_stack_report(false);
    hookConfig_.set_save_file(false);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_FALSE(hookManager_->hookConfig_.malloc_disable());
    EXPECT_FALSE(hookManager_->hookConfig_.mmap_disable());
    EXPECT_FALSE(hookManager_->hookConfig_.save_file());
}

/*
 * @tc.name: MaxConfigValuesCombination
 * @tc.desc: test all config items set to max values.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, MaxConfigValuesCombination, TestSize.Level0)
{
    hookConfig_.set_filter_size(USHRT_MAX);
    hookConfig_.set_max_stack_depth(1024);
    hookConfig_.set_statistics_interval(3600);
    hookConfig_.set_malloc_free_matching_interval(3600);
    hookConfig_.set_malloc_free_matching_cnt(1000);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.filter_size(), USHRT_MAX);
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), 1024);
}

/*
 * @tc.name: MinConfigValuesCombination
 * @tc.desc: test all config items set to min values.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, MinConfigValuesCombination, TestSize.Level0)
{
    hookConfig_.set_filter_size(0);
    hookConfig_.set_max_stack_depth(0);
    hookConfig_.set_statistics_interval(0);
    hookConfig_.set_sample_interval(0);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.filter_size(), 0);
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), 0);
    EXPECT_EQ(hookManager_->hookConfig_.statistics_interval(), 0);
}

/*
 * @tc.name: StartupModeAndProcessName
 * @tc.desc: test startup_mode + process_name combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, StartupModeAndProcessName, TestSize.Level0)
{
    hookConfig_.set_startup_mode(true);
    hookConfig_.set_process_name("com.example.app");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.startup_mode());
    EXPECT_EQ(hookManager_->hookConfig_.process_name(), "com.example.app");
}

/*
 * @tc.name: SaveFileAndFileName
 * @tc.desc: test save_file + file_name combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, SaveFileAndFileName, TestSize.Level0)
{
    hookConfig_.set_save_file(true);
    hookConfig_.set_file_name("/data/local/tmp/test.htrace");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.save_file());
    EXPECT_EQ(hookManager_->hookConfig_.file_name(), "/data/local/tmp/test.htrace");
}

/*
 * @tc.name: MemtraceEnableAndGpuConfig
 * @tc.desc: test memtrace_enable combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, MemtraceEnableAndGpuConfig, TestSize.Level0)
{
    hookConfig_.set_memtrace_enable(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.memtrace_enable());
}

/*
 * @tc.name: FreeStackReportAndMunmapStackReport
 * @tc.desc: test free_stack_report + munmap_stack_report combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, FreeStackReportAndMunmapStackReport, TestSize.Level0)
{
    hookConfig_.set_free_stack_report(true);
    hookConfig_.set_munmap_stack_report(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.free_stack_report());
    EXPECT_TRUE(hookManager_->hookConfig_.munmap_stack_report());
}

/*
 * @tc.name: RecordAccuratelyAndBlockHook
 * @tc.desc: test record_accurately + blocked_hook combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, RecordAccuratelyAndBlockHook, TestSize.Level0)
{
    hookConfig_.set_record_accurately(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.record_accurately());
}

/*
 * @tc.name: JsStackReportAndMaxJsStackDepth
 * @tc.desc: test js_stack_report + max_js_stack_depth combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, JsStackReportAndMaxJsStackDepth, TestSize.Level0)
{
    hookConfig_.set_js_stack_report(1);
    hookConfig_.set_max_js_stack_depth(50);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.js_stack_report(), 1);
    EXPECT_EQ(hookManager_->hookConfig_.max_js_stack_depth(), 50);
}

/*
 * @tc.name: FilterSizeAndSampleInterval
 * @tc.desc: test filter_size + sample_interval combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, FilterSizeAndSampleInterval, TestSize.Level0)
{
    hookConfig_.set_filter_size(1024);
    hookConfig_.set_sample_interval(100);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.filter_size(), 1024);
    EXPECT_EQ(hookManager_->hookConfig_.sample_interval(), 100);
}

/*
 * @tc.name: SmbPagesAndMaxStackDepth
 * @tc.desc: test smb_pages + max_stack_depth combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, SmbPagesAndMaxStackDepth, TestSize.Level0)
{
    hookConfig_.set_smb_pages(256);
    hookConfig_.set_max_stack_depth(64);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.smb_pages(), 256);
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), 64);
}

/*
 * @tc.name: ProcessNameAndPid
 * @tc.desc: test process_name + pid combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, ProcessNameAndPid, TestSize.Level0)
{
    hookConfig_.set_process_name("com.test.app");
    hookConfig_.set_pid(12345);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.process_name(), "com.test.app");
    EXPECT_EQ(hookManager_->hookConfig_.pid(), 12345);
}

/*
 * @tc.name: TargetSoNameAndFilterNapiName
 * @tc.desc: test target_so_name + filter_napi_name combination.
 * @tc.type: FUNC
 */
HWTEST_F(HookCombinationBoundaryTest, TargetSoNameAndFilterNapiName, TestSize.Level0)
{
    hookConfig_.set_target_so_name("libtest.so");
    hookConfig_.set_filter_napi_name("napi_test");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.target_so_name(), "libtest.so");
    EXPECT_EQ(hookManager_->hookConfig_.filter_napi_name(), "napi_test");
}
} // namespace OHOS::Developtools::NativeDaemon
