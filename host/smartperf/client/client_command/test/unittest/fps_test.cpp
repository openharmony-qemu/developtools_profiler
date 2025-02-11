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
#include "FPS.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonFpsTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: FPS::SetFpsCurrentFpsTime
 * @tc.desc: Test SetFpsCurrentFpsTime
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonFpsTest, SetFpsCurrentFpsTimeTestCase001, TestSize.Level1)
{
    bool ret = false;
    FpsInfo fpsInfoResult;
    FPS::GetInstance().SetFpsCurrentFpsTime(fpsInfoResult);
    FpsCurrentFpsTime fcf = FPS::GetInstance().GetFpsCurrentFpsTime();

    if (fcf.currentFpsTime == 0) {
        ret = true;
    }

    EXPECT_TRUE(ret);
}

/**
 * @tc.name: FPS::GetFpsCurrentFpsTime
 * @tc.desc: Test GetFpsCurrentFpsTime
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonFpsTest, GetFpsCurrentFpsTimeTestCase001, TestSize.Level1)
{
    bool ret = false;
    FpsCurrentFpsTime fcf = FPS::GetInstance().GetFpsCurrentFpsTime();

    if (fcf.fps == 0 && fcf.currentFpsTime == 0) {
        ret = true;
    }
    EXPECT_TRUE(ret);
}

}
}