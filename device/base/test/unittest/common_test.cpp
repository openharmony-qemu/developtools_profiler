/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#include <chrono>
#include <thread>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include "common.h"
#include "logging.h"

namespace {
using namespace testing::ext;
using namespace COMMON;

class CommonTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    bool WriteFile(const std::string& filePath, const std::string& fileContent)
    {
        FILE* file = fopen(filePath.c_str(), "w");
        if (file == nullptr) {
            std::string errorMsg = GetErrorMsg();
            HILOG_ERROR(LOG_CORE, "WriteFile: fopen() fail, %s, %s", filePath.c_str(), errorMsg.c_str());
            return false;
        }

        size_t len = fwrite(const_cast<char*>(fileContent.c_str()), 1, fileContent.length(), file);
        if (len < 0) {
            std::string errorMsg = GetErrorMsg();
            HILOG_ERROR(LOG_CORE, "WriteFile: fwrite() fail, %s", errorMsg.c_str());
            (void)fclose(file);
            return false;
        }

        if (fflush(file) == EOF) {
            std::string errorMsg = GetErrorMsg();
            HILOG_ERROR(LOG_CORE, "WriteFile: fflush() error = %s", errorMsg.c_str());
            (void)fclose(file);
            return false;
        }

        fsync(fileno(file));
        if (fclose(file) != 0) {
            std::string errorMsg = GetErrorMsg();
            HILOG_ERROR(LOG_CORE, "CreateConfigFile: fclose() error = %s", errorMsg.c_str());
            return false;
        }
        return true;
    }
};

/**
 * @tc.name: CommonTest
 * @tc.desc: IsProcessExist.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, IsProcessExist, TestSize.Level1)
{
    const std::string procName = "hiprofiler_base_ut";
    int pid = 0;
    EXPECT_TRUE(COMMON::IsProcessExist(procName, pid));
    EXPECT_NE(pid, 0);
    const std::string invalidProcName = "ls";
    pid = 0;
    EXPECT_FALSE(COMMON::IsProcessExist(invalidProcName, pid));
    EXPECT_EQ(pid, 0);
}

/**
 * @tc.name: CommonTest
 * @tc.desc: StartProcess.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, StartAndKillProcess, TestSize.Level1)
{
    constexpr int waitProcMills = 300;
    std::string profilerProcName("hiprofilerd");
    std::vector<char*> argvVec;
    argvVec.push_back(const_cast<char*>(profilerProcName.c_str()));
    int lockFileFd = -1;
    EXPECT_FALSE(COMMON::IsProcessRunning(lockFileFd));
    int procPid = COMMON::StartProcess(profilerProcName, argvVec);
    EXPECT_NE(procPid, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    EXPECT_NE(COMMON::KillProcess(procPid), -1);
}

/**
 * @tc.name: CommonTest
 * @tc.desc: VerifyPath.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, VerifyPath, TestSize.Level1)
{
    std::string filePath = "/data/local/tmp/config.txt";
    std::vector<std::string> validPaths = {};
    EXPECT_TRUE(VerifyPath(filePath, validPaths));

    validPaths = { "/tmp/" };
    EXPECT_FALSE(VerifyPath(filePath, validPaths));

    validPaths = { "/tmp/", "/data/" };
    EXPECT_TRUE(VerifyPath(filePath, validPaths));

    validPaths = { "/tmp/", "/data/local/tmp/" };
    EXPECT_TRUE(VerifyPath(filePath, validPaths));

    filePath = "/data/local/tmpconfig.txt";
    validPaths = { "/tmp/", "/data/local/tmp/" };
    EXPECT_FALSE(VerifyPath(filePath, validPaths));
}

/**
 * @tc.name: CommonTest
 * @tc.desc: ReadFile.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, ReadFile, TestSize.Level1)
{
    std::string fileName = "/data/local/tmp/config.txt";
    std::string fileContent = "Hello world";
    EXPECT_TRUE(WriteFile(fileName, fileContent));

    // invalid path
    std::vector<std::string> validPaths = { "/tmp/" };
    std::string readContent;
    bool ret = ReadFile(fileName, validPaths, readContent);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(readContent.empty());

    // invalid file path
    fileName = "config.txt";
    validPaths = { "/tmp/", "/data/local/tmp/" };
    readContent.clear();
    ret = ReadFile(fileName, validPaths, readContent);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(readContent.empty());

    // invalid file name
    fileName = "configtmp.txt";
    validPaths = { "/tmp/", "/data/local/tmp/" };
    readContent.clear();
    ret = ReadFile(fileName, validPaths, readContent);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(readContent.empty());

    // valid path
    fileName = "/data/local/tmp/config.txt";
    validPaths = { "/tmp/", "/data/local/tmp/" };
    readContent.clear();
    ret = ReadFile(fileName, validPaths, readContent);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(readContent == fileContent);

    // delete file
    fileName = "/data/local/tmp/config.txt";
    std::string cmd = "rm " + fileName;
    system(cmd.c_str());
}

/**
 * @tc.name: CommonTest
 * @tc.desc: WriteFileFailed.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, WriteFileFailed, TestSize.Level1)
{
    std::string fileName = "/data/local/tmp/invalid/config.txt";
    std::string fileContent = "Hello world";
    EXPECT_FALSE(WriteFile(fileName, fileContent));
}

/**
 * @tc.name: CommonTest
 * @tc.desc: GetTimeStr.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, GetTimeStr, TestSize.Level1)
{
    std::string timeStr = GetTimeStr();
    EXPECT_FALSE(timeStr.empty());
}
} // namespace