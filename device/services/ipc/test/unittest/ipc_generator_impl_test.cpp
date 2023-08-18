/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "ipc_generator_impl.h"

using namespace testing::ext;

namespace {
class IpcGeneratorImplTest : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: Service
 * @tc.desc: IPC process testing.
 * @tc.type: FUNC
 */
HWTEST_F(IpcGeneratorImplTest, NormalCase, TestSize.Level1)
{
    IpcGeneratorImpl ipcGeneratorImpl;
    IpcGeneratorImpl ipcGeneratorTest;
    EXPECT_TRUE(ipcGeneratorTest.SetNames("test_case.proto", "") == "test_case");
    ASSERT_TRUE(ipcGeneratorImpl.SetNames("test.proto", "") == "test");
    ASSERT_TRUE(ipcGeneratorImpl.AddService("TestService"));
    ASSERT_TRUE(!ipcGeneratorImpl.AddService("TestService"));
    ASSERT_TRUE(ipcGeneratorImpl.AddServiceMethod("TestService", "DoTest", "DoTestRequest", "DoTestResponse"));
    ASSERT_TRUE(ipcGeneratorImpl.AddServiceMethod("TestService", "DoTest2", "DoTest2Request", "DoTest2Response"));
    ASSERT_TRUE(!ipcGeneratorImpl.AddServiceMethod("TestService", "DoTest", "DoTestRequest", "DoTestResponse"));
    ipcGeneratorImpl.GenHeader();
    ipcGeneratorTest.GenHeader();
    ipcGeneratorImpl.GenSource();
}
} // namespace