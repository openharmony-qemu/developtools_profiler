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

#include <cinttypes>
#include <gtest/gtest.h>
#include <csignal>
#include <filesystem>

#include "ffrt_profiler_manager.h"

namespace fs = std::filesystem;
using namespace testing::ext;
using namespace OHOS::Developtools::Profiler;

namespace {
const std::string OUTPUT_PATH = "/data/local/tmp/hiprofiler_data.htrace";
const std::string FFRT_TEST_EXE = "/data/local/tmp/ffrt_profiler_test_exe";
constexpr int FILE_SIZE = 2000;
class FfrtPofilerTest : public ::testing::Test {
public:
    FfrtPofilerTest() {}
    ~FfrtPofilerTest() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    std::string CreateCommand(const std::string& outputFile, int32_t time, const std::string& model,
        const std::string& procedure) const
    {
        std::string cmdStr =
            "hiprofiler_cmd \\\n"
            "-c - \\\n";
        cmdStr += "-o " + outputFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n";
        cmdStr += "-s \\\n";
        cmdStr += "-k \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 32768\n"
            "  }\n"
            "  result_file: \"/data/local/tmp/hiprofiler_data.htrace\"\n"
            "  sample_duration: 30000\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"ffrt-profiler\"\n"
            "  config_data {\n";
        cmdStr += model + ": " + procedure + '\n';
        cmdStr += "smb_pages: 16384\n"
                "flush_interval: 5\n"
                "block: true\n"
                "clock_id: BOOTTIME\n"
            "  }\n"
            "}\n"
            "CONFIG\n";
        return cmdStr;
    }

    void StartProcess(const std::string& name, const std::string& args)
    {
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            return;
        }

        int processNum = fork();
        if (processNum == 0) {
            execl(name.c_str(), name.c_str(), args.c_str(), NULL);
            _exit(1);
        } else if (processNum < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "Failed to fork process");
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "sub process PID: %d", processNum);
            ffrtPrfolerExePid_ = processNum;
        }
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

    bool CheckFileSize(const std::string& filePath)
    {
        if (!fs::exists(filePath)) {
            return false;
        }
        if (fs::file_size(filePath) < FILE_SIZE) {
            return false;
        }
        return true;
    }

    int ffrtPrfolerExePid_{0};
};

HWTEST_F(FfrtPofilerTest, TestFfrtProfilerRuntime, TestSize.Level1)
{
    StartProcess(FFRT_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    fs::remove(OUTPUT_PATH);
    EXPECT_TRUE(RunCommand(cmd, ret));
    EXPECT_TRUE(ret.find("FAIL") == std::string::npos);
}

HWTEST_F(FfrtPofilerTest, TestFfrtProfilerError, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    fs::remove(OUTPUT_PATH);
    EXPECT_TRUE(RunCommand(cmd, ret));
    EXPECT_TRUE(ret.find("FAIL") == std::string::npos);
    EXPECT_FALSE(CheckFileSize(OUTPUT_PATH));
}
}