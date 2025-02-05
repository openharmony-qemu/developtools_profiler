
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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
#include <array>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <memory>
#include "init_param.h"

using namespace testing::ext;

namespace {
constexpr int READ_BUFFER_SIZE = 1024;
const std::string KEY_HIVIEW_USER_TYPE = "const.logsystem.versiontype";

class NativeDaemonMainTest : public ::testing::Test {
public:
    static void SetUpTestCase(void)
    {
        SystemSetParameter(KEY_HIVIEW_USER_TYPE.c_str(), "beta");
    }

    bool RunCommand(const std::string& cmd, std::string& content)
    {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (pipe == nullptr) {
            std::cout<< "RunCommand: create popen FAILED!" << std::endl;
            return false;
        }
        std::array<char, READ_BUFFER_SIZE> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            content += buffer.data();
        }
        return true;
    }
};

/*
@tc.name: NativeDaemonMainTest001
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest001, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon sa";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}


/*
@tc.name: NativeDaemonMainTest002
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest002, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -p 1233121232";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest003
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest003, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -n test_native_daemon";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
    cmd = "native_daemon -n hiview";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest004
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest004, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -s";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest005
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest005, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -f 1";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -f abc";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -f 101";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest006
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest006, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -d abc";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest007
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest007, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -L 10241024";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest008
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest008, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -F abc.txt";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest009
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest009, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -u a";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest0010
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest010, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -L 10241024";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest011
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest011, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -S a";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest012
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest012, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -i a";
    bool ret = RunCommand(cmd, content);
    std::cout << content <<std::endl;
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest013
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest013, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -O false";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -O true";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -O aaa";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest014
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest014, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -C false";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -C true";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -C aaa";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest015
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest015, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -c false";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -c true";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -c aaa";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest016
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest016, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -r false";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -r true";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -r aaa";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest017
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest017, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -so false";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -so true";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -so aaa";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest018
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest018, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -js a";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);

    cmd = "native_daemon -jsd a";
    ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest019
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest019, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -jn a";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest020
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest020, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -abc 1";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest021
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest021, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -p 1 -f 10";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}

/*
@tc.name: NativeDaemonMainTest022
@tc.desc: test native_daemon func main.
@tc.type: FUNC
*/
HWTEST_F(NativeDaemonMainTest, NativeDaemonMainTest022, TestSize.Level1)
{
    std::string content;
    std::string cmd = "native_daemon -n hiview";
    bool ret = RunCommand(cmd, content);
    ASSERT_TRUE(ret);
}
}