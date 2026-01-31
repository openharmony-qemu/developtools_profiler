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
#include <cstring>
#include "command_line.h"
#include "profiler_command_parser.h"
#include "profiler_command_executor.h"

using namespace testing::ext;

namespace {
const std::string TEST_CONFIG_FILE("/data/local/tmp/test_config.txt");
const std::string TEST_OUTPUT_FILE("/data/local/tmp/test_output.htrace");
}

class ProfilerCommandTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        // Setup for tests
    }

    void TearDown() override
    {
        // Cleanup for tests
    }
};

/**
 * @tc.name: TestParseStartCommand
 * @tc.desc: Test parsing start subcommand
 * @tc.type: FUNC
 * @tc.require: AR000H0F5K
 */
HWTEST_F(ProfilerCommandTest, TestParseStartCommand, Function | MediumTest | Level0)
{
    ProfilerCommandParser& parser = ProfilerCommandParser::GetInstance();
    ProfilerCommandArgs args;
    
    // Simulate: hiprofiler_cmd start --config test.txt --out output.htrace
    char* argv[] = {
        const_cast<char*>("hiprofiler_cmd"),
        const_cast<char*>("start"),
        const_cast<char*>("--config"),
        const_cast<char*>("test.txt"),
        const_cast<char*>("--out"),
        const_cast<char*>("output.htrace"),
        nullptr
    };
    int argc = 6;
    
    bool result = parser.ParseArguments(argc, argv, args);
    EXPECT_TRUE(result);
    EXPECT_EQ(args.commandType, CommandType::START);
    EXPECT_EQ(args.configFile, "test.txt");
    EXPECT_EQ(args.outputFile, "output.htrace");
}

/**
 * @tc.name: TestParseStopCommand
 * @tc.desc: Test parsing stop subcommand
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCommandTest, TestParseStopCommand, Function | MediumTest | Level0)
{
    ProfilerCommandParser& parser = ProfilerCommandParser::GetInstance();
    ProfilerCommandArgs args;
    auto& commandline = CommandLine::GetInstance();
    commandline.Reset();
    // Simulate: hiprofiler_cmd stop
    char* argv[] = {
        const_cast<char*>("hiprofiler_cmd"),
        const_cast<char*>("stop"),
        nullptr
    };
    int argc = 2;
    
    bool result = parser.ParseArguments(argc, argv, args);
    EXPECT_TRUE(result);
    EXPECT_EQ(args.commandType, CommandType::STOP);
}

/**
 * @tc.name: TestParseNormalCommand
 * @tc.desc: Test parsing normal command (without subcommand)
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCommandTest, TestParseNormalCommand, Function | MediumTest | Level0)
{
    ProfilerCommandParser& parser = ProfilerCommandParser::GetInstance();
    ProfilerCommandArgs args;
    auto& commandline = CommandLine::GetInstance();
    commandline.Reset();
    // Simulate: hiprofiler_cmd --config test.txt --out output.htrace
    char* argv[] = {
        const_cast<char*>("hiprofiler_cmd"),
        const_cast<char*>("--config"),
        const_cast<char*>("test.txt"),
        const_cast<char*>("--out"),
        const_cast<char*>("output.htrace"),
        nullptr
    };
    int argc = 5;
    
    bool result = parser.ParseArguments(argc, argv, args);
    EXPECT_TRUE(result);
    EXPECT_EQ(args.commandType, CommandType::NORMAL);
    EXPECT_EQ(args.configFile, "test.txt");
    EXPECT_EQ(args.outputFile, "output.htrace");
}

/**
 * @tc.name: TestParseStartCommandWithTime
 * @tc.desc: Test parsing start command with time parameter
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCommandTest, TestParseStartCommandWithTime, Function | MediumTest | Level0)
{
    ProfilerCommandParser& parser = ProfilerCommandParser::GetInstance();
    ProfilerCommandArgs args;
    auto& commandline = CommandLine::GetInstance();
    commandline.Reset();
    // Simulate: hiprofiler_cmd start --config test.txt --time 1800
    char* argv[] = {
        const_cast<char*>("hiprofiler_cmd"),
        const_cast<char*>("start"),
        const_cast<char*>("--config"),
        const_cast<char*>("test.txt"),
        const_cast<char*>("--time"),
        const_cast<char*>("1800"),
        nullptr
    };
    int argc = 6;
    
    bool result = parser.ParseArguments(argc, argv, args);
    EXPECT_TRUE(result);
    EXPECT_EQ(args.commandType, CommandType::START);
    EXPECT_EQ(args.configFile, "test.txt");
    EXPECT_EQ(args.traceKeepSecond, "1800");
}

/**
 * @tc.name: TestParseStartCommandWithAllParams
 * @tc.desc: Test parsing start command with all parameters
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCommandTest, TestParseStartCommandWithAllParams, Function | MediumTest | Level0)
{
    ProfilerCommandParser& parser = ProfilerCommandParser::GetInstance();
    ProfilerCommandArgs args;
    auto& commandline = CommandLine::GetInstance();
    commandline.Reset();
    // Simulate: hiprofiler_cmd start --config test.txt --out output.htrace --time 1800 --start --kill
    char* argv[] = {
        const_cast<char*>("hiprofiler_cmd"),
        const_cast<char*>("start"),
        const_cast<char*>("--config"),
        const_cast<char*>("test.txt"),
        const_cast<char*>("--out"),
        const_cast<char*>("output.htrace"),
        const_cast<char*>("--time"),
        const_cast<char*>("1800"),
        const_cast<char*>("--start"),
        const_cast<char*>("--kill"),
        nullptr
    };
    int argc = 10;
    
    bool result = parser.ParseArguments(argc, argv, args);
    EXPECT_TRUE(result);
    EXPECT_EQ(args.commandType, CommandType::START);
    EXPECT_EQ(args.configFile, "test.txt");
    EXPECT_EQ(args.outputFile, "output.htrace");
    EXPECT_EQ(args.traceKeepSecond, "1800");
    EXPECT_TRUE(args.isStartProcess);
    EXPECT_TRUE(args.isKillProcess);
}
