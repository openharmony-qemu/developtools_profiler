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

#include <cstdlib>
#include <filesystem>

#include <gtest/hwext/gtest-ext.h>
#include <gtest/hwext/gtest-tag.h>

#include <sys/stat.h>

#include "hidebug_util.h"

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
class HidebugUtilTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override
    {
        system("param set hiviewdfx.debugenv.hidebug_test 0");
        system("param set libc.hook_mode 0");
    }
};

/**
 * @tc.name: GetRealNanoSecondsTimestampTest
 * @tc.desc: test GetRealNanoSecondsTimestamp.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, GetRealNanoSecondsTimestampTest, TestSize.Level1)
{
    EXPECT_GE(GetRealNanoSecondsTimestamp(), 0);
}

/**
 * @tc.name: GetElapsedNanoSecondsSinceBootTest
 * @tc.desc: test GetElapsedNanoSecondsSinceBoot.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, GetElapsedNanoSecondsSinceBootTest, TestSize.Level1)
{
    EXPECT_GE(GetElapsedNanoSecondsSinceBoot(), 0);
}

/**
 * @tc.name: GetProcessDirTest
 * @tc.desc: test GetProcessDir.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, GetProcessDirTest, TestSize.Level1)
{
    std::string cacheDir = GetProcessDir(DirectoryType::CACHE);
    GTEST_LOG_(INFO) << "DumpCatcherCommandTest001: cacheDir." << cacheDir;
    std::string fileDir = GetProcessDir(DirectoryType::FILE);
    GTEST_LOG_(INFO) << "DumpCatcherCommandTest001: cacheDir." << fileDir;
    std::string otherDir = GetProcessDir(static_cast<DirectoryType>(3));
    GTEST_LOG_(INFO) << "DumpCatcherCommandTest001: cacheDir." << fileDir;
    EXPECT_GE(fileDir.length(), 0);
}

/**
 * @tc.name: SplitStrTest
 * @tc.desc: test SplitStr.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, SplitStrTest, TestSize.Level1)
{
    auto split1 = SplitStr("/test/", '/');
    EXPECT_EQ(split1.size(), 3);
    EXPECT_STREQ(split1[1].c_str(), "test");
    auto split2 = SplitStr("/test/", '/', [](std::string& s) {
        return !s.empty();
    });
    EXPECT_EQ(split2.size(), 1);
    EXPECT_STREQ(split2[0].c_str(), "test");
}

/**
 * @tc.name: IsLegalPathTest
 * @tc.desc: test IsLegalPath.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, IsLegalPathTest, TestSize.Level1)
{
    EXPECT_FALSE(IsLegalPath("./"));
    EXPECT_FALSE(IsLegalPath(""));
    EXPECT_TRUE(IsLegalPath("/test"));
}

/**
 * @tc.name: SmartFileTest
 * @tc.desc: test SmartFile.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, SmartFileTest, TestSize.Level1)
{
    ASSERT_EQ(SmartFile::OpenFile("./", "r"), nullptr);
    constexpr auto testFile = "/data/test/testSmartFile.txt";
    ASSERT_EQ(GetFileSize(testFile), 0);
    ASSERT_EQ(SmartFile::OpenFile(testFile, "r"), nullptr);
    auto testFilePtr = SmartFile::OpenFile(testFile, "w+");
    ASSERT_NE(SmartFile::OpenFile(testFile, "r"), nullptr);
    int writeDATA = 10;
    ASSERT_TRUE(testFilePtr->Write(&writeDATA, sizeof(writeDATA), 1));
    testFilePtr = nullptr;
    ASSERT_GT(GetFileSize(testFile), 0);
    testFilePtr = SmartFile::OpenFile(testFile, "r");
    ASSERT_FALSE(testFilePtr->Write(&writeDATA, sizeof(writeDATA), 1));
    int readData = 0;
    ASSERT_GT(testFilePtr->Read(&readData, sizeof(readData), 1), 0);
    ASSERT_EQ(readData, writeDATA);
    remove(testFile);
}

/**
 * @tc.name: CreateDirectoryTest
 * @tc.desc: test CreateDirectory.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, CreateDirectoryTest, TestSize.Level1)
{
    constexpr auto testFileDir = "/data/test/testCreateDirectory";
    ASSERT_TRUE(CreateDirectory(testFileDir, 0000));
    std::string filePath = std::string(testFileDir) + std::string("/") + std::string(256, 'a');
    ASSERT_FALSE(CreateDirectory(filePath, 0000));
    remove(testFileDir);
}

/**
 * @tc.name: CreateFileTest
 * @tc.desc: test CreateFile.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, CreateFileTest, TestSize.Level1)
{
    constexpr auto testFile = "/data/test/test/test.txt";
    ASSERT_FALSE(CreateFile(testFile));
    constexpr auto testFileDir = "/data/test/test";
    ASSERT_TRUE(CreateDirectory(testFileDir, 0755));
    ASSERT_TRUE(CreateFile(testFile));
    ASSERT_TRUE(CreateFile(testFile));
    remove(testFile);
    remove(testFileDir);
}

/**
 * @tc.name: XAttrTest
 * @tc.desc: test XAttr.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, XAttrTest, TestSize.Level1)
{
    constexpr auto key = "user.test";
    constexpr auto value = "test";
    constexpr auto testFile = "/data/test/testXAttr.txt";
    ASSERT_FALSE(SetXAttr(testFile, key, value));
    std::string readValue;
    ASSERT_FALSE(GetXAttr(testFile, key, readValue, 128));
    ASSERT_TRUE(CreateFile(testFile));
    ASSERT_TRUE(SetXAttr(testFile, key, value));
    ASSERT_TRUE(GetXAttr(testFile, key, readValue, 128));
    ASSERT_STREQ(readValue.c_str(), "test");
    remove(testFile);
}

/**
 * @tc.name: IsBetaVersionTest
 * @tc.desc: test IsBetaVersion.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, IsBetaVersionTest, TestSize.Level1)
{
    IsBetaVersion();
    ASSERT_TRUE(true);
}

/**
 * @tc.name: IsDebuggableHapTest
 * @tc.desc: test IsDebuggableHap.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, IsDebuggableHapTest, TestSize.Level1)
{
    unsetenv("HAP_DEBUGGABLE");
    EXPECT_FALSE(IsDebuggableHap());
    setenv("HAP_DEBUGGABLE", "true", 1);
    EXPECT_TRUE(IsDebuggableHap());
}

/**
 * @tc.name: IsDeveloperOptionsEnabledTest
 * @tc.desc: test IsDeveloperOptionsEnabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugUtilTest, IsDeveloperOptionsEnabledTest, TestSize.Level1)
{
    IsDeveloperOptionsEnabled();
    ASSERT_TRUE(true);
}
}
}