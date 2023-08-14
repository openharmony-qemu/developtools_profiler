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
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <string>

#include "process_utils.h"

namespace {
using testing::ext::TestSize;
class ProcessUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}

    std::string FindCmd(const std::string& cmd)
    {
        std::vector<std::string> dirs = {
            "/bin/",
            "/system/bin/",
            "/vendor/bin/",
        };
        for (auto& dir : dirs) {
            std::string path = dir + cmd;
            if (access(path.c_str(), X_OK) == 0) {
                return path;
            }
        }
        return "";
    }
};

/*
 * @tc.name: ExecuteNormal
 * @tc.desc: test ProcessUtils::Execute with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessUtilsTest, ExecuteNormal, TestSize.Level1)
{
    ExecuteArgs args;
    args.bin_ = FindCmd("ls");
    args.argv_ = {"ls", "--color"};
    args.out2pipe_ = true;
    args.err2pipe_ = true;

    std::string output;
    EXPECT_EQ(ProcessUtils::Execute(args, output), 0);
    EXPECT_NE(output, "");
}
} // namespace
