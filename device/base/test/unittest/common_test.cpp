/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

namespace {
using namespace testing::ext;

class CommonTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: CommonTest
 * @tc.desc: IsProcessExist.
 * @tc.type: FUNC
 */
HWTEST_F(CommonTest, IsProcessExist, TestSize.Level1)
{
    std::string procName = "hiprofiler_base_ut";
    int pid = 0;
    EXPECT_TRUE(COMMON::IsProcessExist(procName, pid));
    EXPECT_NE(pid, 0);
    procName = "ls";
    pid = 0;
    EXPECT_FALSE(COMMON::IsProcessExist(procName, pid));
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
    EXPECT_FALSE(COMMON::IsProcessRunning());
    int procPid = COMMON::StartProcess(profilerProcName, argvVec);
    EXPECT_NE(procPid, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    EXPECT_NE(COMMON::KillProcess(procPid), -1);
}
} // namespace