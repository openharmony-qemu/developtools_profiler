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

#include "command_helper.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class CommandHelperTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: Start
 * @tc.desc: Test CommandHelper Start
 * @tc.type: FUNC
 */
HWTEST_F(CommandHelperTest, Start, TestSize.Level1)
{
    int argc = 1;
    char arg1[] = "hiebpf";
    char* argv1[] = {arg1};
    char **argv = argv1;
    auto commandHelper = CommandHelper::GetInstance();
    int ret = commandHelper.Start(&argc, &argv, "help");
    EXPECT_EQ(ret, 0);

    argc++;
    char arg2[] = "h";
    char* argv2[] = {arg1, arg2};
    argv = argv2;
    ret = commandHelper.Start(&argc, &argv, "none");
    EXPECT_EQ(ret, 0);

    ret = commandHelper.Start(&argc, &argv, "help");
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: DoHelp
 * @tc.desc: Test CommandHelper DoHelp
 * @tc.type: FUNC
 */
HWTEST_F(CommandHelperTest, DoHelp, TestSize.Level1)
{
    testing::internal::CaptureStdout();
    auto commandHelper = CommandHelper::GetInstance();
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_EXCLUDE_TRACER]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_MAX_STACK_DEPTH]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_DURATION]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_EVENTS]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_PIDS]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_DUMP_EVENTS]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_UNWIND_STACK]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_BPF_LOG_LEVEL]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_BPF_LOG_FILE]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_LIBBPF_LOG_LEVEL]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_LIBBPF_LOG_FILE]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_HHLOG_LEVEL]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_HHLOG_FILE]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_SERVER_START]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::ARG_SERVER_STOP]);
    commandHelper.DoHelp(commandHelper.SUPPORTED_ARGS[CommandHelper::NR_SUPPORTED_ARGS]);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_EQ(out.empty(), false);
    EXPECT_TRUE(out.find(commandHelper.SUPPORTED_ARGS[CommandHelper::NR_SUPPORTED_ARGS]) != std::string::npos);
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
