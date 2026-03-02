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
#include <gtest/gtest.h>
#include "common.h"
#include "logging.h"
#include <malloc.h>
#include <sys/stat.h>

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
            PROFILER_LOG_ERROR(LOG_CORE, "WriteFile: fopen() fail, %s, %s", filePath.c_str(), errorMsg.c_str());
            return false;
        }

        size_t len = fwrite(const_cast<char*>(fileContent.c_str()), 1, fileContent.length(), file);
        if (len != fileContent.length()) {
            std::string errorMsg = GetErrorMsg();
            PROFILER_LOG_ERROR(LOG_CORE, "WriteFile: fwrite() fail, %s", errorMsg.c_str());
            (void)fclose(file);
            return false;
        }

        if (fflush(file) == EOF) {
            std::string errorMsg = GetErrorMsg();
            PROFILER_LOG_ERROR(LOG_CORE, "WriteFile: fflush() error = %s", errorMsg.c_str());
            (void)fclose(file);
            return false;
        }

        fsync(fileno(file));
        if (fclose(file) != 0) {
            std::string errorMsg = GetErrorMsg();
            PROFILER_LOG_ERROR(LOG_CORE, "CreateConfigFile: fclose() error = %s", errorMsg.c_str());
            return false;
        }
        // delay, avoid generating same update time.
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return true;
    }

    bool IsFileExists(const char* filePath)
    {
        struct stat buffer;
        return stat(filePath, &buffer) == 0;
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
 * @tc.desc: GetUidGidFromPid.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, GetUidGidFromPid, TestSize.Level1)
{
    const std::string procName = "hiprofiler_base_ut";
    int pid = 0;
    EXPECT_TRUE(COMMON::IsProcessExist(procName, pid));
    EXPECT_NE(pid, 0);
    uid_t uid = 0;
    gid_t gid = 0;
    EXPECT_TRUE(COMMON::GetUidGidFromPid(static_cast<pid_t>(pid), uid, gid));
    EXPECT_EQ(uid, 0);
    EXPECT_EQ(gid, 0);

    const std::string hiviewName = "hiview";
    pid = 0;
    EXPECT_TRUE(COMMON::IsProcessExist(hiviewName, pid));
    EXPECT_NE(pid, 0);
    uid = 0;
    gid = 0;
    EXPECT_TRUE(COMMON::GetUidGidFromPid(static_cast<pid_t>(pid), uid, gid));
    EXPECT_NE(uid, 0);
    EXPECT_NE(gid, 0);
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
    int procPid = COMMON::StartProcess("/system/bin/hiprofilerd", argvVec);
    EXPECT_NE(procPid, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    EXPECT_NE(COMMON::KillProcess(procPid), -1);
}

/**
 * @tc.name: CommonTest
 * @tc.desc: StartProcess.Start process name is not exit or illegal
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, StartNoexitProcess, TestSize.Level1)
{
    std::vector<char*> argvVec;
    EXPECT_EQ(COMMON::StartProcess("/system/bin/test_thread", argvVec), -1);
    argvVec.push_back(const_cast<char*>(""));
    EXPECT_EQ(COMMON::StartProcess("/system/bin/test_thread", argvVec), -1);
    argvVec.push_back(const_cast<char*>("test||"));
    EXPECT_EQ(COMMON::StartProcess("/system/bin/hiprofilerd", argvVec), -1);
    argvVec.clear();
    EXPECT_EQ(COMMON::KillProcess(-1), -1);
    EXPECT_EQ(COMMON::KillProcess(999999999), -1);
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

    // Absolute path
    fileName = "data/log/bbox";
    validPaths = { "/tmp/", "/data/local/tmp/" };
    readContent.clear();
    ret = ReadFile(fileName, validPaths, readContent);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(readContent.empty());

// Relative path
    fileName = "./log/faultlog/faultlogger";
    validPaths = { "/tmp/", "/data/local/tmp/" };
    readContent.clear();
    ret = ReadFile(fileName, validPaths, readContent);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(readContent.empty());
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
 * @tc.desc: CanonicalizeSpecPath.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CanonicalizeSpecPath, TestSize.Level0)
{
    EXPECT_EQ(CanonicalizeSpecPath(nullptr), "");
    EXPECT_EQ(CanonicalizeSpecPath("/data/local/tmp/test/../test.txt"), "");
    EXPECT_EQ(CanonicalizeSpecPath(""), "");
    EXPECT_EQ(CanonicalizeSpecPath("/data/local/tmp/nonexistent.txt"), "/data/local/tmp/nonexistent.txt");
    string largePath = "./";
    for (int i = 0; i < 512; i++) { // 512: loop size
        largePath += "testpath";
    }
    largePath += ".txt";
    EXPECT_EQ(CanonicalizeSpecPath(largePath.c_str()), "");
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

/**
 * @tc.name: CommonTest
 * @tc.desc: PluginWriteToHisysevent.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, PluginWriteToHisysevent001, TestSize.Level1)
{
    int ret = PluginWriteToHisysevent("CPU_PLUGIN", "tdd_test", "test_args", RET_SUCC, "success");
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CommonTest
 * @tc.desc: PluginWriteToHisysevent.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, PluginWriteToHisysevent002, TestSize.Level1)
{
    int ret = PluginWriteToHisysevent("DISKIO_PLUGIN", "tdd_test", "test_args", RET_SUCC, "success");
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CommonTest
 * @tc.desc: test CheckSubscribeVersion and PrintMallinfoLog
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CheckSubscribeVersion, TestSize.Level1)
{
    static struct mallinfo2 miStart = {0};
    miStart.arena = 1;
    miStart.ordblks = 2;
    miStart.smblks = 3;
    miStart.hblks = 4;
    miStart.hblkhd = 5;
    miStart.usmblks = 6;
    miStart.fsmblks = 7;
    miStart.uordblks = 8;
    miStart.fordblks = 9;
    miStart.keepcost = 10;
    std::string testStr = "test:";
    COMMON::PrintMallinfoLog(testStr, miStart);
    EXPECT_EQ(COMMON::CheckSubscribeVersion("0.5_2"), false);
    EXPECT_EQ(COMMON::CheckSubscribeVersion("1.0"), true);
}

/**
 * @tc.name: CommonTest
 * @tc.desc: StartProcess.CustomPopen command name is not exit or illegal
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CustomPopen, TestSize.Level1)
{
    std::vector<std::string> fullCmdTest;
    fullCmdTest.push_back("/system/bin/test");
    fullCmdTest.push_back("hisysevent");
    fullCmdTest.push_back("-rd");
    volatile pid_t childPid = -1;
    int pipeFds[2] = {-1, -1};
    FILE* fpr = COMMON::CustomPopen(fullCmdTest, nullptr, pipeFds, childPid, true);
    EXPECT_EQ(fpr, nullptr);
    EXPECT_EQ(COMMON::CustomPopen(fullCmdTest, "w", pipeFds, childPid), nullptr);
    fullCmdTest.clear();
    fullCmdTest.push_back("/system/bin/hiprofilerd");
    fullCmdTest.push_back("hiprofilerd&");
    fullCmdTest.push_back("-rd");
    EXPECT_EQ(COMMON::CustomPopen(fullCmdTest, "w", pipeFds, childPid), nullptr);
}

/**
 * @tc.name: CommonTest
 * @tc.desc: IsNumeric and SplitString
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, IsNumeric, TestSize.Level1)
{
    EXPECT_EQ(COMMON::IsNumeric("test"), false);
    EXPECT_EQ(COMMON::IsNumeric("1111test"), false);
    EXPECT_EQ(COMMON::IsNumeric("1111"), true);

    string str = "";
    string seq = "_";
    std::vector<string> ret;
    COMMON::SplitString(str, seq, ret);
    EXPECT_EQ(ret.size(), 0);
}
/**
 * @tc.name: AdaptSandboxPath
 * @tc.desc: AdaptSandboxPath
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, AdaptSandboxPath, TestSize.Level1)
{
    std::string path = "/data/storage/test";
    COMMON::AdaptSandboxPath(path, 1);
    EXPECT_EQ(path, "/proc/1/root/data/storage/test");
}

/**
 * @tc.name: GetBundleNameByPid
 * @tc.desc: GetBundleNameByPid
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, GetBundleNameByPid, TestSize.Level1)
{
    const std::string procName = "hiprofiler_base_ut";
    int pid = 0;
    EXPECT_TRUE(COMMON::IsProcessExist(procName, pid));
    
    std::string bundleName = COMMON::GetBundleNameByPid(pid);
    EXPECT_NE(bundleName, "");
}

/**
 * @tc.name: GetFullTracePath
 * @tc.desc: GetFullTracePath
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, GetFullTracePath, TestSize.Level1)
{
    pid_t demoUid = 100;
    pid_t demoPid = 1;
    std::string demoFileName = "demo.htrace";
    std::string fullPath = COMMON::GetFullTracePath(demoUid, demoPid, demoFileName);
    // avoid outputing absolute path
    EXPECT_NE(fullPath, "");
}

/**
 * @tc.name: GetFreeRomPercent
 * @tc.desc: GetFreeRomPercent
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, GetFreeRomPercent, TestSize.Level1)
{
    double freePercent = COMMON::GetFreeRomPercent();
    // default /data/
    EXPECT_GT(freePercent, 0.0);
}

/**
 * @tc.name: CopyFileFast
 * @tc.desc: CopyFileFast
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CopyFileFast, TestSize.Level1)
{
    std::string fileName = "/data/local/tmp/config.txt";
    std::string fileContent = "Hello world";
    EXPECT_TRUE(WriteFile(fileName, fileContent));

    std::string destName = "/data/local/tmp/config_cp.txt";
    bool ret = COMMON::CopyFileFast(fileName, destName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetBundleNameByPid
 * @tc.desc: GetBundleNameByPid
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CleanOldenFiles, TestSize.Level1)
{
    std::string dir = "/data/local/tmp/";
    std::vector<std::string> traceFileArray = {
        "fd-com.xxx.xxx-1000-20060101_000000.htrace",
        "fd-com.xxx.xxx-1000-20060101_000001.htrace",
        "fd-com.xxx.xxx-1000-20060101_000002.htrace",
        "fd-com.xxx.xxx-1000-20060101_000003.htrace",
        "fd-com.xxx.xxx-1000-20060101_000004.htrace",
        "fd-com.xxx.xxx-1000-20060101_000005.htrace"
    };
    std::string content = "hello world";
    for (int i = 0; i < traceFileArray.size(); i++) {
        std::string fullPath = dir + traceFileArray[i];
        WriteFile(fullPath, content);
    }
    bool ret = COMMON::CleanOldenFiles(dir);
    EXPECT_TRUE(ret);

    std::string deletedFile = dir + traceFileArray[0];
    ret = IsFileExists(deletedFile.c_str());
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CleanCmdOldenTraceFiles
 * @tc.desc: CleanCmdOldenTraceFiles
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CleanCmdOldenTraceFiles, TestSize.Level1)
{
    std::string dir = "/data/local/tmp/";
    std::vector<std::string> traceFileArray = {
        "1.htrace",
        "2.htrace",
        "3.htrace",
        "4.htrace",
        "5.htrace",
        "6.htrace"
    };
    std::string content = "hello world";
    for (int i = 0; i < traceFileArray.size(); i++) {
        std::string fullPath = dir + traceFileArray[i];
        WriteFile(fullPath, content);
    }
    bool ret = COMMON::CleanCmdOldenTraceFiles(dir);
    EXPECT_TRUE(ret);

    std::string deletedFile = dir + traceFileArray[0];
    ret = IsFileExists(deletedFile.c_str());
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CreateFile
 * @tc.desc: CreateFile
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CreateFile, TestSize.Level1)
{
    std::string filePath = "/data/local/tmp/demo.htrace";
    bool ret = COMMON::CreateFile(filePath);
    EXPECT_TRUE(ret);
}
} // namespace