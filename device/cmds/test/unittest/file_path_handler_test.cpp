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
