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

#include "hook_string_boundary_test.h"
#include <climits>
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
void HookStringBoundaryTest::SetUpTestCase(void) {}
void HookStringBoundaryTest::TearDownTestCase(void) {}

void HookStringBoundaryTest::SetUp()
{
    hookManager_ = std::make_shared<HookManager>();
    hookConfig_.Clear();
}

void HookStringBoundaryTest::TearDown()
{
    hookManager_ = nullptr;
}

/*
 * @tc.name: ProcessNameEmptyBoundary
 * @tc.desc: test process_name = "" boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, ProcessNameEmptyBoundary, TestSize.Level0)
{
    hookConfig_.set_process_name("");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.process_name(), "");
}

/*
 * @tc.name: ProcessNameSingleCharBoundary
 * @tc.desc: test process_name = "a" boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, ProcessNameSingleCharBoundary, TestSize.Level0)
{
    hookConfig_.set_process_name("a");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.process_name(), "a");
}

/*
 * @tc.name: ProcessNameSpecialCharsBoundary
 * @tc.desc: test process_name with special characters.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, ProcessNameSpecialCharsBoundary, TestSize.Level0)
{
    hookConfig_.set_process_name("test_process-123");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.process_name(), "test_process-123");
}

/*
 * @tc.name: FileNameEmptyBoundary
 * @tc.desc: test file_name = "" boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FileNameEmptyBoundary, TestSize.Level0)
{
    hookConfig_.set_file_name("");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.file_name(), "");
}

/*
 * @tc.name: FileNameRelativePathBoundary
 * @tc.desc: test file_name with relative path.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FileNameRelativePathBoundary, TestSize.Level0)
{
    hookConfig_.set_file_name("./test.htrace");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.file_name(), "./test.htrace");
}

/*
 * @tc.name: FileNameAbsolutePathBoundary
 * @tc.desc: test file_name with absolute path.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FileNameAbsolutePathBoundary, TestSize.Level0)
{
    hookConfig_.set_file_name("/data/local/tmp/test.htrace");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.file_name(), "/data/local/tmp/test.htrace");
}

/*
 * @tc.name: TargetSoNameEmptyBoundary
 * @tc.desc: test target_so_name = "" boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, TargetSoNameEmptyBoundary, TestSize.Level0)
{
    hookConfig_.set_target_so_name("");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.target_so_name(), "");
}

/*
 * @tc.name: TargetSoNameWithExtBoundary
 * @tc.desc: test target_so_name with .so extension.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, TargetSoNameWithExtBoundary, TestSize.Level0)
{
    hookConfig_.set_target_so_name("libtest.so");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.target_so_name(), "libtest.so");
}

/*
 * @tc.name: TargetSoNameWithoutExtBoundary
 * @tc.desc: test target_so_name without .so extension.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, TargetSoNameWithoutExtBoundary, TestSize.Level0)
{
    hookConfig_.set_target_so_name("libtest");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.target_so_name(), "libtest");
}

/*
 * @tc.name: FilterNapiNameEmptyBoundary
 * @tc.desc: test filter_napi_name = "" boundary.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FilterNapiNameEmptyBoundary, TestSize.Level0)
{
    hookConfig_.set_filter_napi_name("");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.filter_napi_name(), "");
}

/*
 * @tc.name: FilterNapiNameValidBoundary
 * @tc.desc: test filter_napi_name with valid name.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FilterNapiNameValidBoundary, TestSize.Level0)
{
    hookConfig_.set_filter_napi_name("napi_test_function");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.filter_napi_name(), "napi_test_function");
}

/*
 * @tc.name: ProcessNameLongStringBoundary
 * @tc.desc: test process_name with long string.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, ProcessNameLongStringBoundary, TestSize.Level0)
{
    std::string longName(255, 'a');
    hookConfig_.set_process_name(longName);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.process_name(), longName);
}

/*
 * @tc.name: FileNameLongPathBoundary
 * @tc.desc: test file_name with long path.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FileNameLongPathBoundary, TestSize.Level0)
{
    std::string longPath = "/data/local/tmp/";
    longPath += std::string(200, 'a');
    longPath += ".htrace";
    hookConfig_.set_file_name(longPath);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.file_name(), longPath);
}

/*
 * @tc.name: ProcessNameUnicodeBoundary
 * @tc.desc: test process_name with unicode characters.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, ProcessNameUnicodeBoundary, TestSize.Level0)
{
    hookConfig_.set_process_name("测试进程");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.process_name(), "测试进程");
}

/*
 * @tc.name: FileNameWithSpacesBoundary
 * @tc.desc: test file_name with spaces.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FileNameWithSpacesBoundary, TestSize.Level0)
{
    hookConfig_.set_file_name("/data/local/tmp/test file.htrace");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.file_name(), "/data/local/tmp/test file.htrace");
}

/*
 * @tc.name: TargetSoNameFullPathBoundary
 * @tc.desc: test target_so_name with full path.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, TargetSoNameFullPathBoundary, TestSize.Level0)
{
    hookConfig_.set_target_so_name("/system/lib64/libtest.so");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.target_so_name(), "/system/lib64/libtest.so");
}

/*
 * @tc.name: FilterNapiNameLongBoundary
 * @tc.desc: test filter_napi_name with long name.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FilterNapiNameLongBoundary, TestSize.Level0)
{
    std::string longName(200, 'n');
    hookConfig_.set_filter_napi_name(longName);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.filter_napi_name(), longName);
}

/*
 * @tc.name: ProcessNameWithDotBoundary
 * @tc.desc: test process_name with dot separator.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, ProcessNameWithDotBoundary, TestSize.Level0)
{
    hookConfig_.set_process_name("com.example.test.app");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.process_name(), "com.example.test.app");
}

/*
 * @tc.name: FileNameWithExtensionBoundary
 * @tc.desc: test file_name with different extensions.
 * @tc.type: FUNC
 */
HWTEST_F(HookStringBoundaryTest, FileNameWithExtensionBoundary, TestSize.Level0)
{
    hookConfig_.set_file_name("/data/local/tmp/test.trace");
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.file_name(), "/data/local/tmp/test.trace");
}
} // namespace OHOS::Developtools::NativeDaemon
