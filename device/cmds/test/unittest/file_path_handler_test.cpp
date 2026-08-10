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

#include <gtest/gtest.h>
#include "file_path_handler.h"
#include "profiler_service.grpc.pb.h"

using namespace testing::ext;

namespace {
const std::string TEST_OUTPUT_FILE("/data/local/tmp/test_output.htrace");
const std::string TEST_CONFIG_FILE("test_config.txt");
const std::string TEST_COMPOSITE_OUTPUT_FILE("/data/local/tmp/hiprofiler_data.zip");
const std::string TEST_COMPOSITE_INVALID_FILE("/data/local/tmp/hiprofiler_data.htrace");
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
