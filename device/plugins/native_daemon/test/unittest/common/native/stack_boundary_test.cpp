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

#include "stack_boundary_test.h"
#include <climits>
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
constexpr uint32_t BUFF_SIZE = 1024;

void StackBoundaryTest::SetUpTestCase(void) {}
void StackBoundaryTest::TearDownTestCase(void) {}

void StackBoundaryTest::SetUp()
{
    hookManager_ = std::make_shared<HookManager>();
    hookConfig_.Clear();
    dataRepeater_ = std::make_shared<StackDataRepeater>(BUFF_SIZE);
}

void StackBoundaryTest::TearDown()
{
    stackPreprocess_ = nullptr;
    hookManager_ = nullptr;
    dataRepeater_ = nullptr;
}

/*
 * @tc.name: MaxStackDepthMaxBoundary
 * @tc.desc: test max_stack_depth = 1024 (extreme value) boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, MaxStackDepthMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_max_stack_depth(BUFF_SIZE);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), BUFF_SIZE);
}

/*
 * @tc.name: MaxJsStackDepthMaxBoundary
 * @tc.desc: test max_js_stack_depth = 1000 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, MaxJsStackDepthMaxBoundary, TestSize.Level0)
{
    hookConfig_.set_max_js_stack_depth(1000);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_js_stack_depth(), 1000);
}

/*
 * @tc.name: CallFrameIdZeroBoundary
 * @tc.desc: test callFrameId = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, CallFrameIdZeroBoundary, TestSize.Level0)
{
    CallFrame frame(0);
    frame.callFrameId_ = 0;
    EXPECT_EQ(frame.callFrameId_, 0);
}

/*
 * @tc.name: CallFrameIdMaxBoundary
 * @tc.desc: test callFrameId = UINT32_MAX boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, CallFrameIdMaxBoundary, TestSize.Level0)
{
    CallFrame frame(UINT32_MAX);
    frame.callFrameId_ = UINT32_MAX;
    EXPECT_EQ(frame.callFrameId_, UINT32_MAX);
}

/*
 * @tc.name: IpAddressZeroBoundary
 * @tc.desc: test IP address = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, IpAddressZeroBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.ip_ = 0;
    EXPECT_EQ(frame.ip_, 0);
}

/*
 * @tc.name: IpAddressMaxBoundary
 * @tc.desc: test IP address = UINT64_MAX boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, IpAddressMaxBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.ip_ = UINT64_MAX;
    EXPECT_EQ(frame.ip_, UINT64_MAX);
}

/*
 * @tc.name: SpAddressZeroBoundary
 * @tc.desc: test SP address = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, SpAddressZeroBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.sp_ = 0;
    EXPECT_EQ(frame.sp_, 0);
}

/*
 * @tc.name: SpAddressMaxBoundary
 * @tc.desc: test SP address = UINT64_MAX boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, SpAddressMaxBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.sp_ = UINT64_MAX;
    EXPECT_EQ(frame.sp_, UINT64_MAX);
}

/*
 * @tc.name: vaddrInFileAddressZeroBoundary
 * @tc.desc: test SP address = 0 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, vaddrInFileAddressZeroBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.vaddrInFile_ = 0;
    EXPECT_EQ(frame.vaddrInFile_, 0);
}

/*
 * @tc.name: vaddrInFileAddressMaxBoundary
 * @tc.desc: test SP address = UINT64_MAX boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, vaddrInFileAddressMaxBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.vaddrInFile_ = UINT64_MAX;
    EXPECT_EQ(frame.vaddrInFile_, UINT64_MAX);
}

/*
 * @tc.name: OfflineSymbolizationEnabled
 * @tc.desc: test offline_symbolization = true boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, OfflineSymbolizationEnabled, TestSize.Level0)
{
    hookConfig_.set_offline_symbolization(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.offline_symbolization());
}

/*
 * @tc.name: FpUnwindEnabled
 * @tc.desc: test fp_unwind = true boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, FpUnwindEnabled, TestSize.Level0)
{
    hookConfig_.set_fp_unwind(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.fp_unwind());
}

/*
 * @tc.name: CallframeCompressEnabled
 * @tc.desc: test callframe_compress = true boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, CallframeCompressEnabled, TestSize.Level0)
{
    hookConfig_.set_callframe_compress(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.callframe_compress());
}

/*
 * @tc.name: StringCompressedEnabled
 * @tc.desc: test string_compressed = true boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, StringCompressedEnabled, TestSize.Level0)
{
    hookConfig_.set_string_compressed(true);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_TRUE(hookManager_->hookConfig_.string_compressed());
}

/*
 * @tc.name: MaxStackDepthMinBoundary
 * @tc.desc: test max_stack_depth = 1 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, MaxStackDepthMinBoundary, TestSize.Level0)
{
    hookConfig_.set_max_stack_depth(1);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_stack_depth(), 1);
}

/*
 * @tc.name: MaxJsStackDepthMinBoundary
 * @tc.desc: test max_js_stack_depth = 1 boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, MaxJsStackDepthMinBoundary, TestSize.Level0)
{
    hookConfig_.set_max_js_stack_depth(1);
    hookManager_->SetHookConfig(hookConfig_);
    EXPECT_EQ(hookManager_->hookConfig_.max_js_stack_depth(), 1);
}

/*
 * @tc.name: CallFrameSymbolNameBoundary
 * @tc.desc: test CallFrame symbol name boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, CallFrameSymbolNameBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.symbolName_ = "test_symbol_name";
    EXPECT_EQ(frame.symbolName_, "test_symbol_name");
}

/*
 * @tc.name: CallFrameFilePathBoundary
 * @tc.desc: test CallFrame file path boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, CallFrameFilePathBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.filePath_ = "/data/local/tmp/test.so";
    EXPECT_EQ(frame.filePath_, "/data/local/tmp/test.so");
}

/*
 * @tc.name: CallFrameOffsetBoundary
 * @tc.desc: test CallFrame offset boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, CallFrameOffsetBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.offset_ = UINT64_MAX;
    EXPECT_EQ(frame.offset_, UINT64_MAX);
}

/*
 * @tc.name: CallFrameOffsetBoundary
 * @tc.desc: test CallFrame offset boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, CallFrameOffsetZeroBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.offset_ = 0;
    EXPECT_EQ(frame.offset_, 0);
}

/*
 * @tc.name: CallFrameSymbolOffsetBoundary
 * @tc.desc: test CallFrame symbol offset boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StackBoundaryTest, CallFrameSymbolOffsetBoundary, TestSize.Level0)
{
    CallFrame frame(1);
    frame.symbolOffset_ = UINT64_MAX;
    EXPECT_EQ(frame.symbolOffset_, UINT64_MAX);
}
} // namespace OHOS::Developtools::NativeDaemon
