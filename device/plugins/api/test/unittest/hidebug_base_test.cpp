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


#include <gtest/gtest.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include "hidebug_base.h"

using namespace testing::ext;

namespace {
class HidebugBaseTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: InitEnvironmentParam
 * @tc.desc: test InitEnvironmentParam
 * @tc.type: FUNC
 */
HWTEST_F(HidebugBaseTest, InitEnvironmentParam1, TestSize.Level1)
{
    system("param set hiviewdfx.debugenv.hidebug_test aaa:bbb");
    EXPECT_TRUE(InitEnvironmentParam("hidebug_test"));
    EXPECT_FALSE(InitEnvironmentParam("hidebug_test/"));
    EXPECT_FALSE(InitEnvironmentParam(nullptr));
}
}