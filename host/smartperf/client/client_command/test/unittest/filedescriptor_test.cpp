/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <regex>
#include <string>
#include "FileDescriptor.h"
#include "sp_utils.h"
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonFdsTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

static bool VerifResult(const std::string &result)
{
    int fdTotalInt = 0;
    std::regex fdTotalRegex(R"(fdTotal=(\d+))");
    std::smatch match;
    std::string::const_iterator searchStart(result.cbegin());
    while (std::regex_search(searchStart, result.cend(), match, fdTotalRegex)) {
        std::cout << "Match found: " << match.str(0) << std::endl;
        std::string fdTotalStr  = match.str(1);
        fdTotalInt = std::stoi(fdTotalStr);
        std::cout << "fdTotalInt as integer: " << fdTotalInt << std::endl;
        searchStart = match.suffix().first;
    }
    return fdTotalInt > 0;
}

/**
 * @tc.name: FdsTestCase01
 * @tc.desc: Test Fds by packagename
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonFdsTest, FdsTestCase01, TestSize.Level1)
{
    std::string cmd = "SP_daemon -N 1 -PKG ohos.samples.ecg -fds";
    std::string result = "";
    bool flag = false;
    bool ret = SPUtils::LoadCmd(cmd, result);
    std::string::size_type strOne = result.find("command exec finished!");
    if ((strOne != result.npos)) {
        flag = true;
    }
    EXPECT_EQ(ret, true);
    EXPECT_TRUE(flag);
}

/**
 * @tc.name: FdsTestCase03
 * @tc.desc: Test Fds by pid not exit
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonFdsTest, FdsTestCase03, TestSize.Level1)
{
    std::string cmd = "SP_daemon -N 1 -fds -PID 88888888"; // 88888888 is not exit
    std::string result = "";
    bool ret = SPUtils::LoadCmd(cmd, result);
    EXPECT_EQ(ret, true);
    ret = VerifResult(result);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FdsTestCase04
 * @tc.desc: Test Fds
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonFdsTest, FdsTestCase04, TestSize.Level1)
{
    FileDescriptor &fdsInstance = FileDescriptor::GetInstance();
    std::string packName = "init";
    fdsInstance.SetPackageName(packName);
    fdsInstance.SetProcessId("1");
    std::map<std::string, std::string> fdsItemData = fdsInstance.ItemData();
    std::string fdTotal = fdsItemData["fdTotal"];
    std::string fds = fdsItemData["fds"];
    EXPECT_EQ(fdTotal.empty(), false);
    EXPECT_EQ(fds.empty(), false);
}
} // namespace OHOS
} // namespace SmartPerf