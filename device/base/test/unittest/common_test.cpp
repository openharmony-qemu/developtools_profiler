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
#include <fcntl.h>
#include <climits>

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
    std::string demoName;
    std::string demoFileName = "demo.htrace";
    std::string fullPath = COMMON::GetFullTracePath(demoUid, demoPid, demoName, demoFileName);
    // avoid outputing absolute path
    EXPECT_NE(fullPath, "");
    demoName = "demoName";
    fullPath = COMMON::GetFullTracePath(demoUid, demoPid, demoName, demoFileName);
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
 * @tc.name: GetRomSpaceInfo
 * @tc.desc: GetRomSpaceInfo
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, GetRomSpaceInfo, TestSize.Level1)
{
    uint64_t totalBytes = 0;
    uint64_t freeBytes = 0;
    bool result = COMMON::GetRomSpaceInfo(COMMON::DEFAULT_RW_DIR, totalBytes, freeBytes);
    EXPECT_TRUE(result);
    EXPECT_GT(totalBytes, 0ULL);
    EXPECT_GT(freeBytes, 0ULL);
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
    int srcFd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    bool ret = COMMON::CopyFileFast(srcFd, destName);
    close(srcFd);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CommonTest
 * @tc.desc: CopyFileToFdFastFromPath.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CopyFileToFdFastFromPath, TestSize.Level1)
{
    std::string srcName = "/data/local/tmp/copy_src.txt";
    std::string fileContent = "Hello world, CopyFileToFdFastFromPath";
    EXPECT_TRUE(WriteFile(srcName, fileContent));

    std::string destName = "/data/local/tmp/copy_dest.txt";
    int destFd = open(destName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    EXPECT_GE(destFd, 0);
    bool ret = COMMON::CopyFileToFdFastFromPath(srcName, destFd);
    close(destFd);
    EXPECT_TRUE(ret);

    std::vector<std::string> validPaths = { "/data/local/tmp/" };
    std::string readContent;
    EXPECT_TRUE(COMMON::ReadFile(destName, validPaths, readContent));
    EXPECT_EQ(readContent, fileContent);

    std::string cmd = "rm " + srcName + " " + destName;
    system(cmd.c_str());
}

/**
 * @tc.name: CommonTest
 * @tc.desc: CopyFileToFdFastFromPath with invalid source path.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CopyFileToFdFastFromPathInvalid, TestSize.Level1)
{
    std::string destName = "/data/local/tmp/copy_dest_invalid.txt";
    int destFd = open(destName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    EXPECT_GE(destFd, 0);
    EXPECT_FALSE(COMMON::CopyFileToFdFastFromPath("/data/local/tmp/nonexistent_src.txt", destFd));
    close(destFd);

    std::string cmd = "rm -f " + destName;
    system(cmd.c_str());
}

/**
 * @tc.name: CommonTest
 * @tc.desc: CopyFileToFdFastFromPath with a symlink source path should fail (O_NOFOLLOW).
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CopyFileToFdFastFromPathSymlink, TestSize.Level1)
{
    std::string realName = "/data/local/tmp/copy_real.txt";
    std::string fileContent = "Hello world, symlink";
    EXPECT_TRUE(WriteFile(realName, fileContent));

    std::string linkName = "/data/local/tmp/copy_link.txt";
    unlink(linkName.c_str());
    ASSERT_EQ(symlink(realName.c_str(), linkName.c_str()), 0);
    struct stat st;
    ASSERT_EQ(lstat(linkName.c_str(), &st), 0);
    ASSERT_TRUE(S_ISLNK(st.st_mode));

    std::string destName = "/data/local/tmp/copy_dest_symlink.txt";
    int destFd = open(destName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    EXPECT_GE(destFd, 0);
    EXPECT_FALSE(COMMON::CopyFileToFdFastFromPath(linkName, destFd));
    close(destFd);

    std::string cmd = "rm -f " + realName + " " + linkName + " " + destName;
    system(cmd.c_str());
}

/**
 * @tc.name: CommonTest
 * @tc.desc: CopyFileFastFromPath with normalized src path validation.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, CopyFileFastFromPathInvalidSrc, TestSize.Level1)
{
    std::string srcName = "/data/local/tmp/fast_src.txt";
    std::string fileContent = "Hello world, CopyFileFastFromPath";
    EXPECT_TRUE(WriteFile(srcName, fileContent));

    std::string destName = "/data/local/tmp/fast_dest.txt";
    EXPECT_TRUE(COMMON::CopyFileFastFromPath(srcName, destName));

    EXPECT_FALSE(COMMON::CopyFileFastFromPath("relative/fast_src.txt", destName));
    EXPECT_FALSE(COMMON::CopyFileFastFromPath("/data/local/tmp/sub/../../fast_src.txt", destName));
    EXPECT_FALSE(COMMON::CopyFileFastFromPath("/data/local/tmp/", destName));
    EXPECT_FALSE(COMMON::CopyFileFastFromPath("", destName));

    std::string cmd = "rm -f " + srcName + " " + destName;
    system(cmd.c_str());
}

/**
 * @tc.name: CommonTest
 * @tc.desc: NormalizeAndCheckSrcPath path validation and normalization.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, NormalizeAndCheckSrcPath, TestSize.Level1)
{
    std::string srcName = "/data/local/tmp/normalize_src.txt";
    std::string fileContent = "Hello world, NormalizeAndCheckSrcPath";
    EXPECT_TRUE(WriteFile(srcName, fileContent));

    std::string normalized;
    EXPECT_TRUE(NormalizeAndCheckSrcPath(srcName, normalized));
    EXPECT_EQ(normalized, srcName);

    EXPECT_FALSE(NormalizeAndCheckSrcPath("", normalized));
    EXPECT_EQ(normalized, "");

    EXPECT_FALSE(NormalizeAndCheckSrcPath("relative/path.txt", normalized));

    EXPECT_FALSE(NormalizeAndCheckSrcPath("/data/local/tmp/sub/../../normalize_src.txt", normalized));
    EXPECT_FALSE(NormalizeAndCheckSrcPath("/data/local/tmp/..", normalized));
    EXPECT_FALSE(NormalizeAndCheckSrcPath("/data/local/tmp/.", normalized));

    EXPECT_FALSE(NormalizeAndCheckSrcPath("/data/local/tmp/", normalized));

    EXPECT_FALSE(NormalizeAndCheckSrcPath("/nonexistent_dir_xyz/normalize_src.txt", normalized));

    std::string longPath = "/data/local/tmp/" + std::string(PATH_MAX, 'a');
    EXPECT_FALSE(NormalizeAndCheckSrcPath(longPath, normalized));

    std::string cmd = "rm -f " + srcName;
    system(cmd.c_str());
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

/**
 * @tc.name: DeleteFiles
 * @tc.desc: DeleteFiles
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, DeleteFiles, TestSize.Level1)
{
    std::string dir = "/data/local/tmp/";
    std::string content = "hello world";
    std::vector<std::string> files = {
        "del_test_a.htrace",
        "del_test_b.htrace",
        "del_test_c.txt",
        "keep_test_d.htrace"
    };
    for (const auto& f : files) {
        WriteFile(dir + f, content);
    }

    auto filter = [](const std::string& filename) {
        return filename.find("del_test_") == 0 && filename.find(".htrace") != std::string::npos;
    };
    bool ret = COMMON::DeleteFiles(dir, filter);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(IsFileExists((dir + "del_test_a.htrace").c_str()));
    EXPECT_FALSE(IsFileExists((dir + "del_test_b.htrace").c_str()));
    EXPECT_TRUE(IsFileExists((dir + "del_test_c.txt").c_str()));
    EXPECT_TRUE(IsFileExists((dir + "keep_test_d.htrace").c_str()));

    for (const auto& f : files) {
        std::string cmd = "rm " + dir + f;
        system(cmd.c_str());
    }
}

/**
 * @tc.name: DeleteFilesInvalidParam
 * @tc.desc: DeleteFiles with invalid parameters
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, DeleteFilesInvalidParam, TestSize.Level1)
{
    EXPECT_FALSE(COMMON::DeleteFiles("", nullptr));
    EXPECT_FALSE(COMMON::DeleteFiles("", [](const std::string&) { return true; }));
    EXPECT_FALSE(COMMON::DeleteFiles("/nonexistent_dir", [](const std::string&) { return true; }));
    EXPECT_FALSE(COMMON::DeleteFiles("/data/local/tmp/", nullptr));
}

/**
 * @tc.name: GetNSpid
 * @tc.desc: GetNSpid with invalid pid (file not open) and valid pid (current process).
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, GetNSpid, TestSize.Level1)
{
    // invalid pid: /proc/<pid>/status does not exist, file cannot be opened
    EXPECT_EQ(COMMON::GetNSpid(999999999), -1);

    // valid pid: not container process
    int32_t pid = static_cast<int32_t>(getpid());
    int32_t nspid = COMMON::GetNSpid(pid);
    EXPECT_EQ(nspid, -1);
}

/**
 * @tc.name: StringMatch
 * @tc.desc: StringMatch
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, StringMatch, TestSize.Level1)
{
    EXPECT_FALSE(COMMON::StringMatch("hello_world", "hello_", "_world"));
    EXPECT_TRUE(COMMON::StringMatch("abc.txt", "abc", ".txt"));
    EXPECT_TRUE(COMMON::StringMatch("abc", "", ""));
    EXPECT_TRUE(COMMON::StringMatch("prefixmiddle", "prefix", ""));
    EXPECT_TRUE(COMMON::StringMatch("middlesuffix", "", "suffix"));
    EXPECT_FALSE(COMMON::StringMatch("ab", "abc", ""));
    EXPECT_FALSE(COMMON::StringMatch("ab", "", "abc"));
    EXPECT_FALSE(COMMON::StringMatch("abc", "xyz", ""));
    EXPECT_FALSE(COMMON::StringMatch("abc", "", "xyz"));
    EXPECT_FALSE(COMMON::StringMatch("abcde", "ab", "de_long_suffix"));
}
} // namespace