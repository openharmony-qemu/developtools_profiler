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

#include <array>
#include <dlfcn.h>
#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "logging.h"
#include "openssl/sha.h"

using namespace testing::ext;

#define HHB(v) (((v) & 0xF0) >> 4)
#define LHB(v)  ((v) & 0x0F)

namespace {
#if defined(__LP64__)
const std::string DEFAULT_SO_PATH("/system/lib64/");
#else
const std::string DEFAULT_SO_PATH("/system/lib/");
#endif
const std::string DEFAULT_HIPROFILERD_PATH("/system/bin/hiprofilerd");
const std::string DEFAULT_HIPROFILER_PLUGINS_PATH("/system/bin/hiprofiler_plugins");
const std::string DEFAULT_HIPROFILERD_NAME("hiprofilerd");
const std::string DEFAULT_HIPROFILER_PLUGINS_NAME("hiprofiler_plugins");
const std::string DEFAULT_HIPROFILER_CMD_PATH("/system/bin/hiprofiler_cmd");
const std::string FTRACE_PLUGIN_PATH("/data/local/tmp/libftrace_plugin.z.so");
std::string DEFAULT_PATH("/data/local/tmp/");
constexpr uint32_t READ_BUFFER_SIZE = 1024;
constexpr int SLEEP_TIME = 3;
constexpr int FILE_READ_CHUNK_SIZE = 4096;
constexpr char HEX_CHARS[] = "0123456789abcdef";
constexpr int LINE_SIZE = 1000;


class HiprofilerCmdTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void StartServerStub(std::string name)
    {
        int processNum = fork();
        if (processNum == 0) {
            if (DEFAULT_HIPROFILERD_PATH == name) {
                // start running hiprofilerd
                execl(name.c_str(), nullptr, nullptr);
            } else if (DEFAULT_HIPROFILER_PLUGINS_PATH == name) {
                // start running hiprofiler_plugins
                execl(name.c_str(), DEFAULT_PATH.c_str(), nullptr);
            }
            _exit(1);
        } else if (DEFAULT_HIPROFILERD_PATH == name) {
            hiprofilerdPid_ = processNum;
        } else if (DEFAULT_HIPROFILER_PLUGINS_PATH == name) {
            hiprofilerPluginsPid_ = processNum;
        }
    }

    void StopProcessStub(int processNum)
    {
        std::string stopCmd = "kill " + std::to_string(processNum);
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(stopCmd.c_str(), "r"), pclose);
    }

    bool RunCommand(const std::string& cmd, std::string& content)
    {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        CHECK_TRUE(pipe, false, "RunCommand: create popen FAILED!");

        std::array<char, READ_BUFFER_SIZE> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            content += buffer.data();
        }
        return true;
    }

    std::string ComputeFileSha256(const std::string& path)
    {
        uint8_t out[SHA256_DIGEST_LENGTH];
        uint8_t buffer[FILE_READ_CHUNK_SIZE];
        char realPath[PATH_MAX + 1] = {0};

        SHA256_CTX sha;
        SHA256_Init(&sha);

        size_t nbytes = 0;

        if ((strlen(path.c_str()) >= PATH_MAX) || (realpath(path.c_str(), realPath) == nullptr)) {
            HILOG_ERROR(LOG_CORE, "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
            return "";
        }
        FILE* file = fopen(realPath, "rb");
        if (file == nullptr) {
            return "";
        }

        std::unique_ptr<FILE, decltype(fclose)*> fptr(file, fclose);
        if (fptr == nullptr) {
            return "";
        }

        while ((nbytes = fread(buffer, 1, sizeof(buffer), fptr.get())) > 0) {
            SHA256_Update(&sha, buffer, nbytes);
        }
        SHA256_Final(out, &sha);

        std::string result;
        result.reserve(SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH);
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            result.push_back(HEX_CHARS[HHB(out[i])]);
            result.push_back(HEX_CHARS[LHB(out[i])]);
        }

        HILOG_DEBUG(LOG_CORE, "%s:%s-(%s)", __func__, path.c_str(), result.c_str());
        return result;
    }

    void CreateConfigFile(const std::string configFile)
    {
        // 构建config文件
        std::string configStr =
            "request_id: 26\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 1000\n"
            "  }\n"
            "  result_file: \"/data/local/tmp/hiprofiler_data.htrace\"\n"
            "  sample_duration: 10000\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"ftrace-plugin\"\n"
            "  sample_interval: 2000\n"
            "  config_data: {\n"
            "    ftrace_events: \"sched/sched_switch\"\n"
            "    ftrace_events: \"sched/sched_wakeup\"\n"
            "    ftrace_events: \"sched/sched_wakeup_new\"\n"
            "    ftrace_events: \"sched/sched_waking\"\n"
            "    ftrace_events: \"sched/sched_process_exit\"\n"
            "    ftrace_events: \"sched/sched_process_free\"\n"
            "    buffer_size_kb: 51200\n"
            "    flush_interval_ms: 1000\n"
            "    flush_threshold_kb: 4096\n"
            "    parse_ksyms: true\n"
            "    clock: \"mono\"\n"
            "    trace_period_ms: 200\n"
            "    debug_on: false\n"
            "  }\n"
            "}\n";

        // 根据构建的config写文件
        FILE* writeFp = fopen(configFile.c_str(), "w");
        if (writeFp == nullptr) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "CreateConfigFile: fopen() error = %s", buf);
            return;
        }

        size_t len = fwrite(const_cast<char*>(configStr.c_str()), 1, configStr.length(), writeFp);
        if (len < 0) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "CreateConfigFile: fwrite() error = %s", buf);
            if (fclose(writeFp) != 0) {
                HILOG_ERROR(LOG_CORE, "fclose() error");
            }
            return;
        }

        int ret = fflush(writeFp);
        if (ret == EOF) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "CreateConfigFile: fflush() error = %s", buf);
            if (fclose(writeFp) != 0) {
                HILOG_ERROR(LOG_CORE, "fclose() error");
            }
            return;
        }

        fsync(fileno(writeFp));
        ret = fclose(writeFp);
        if (ret != 0) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "CreateConfigFile: fclose() error = %s", buf);
            return;
        }
    }

    std::string CreateCommand(const std::string &outFile, int time) const
    {
        std::string cmdStr =
            "hiprofiler_cmd \\\n"
            "-c - \\\n";
        cmdStr += "-k \\\n";
        cmdStr += "-o " + outFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 1000\n"
            "  }\n"
            "  result_file: \"/data/local/tmp/hiprofiler_data.htrace\"\n"
            "  sample_duration: 3000\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"ftrace-plugin\"\n"
            "  sample_interval: 1000\n"
            "  config_data {\n"
            "    ftrace_events: \"sched/sched_switch\"\n"
            "    ftrace_events: \"sched/sched_wakeup\"\n"
            "    ftrace_events: \"sched/sched_wakeup_new\"\n"
            "    ftrace_events: \"sched/sched_waking\"\n"
            "    ftrace_events: \"sched/sched_process_exit\"\n"
            "    ftrace_events: \"sched/sched_process_free\"\n"
            "    buffer_size_kb: 51200\n"
            "    flush_interval_ms: 1000\n"
            "    flush_threshold_kb: 4096\n"
            "    parse_ksyms: true\n"
            "    clock: \"mono\"\n"
            "    trace_period_ms: 200\n"
            "    debug_on: false\n"
            "  }\n"
            "}\n"
            "CONFIG\n";
        return cmdStr;
    }

    void KillProcess(const std::string processName)
    {
        int pid = -1;
        std::string findpid = "pidof " + processName;
        HILOG_INFO(LOG_CORE, "find pid command : %s", findpid.c_str());
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(findpid.c_str(), "r"), pclose);

        char line[LINE_SIZE];
        do {
            if (fgets(line, sizeof(line), pipe.get()) == nullptr) {
                HILOG_INFO(LOG_CORE, "not find processName : %s", processName.c_str());
                return;
            } else if (strlen(line) > 0 && isdigit(static_cast<unsigned char>(line[0]))) {
                pid = atoi(line);
                HILOG_INFO(LOG_CORE, "find processName : %s, pid: %d", processName.c_str(), pid);
                break;
            }
        } while (1);

        if (pid != -1) {
            StopProcessStub(pid);
        }
    }
private:
    int hiprofilerdPid_ = -1;
    int hiprofilerPluginsPid_ = -1;
};

/**
 * @tc.name: native hook
 * @tc.desc: Test hiprofiler_cmd with -h -q.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0110, Function | MediumTest | Level1)
{
    KillProcess(DEFAULT_HIPROFILERD_NAME);
    KillProcess(DEFAULT_HIPROFILER_PLUGINS_NAME);

    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -h";
    std::string content = "";
    EXPECT_TRUE(RunCommand(cmd, content));
    std::string destStr = "help";
    EXPECT_EQ(strncmp(content.c_str(), destStr.c_str(), strlen(destStr.c_str())), 0);

    content = "";
    cmd = DEFAULT_HIPROFILER_CMD_PATH + " -q";
    EXPECT_TRUE(RunCommand(cmd, content));
    destStr = "Service not started";
    EXPECT_EQ(strncmp(content.c_str(), destStr.c_str(), strlen(destStr.c_str())), 0);

    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    content = "";
    EXPECT_TRUE(RunCommand(cmd, content));
    destStr = "OK";
    EXPECT_EQ(strncmp(content.c_str(), destStr.c_str(), strlen(destStr.c_str())), 0);
    StopProcessStub(hiprofilerdPid_);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hiprofiler_cmd with -c file.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0120, Function | MediumTest | Level1)
{
    KillProcess(DEFAULT_HIPROFILERD_NAME);
    KillProcess(DEFAULT_HIPROFILER_PLUGINS_NAME);

    // 测试不存在的config文件
    std::string configTestFile = DEFAULT_PATH + "1234.txt";
    std::string outFile = DEFAULT_PATH + "trace.htrace";
    std::string content = "";
    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -c " + configTestFile + " -o " + outFile + " -t 3";
    EXPECT_TRUE(RunCommand(cmd, content));
    std::string destStr = "Read " + configTestFile + " fail";
    EXPECT_TRUE(content.find(destStr) != std::string::npos);

    // 创建有效的config文件
    const std::string configFile = DEFAULT_PATH + "ftrace.config";
    CreateConfigFile(configFile);

    // 测试有效的config文件，不开启hiprofilerd和hiprofiler_plugin进程
    content = "";
    cmd = DEFAULT_HIPROFILER_CMD_PATH + " -c " + configFile + " -o " + outFile + " -t 3";
    EXPECT_TRUE(RunCommand(cmd, content));
    sleep(SLEEP_TIME);
    EXPECT_NE(access(outFile.c_str(), F_OK), 0);

    // 开启hiprofilerd和hiprofiler_plugin进程，可以生成trace文件
    content = "";
    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);
    EXPECT_TRUE(RunCommand(cmd, content));
    sleep(SLEEP_TIME);
    EXPECT_EQ(access(outFile.c_str(), F_OK), 0);

    // 删除资源文件和生成的trace文件
    cmd = "rm " + configFile + " " + outFile;
    system(cmd.c_str());
    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hiprofiler_cmd with -c string.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0130, Function | MediumTest | Level1)
{
    std::string cmd = "cp " + DEFAULT_SO_PATH + "libftrace_plugin.z.so " + DEFAULT_PATH;
    system(cmd.c_str());

    // 开启hiprofilerd和hiprofiler_plugin进程，验证字符串格式的config
    std::string content = "";
    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);
    std::string outFile = DEFAULT_PATH + "trace.htrace";
    int time = 3;
    cmd = CreateCommand(outFile, time);
    EXPECT_TRUE(RunCommand(cmd, content));
    sleep(time);
    EXPECT_EQ(access(outFile.c_str(), F_OK), 0);

    // 删除资源文件和生成的trace文件
    cmd = "rm " + FTRACE_PLUGIN_PATH + " " + outFile;
    system(cmd.c_str());
    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hiprofiler_cmd with -s -l -k.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0140, Function | MediumTest | Level1)
{
    KillProcess(DEFAULT_HIPROFILERD_NAME);
    KillProcess(DEFAULT_HIPROFILER_PLUGINS_NAME);

    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -s -l -k";
    std::string content = "";
    EXPECT_TRUE(RunCommand(cmd, content));
    std::string destStr = "plugin";
    EXPECT_TRUE(content.find(destStr) != std::string::npos);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hiprofiler_cmd with -l -k.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0150, Function | MediumTest | Level1)
{
    KillProcess(DEFAULT_HIPROFILERD_NAME);
    KillProcess(DEFAULT_HIPROFILER_PLUGINS_NAME);

    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -l -k";
    std::string content = "";
    EXPECT_TRUE(RunCommand(cmd, content));
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hiprofiler_cmd with -k.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0160, Function | MediumTest | Level1)
{
    KillProcess(DEFAULT_HIPROFILERD_NAME);
    KillProcess(DEFAULT_HIPROFILER_PLUGINS_NAME);

    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -k";
    std::string content = "";
    EXPECT_TRUE(RunCommand(cmd, content));
}
}
