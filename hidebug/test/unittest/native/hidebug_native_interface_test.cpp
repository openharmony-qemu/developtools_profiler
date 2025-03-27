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

#include <cstdlib>
#include <filesystem>

#include <gtest/hwext/gtest-ext.h>
#include <gtest/hwext/gtest-tag.h>

#include "hidebug/hidebug.h"
#include "hidebug/hidebug_type.h"
#include "hidebug_native_interface.h"

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
class HidebugNativeInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override
    {
        system("param set hiviewdfx.debugenv.hidebug_test 0");
        system("param set libc.hook_mode 0");
    }
};

/**
 * @tc.name: OH_HiDebug_GetGraphicsMemory
 * @tc.desc: test OH_HiDebug_GetRealNanoSecondsTimestamp. get graphics memory.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, IsDebuggerConnectedTest, TestSize.Level1)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    ASSERT_FALSE(interface.IsDebuggerConnected());
}

#ifdef __aarch64__
/**
 * @tc.name: OH_HiDebug_GetGraphicsMemory
 * @tc.desc: test OH_HiDebug_GetRealNanoSecondsTimestamp. get graphics memory.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugNativeInterfaceTest, GetMemoryLeakResourceTest, TestSize.Level1)
{
    auto& interface = HidebugNativeInterface::GetInstance();
    ASSERT_EQ(interface.GetMemoryLeakResource("test", 10, false), MemoryState::MEMORY_SUCCESS);
}
#endif
}
}