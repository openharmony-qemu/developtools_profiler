/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <gtest/gtest.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <chrono>
#include "sp_utils.h"
#include "GpuCounter.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonGpuCounterTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: GpuCounter::ItemData
 * @tc.desc: Test ItemData
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonGpuCounterTest, ItemDataTestCase, TestSize.Level1)
{
    std::map<std::string, std::string> result = GpuCounter::GetInstance().ItemData();
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: GpuCounterTestCase
 * @tc.desc: Test GpuCounter
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonGpuCounterTest, GpuCounterTestCase, TestSize.Level1)
{
    std::string cmd = "SP_daemon -N 5 -gc";
    std::string result = "";
    bool flag = false;
    auto ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("gpuCounterData");
    printf("gpuCounterData: %s\n", result.c_str());
    if ((strOne != result.npos)) {
        flag = true;
    }
    EXPECT_EQ(ret, true);
    EXPECT_EQ(flag, true);
}
}
}
