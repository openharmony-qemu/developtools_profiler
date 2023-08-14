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

#include <gtest/gtest.h>

#include "hiebpf_data_file.h"

using namespace testing::ext;
using namespace std;
namespace {
const std::string FILE_NAME = "/data/local/tmp/hiebpf.txt";
} // namespace

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class HiebpfDataFileTest : public ::testing::Test {
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
 * @tc.name: MakeShared
 * @tc.desc: Test HiebpfDataFile MakeShared
 * @tc.type: FUNC
 */
HWTEST_F(HiebpfDataFileTest, MakeShared, TestSize.Level1)
{
    auto hiebpfDataFile = HiebpfDataFile::MakeShared("", "");
    EXPECT_TRUE(hiebpfDataFile == nullptr);

    hiebpfDataFile = HiebpfDataFile::MakeShared("hiebpf", FILE_NAME);
    EXPECT_TRUE(hiebpfDataFile != nullptr);
}

/**
 * @tc.name: MapFile
 * @tc.desc: Test HiebpfDataFile MapFile
 * @tc.type: FUNC
 */
HWTEST_F(HiebpfDataFileTest, MapFile, TestSize.Level1)
{
    HiebpfDataFile hiebpfDataFile("hiebpf", FILE_NAME);
    auto ret = hiebpfDataFile.OpenFile();
    EXPECT_EQ(ret, 0);
    ret = hiebpfDataFile.MapFile();
    EXPECT_EQ(ret, 0);
    ret = hiebpfDataFile.RemapFile(1);
    EXPECT_EQ(ret, 0);

    close(hiebpfDataFile.fd_);
    hiebpfDataFile.fd_ = -1;
    ret = hiebpfDataFile.MapFile();
    EXPECT_EQ(ret, -1);
    ret = hiebpfDataFile.RemapFile(1);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: WriteKernelSymbol
 * @tc.desc: Test HiebpfDataFile WriteKernelSymbol
 * @tc.type: FUNC
 */
HWTEST_F(HiebpfDataFileTest, WriteKernelSymbol, TestSize.Level1)
{
    HiebpfDataFile hiebpfDataFile("hiebpf", FILE_NAME);
    hiebpfDataFile.WriteKernelSymbol();
    EXPECT_TRUE(hiebpfDataFile.mapAddr_ == MAP_FAILED);

    auto ret = hiebpfDataFile.OpenFile();
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(hiebpfDataFile.mapAddr_ == MAP_FAILED);
    ret = hiebpfDataFile.MapFile();
    EXPECT_EQ(ret, 0);
    hiebpfDataFile.WriteKernelSymbol();
    EXPECT_TRUE(hiebpfDataFile.mapAddr_ != MAP_FAILED);
}

/**
 * @tc.name: Discard
 * @tc.desc: Test HiebpfDataFile Discard
 * @tc.type: FUNC
 */
HWTEST_F(HiebpfDataFileTest, Discard, TestSize.Level1)
{
    HiebpfDataFile hiebpfDataFile("hiebpf", FILE_NAME);
    hiebpfDataFile.Discard(nullptr);

    auto ret = hiebpfDataFile.OpenFile();
    EXPECT_EQ(ret, 0);
    ret = hiebpfDataFile.MapFile();
    EXPECT_EQ(ret, 0);
    void *dest = hiebpfDataFile.Reserve(1);
    EXPECT_TRUE(dest != nullptr);
    hiebpfDataFile.Discard(dest);
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
