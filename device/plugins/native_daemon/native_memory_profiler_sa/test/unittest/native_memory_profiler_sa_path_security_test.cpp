/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#ifndef NATIVE_MEMORY_PROFILER_SA_PATH_SECURITY_TEST_H
#define NATIVE_MEMORY_PROFILER_SA_PATH_SECURITY_TEST_H

#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "common.h"
#include "test_common.h"

using namespace testing::ext;

namespace {
const std::string TEST_BASE = "/data/local/tmp/path_sec_ut";
const std::string TEST_DIR = TEST_BASE + "/dir";
const std::string ESCAPE_TARGET = TEST_BASE + "/escape_target.htrace";
const std::string SYMLINK_PATH = TEST_DIR + "/symlink.htrace";
const std::string SRC_FILE = TEST_DIR + "/src.htrace";
constexpr mode_t DIR_MODE = S_IRWXU | S_IRGRP | S_IXGRP;
constexpr mode_t FILE_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

void CleanupTestEnv()
{
    (void)unlink(SYMLINK_PATH.c_str());
    (void)unlink(ESCAPE_TARGET.c_str());
    (void)unlink(SRC_FILE.c_str());
    (void)rmdir(TEST_DIR.c_str());
    (void)rmdir(TEST_BASE.c_str());
}

bool SetupTestEnv()
{
    CleanupTestEnv();
    if (mkdir(TEST_BASE.c_str(), DIR_MODE) != 0) {
        return false;
    }
    if (mkdir(TEST_DIR.c_str(), DIR_MODE) != 0) {
        return false;
    }
    int fd = open(ESCAPE_TARGET.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    if (fd < 0) {
        return false;
    }
    const char* original = "original content";
    (void)write(fd, original, strlen(original));
    (void)fsync(fd);
    close(fd);
    return true;
}
}

class PathSecurityTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        CleanupTestEnv();
    }
    void TearDown()
    {
        CleanupTestEnv();
    }
};

/* ==========================================================================
 * Test StartPidVoid typeFileName_ validation (Fix 1)
 * Directly tests the validation logic in StartPidVoid:
 *   typeFileName_ must not be empty and must not contain ".", "..", "/"
 * Since StartPidVoid requires full IPC environment, we test the
 * validation logic by verifying GetFullTracePath behavior with the
 * same typeFileName_ input (as CopyApiCollectorFile uses it).
 * ========================================================================== */

/**
 * @tc.name: TypeFileName_RejectDotDot
 * @tc.desc: GetFullTracePath rejects typeFileName_ containing ".."
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, TypeFileName_RejectDotDot, TestSize.Level1)
{
    using namespace COMMON;

    // typeFileName_ with ".." should be rejected
    std::string result = GetFullTracePath(200000, 1234,
        "com.test.app", "../../../etc/evil.htrace");

    EXPECT_TRUE(result.empty()) <<
            "typeFileName_ with '..' should be rejected";
}

/**
 * @tc.name: TypeFileName_RejectSlash
 * @tc.desc: GetFullTracePath rejects typeFileName_ containing "/"
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, TypeFileName_RejectSlash, TestSize.Level1)
{
    using namespace COMMON;

    std::string result = GetFullTracePath(200000, 1234,
        "com.test.app", "subdir/file.htrace");

    EXPECT_TRUE(result.empty()) <<
            "typeFileName_ with '/' should be rejected";
}

/**
 * @tc.name: TypeFileName_AcceptSafe
 * @tc.desc: GetFullTracePath accepts valid typeFileName_
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, TypeFileName_AcceptSafe, TestSize.Level1)
{
    using namespace COMMON;

    std::string result = GetFullTracePath(200000, 1234,
        "com.test.app", "native_memory_12345_htrace");

    EXPECT_FALSE(result.empty()) <<
            "valid typeFileName_ should be accepted";
}

/* ==========================================================================
 * Test GetFullTracePath (Fix 2: path normalization in CopyApiCollectorFile)
 * All cases call the actual COMMON::GetFullTracePath production function.
 * Before fix: returns full escaped path -> test fails
 * After fix: returns empty string -> test passes
 * ========================================================================== */

/**
 * @tc.name: GetFullTracePath_RejectDotDot
 * @tc.desc: Production function rejects filename with ".."
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, GetFullTracePath_RejectDotDot, TestSize.Level1)
{
    using namespace COMMON;

    std::string result = GetFullTracePath(200000, 1234,
        "com.test.app", "../../../etc/evil.htrace");

    EXPECT_TRUE(result.empty()) <<
            "path should be empty for dot-dot input";
}

/**
 * @tc.name: GetFullTracePath_RejectSlash
 * @tc.desc: Production function rejects filename with "/"
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, GetFullTracePath_RejectSlash, TestSize.Level1)
{
    using namespace COMMON;

    std::string result = GetFullTracePath(200000, 1234,
        "com.test.app", "subdir/evil.htrace");

    EXPECT_TRUE(result.empty()) <<
            "path should be empty for slash input";
}

/**
 * @tc.name: GetFullTracePath_AcceptSafe
 * @tc.desc: Production function accepts safe filename
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, GetFullTracePath_AcceptSafe, TestSize.Level1)
{
    using namespace COMMON;

    std::string result = GetFullTracePath(200000, 1234,
        "com.test.app", "native_memory_12345.htrace");

    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("native_memory_12345.htrace"),
        std::string::npos);
}

/* ==========================================================================
 * Test CopyApiCollectorFile destPath path normalization (Fix 1)
 * Simulates the CopyApiCollectorFile flow:
 *   1. typeFileName_ passed to GetFullTracePath -> destPath
 *   2. destPath validated (must not be empty after filename check)
 *   3. CopyFileFast called with validated destPath
 * Before fix: escaped destPath passed to CopyFileFast, attack succeeds
 * After fix: empty destPath rejected, CopyFileFast never called
 * ========================================================================== */

/**
 * @tc.name: DestPath_NormalizeAndValidate
 * @tc.desc: Simulated CopyApiCollectorFile rejects escaped destPath
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, DestPath_NormalizeAndValidate, TestSize.Level1)
{
    using namespace COMMON;

    // Simulate typeFileName_ from IPC input (attacker controlled)
    std::string typeFileName = "../../../etc/evil.htrace";

    // Step 1: CopyApiCollectorFile calls GetFullTracePath
    std::string destPath = GetFullTracePath(200000, 1234,
        "com.test.app", typeFileName);

    // Step 2: CopyApiCollectorFile validates destPath
    // Before fix: destPath="/data/app/el2/0/base/.../../../../etc/evil.htrace"
    // After fix: destPath="" (rejected by filename validation)
    if (destPath.empty()) {
        // Fixed path: validation caught it
        SUCCEED();
        return;
    }

    // Step 3: Verify realpath does not escape sandbox prefix
    const std::string sandboxPrefix = "/data/app/el2/";
    char resolved[PATH_MAX] = {0};
    if (realpath(destPath.c_str(), resolved) != nullptr) {
        std::string resolvedPath(resolved);
        EXPECT_TRUE(resolvedPath.find(sandboxPrefix) == 0 ||
            resolvedPath.find("..") == std::string::npos) <<
                "destPath escapes sandbox after realpath: " << resolvedPath;
    }

    // Step 4: If we got here without returning early, CopyFileFast would be called
    // This should not happen for malicious input
    FAIL() << "CopyApiCollectorFile should reject escaped destPath";
}

/**
 * @tc.name: DestPath_SafePathAccepted
 * @tc.desc: Simulated CopyApiCollectorFile accepts valid destPath
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, DestPath_SafePathAccepted, TestSize.Level1)
{
    using namespace COMMON;

    std::string typeFileName = "native_memory_12345.htrace";

    std::string destPath = GetFullTracePath(200000, 1234,
        "com.test.app", typeFileName);

    // CopyApiCollectorFile: destPath must not be empty
    EXPECT_FALSE(destPath.empty()) <<
            "valid destPath should not be rejected";

    // Verify path contains expected components
    EXPECT_NE(destPath.find("com.test.app"), std::string::npos);
    EXPECT_NE(destPath.find("native_memory_12345.htrace"),
        std::string::npos);
    EXPECT_EQ(destPath.find(".."), std::string::npos);
}

/* ==========================================================================
 * Test CopyFileFast O_NOFOLLOW (Fix 3)
 * Calls the actual COMMON::CopyFileFast production function.
 * Before fix: CopyFileFast opens symlink without O_NOFOLLOW,
 *             copies data to target -> copyResult=true, target corrupted
 * After fix: CopyFileFast opens with O_NOFOLLOW, open fails with ELOOP,
 *             returns false -> copyResult=false, target unchanged
 * ========================================================================== */

/**
 * @tc.name: CopyFileFast_BlockSymlink
 * @tc.desc: Production CopyFileFast blocks symlink destination
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, CopyFileFast_BlockSymlink, TestSize.Level1)
{
    if (!SetupTestEnv()) {
        GTEST_SKIP() << "Failed to setup test environment";
    }

    int ret = symlink(ESCAPE_TARGET.c_str(), SYMLINK_PATH.c_str());
    ASSERT_EQ(ret, 0) << "Failed to create symlink for test setup";

    std::ifstream originalFile(ESCAPE_TARGET);
    std::string originalContent(
        (std::istreambuf_iterator<char>(originalFile)),
        std::istreambuf_iterator<char>());
    originalFile.close();

    int srcFd = open(SRC_FILE.c_str(), O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
    ASSERT_GT(srcFd, 0);
    const char* srcData = "ATTACK_DATA";
    (void)write(srcFd, srcData, strlen(srcData));
    (void)fsync(srcFd);

    // Call ACTUAL production CopyFileFast
    bool copyResult = COMMON::CopyFileFast(srcFd, SYMLINK_PATH);

    std::ifstream targetFile(ESCAPE_TARGET);
    std::string targetContent(
        (std::istreambuf_iterator<char>(targetFile)),
        std::istreambuf_iterator<char>());
    targetFile.close();

    close(srcFd);
    (void)unlink(SRC_FILE.c_str());
    (void)unlink(SYMLINK_PATH.c_str());

    // Before fix: copyResult=true, targetContent=="ATTACK_DATA" -> FAILS
    // After fix: copyResult=false, targetContent=="original content" -> PASSES
    EXPECT_FALSE(copyResult) <<
            "CopyFileFast should fail when destination is a symlink";
    EXPECT_EQ(targetContent, originalContent) <<
            "target file should not be overwritten via symlink";
}

/**
 * @tc.name: CopyFileFast_NormalFile
 * @tc.desc: Production CopyFileFast works correctly for normal file
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, CopyFileFast_NormalFile, TestSize.Level1)
{
    if (!SetupTestEnv()) {
        GTEST_SKIP() << "Failed to setup test environment";
    }

    std::string normalDest = TEST_DIR + "/normal_dest.htrace";

    int srcFd = open(SRC_FILE.c_str(), O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
    ASSERT_GT(srcFd, 0);
    const char* srcData = "VALID_SOURCE_DATA";
    (void)write(srcFd, srcData, strlen(srcData));
    (void)fsync(srcFd);

    bool copyResult = COMMON::CopyFileFast(srcFd, normalDest);

    std::ifstream destFile(normalDest);
    std::string destContent(
        (std::istreambuf_iterator<char>(destFile)),
        std::istreambuf_iterator<char>());
    destFile.close();

    close(srcFd);
    (void)unlink(SRC_FILE.c_str());
    (void)unlink(normalDest.c_str());

    EXPECT_TRUE(copyResult) <<
            "CopyFileFast should succeed for normal file destination";
    EXPECT_EQ(destContent, "VALID_SOURCE_DATA");
}

/**
 * @tc.name: Safe_RejectsDotDotPath
 * @tc.desc: CheckNotExistsFilePathSafe rejects path containing ..
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, Safe_RejectsDotDotPath, TestSize.Level1)
{
    using namespace COMMON;
    auto ret = CheckNotExistsFilePathSafe("/data/local/tmp/../../../../etc/evil.htrace");
    EXPECT_FALSE(ret.first);
    EXPECT_TRUE(ret.second.empty());
}

/**
 * @tc.name: Safe_RejectsPathOutsideDataTmpDir
 * @tc.desc: CheckNotExistsFilePathSafe rejects absolute path outside /data/local/tmp/
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, Safe_RejectsPathOutsideDataTmpDir, TestSize.Level1)
{
    using namespace COMMON;
    auto ret = CheckNotExistsFilePathSafe("/etc/evil.htrace");
    EXPECT_FALSE(ret.first);
    EXPECT_TRUE(ret.second.empty());
}

/**
 * @tc.name: Safe_RejectsRelativePath
 * @tc.desc: CheckNotExistsFilePathSafe rejects relative path
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, Safe_RejectsRelativePath, TestSize.Level1)
{
    using namespace COMMON;
    EXPECT_FALSE(CheckNotExistsFilePathSafe("evil.htrace").first);
    EXPECT_FALSE(CheckNotExistsFilePathSafe("sub/evil.htrace").first);
}

/**
 * @tc.name: Safe_RejectsEmpty
 * @tc.desc: CheckNotExistsFilePathSafe rejects empty path
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, Safe_RejectsEmpty, TestSize.Level1)
{
    using namespace COMMON;
    EXPECT_FALSE(CheckNotExistsFilePathSafe("").first);
}

/**
 * @tc.name: Safe_RejectsTrailingSlash
 * @tc.desc: CheckNotExistsFilePathSafe rejects path ending with '/'
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, Safe_RejectsTrailingSlash, TestSize.Level1)
{
    using namespace COMMON;
    auto ret = CheckNotExistsFilePathSafe("/data/local/tmp/");
    EXPECT_FALSE(ret.first);
    EXPECT_TRUE(ret.second.empty());
}

/**
 * @tc.name: Safe_AcceptsValidPath
 * @tc.desc: CheckNotExistsFilePathSafe accepts valid path under /data/local/tmp/
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, Safe_AcceptsValidPath, TestSize.Level1)
{
    using namespace COMMON;
    auto ret = CheckNotExistsFilePathSafe("/data/local/tmp/native_memory_1234.htrace");
    EXPECT_TRUE(ret.first);
    EXPECT_EQ(ret.second, "/data/local/tmp/native_memory_1234.htrace");
}

/**
 * @tc.name: Safe_RejectsInvalidSuffix
 * @tc.desc: CheckNotExistsFilePathSafe rejects path whose suffix is not .htrace
 * @tc.type: FUNC
 */
HWTEST_F(PathSecurityTest, Safe_RejectsInvalidSuffix, TestSize.Level1)
{
    using namespace COMMON;
    auto ret = CheckNotExistsFilePathSafe("/data/local/tmp/native_memory_1234.txt");
    EXPECT_FALSE(ret.first);
    EXPECT_TRUE(ret.second.empty());
}

#endif // NATIVE_MEMORY_PROFILER_SA_PATH_SECURITY_TEST_H
