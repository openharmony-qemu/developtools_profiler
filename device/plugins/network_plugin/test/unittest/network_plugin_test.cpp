/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <dlfcn.h>
#include <gtest/gtest.h>

#include "network_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
const std::string DEFAULT_TEST_PATH("/data/local/tmp/");
const std::string DEFAULT_NET_PATH("/proc/net/xt_qtaguid/stats");
//constexpr uint32_t BUF_SIZE = 4 * 1024 * 1024;

std::string g_path;

struct NetDetails {
    uint64_t tx;
    uint64_t rx;
    std::string type;
};

struct TestElement {
    int32_t pid;
    int32_t uid;
    uint64_t tx;
    uint64_t rx;
    std::vector<NetDetails> details;
};

TestElement g_expectBegin = {9553, 10194, 0, 0};
TestElement g_expectEnd = {9553, 10194, 27085, 751549, {{27085, 751549, std::string("wlan0")}}};

std::string GetFullPath(std::string path);

class NetworkPluginTest : public ::testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase()
    {
        if (access(g_path.c_str(), F_OK) == 0) {
            std::string str = "rm -rf " + GetFullPath(DEFAULT_TEST_PATH) + "utresources";
            system(str.c_str());
        }
    }
    void SetUp() {}
    void TearDown() {}
};

string Getexepath()
{
    char buf[PATH_MAX] = "";
    std::string path = "/proc/self/exe";
    size_t rslt = readlink(path.c_str(), buf, sizeof(buf));
    if (rslt < 0 || (rslt >= sizeof(buf))) {
        return "";
    }
    buf[rslt] = '\0';
    for (int i = rslt; i >= 0; i--) {
        if (buf[i] == '/') {
            buf[i + 1] = '\0';
            break;
        }
    }
    return buf;
}


std::string GetFullPath(std::string path)
{
    if (path.size() > 0 && path[0] != '/') {
        return Getexepath() + path;
    }
    return path;
}

void NetworkPluginTest::SetUpTestCase()
{
    g_path = GetFullPath(DEFAULT_TEST_PATH);
    EXPECT_NE("", g_path);
    g_path += "utresources";
}

/**
 * @tc.name: network plugin
 * @tc.desc: Test whether the utresource path exists.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, Testpath, TestSize.Level1)
{
    EXPECT_NE(g_path, "");
}

} // namespace
