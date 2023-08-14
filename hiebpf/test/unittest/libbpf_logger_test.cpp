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

#include "libbpf_logger.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class LIBBPFLoggerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: MakeUnique
 * @tc.desc: Test LIBBPFLogger MakeUnique
 * @tc.type: FUNC
 */
HWTEST_F(LIBBPFLoggerTest, MakeUnique, TestSize.Level1)
{
    auto logger = LIBBPFLogger::MakeUnique("", -1);
    EXPECT_TRUE(logger == nullptr);

    logger = LIBBPFLogger::MakeUnique("stdout", 0);
    EXPECT_TRUE(logger != nullptr);
}

/**
 * @tc.name: Printf
 * @tc.desc: Test LIBBPFLogger Printf
 * @tc.type: FUNC
 */
HWTEST_F(LIBBPFLoggerTest, Printf, TestSize.Level1)
{
    LIBBPFLogger logger(0);
    va_list args;
    auto ret = logger.Printf(1, nullptr, args);
    EXPECT_EQ(ret, 0);

    char fmt[] = "text";
    ret = logger.Printf(0, fmt, args);
    EXPECT_EQ(ret, -1);

    ret = logger.OpenLogFile("stdout");
    EXPECT_EQ(ret, 0);
    ret = logger.Printf(0, fmt, args);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: OpenLogFile
 * @tc.desc: Test LIBBPFLogger OpenLogFile
 * @tc.type: FUNC
 */
HWTEST_F(LIBBPFLoggerTest, OpenLogFile, TestSize.Level1)
{
    LIBBPFLogger logger(0);
    auto ret = logger.OpenLogFile("");
    EXPECT_EQ(ret, -1);

    ret = logger.OpenLogFile("stdout");
    EXPECT_EQ(ret, 0);

    std::string fileName = "/data/local/tmp/libebpf.log";
    ret = logger.OpenLogFile(fileName);
    ASSERT_EQ(ret, 0);
    std::string cmd = "rm " + fileName;
    system(cmd.c_str());
}

/**
 * @tc.name: GetLogFileName
 * @tc.desc: Test LIBBPFLogger GetLogFileName
 * @tc.type: FUNC
 */
HWTEST_F(LIBBPFLoggerTest, GetLogFileName, TestSize.Level1)
{
    LIBBPFLogger logger(0);
    std::string fileName = logger.GetLogFileName();
    EXPECT_STRNE(fileName.c_str(), "");
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
