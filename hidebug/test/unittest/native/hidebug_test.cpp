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

#include "hidebug_base.h"

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include <cstdlib>

using namespace testing::ext;

namespace {
class HidebugTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override
    {
        system("param set hiviewdfx.debugenv.hidebug_test 0");
        system("param set libc.hook_mode 0");
    }
};

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam1, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    const char* inputName = "hidebug_test";
    EXPECT_TRUE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for input is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam2, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    const char* inputName = nullptr;
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for input is wrong
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam3, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    const char* inputName = "hidebug_test/";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for param set wrong
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam4, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test error_input");
    const char* inputName = "hidebug_test";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam5, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    system("param set libc.hook_mode startup:hidebug_test");
    const char* inputName = "hidebug_test";
    EXPECT_TRUE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode param set fail
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam6, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test error_input");
    system("param set libc.hook_mode error_set:hidebug_test");
    const char* inputName = "hidebug_test";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode fail
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam7, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test error_input");
    system("param set libc.hook_mode error_set:hidebug_test");
    const char* inputName = "hidebug_test";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode input error
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam8, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    system("param set libc.hook_mode error_set:hidebug_test");
    const char* inputName = "error_input";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam for libc.hook_mode param set wrong_proc
 * @tc.type: FUNC
 */
HWTEST_F(HidebugTest, InitEnvironmentParam9, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test error_input");
    system("param set libc.hook_mode start_up:wrong_proc");
    const char* inputName = "hidebug";
    EXPECT_FALSE(InitEnvironmentParam(inputName));
}
} // namespace
