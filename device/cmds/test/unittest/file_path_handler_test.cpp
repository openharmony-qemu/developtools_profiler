/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <cstdint>
#include <gtest/gtest.h>
#include "file_path_handler.h"
#include "profiler_service.grpc.pb.h"

using namespace testing::ext;

namespace {
const std::string TEST_OUTPUT_FILE("/data/local/tmp/test_output.htrace");
const std::string TEST_CONFIG_FILE("test_config.txt");
const std::string TEST_COMPOSITE_OUTPUT_FILE("/data/local/tmp/hiprofiler_data.zip");
const std::string TEST_COMPOSITE_INVALID_FILE("/data/local/tmp/hiprofiler_data.htrace");
const std::string TEST_SANDBOX_PATH_ROOT("/storage/Users/currentUser/");
const std::string TEST_HOME_PATH_PREFIX("~/");
constexpr int32_t TEST_SANDBOX_CALLING_UID = 20000001;
constexpr int32_t TEST_NON_SANDBOX_CALLING_UID = 20000000;
const std::string TEST_PHYSICAL_PATH_ROOT("/storage/media/100/local/files/Docs/");
}

class FilePathHandlerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: TestNormalFilePathHandler
 * @tc.desc: Test normal file path handler functionality
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestNormalFilePathHandler, Function | MediumTest | Level0)
{
    auto handler = FilePathHandlerFactory::CreateHandler();
    ASSERT_NE(handler, nullptr);
    
    // Test GetConfigFilePath
    std::string configPath = handler->GetConfigFilePath(TEST_CONFIG_FILE);
    EXPECT_FALSE(configPath.empty());
    
    // Test GetValidPaths
    std::vector<std::string> validPaths = handler->GetValidPaths();
    EXPECT_FALSE(validPaths.empty());
}

/**
 * @tc.name: TestFilePathHandlerFactory
 * @tc.desc: Test file path handler factory creates appropriate handler
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestFilePathHandlerFactory, Function | MediumTest | Level0)
{
    auto handler = FilePathHandlerFactory::CreateHandler();
    ASSERT_NE(handler, nullptr);
    
    std::string configPath = handler->GetConfigFilePath(TEST_CONFIG_FILE);
    EXPECT_FALSE(configPath.empty());
}

/**
 * @tc.name: TestNormalHandlerConfigPath
 * @tc.desc: Test normal handler config path resolution
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestNormalHandlerConfigPath, Function | MediumTest | Level0)
{
    NormalFilePathHandler handler;
    
    // Test with relative path
    std::string relativePath = handler.GetConfigFilePath("config.txt");
    EXPECT_FALSE(relativePath.empty());
    EXPECT_NE(relativePath.find("config.txt"), std::string::npos);
    
    // Test with absolute path
    std::string absolutePath = handler.GetConfigFilePath("/data/local/tmp/config.txt");
    EXPECT_EQ(absolutePath, "/data/local/tmp/config.txt");
}

/**
 * @tc.name: TestNormalHandlerValidPaths
 * @tc.desc: Test normal handler valid paths
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestNormalHandlerValidPaths, Function | MediumTest | Level0)
{
    NormalFilePathHandler handler;
    
    std::vector<std::string> validPaths = handler.GetValidPaths();
    EXPECT_FALSE(validPaths.empty());
    EXPECT_NE(validPaths[0].find("/data/local/tmp"), std::string::npos);
}

/**
 * @tc.name: TestCompositeNormalHandlerEmptyOutput
 * @tc.desc: Test CompositeNormalFilePathHandler with empty output uses default with .zip suffix
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeNormalHandlerEmptyOutput, Function | MediumTest | Level0)
{
    CompositeNormalFilePathHandler handler;
    ProfilerSessionConfig sessionConfig;

    bool ret = handler.HandleOutputFilePath("", &sessionConfig);
    EXPECT_TRUE(ret);
    ASSERT_TRUE(sessionConfig.result_file().length() > 4);
    EXPECT_EQ(sessionConfig.result_file().substr(sessionConfig.result_file().length() - 4), ".zip");
    EXPECT_NE(sessionConfig.result_file().find("/data/local/tmp/"), std::string::npos);
    EXPECT_NE(sessionConfig.result_file().find("hiprofiler_data_"), std::string::npos);
}

/**
 * @tc.name: TestCompositeNormalHandlerOutputWithZip
 * @tc.desc: Test CompositeNormalFilePathHandler with valid .zip output path
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeNormalHandlerOutputWithZip, Function | MediumTest | Level0)
{
    CompositeNormalFilePathHandler handler;
    ProfilerSessionConfig sessionConfig;

    bool ret = handler.HandleOutputFilePath(TEST_COMPOSITE_OUTPUT_FILE, &sessionConfig);
    EXPECT_TRUE(ret);
    EXPECT_EQ(sessionConfig.result_file(), TEST_COMPOSITE_OUTPUT_FILE);
}

/**
 * @tc.name: TestCompositeNormalHandlerOutputWithoutZip
 * @tc.desc: Test CompositeNormalFilePathHandler rejects non-.zip output path
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeNormalHandlerOutputWithoutZip, Function | MediumTest | Level0)
{
    CompositeNormalFilePathHandler handler;
    ProfilerSessionConfig sessionConfig;

    bool ret = handler.HandleOutputFilePath(TEST_COMPOSITE_INVALID_FILE, &sessionConfig);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: TestCompositeNormalHandlerInvalidPath
 * @tc.desc: Test CompositeNormalFilePathHandler rejects path not under /data/local/tmp/
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeNormalHandlerInvalidPath, Function | MediumTest | Level0)
{
    CompositeNormalFilePathHandler handler;
    ProfilerSessionConfig sessionConfig;

    bool ret = handler.HandleOutputFilePath("/system/test.zip", &sessionConfig);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: TestCompositeNormalHandlerShortZipPath
 * @tc.desc: Test CompositeNormalFilePathHandler rejects path too short to be valid .zip
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeNormalHandlerShortZipPath, Function | MediumTest | Level0)
{
    CompositeNormalFilePathHandler handler;
    ProfilerSessionConfig sessionConfig;

    bool ret = handler.HandleOutputFilePath("/data/local/tmp/.zip", &sessionConfig);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: TestCompositeNormalHandlerConfigPath
 * @tc.desc: Test CompositeNormalFilePathHandler inherits config path resolution from NormalFilePathHandler
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeNormalHandlerConfigPath, Function | MediumTest | Level0)
{
    CompositeNormalFilePathHandler handler;

    std::string relativePath = handler.GetConfigFilePath("config.txt");
    EXPECT_NE(relativePath.find("config.txt"), std::string::npos);

    std::string absolutePath = handler.GetConfigFilePath("/data/local/tmp/config.txt");
    EXPECT_EQ(absolutePath, "/data/local/tmp/config.txt");
}

/**
 * @tc.name: TestCompositeNormalHandlerValidPaths
 * @tc.desc: Test CompositeNormalFilePathHandler inherits valid paths from NormalFilePathHandler
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeNormalHandlerValidPaths, Function | MediumTest | Level0)
{
    CompositeNormalFilePathHandler handler;

    std::vector<std::string> validPaths = handler.GetValidPaths();
    EXPECT_FALSE(validPaths.empty());
    EXPECT_NE(validPaths[0].find("/data/local/tmp"), std::string::npos);
}

/**
 * @tc.name: TestCompositeSandboxHandlerConfigPath
 * @tc.desc: Test CompositeSandboxFilePathHandler inherits config path resolution from SandboxFilePathHandler
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeSandboxHandlerConfigPath, Function | MediumTest | Level0)
{
    CompositeSandboxFilePathHandler handler;

    std::string relativePath = handler.GetConfigFilePath("config.txt");
    EXPECT_NE(relativePath.find("config.txt"), std::string::npos);

    std::string absolutePath = handler.GetConfigFilePath("/data/local/tmp/config.txt");
    EXPECT_EQ(absolutePath, "/data/local/tmp/config.txt");
}

/**
 * @tc.name: TestCompositeSandboxHandlerValidPaths
 * @tc.desc: Test CompositeSandboxFilePathHandler inherits valid paths from SandboxFilePathHandler
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestCompositeSandboxHandlerValidPaths, Function | MediumTest | Level0)
{
    CompositeSandboxFilePathHandler handler;

    std::vector<std::string> validPaths = handler.GetValidPaths();
    EXPECT_FALSE(validPaths.empty());
    EXPECT_NE(validPaths[0].find("/storage/Users/currentUser/"), std::string::npos);
}

/**
 * @tc.name: TestFactoryCreateCompositeHandler
 * @tc.desc: Test FilePathHandlerFactory creates composite handler when composite=true
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestFactoryCreateCompositeHandler, Function | MediumTest | Level0)
{
    auto handler = FilePathHandlerFactory::CreateHandler(true);
    ASSERT_NE(handler, nullptr);

    ProfilerSessionConfig sessionConfig;
    bool ret = handler->HandleOutputFilePath("", &sessionConfig);
    EXPECT_TRUE(ret);
    EXPECT_EQ(sessionConfig.result_file().substr(sessionConfig.result_file().length() - 4), ".zip");
}

/**
 * @tc.name: TestFactoryCreateNonCompositeHandler
 * @tc.desc: Test FilePathHandlerFactory creates non-composite handler when composite=false
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestFactoryCreateNonCompositeHandler, Function | MediumTest | Level0)
{
    auto handler = FilePathHandlerFactory::CreateHandler(false);
    ASSERT_NE(handler, nullptr);

    ProfilerSessionConfig sessionConfig;
    bool ret = handler->HandleOutputFilePath("", &sessionConfig);
    EXPECT_TRUE(ret);
    EXPECT_NE(sessionConfig.result_file().substr(sessionConfig.result_file().length() - 4), ".zip");
}

/**
 * @tc.name: TestConvertSandboxPathToPhysical
 * @tc.desc: Convert /storage/Users/currentUser/ outfile to physical Docs path with sandbox uid
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertSandboxPathToPhysical, Function | MediumTest | Level1)
{
    std::string sandboxPath = TEST_SANDBOX_PATH_ROOT + "perf.data";
    std::string physicalPath = SandboxFilePathHandler::ConvertToPhysicalPath(sandboxPath, TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(physicalPath, TEST_PHYSICAL_PATH_ROOT + "perf.data");
}

/**
 * @tc.name: TestConvertSandboxSubdirToPhysical
 * @tc.desc: Convert sandbox subdirectory outfile to the corresponding physical path
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertSandboxSubdirToPhysical, Function | MediumTest | Level1)
{
    std::string sandboxPath = TEST_SANDBOX_PATH_ROOT + "hiperf/perf.data";
    std::string physicalPath = SandboxFilePathHandler::ConvertToPhysicalPath(sandboxPath, TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(physicalPath, TEST_PHYSICAL_PATH_ROOT + "hiperf/perf.data");
}

/**
 * @tc.name: TestConvertHomePathToPhysical
 * @tc.desc: Convert ~/ outfile to physical Docs path with sandbox uid
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertHomePathToPhysical, Function | MediumTest | Level1)
{
    std::string homePath = TEST_HOME_PATH_PREFIX + "perf.data";
    std::string physicalPath = SandboxFilePathHandler::ConvertToPhysicalPath(homePath, TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(physicalPath, TEST_PHYSICAL_PATH_ROOT + "perf.data");
}

/**
 * @tc.name: TestConvertSandboxPathNonAppUidUnchanged
 * @tc.desc: Keep sandbox path unchanged when callingUid is not a sandbox app
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertSandboxPathNonAppUidUnchanged, Function | MediumTest | Level1)
{
    std::string sandboxPath = TEST_SANDBOX_PATH_ROOT + "perf.data";
    std::string resultPath = SandboxFilePathHandler::ConvertToPhysicalPath(sandboxPath, TEST_NON_SANDBOX_CALLING_UID);
    EXPECT_EQ(resultPath, sandboxPath);
}

/**
 * @tc.name: TestConvertNormalPathUnchanged
 * @tc.desc: Keep /data/local/tmp path unchanged even with sandbox uid
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertNormalPathUnchanged, Function | MediumTest | Level1)
{
    std::string normalPath = TEST_OUTPUT_FILE;
    std::string resultPath = SandboxFilePathHandler::ConvertToPhysicalPath(normalPath, TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(resultPath, normalPath);
}

/**
 * @tc.name: TestConvertEmptyPathUnchanged
 * @tc.desc: Keep empty path unchanged
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertEmptyPathUnchanged, Function | MediumTest | Level2)
{
    std::string resultPath = SandboxFilePathHandler::ConvertToPhysicalPath("", TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(resultPath, "");
}

/**
 * @tc.name: TestConvertAlreadyPhysicalPathUnchanged
 * @tc.desc: Keep already converted physical path unchanged
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertAlreadyPhysicalPathUnchanged, Function | MediumTest | Level2)
{
    std::string physicalPath = TEST_PHYSICAL_PATH_ROOT + "perf.data";
    std::string resultPath = SandboxFilePathHandler::ConvertToPhysicalPath(physicalPath, TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(resultPath, physicalPath);
}

/**
 * @tc.name: TestConvertSandboxPathMapsUserId
 * @tc.desc: Physical path userId is callingUid / 200000
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertSandboxPathMapsUserId, Function | MediumTest | Level1)
{
    constexpr int32_t callingUid = 40000001;
    std::string sandboxPath = TEST_SANDBOX_PATH_ROOT + "perf.data";
    std::string physicalPath = SandboxFilePathHandler::ConvertToPhysicalPath(sandboxPath, callingUid);
    EXPECT_EQ(physicalPath, "/storage/media/200/local/files/Docs/perf.data");
}

/**
 * @tc.name: TestIsSandboxAppUid
 * @tc.desc: callingUid greater than 20000000 is treated as a sandbox app
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestIsSandboxAppUid, Function | MediumTest | Level1)
{
    EXPECT_TRUE(SandboxFilePathHandler::IsSandboxAppUid(TEST_SANDBOX_CALLING_UID));
    EXPECT_FALSE(SandboxFilePathHandler::IsSandboxAppUid(TEST_NON_SANDBOX_CALLING_UID));
    EXPECT_FALSE(SandboxFilePathHandler::IsSandboxAppUid(0));
    EXPECT_FALSE(SandboxFilePathHandler::IsSandboxAppUid(-1));
}

/**
 * @tc.name: TestConvertHomePathNonAppUidUnchanged
 * @tc.desc: Keep ~/ path unchanged when callingUid is not a sandbox app
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertHomePathNonAppUidUnchanged, Function | MediumTest | Level1)
{
    std::string homePath = TEST_HOME_PATH_PREFIX + "perf.data";
    std::string resultPath = SandboxFilePathHandler::ConvertToPhysicalPath(homePath, TEST_NON_SANDBOX_CALLING_UID);
    EXPECT_EQ(resultPath, homePath);
}

/**
 * @tc.name: TestConvertTildeWithoutSlashUnchanged
 * @tc.desc: Keep a path that does not start with ~/ unchanged
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertTildeWithoutSlashUnchanged, Function | MediumTest | Level2)
{
    std::string tildePath = "~perf.data";
    std::string resultPath = SandboxFilePathHandler::ConvertToPhysicalPath(tildePath, TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(resultPath, tildePath);
}

/**
 * @tc.name: TestConvertSandboxPathWithDotDot
 * @tc.desc: Sandbox path containing .. is rejected by path canonicalization
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertSandboxPathWithDotDot, Function | MediumTest | Level2)
{
    std::string sandboxPath = TEST_SANDBOX_PATH_ROOT + "../perf.data";
    std::string resultPath = SandboxFilePathHandler::ConvertToPhysicalPath(sandboxPath, TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(resultPath, "");
}

/**
 * @tc.name: TestConvertHomeSubdirToPhysical
 * @tc.desc: Convert ~/ subdirectory outfile to the corresponding physical path
 * @tc.type: FUNC
 */
HWTEST_F(FilePathHandlerTest, TestConvertHomeSubdirToPhysical, Function | MediumTest | Level1)
{
    std::string homePath = TEST_HOME_PATH_PREFIX + "hiperf/perf.data";
    std::string physicalPath = SandboxFilePathHandler::ConvertToPhysicalPath(homePath, TEST_SANDBOX_CALLING_UID);
    EXPECT_EQ(physicalPath, TEST_PHYSICAL_PATH_ROOT + "hiperf/perf.data");
}
