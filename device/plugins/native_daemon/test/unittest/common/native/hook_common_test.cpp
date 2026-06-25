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
#include "hook_common.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
class HookCommonTest : public ::testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};
}

/**
 * @tc.name: HookCommonTest001
 * @tc.desc: Test arkuiMemTraceEnable propagation in IPC (ToIpc and FromIpc)
 * @tc.type: FUNC
 */
HWTEST_F(HookCommonTest, HookCommonTest001, TestSize.Level3)
{
    ClientConfig config;
    config.arkuiMemTraceEnable = true;

    ClientConfigIpc ipc;
    EXPECT_TRUE(ConvertClientConfigToIpc(config, ipc));
    EXPECT_EQ(ipc.arkuiMemTraceEnable, true);

    ClientConfig resultConfig;
    ConvertClientConfigFromIpc(ipc, resultConfig);
    EXPECT_EQ(resultConfig.arkuiMemTraceEnable, true);
}

/**
 * @tc.name: HookCommonTest002
 * @tc.desc: Test default value of arkuiMemTraceEnable (should be false)
 * @tc.type: FUNC
 */
HWTEST_F(HookCommonTest, HookCommonTest002, TestSize.Level3)
{
    ClientConfig config;
    EXPECT_EQ(config.arkuiMemTraceEnable, false);

    ClientConfigIpc ipc;
    ConvertClientConfigToIpc(config, ipc);
    EXPECT_EQ(ipc.arkuiMemTraceEnable, false);
}
