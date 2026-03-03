/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include <chrono>
#include <dlfcn.h>
#include <future>
#include <gtest/gtest.h>
#include <memory_trace.h>
#include <thread>
#include "init_param.h"
#include "logging.h"
#include "parameters.h"
#include "preload_client.h"
#include "preload_socket_client.h"

using namespace testing::ext;

namespace {
constexpr uint32_t SIZE = 1024;
constexpr int SLEEP_SECONDS = 5;
const std::string OUTPUT_PATH = "/data/local/tmp/hiprofiler_data.htrace";
const int SMB_PAGES = 16384;

class PreloadClientTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

std::string CreateCommand(const std::string& outputFile, const int32_t time, const std::string& processName)
{
    std::ostringstream cmdStream;
    cmdStream << "hiprofiler_cmd \\\n"
              << "-c - \\\n"
              << "-o " << outputFile << " \\\n"
              << "-t " << time << " \\\n"
              << "-s \\\n"
              << "-k \\\n"
              << "<<CONFIG\n"
              << "request_id: 1\n"
              << "session_config {\n"
              << "  buffers {\n"
              << "    pages: 14848" << "\n"
              << "  }\n"
              << "}\n"
              << "plugin_configs {\n"
              << "  plugin_name: \"nativehook\"\n"
              << "  config_data {\n"
              << "process_name: \"" << processName << "\"\n"
              << "smb_pages: " << SMB_PAGES << "\n"
              << "blocked: true\n"
              << "offline_symbolization: true\n"
              << "startup_mode: false\n"
              << "memtrace_enable: true\n"
              << "malloc_disable: true\n"
              << "restrace_tag: \"RES_ARK_LOCAL_HANDLE\"\n"
              << "  }\n"
              << "}\n"
              << "CONFIG\n";
    return cmdStream.str();
}

bool RunCommand(const std::string& cmd, std::string& content)
{
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    CHECK_TRUE(pipe, false, "RunCommand: create popen FAILED!");
    static constexpr int buffSize = 1024;
    std::array<char, buffSize> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        content += buffer.data();
    }
    return true;
}

/**
 * @tc.name: GetPreloadSoListTest1
 * @tc.desc: Test GetPreloadSoList, first param invalid.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadClientTest, GetPreloadSoListTest1, TestSize.Level0)
{
    char* mem = new char[SIZE];
    bool ret = GetPreloadSoList(nullptr, mem, SIZE);
    delete[] mem;
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetPreloadSoListTest2
 * @tc.desc: Test GetPreloadSoList, second param invalid.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadClientTest, GetPreloadSoListTest2, TestSize.Level0)
{
    std::string appName("com.example.myapplication");
    bool ret = GetPreloadSoList(appName.c_str(), nullptr, SIZE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetPreloadSoListTest3
 * @tc.desc: Test GetPreloadSoList, third param invalid.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadClientTest, GetPreloadSoListTest3, TestSize.Level0)
{
    char* mem = new char[SIZE];
    std::string appName("com.example.myapplication");
    bool ret = GetPreloadSoList(appName.c_str(), mem, 0);
    delete[] mem;
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetPreloadSoListTest4
 * @tc.desc: Test GetPreloadSoList.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadClientTest, GetPreloadSoListTest4, TestSize.Level0)
{
    char* mem = new char[SIZE];
    std::string appName("com.ohos.sceneboard");
    bool ret = GetPreloadSoList(appName.c_str(), mem, SIZE);
    delete[] mem;
    EXPECT_FALSE(ret);
}

int ThreadFunc(int size)
{
    std::string appName("hiview");
    char* mem = new char[SIZE];
    sleep(SLEEP_SECONDS);
    bool ret = GetPreloadSoList(appName.c_str(), mem, size);
    delete[] mem;

    if (ret) {
        return 0;
    } else {
        return (-1);
    }
}

/**
 * @tc.name: GetPreloadSoListTest5
 * @tc.desc: Test GetPreloadSoList, hiprofiler_cmd process name and GetPreloadSoList input appName not equal.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadClientTest, GetPreloadSoListTest5, TestSize.Level0)
{
    std::string appName("com.ohos.test");
    std::future<int> ret = std::async(std::launch::async, ThreadFunc, SIZE);
    sleep(1);
    std::string cmd = CreateCommand(OUTPUT_PATH, 20, appName);
    std::string result;
    EXPECT_TRUE(RunCommand(cmd, result));
    EXPECT_TRUE(result.find("FAIL") == std::string::npos);

    EXPECT_TRUE(ret.get() != 0);
}

/**
 * @tc.name: GetPreloadSoListTest6
 * @tc.desc: Test GetPreloadSoList, buf size less than diag path size.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadClientTest, GetPreloadSoListTest6, TestSize.Level0)
{
    std::string appName("hiview");
    std::future<int> ret = std::async(std::launch::async, ThreadFunc, 5);
    sleep(1);
    std::string cmd = CreateCommand(OUTPUT_PATH, 20, appName);
    std::string result;
    EXPECT_TRUE(RunCommand(cmd, result));
    EXPECT_TRUE(result.find("FAIL") == std::string::npos);

    EXPECT_TRUE(ret.get() != 0);
}

} // namespace
