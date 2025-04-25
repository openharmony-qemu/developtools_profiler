/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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
#if is_ohos && !is_mingw
#include <gtest/hwext/gtest-ext.h>
#include <gtest/hwext/gtest-tag.h>

#include "hidebug/hidebug.h"

using namespace testing::ext;

class HidebugBacktraceTest : public ::testing::Test {};
/**
 * @tc.name: BacktraceFromFpTest
 * @tc.desc: test OH_HiDebug_BacktraceFromFp
 * @tc.type: FUNC
 */
HWTEST_F(HidebugBacktraceTest, BacktraceFromFpTest, TestSize.Level1)
{
    EXPECT_EQ(OH_HiDebug_BacktraceFromFp(nullptr, nullptr, nullptr, 0), -1);
    BacktraceObject obj = OH_HiDebug_CreateBacktraceObject();
    EXPECT_NE(obj, nullptr);
    EXPECT_EQ(OH_HiDebug_BacktraceFromFp(obj, nullptr, nullptr, 0), -1);
    EXPECT_EQ(OH_HiDebug_BacktraceFromFp(obj, __builtin_frame_address(0), nullptr, 0), -1);
    constexpr auto testArraySize = 10;
    void *pcArray[testArraySize]{};
    EXPECT_EQ(OH_HiDebug_BacktraceFromFp(obj, __builtin_frame_address(0), pcArray, 0), -1);
    EXPECT_GT(OH_HiDebug_BacktraceFromFp(obj, __builtin_frame_address(0), pcArray, testArraySize), 0);
    EXPECT_LE(OH_HiDebug_BacktraceFromFp(obj, __builtin_frame_address(0), pcArray, testArraySize), testArraySize);
    EXPECT_EQ(pcArray[0], __builtin_return_address(0));
    EXPECT_EQ(pcArray[1], __builtin_return_address(1));
    EXPECT_EQ(pcArray[2], __builtin_return_address(2));
    OH_HiDebug_DestroyBacktraceObject(obj);
}

/**
 * @tc.name: SymbolicAddressTest
 * @tc.desc: test OH_HiDebug_SymbolicAddress
 * @tc.type: FUNC
 */
HWTEST_F(HidebugBacktraceTest, SymbolicAddressTest, TestSize.Level1)
{
    EXPECT_EQ(OH_HiDebug_SymbolicAddress(nullptr, nullptr, nullptr, nullptr), HIDEBUG_INVALID_ARGUMENT);
    BacktraceObject obj = OH_HiDebug_CreateBacktraceObject();
    EXPECT_NE(obj, nullptr);
    EXPECT_EQ(OH_HiDebug_SymbolicAddress(obj, nullptr, nullptr, nullptr), HIDEBUG_INVALID_ARGUMENT);
    EXPECT_EQ(OH_HiDebug_SymbolicAddress(obj, __builtin_return_address(0), nullptr, nullptr), HIDEBUG_INVALID_ARGUMENT);
    EXPECT_EQ(OH_HiDebug_SymbolicAddress(obj, __builtin_return_address(0), nullptr, nullptr), HIDEBUG_INVALID_ARGUMENT);
    auto callback = [] (void* pc, void* arg, const HiDebug_StackFrame* frame) {
        EXPECT_EQ(frame->type, HiDebug_StackFrameType::HIDEBUG_STACK_FRAME_TYPE_NATIVE);
        (*static_cast<int*>(arg))++;
    };
    EXPECT_EQ(OH_HiDebug_SymbolicAddress(obj, &callback, nullptr, callback), HIDEBUG_INVALID_SYMBOLIC_PC_ADDRESS);
    int testNum = 0;
    EXPECT_EQ(OH_HiDebug_SymbolicAddress(obj, __builtin_return_address(0), &testNum, callback), HIDEBUG_SUCCESS);
    EXPECT_EQ(testNum, 1);
    OH_HiDebug_DestroyBacktraceObject(obj);
}
#endif
