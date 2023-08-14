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

#include "bpf_log_reader.h"

using namespace testing::ext;
using namespace std;
namespace {
const std::string FILE_NAME = "/data/local/tmp/libebpf.log";
} // namespace

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class BPFLogReaderTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase()
    {
        if (access(FILE_NAME.c_str(), F_OK) == 0) {
            std::string cmd = "rm " + FILE_NAME;
            system(cmd.c_str());
        }
    }

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: MakeUnique
 * @tc.desc: Test BPFLogReader MakeUnique
 * @tc.type: FUNC
 */
HWTEST_F(BPFLogReaderTest, MakeUnique, TestSize.Level1)
{
    auto logger = BPFLogReader::MakeUnique("");
    EXPECT_TRUE(logger == nullptr);

    logger = BPFLogReader::MakeUnique("stdout");
    EXPECT_TRUE(logger != nullptr);

    logger = BPFLogReader::MakeUnique(FILE_NAME);
    EXPECT_TRUE(logger != nullptr);
}

/**
 * @tc.name: OpenLogFile
 * @tc.desc: Test BPFLogReader OpenLogFile
 * @tc.type: FUNC
 */
HWTEST_F(BPFLogReaderTest, OpenLogFile, TestSize.Level1)
{
    BPFLogReader logger;
    auto ret = logger.OpenLogFile("");
    EXPECT_EQ(ret, -1);

    ret = logger.OpenLogFile("stdout");
    EXPECT_EQ(ret, 0);

    ret = logger.OpenLogFile(FILE_NAME);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: GetLogFileName
 * @tc.desc: Test BPFLogReader GetLogFileName
 * @tc.type: FUNC
 */
HWTEST_F(BPFLogReaderTest, GetLogFileName, TestSize.Level1)
{
    BPFLogReader logger;
    std::string fileName = logger.GetLogFileName();
    EXPECT_STRNE(fileName.c_str(), "");
}

/**
 * @tc.name: Start
 * @tc.desc: Test BPFLogReader Start
 * @tc.type: FUNC
 */
HWTEST_F(BPFLogReaderTest, Start, TestSize.Level1)
{
    BPFLogReader logger;
    auto ret = logger.Start();
    EXPECT_EQ(ret, 0);

    sleep(1);

    ret = logger.Stop();
    EXPECT_EQ(ret, 0);
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
