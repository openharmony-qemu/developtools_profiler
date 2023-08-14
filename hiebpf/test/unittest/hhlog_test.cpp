/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#include <cstdio>
#include <string>

#include <gtest/gtest.h>

#include "hhlog.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class HHLoggerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: GetLogFileName
 * @tc.desc: Test HHLogger GetLogFileName
 * @tc.type: FUNC
 */
HWTEST_F(HHLoggerTest, GetLogFileName, TestSize.Level1)
{
    std::string fileName = HHLogger::GetInstance().GetLogFileName();
    EXPECT_STRNE(fileName.c_str(), "");
}

/**
 * @tc.name: GetFormatTime
 * @tc.desc: Test HHLogger GetFormatTime
 * @tc.type: FUNC
 */
HWTEST_F(HHLoggerTest, GetFormatTime, TestSize.Level1)
{
    std::string formatTime = HHLogger::GetInstance().GetFormatTime();
    EXPECT_STRNE(formatTime.c_str(), "");
}

/**
 * @tc.name: UpdateTimer
 * @tc.desc: Test HHLogger UpdateTimer
 * @tc.type: FUNC
 */
HWTEST_F(HHLoggerTest, UpdateTimer, TestSize.Level1)
{
    int ret = HHLogger::GetInstance().UpdateTimer();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: InitLogger
 * @tc.desc: Test HHLogger InitLogger
 * @tc.type: FUNC
 */
HWTEST_F(HHLoggerTest, InitLogger, TestSize.Level1)
{
    auto ret = HHLogger::GetInstance().InitLogger(HHLOG_DEBUG, "");
    EXPECT_EQ(ret, -1);

    ret = HHLogger::GetInstance().InitLogger(HHLOG_DEBUG, "stdout");
    EXPECT_EQ(ret, 0);

    std::string fileName = "/data/local/tmp/libebpf.log";
    ret = HHLogger::GetInstance().InitLogger(HHLOG_DEBUG, fileName);
    ASSERT_EQ(ret, 0);
    std::string cmd = "rm " + fileName;
    system(cmd.c_str());
}

/**
 * @tc.name: Start
 * @tc.desc: Test HHLogger Start
 * @tc.type: FUNC
 */
HWTEST_F(HHLoggerTest, Start, TestSize.Level1)
{
    HHLogger logger;
    auto ret = logger.Start(HHLOG_NONE);
    EXPECT_EQ(ret, 0);
    ret = logger.GetLogLevel();
    EXPECT_EQ(ret, HHLOG_NONE);

    ret = logger.Start(HHLOG_DEBUG, "");
    EXPECT_EQ(ret, -1);

    ret = logger.Start(HHLOG_DEBUG, "stdout");
    EXPECT_EQ(ret, 0);
    HHLOGD(true, HHLOG_DEBUG, "this is test");
    sleep(1);

    ret = logger.GetLogLevel();
    EXPECT_EQ(ret, 0);
    auto isStop = logger.IsStopped();
    EXPECT_FALSE(isStop);
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
