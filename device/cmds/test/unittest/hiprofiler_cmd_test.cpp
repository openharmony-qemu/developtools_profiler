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
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "logging.h"
#include "openssl/sha.h"
#include "parameters.h"

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

const std::string DEFAULT_HIPROFILER_CMD_PATH("/system/bin/hiprofiler_cmd");
const std::string FTRACE_PLUGIN_PATH("/data/local/tmp/libftrace_plugin.z.so");
const std::string HIPERF_PLUGIN_PATH("/data/local/tmp/libhiperfplugin.z.so");
std::string DEFAULT_PATH("/data/local/tmp/");
constexpr uint32_t READ_BUFFER_SIZE = 1024;
constexpr int SLEEP_TIME = 3;
constexpr int FILE_READ_CHUNK_SIZE = 4096;
constexpr char HEX_CHARS[] = "0123456789abcdef";


class HiprofilerCmdTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void StartServerStub(const std::string name)
    {
        if (DEFAULT_HIPROFILERD_PATH == name) {
            // start running hiprofilerd
            OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
        } else if (DEFAULT_HIPROFILER_PLUGINS_PATH == name) {
            // start running hiprofiler_plugins
            OHOS::system::SetParameter("hiviewdfx.hiprofiler.plugins.start", "1");
        }
    }

    void StopProcessStub(const std::string name)
    {
        if (DEFAULT_HIPROFILERD_PATH == name) {
            // start running hiprofilerd
            OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
        } else if (DEFAULT_HIPROFILER_PLUGINS_PATH == name) {
            // start running hiprofiler_plugins
            OHOS::system::SetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
        }
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
            PROFILER_LOG_ERROR(LOG_CORE, "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
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

        PROFILER_LOG_DEBUG(LOG_CORE, "%s:%s-(%s)", __func__, path.c_str(), result.c_str());
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
            PROFILER_LOG_ERROR(LOG_CORE, "CreateConfigFile: fopen() error = %s", buf);
            return;
        }

        size_t len = fwrite(const_cast<char*>(configStr.c_str()), 1, configStr.length(), writeFp);
        if (len < 0) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "CreateConfigFile: fwrite() error = %s", buf);
            if (fclose(writeFp) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "fclose() error");
            }
            return;
        }

        int ret = fflush(writeFp);
        if (ret == EOF) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "CreateConfigFile: fflush() error = %s", buf);
            if (fclose(writeFp) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "fclose() error");
            }
            return;
        }

        fsync(fileno(writeFp));
        ret = fclose(writeFp);
        if (ret != 0) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "CreateConfigFile: fclose() error = %s", buf);
            return;
        }
    }

    std::string CreateCommand(const std::string &outFile, int time) const
    {
        std::string cmdStr =
            "hiprofiler_cmd \\\n"
            "-c - \\\n";
        cmdStr += "-o " + outFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n";
        cmdStr += "-k \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 1000\n"
            "  }\n"
            "  result_file: \"/data/local/tmp/hiprofiler_data.htrace\"\n"
            "  sample_duration: 1000\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"ftrace-plugin\"\n"
            "  sample_interval: 1000\n"
            "  is_protobuf_serialize: true\n"
            "  config_data {\n"
            "    ftrace_events: \"sched/sched_switch\"\n"
            "    ftrace_events: \"sched/sched_wakeup\"\n"
            "    ftrace_events: \"sched/sched_wakeup_new\"\n"
            "    ftrace_events: \"sched/sched_waking\"\n"
            "    ftrace_events: \"sched/sched_process_exit\"\n"
            "    ftrace_events: \"sched/sched_process_free\"\n"
            "    hitrace_categories: \"ability\"\n"
            "    hitrace_categories: \"ace\"\n"
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

    std::string CreateHiperfCommand(const std::string &outFile, int time) const
    {
        std::string cmdStr =
            "hiprofiler_cmd \\\n"
            "-c - \\\n";
        cmdStr += "-o " + outFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n";
        cmdStr += "-k \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 1000\n"
            "  }\n"
            "  result_file: \"/data/local/tmp/hiprofiler_data.htrace\"\n"
            "  sample_duration: 1000\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"hiperf-plugin\"\n"
            "  sample_interval: 1000\n"
            "  is_protobuf_serialize: true\n"
            "  config_data {\n"
            "    is_root: false\n"
            "    outfile_name: \"/data/local/tmp/perf.data\"\n"
            "    record_args: \"-f 1000 -a --call-stack dwarf\"\n"
            "  }\n"
            "}\n"
            "CONFIG\n";
        return cmdStr;
    }

    std::string CreateEncoderCommand(const std::string &outFile, int time) const
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
            "    hitrace_categories: \"ability\"\n"
            "    hitrace_categories: \"ace\"\n"
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

    std::string CreateSplitHtraceCommand(const std::string &outFile, int time) const
    {
        std::string cmdStr =
            "hiprofiler_cmd -s -k \\\n"
            "-c - \\\n";
        cmdStr += "-o " + outFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 16384\n"
            "  }\n"
            "  split_file: true\n"
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

    std::string CreateSplitNetworkProfilerCommand(const std::string &outFile, int time) const
    {
        std::string cmdStr =
            "hiprofiler_cmd -s -k \\\n"
            "-c - \\\n";
        cmdStr += "-o " + outFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 16384\n"
            "  }\n"
            "  split_file: true\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"network-profiler\"\n"
            "  config_data {\n"
            "    clock_id: 1\n"
            "    smb_pages: 16384\n"
            "    startup_process_name: \"com.ohos.systemui\"\n"
            "    block: true\n"
            "    flush_interval: 5\n"
            "  }\n"
            "}\n"
            "CONFIG\n";
        return cmdStr;
    }

    std::string CreateSplitHiperfCommand(const std::string &outFile, const std::string &perfFile,
                                    const std::string &perfSplitFile, int time) const
    {
        std::string cmdStr =
            "hiprofiler_cmd -s -k \\\n"
            "-c - \\\n";
        cmdStr += "-o " + outFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 16384\n"
            "  }\n"
            "  split_file: true\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"hiperf-plugin\"\n"
            "  config_data {\n"
            "    is_root: false\n"
            "    outfile_name: \"" + perfFile + "\"\n"
            "    record_args: \"-f 1000 -a  --cpu-limit 100 -e hw-cpu-cycles,sched:sched_waking --call-stack dwarf --clockid monotonic --offcpu -m 256\"\n"
            "    split_outfile_name: \"" + perfSplitFile + "\"\n"
            "  }\n"
            "}\n"
            "CONFIG\n";
        return cmdStr;
    }

    std::string CreateSplitHiebpfCommand(const std::string &outFile, const std::string &ebpfFile,
                                            const std::string &ebpfSplitFile, int time) const
    {
        std::string cmdStr =
            "hiprofiler_cmd -s -k \\\n"
            "-c - \\\n";
        cmdStr += "-o " + outFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 16384\n"
            "  }\n"
            "  split_file: true\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"hiebpf-plugin\"\n"
            "  config_data {\n"
            "    cmd_line: \"hiebpf --events fs,ptrace,bio --duration 200 --max_stack_depth 10\"\n"
            "    outfile_name: \"" + ebpfFile + "\"\n"
            "    split_outfile_name: \"" + ebpfSplitFile + "\"\n"
            "  }\n"
            "}\n"
            "CONFIG\n";
        return cmdStr;
    }

    unsigned long GetFileSize(const char* filename)
    {
        struct stat buf;

        if (stat(filename, &buf) < 0) {
            return 0;
        }
        return static_cast<unsigned long>(buf.st_size);
    }
};

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with -h -q.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0110, Function | MediumTest | Level1)
{
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);

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
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with -c file.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0120, Function | MediumTest | Level1)
{
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);

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
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
}

/**
 * @tc.name: hiprofiler_cmd
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
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with -s -l -k.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0140, Function | MediumTest | Level1)
{
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);

    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -s -l -k";
    std::string content = "";
    EXPECT_TRUE(RunCommand(cmd, content));
    std::string destStr = "plugin";
    EXPECT_TRUE(content.find(destStr) != std::string::npos);
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with -l -k.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0150, Function | MediumTest | Level1)
{
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);

    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -l -k";
    std::string content = "";
    EXPECT_TRUE(RunCommand(cmd, content));
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with -k.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0160, Function | MediumTest | Level1)
{
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);

    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -k";
    std::string content = "";
    EXPECT_TRUE(RunCommand(cmd, content));
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with proto encoder.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0170, Function | MediumTest | Level1)
{
    std::string cmd = "cp " + DEFAULT_SO_PATH + "libftrace_plugin.z.so " + DEFAULT_PATH;
    system(cmd.c_str());

    // 开启hiprofilerd和hiprofiler_plugin进程，验证字符串格式的config
    std::string content = "";
    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);
    std::string outFile = DEFAULT_PATH + "trace_encoder.htrace";
    int time = 3;
    cmd = CreateEncoderCommand(outFile, time);
    EXPECT_TRUE(RunCommand(cmd, content));
    sleep(time);
    EXPECT_EQ(access(outFile.c_str(), F_OK), 0);

    // 删除资源文件和生成的trace文件
    cmd = "rm " + FTRACE_PLUGIN_PATH + " " + outFile;
    system(cmd.c_str());
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with ctrl+c.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0180, Function | MediumTest | Level1)
{
    std::string content = "";
    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -s";
    EXPECT_TRUE(RunCommand(cmd, content));
    sleep(2); // 2: wait hiprofilerd start
    pid_t pid = fork();
    EXPECT_GE(pid, 0);
    if (pid == 0) {
        content = "";
        const int time = 20;
        std::string outFile = DEFAULT_PATH + "trace.htrace";
        cmd = CreateCommand(outFile, time);
        EXPECT_TRUE(RunCommand(cmd, content));
        EXPECT_EQ(access(outFile.c_str(), F_OK), 0);
        // 删除生成的trace文件
        cmd = "rm " + outFile;
        system(cmd.c_str());
        _exit(0);
    } else if (pid > 0) {
        sleep(1); // 1: wait child process start
        content = "";
        cmd = "pidof hiprofiler_cmd";
        EXPECT_TRUE(RunCommand(cmd, content));
        ASSERT_STRNE(content.c_str(), "");
        cmd = "kill -2 " + content;
        content = "";
        EXPECT_TRUE(RunCommand(cmd, content));
        EXPECT_STREQ(content.c_str(), "");
        // 等待子进程结束
        waitpid(pid, nullptr, 0);
        cmd = DEFAULT_HIPROFILER_CMD_PATH + " -k";
        EXPECT_TRUE(RunCommand(cmd, content));
        sleep(5); // 5: wait hiprofilerd exit
        content = "";
        cmd = "pidof " + DEFAULT_HIPROFILERD_NAME;
        EXPECT_TRUE(RunCommand(cmd, content));
        EXPECT_STREQ(content.c_str(), "");
    }
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with -c string.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0190, Function | MediumTest | Level1)
{
    std::string cmd = "cp " + DEFAULT_SO_PATH + "libhiperfplugin.z.so " + DEFAULT_PATH;
    system(cmd.c_str());

    // 开启hiprofilerd和hiprofiler_plugin进程，验证字符串格式的config
    std::string content = "";
    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);
    std::string outFile = DEFAULT_PATH + "trace.htrace";
    int time = 10;
    cmd = CreateHiperfCommand(outFile, time);
    EXPECT_TRUE(RunCommand(cmd, content));
    sleep(time);
    EXPECT_EQ(access(outFile.c_str(), F_OK), 0);

    // 删除资源文件和生成的trace文件
    cmd = "rm " + HIPERF_PLUGIN_PATH + " " + outFile;
    system(cmd.c_str());
    StopProcessStub(DEFAULT_HIPROFILER_PLUGINS_PATH);
    sleep(1);
    StopProcessStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1);
}
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with split Htrace file.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0200, Function | MediumTest | Level1)
{
    std::string outFileName = "split_htrace";
    std::string outFile = DEFAULT_PATH + outFileName + ".htrace";
    std::string content = "";
    int time = 10;
    std::string cmd = CreateSplitHtraceCommand(outFile, time);
    EXPECT_TRUE(RunCommand(cmd, content));

    EXPECT_NE(access(outFile.c_str(), F_OK), 0);

    cmd = "ls " + DEFAULT_PATH + outFileName + "*_1.htrace";
    EXPECT_TRUE(RunCommand(cmd, content));
    EXPECT_STRNE(content.c_str(), "");

    cmd = "rm " + DEFAULT_PATH + outFileName + "*.htrace";
    system(cmd.c_str());
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with split hiperf file.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0220, Function | MediumTest | Level1)
{
    std::string outFileName = "split_hiperf";
    std::string outFile = DEFAULT_PATH + outFileName + ".htrace";
    std::string perfFile = "/data/local/tmp/perf.data";
    std::string perfSplitFile = "/data/local/tmp/split_perf_data.htrace";
    std::string content = "";
    int time = 10;
    std::string cmd = CreateSplitHiperfCommand(outFile, perfFile, perfSplitFile, time);
    EXPECT_TRUE(RunCommand(cmd, content));

    EXPECT_NE(access(outFile.c_str(), F_OK), 0);

    cmd = "ls " + DEFAULT_PATH + outFileName + "*_1.htrace";
    EXPECT_TRUE(RunCommand(cmd, content));
    EXPECT_STRNE(content.c_str(), "");

    EXPECT_EQ(access(perfSplitFile.c_str(), F_OK), 0);
    if (access(perfFile.c_str(), F_OK) == 0) {
        const int headerSize = 1024 + 1024; // htrace header + hiperf header
        auto perfFileSize = GetFileSize(perfFile.c_str());
        auto perfSplitFileSize = GetFileSize(perfSplitFile.c_str());
        EXPECT_GT(perfSplitFileSize, perfFileSize + headerSize);
    }

    cmd = "rm " + DEFAULT_PATH + outFileName + "*.htrace " + perfFile + " " + perfSplitFile;
    system(cmd.c_str());
}

/**
 * @tc.name: hiprofiler_cmd
 * @tc.desc: Test hiprofiler_cmd with split hiebpf file.
 * @tc.type: FUNC
 */
HWTEST_F(HiprofilerCmdTest, DFX_DFR_Hiprofiler_0240, Function | MediumTest | Level1)
{
    std::string outFileName = "split_htrace";
    std::string outFile = DEFAULT_PATH + outFileName + ".htrace";
    std::string content = "";
    int time = 10;
    std::string cmd = CreateSplitNetworkProfilerCommand(outFile, time);
    EXPECT_TRUE(RunCommand(cmd, content));

    EXPECT_NE(access(outFile.c_str(), F_OK), 0);

    cmd = "ls " + DEFAULT_PATH + outFileName + "*_1.htrace";
    EXPECT_TRUE(RunCommand(cmd, content));
    EXPECT_STRNE(content.c_str(), "");

    cmd = "rm " + DEFAULT_PATH + outFileName + "*.htrace";
    system(cmd.c_str());
}
