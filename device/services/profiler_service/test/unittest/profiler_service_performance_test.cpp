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

#include <dlfcn.h>
#include <fcntl.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <thread>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <vector>

#include "common_types.pb.h"
#include "cpu_plugin_config.pb.h"
#include "cpu_plugin_result.pb.h"
#include "diskio_plugin_result.pb.h"
#include "hilog_plugin_config.pb.h"
#include "hilog_plugin_result.pb.h"
#include "logging.h"
#include "memory_plugin_common.pb.h"
#include "memory_plugin_config.pb.h"
#include "memory_plugin_result.pb.h"
#include "network_plugin_config.pb.h"
#include "network_plugin_result.pb.h"
#include "plugin_module_api.h"
#include "plugin_service_types.pb.h"
#include "process_plugin_config.pb.h"
#include "process_plugin_result.pb.h"
#include "profiler_service.pb.h"
#include "profiler_service.grpc.pb.h"

#pragma clang optimize off

using namespace testing::ext;

namespace {
const std::string DEFAULT_HIPROFILERD_PATH("/system/bin/hiprofilerd");
const std::string DEFAULT_HIPROFILER_PLUGINS_PATH("/system/bin/hiprofiler_plugins");
const std::string WRITE_FILE = "/data/local/tmp/diskio_write_test.txt";
const std::string HILOG_RESULT_FILE = "/data/local/tmp/hilog.txt";
constexpr int ROUND_COUNT = 50;
constexpr int SAMPLE_INTERVAL = 40;
constexpr int TIMEOUT_US = 3 * 1000 * 1000;
constexpr int FETCHDATA_TIMEOUT_US = 20 * 1000 * 1000;
constexpr int KEEP_SESSION_TIMEOUT_MS = 5 * 1000;
constexpr int KEEP_SESSION_TIME_US = KEEP_SESSION_TIMEOUT_MS * 1000;
constexpr int KEEP_SESSION_SLEEP_US = 3 * 1000 * 1000;
constexpr int ACTIVE_PROCESS_SLEEP_US = 10000;
const size_t MB_PER_BYTE = 0x100000;
constexpr int BLOCK_LEN = 100 * 1024;
constexpr int WRITE_KB = 100;
constexpr int DEFAULT_PAGES = 10;
constexpr int MEMINFO_SIZE = 2;

class Timer {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    Timer() : startTime_(Now()) {}

    ~Timer() {}

    long ElapsedUs()
    {
        auto currentTime = Now();
        return std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime_).count();
    }

    void Reset()
    {
        startTime_ = Now();
    }

protected:
    TimePoint Now()
    {
        return Clock::now();
    }

private:
    TimePoint startTime_;
};

class ProfilerServicePerformanceTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        profilerStub_ = GetProfilerServiceStub();
    }

    void StartServerStub(std::string name)
    {
        int processNum = fork();
        if (processNum == 0) {
            if (DEFAULT_HIPROFILERD_PATH == name) {
                // start running hiprofilerd
                execl(name.c_str(), nullptr, nullptr);
            } else if (DEFAULT_HIPROFILER_PLUGINS_PATH == name) {
                // start running hiprofiler_plugins
                execl(name.c_str(), "/data/local/tmp/", nullptr);
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

    std::unique_ptr<IProfilerService::Stub> GetProfilerServiceStub()
    {
        auto grpcChannel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
        if (grpcChannel == nullptr) {
            HILOG_INFO(LOG_CORE, "Create GRPC channel failed!");
            return nullptr;
        }
        return IProfilerService::NewStub(grpcChannel);
    }

    void SetMemPluginConfig(CreateSessionRequest& createRequest, int requstMemoryPid, bool isFirstConfig)
    {
        ProfilerPluginConfig* ppc = createRequest.add_plugin_configs();
        ppc->set_name("memory-plugin");
        ppc->set_sample_interval(SAMPLE_INTERVAL);
        MemoryConfig mc;
        mc.set_report_process_mem_info(true);
        mc.set_report_app_mem_info(isFirstConfig);
        mc.set_report_sysmem_mem_info(true);
        mc.add_sys_meminfo_counters(SysMeminfoType::PMEM_MEM_TOTAL);
        mc.add_sys_meminfo_counters(SysMeminfoType::PMEM_MEM_FREE);
        mc.set_report_sysmem_vmem_info(true);
        mc.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_FREE_PAGES);
        mc.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_ALLOC_BATCH);
        mc.add_pid(idlePid_);
        if (isFirstConfig) {
            mc.add_pid(requstMemoryPid);
        }
        std::vector<uint8_t> configData(mc.ByteSizeLong());
        mc.SerializeToArray(configData.data(), configData.size());
        ppc->set_config_data((const void*)configData.data(), configData.size());
        auto sessionConfig = createRequest.mutable_session_config();
        ProfilerSessionConfig::BufferConfig* bfs = sessionConfig->add_buffers();
        bfs->set_pages(DEFAULT_PAGES);
        bfs->set_policy(ProfilerSessionConfig::BufferConfig::RECYCLE);
        sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
    }

    void SetCpuPluginConfig(CreateSessionRequest& createRequest, int pid)
    {
        ProfilerPluginConfig* ppc = createRequest.add_plugin_configs();
        ppc->set_name("cpu-plugin");
        ppc->set_sample_interval(SAMPLE_INTERVAL);
        CpuConfig cc;
        cc.set_pid(pid);
        std::vector<uint8_t> configData(cc.ByteSizeLong());
        cc.SerializeToArray(configData.data(), configData.size());
        ppc->set_config_data((const void*)configData.data(), configData.size());
        auto sessionConfig = createRequest.mutable_session_config();
        ProfilerSessionConfig::BufferConfig* bfs = sessionConfig->add_buffers();
        bfs->set_pages(DEFAULT_PAGES);
        bfs->set_policy(ProfilerSessionConfig::BufferConfig::RECYCLE);
        sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
    }

    void SetDiskioPluginConfig(CreateSessionRequest& createRequest)
    {
        ProfilerPluginConfig* ppc = createRequest.add_plugin_configs();
        ppc->set_name("diskio-plugin");
        ppc->set_sample_interval(SAMPLE_INTERVAL);
        auto sessionConfig = createRequest.mutable_session_config();
        ProfilerSessionConfig::BufferConfig* bfs = sessionConfig->add_buffers();
        bfs->set_pages(DEFAULT_PAGES);
        bfs->set_policy(ProfilerSessionConfig::BufferConfig::RECYCLE);
        sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
    }

    void SetProcessPluginConfig(CreateSessionRequest& createRequest)
    {
        ProfilerPluginConfig* ppc = createRequest.add_plugin_configs();
        ppc->set_name("process-plugin");
        ppc->set_sample_interval(SAMPLE_INTERVAL);
        ProcessConfig pc;
        pc.set_report_process_tree(true);
        std::vector<uint8_t> configData(pc.ByteSizeLong());
        pc.SerializeToArray(configData.data(), configData.size());
        ppc->set_config_data((const void*)configData.data(), configData.size());
        auto sessionConfig = createRequest.mutable_session_config();
        ProfilerSessionConfig::BufferConfig* bfs = sessionConfig->add_buffers();
        bfs->set_pages(DEFAULT_PAGES);
        bfs->set_policy(ProfilerSessionConfig::BufferConfig::RECYCLE);
        sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
    }

    void SetHilogPluginConfig(CreateSessionRequest& createRequest, bool isFirstConfig)
    {
        ProfilerPluginConfig* ppc = createRequest.add_plugin_configs();
        ppc->set_name("hilog-plugin");
        ppc->set_sample_interval(SAMPLE_INTERVAL);
        HilogConfig hc;
        hc.set_device_type(Type::HI3516);
        hc.set_need_record(true);
        hc.set_need_clear(true);
        std::vector<uint8_t> configData(hc.ByteSizeLong());
        hc.SerializeToArray(configData.data(), configData.size());
        ppc->set_config_data((const void*)configData.data(), configData.size());
        auto sessionConfig = createRequest.mutable_session_config();
        ProfilerSessionConfig::BufferConfig* bfs = sessionConfig->add_buffers();
        bfs->set_pages(DEFAULT_PAGES);
        bfs->set_policy(ProfilerSessionConfig::BufferConfig::RECYCLE);
        if (isFirstConfig) {
            sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
        } else {
            sessionConfig->set_session_mode(ProfilerSessionConfig::OFFLINE);
            sessionConfig->set_result_file(HILOG_RESULT_FILE);
        }
    }

    void SetNetworkPluginConfig(CreateSessionRequest& createRequest)
    {
        ProfilerPluginConfig* ppc = createRequest.add_plugin_configs();
        ppc->set_name("network-plugin");
        ppc->set_sample_interval(SAMPLE_INTERVAL);
        NetworkConfig nc;
        nc.add_pid(1);
        std::vector<uint8_t> configData(nc.ByteSizeLong());
        nc.SerializeToArray(configData.data(), configData.size());
        ppc->set_config_data((const void*)configData.data(), configData.size());
        auto sessionConfig = createRequest.mutable_session_config();
        ProfilerSessionConfig::BufferConfig* bfs = sessionConfig->add_buffers();
        bfs->set_pages(DEFAULT_PAGES);
        bfs->set_policy(ProfilerSessionConfig::BufferConfig::RECYCLE);
        sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
    }

    bool isExistSpecifyPlugin(GetCapabilitiesResponse& capResponse, std::string pluginName)
    {
        bool isExistPlugin = false;
        for (int i = 0; i < capResponse.capabilities().size(); i++) {
            ProfilerPluginCapability ppc = capResponse.capabilities()[i];
            if (ppc.name() == pluginName) {
                pluginVec_.push_back(ppc.name());
                isExistPlugin = true;
            }
        }
        return isExistPlugin;
    }

    bool GetPluginCapabilities(int count, GetCapabilitiesResponse& capResponse)
    {
        grpc::Status status;
        for (int i = 0; i < count; i++) {
            GetCapabilitiesRequest request;
            request.set_request_id(0);
            grpc::ClientContext context;
            status = profilerStub_->GetCapabilities(&context, request, &capResponse);
            if (!status.ok()) {
                return false;
            }
        }
        return true;
    }

    bool CreatePluginSession(uint32_t& sessionId, int pid, bool isFirstConfig = true)
    {
        CreateSessionResponse createResponse;
        grpc::ClientContext createContext;
        CreateSessionRequest createRequest = CreateSessionRequest();
        createRequest.set_request_id(0);
        for (int i = 0; i < pluginVec_.size(); i++) {
            if (pluginVec_[i] == "memory-plugin") {
                SetMemPluginConfig(createRequest, pid, isFirstConfig);
            } else if (pluginVec_[i] == "cpu-plugin") {
                SetCpuPluginConfig(createRequest, pid);
            } else if (pluginVec_[i] == "diskio-plugin") {
                SetDiskioPluginConfig(createRequest);
            } else if (pluginVec_[i] == "process-plugin") {
                SetProcessPluginConfig(createRequest);
            } else if (pluginVec_[i] == "hilog-plugin") {
                SetHilogPluginConfig(createRequest, isFirstConfig);
            } else if (pluginVec_[i] == "network-plugin") {
                SetNetworkPluginConfig(createRequest);
            }
        }
        grpc::Status createStatus = profilerStub_->CreateSession(&createContext, createRequest, &createResponse);
        sessionId = createResponse.session_id();
        return createStatus.ok();
    }

    bool KeepPluginSession(int count, uint32_t sessionId)
    {
        grpc::Status status;
        for (int i = 0; i < count; i++) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (!status.ok()) {
                return false;
            }
        }
        return true;
    }

    bool StartPluginSession(uint32_t sessionId)
    {
        StartSessionResponse startResponse;
        grpc::ClientContext startContext;
        StartSessionRequest startRequest = StartSessionRequest();
        startRequest.set_request_id(0);
        startRequest.set_session_id(sessionId);
        grpc::Status startStatus = profilerStub_->StartSession(&startContext, startRequest, &startResponse);
        return startStatus.ok();
    }

    bool StopPluginSession(uint32_t sessionId)
    {
        StopSessionResponse stopResponse;
        grpc::ClientContext stopContext;
        StopSessionRequest stopRequest = StopSessionRequest();
        stopRequest.set_request_id(0);
        stopRequest.set_session_id(sessionId);
        grpc::Status stopStatus = profilerStub_->StopSession(&stopContext, stopRequest, &stopResponse);
        return stopStatus.ok();
    }

    bool DestroyPluginSession(uint32_t sessionId)
    {
        DestroySessionResponse destroyResponse;
        grpc::ClientContext destroyContext;
        DestroySessionRequest destroyRequest = DestroySessionRequest();
        destroyRequest.set_request_id(0);
        destroyRequest.set_session_id(sessionId);
        grpc::Status destroyStatus = profilerStub_->DestroySession(&destroyContext, destroyRequest, &destroyResponse);
        return destroyStatus.ok();
    }

    bool StartActiveProcess(int& cpuActivePid)
    {
        int processNum = fork();
        if (processNum == 0) {
            // consume cpu
            CpuConfig protoConfig;
#if defined(__LP64__)
            void* handle = dlopen("/system/lib64/libcpudataplugin.z.so", RTLD_LAZY);
#else
            void* handle = dlopen("/system/lib/libcpudataplugin.z.so", RTLD_LAZY);
#endif
            if (handle == nullptr) {
                const int bufSize = 256;
                char buf[bufSize] = { 0 };
                strerror_r(errno, buf, bufSize);
                HILOG_ERROR(LOG_CORE, "test:dlopen err, errno(%d:%s)", errno, buf);
                return false;
            }

            PluginModuleStruct* cpuPlugin = (PluginModuleStruct*)dlsym(handle, "g_pluginModule");
            if (cpuPlugin == nullptr) {
                HILOG_ERROR(LOG_CORE, "test: cpuPlugin is null");
                return false;
            }

            // Serialize config
            protoConfig.set_pid(getpid());
            int configLength = protoConfig.ByteSizeLong();
            std::vector<uint8_t> configBuffer(configLength);
            protoConfig.SerializeToArray(configBuffer.data(), configLength);

            // run plugin
            std::vector<uint8_t> dataBuffer(cpuPlugin->resultBufferSizeHint);
            cpuPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength);

            // 循环上报数据消耗cpu
            while (1) {
                int len = cpuPlugin->callbacks->onPluginReportResult(dataBuffer.data(),
                    cpuPlugin->resultBufferSizeHint);
                if (len > 0) {
                    CpuData cpuData;
                    cpuData.ParseFromArray(dataBuffer.data(), len);
                }
                usleep(ACTIVE_PROCESS_SLEEP_US);
            }

            cpuPlugin->callbacks->onPluginSessionStop();
        } else {
            cpuActivePid = processNum;
        }
        return true;
    }

    void StartIdleProcess()
    {
        int processNum = fork();
        if (processNum == 0) {
            // not to do
            while (1) {
                sleep(1);
            }
        } else {
            idlePid_ = processNum;
        }
    }

    void StartRequestMemoryProcess(int& requestMemoryPid)
    {
        int processNum = fork();
        if (processNum == 0) {
            auto requestBuff = std::make_unique<char[]>(MB_PER_BYTE);
            if (requestBuff == nullptr) {
                const int bufSize = 256;
                char buf[bufSize] = { 0 };
                strerror_r(errno, buf, bufSize);
                HILOG_ERROR(LOG_CORE, "StartRequestMemoryProcess: malloc %zu fail, errno(%d:%s)",
                    MB_PER_BYTE, errno, buf);
            }
            while (1) {
                sleep(1);
            }
        } else {
            requestMemoryPid = processNum;
        }
    }

    void IoTest()
    {
        // 一次累加16B，直至100KB
        std::string str = "";
        while (str.length() < BLOCK_LEN) {
            str += "this is IO test.";
        }

        // 写100K数据
        FILE* writeFp = fopen(WRITE_FILE.c_str(), "w");
        if (writeFp == nullptr) {
            HILOG_ERROR(LOG_CORE, "fopen() error");
            return;
        }
        size_t len = fwrite(const_cast<char*>(str.c_str()), 1, BLOCK_LEN, writeFp);
        if (len < 0) {
            HILOG_ERROR(LOG_CORE, "fwrite() error");
            if (fclose(writeFp) != 0) {
                HILOG_ERROR(LOG_CORE, "fclose() error");
            }
            return;
        }
        int ret = fflush(writeFp);
        if (ret == EOF) {
            HILOG_ERROR(LOG_CORE, "fflush() error");
            if (fclose(writeFp) != 0) {
                HILOG_ERROR(LOG_CORE, "fclose() error");
            }
            return;
        }
        fsync(fileno(writeFp));
        ret = fclose(writeFp);
        if (ret != 0) {
            HILOG_ERROR(LOG_CORE, "fclose() error");
            return;
        }

        // delete file
        std::string command = "rm " + WRITE_FILE;
        system(command.c_str());
    }
    void StartTestDiskioProcess(int& diskioPid)
    {
        int processNum = fork();
        if (processNum == 0) {
            IoTest();
            while (1) {
                sleep(1);
            }
        } else {
            diskioPid = processNum;
        }
    }

private:
    std::unique_ptr<IProfilerService::Stub> profilerStub_;
    std::vector<std::string> pluginVec_;
    int hiprofilerdPid_ = -1;
    int hiprofilerPluginsPid_ = -1;
    pid_t idlePid_ = -1;
};

/**
 * @tc.name: server
 * @tc.desc: RPC interface performance for memory_plugin normal test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServicePerformanceTest, DFX_DFR_Hiprofiler_0010, Function | MediumTest | Level1)
{
    ASSERT_TRUE(profilerStub_ != nullptr);
    int requestMemoryPid = -1;
    StartIdleProcess();
    StartRequestMemoryProcess(requestMemoryPid);

    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1); // 睡眠1s确保hiprofilerd进程启动之后再启动hiprofiler_plugins进程
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);

    sleep(1); // 睡眠1s确保hiprofiler_plugins进程可以监控到插件

    Timer timer = {};
    GetCapabilitiesResponse capResponse;
    EXPECT_TRUE(GetPluginCapabilities(ROUND_COUNT, capResponse));
    auto timeCost = timer.ElapsedUs();
    printf("GetCapabilities %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    ASSERT_TRUE(isExistSpecifyPlugin(capResponse, "memory-plugin"));

    timer.Reset();
    uint32_t sessionId1;
    EXPECT_TRUE(CreatePluginSession(sessionId1, requestMemoryPid));
    timeCost = timer.ElapsedUs();
    printf("CreateSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // KeepSession
    timer.Reset();
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId1));
    timeCost = timer.ElapsedUs();
    printf("KeepSession %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart = true;
    std::thread keepSessionThread1([&]() {
        while (sendHeart) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId1);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    // StartSession
    timer.Reset();
    EXPECT_TRUE(StartPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StartSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // FetchData
    timer.Reset();
    std::vector<CpuData> cpuDataVec1;
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse = nullptr;
    grpc::ClientContext fetchContext;
    FetchDataRequest fetchRequest = FetchDataRequest();
    fetchRequest.set_request_id(0);
    fetchRequest.set_session_id(sessionId1);
    fetchResponse = profilerStub_->FetchData(&fetchContext, fetchRequest);
    ASSERT_TRUE(fetchResponse != nullptr);
    std::thread fetchDataThread1([&]() {
        int ict = 0;
        while (1) {
            FetchDataResponse rp;
            fetchResponse.get()->Read(&rp);
            for (int i = 0; i < rp.plugin_data().size(); i++) {
                ProfilerPluginData pd = rp.plugin_data(i);
                if (ict == ROUND_COUNT) {
                    return;
                }

                ASSERT_STREQ(pd.name().c_str(), "memory-plugin");

                MemoryData md;
                auto& data = pd.data();
                md.ParseFromArray(data.data(), data.size());
                EXPECT_LE(md.processesinfo(0).vm_size_kb(), md.processesinfo(1).vm_size_kb());

                EXPECT_GT(md.processesinfo(0).pid(), 0);
                EXPECT_GT(md.processesinfo(1).pid(), 0);
                EXPECT_STRNE(md.processesinfo(0).name().c_str(), "");
                EXPECT_STRNE(md.processesinfo(1).name().c_str(), "");
                EXPECT_GE(md.processesinfo(0).vm_size_kb(), 0);
                EXPECT_GE(md.processesinfo(1).vm_size_kb(), 0);
                EXPECT_GE(md.processesinfo(0).vm_rss_kb(), 0);
                EXPECT_GE(md.processesinfo(1).vm_rss_kb(), 0);
                EXPECT_GE(md.processesinfo(0).rss_anon_kb(), 0);
                EXPECT_GE(md.processesinfo(1).rss_anon_kb(), 0);
                EXPECT_GE(md.processesinfo(0).rss_file_kb(), 0);
                EXPECT_GE(md.processesinfo(1).rss_file_kb(), 0);
                EXPECT_GE(md.processesinfo(0).rss_shmem_kb(), 0);
                EXPECT_GE(md.processesinfo(1).rss_shmem_kb(), 0);
                EXPECT_GE(md.processesinfo(0).vm_swap_kb(), 0);
                EXPECT_GE(md.processesinfo(1).vm_swap_kb(), 0);
                EXPECT_GE(md.processesinfo(0).vm_locked_kb(), 0);
                EXPECT_GE(md.processesinfo(1).vm_locked_kb(), 0);
                EXPECT_GE(md.processesinfo(0).vm_hwm_kb(), 0);
                EXPECT_GE(md.processesinfo(1).vm_hwm_kb(), 0);

                EXPECT_TRUE(md.processesinfo(0).has_memsummary());
                EXPECT_GE(md.processesinfo(0).memsummary().java_heap(), 0);
                EXPECT_GE(md.processesinfo(1).memsummary().java_heap(), 0);
                EXPECT_GE(md.processesinfo(0).memsummary().native_heap(), 0);
                EXPECT_GE(md.processesinfo(1).memsummary().native_heap(), 0);
                EXPECT_GE(md.processesinfo(0).memsummary().code(), 0);
                EXPECT_GE(md.processesinfo(1).memsummary().code(), 0);
                EXPECT_GE(md.processesinfo(0).memsummary().stack(), 0);
                EXPECT_GE(md.processesinfo(1).memsummary().stack(), 0);
                EXPECT_GE(md.processesinfo(0).memsummary().graphics(), 0);
                EXPECT_GE(md.processesinfo(1).memsummary().graphics(), 0);
                EXPECT_GE(md.processesinfo(0).memsummary().private_other(), 0);
                EXPECT_GE(md.processesinfo(1).memsummary().private_other(), 0);
                EXPECT_GE(md.processesinfo(0).memsummary().system(), 0);
                EXPECT_GE(md.processesinfo(1).memsummary().system(), 0);

                EXPECT_EQ(md.meminfo().size(), MEMINFO_SIZE);
                EXPECT_EQ(md.meminfo(0).key(), SysMeminfoType::PMEM_MEM_TOTAL);
                EXPECT_GE(md.meminfo(0).value(), 0);
                EXPECT_EQ(md.meminfo(1).key(), SysMeminfoType::PMEM_MEM_FREE);
                EXPECT_GE(md.meminfo(1).value(), 0);

                EXPECT_EQ(md.vmeminfo().size(), 1);
                EXPECT_EQ(md.vmeminfo(0).key(), SysVMeminfoType::VMEMINFO_NR_FREE_PAGES);
                EXPECT_GE(md.vmeminfo(0).value(), 0);

                ict += 1;
            }
        }
    });
    fetchDataThread1.join();
    timeCost = timer.ElapsedUs();
    printf("FetchData cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, FETCHDATA_TIMEOUT_US);

    // StopSession
    timer.Reset();
    EXPECT_TRUE(StopPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StopSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // DestroySession
    timer.Reset();
    EXPECT_TRUE(DestroyPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("DestroySession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 销毁会话之后停止心跳发送
    sendHeart = false;
    keepSessionThread1.join();

    // 创建不同的memory config
    uint32_t sessionId2;
    EXPECT_TRUE(CreatePluginSession(sessionId2, idlePid_, false));
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId2));

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart2 = true;
    std::thread keepSessionThread2([&]() {
        while (sendHeart2) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId2);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart2) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    EXPECT_TRUE(StartPluginSession(sessionId2));

    // FetchData
    std::vector<CpuData> cpuDataVec2;
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse2 = nullptr;
    grpc::ClientContext fetchContext2;
    FetchDataRequest fetchRequest2 = FetchDataRequest();
    fetchRequest2.set_request_id(0);
    fetchRequest2.set_session_id(sessionId2);
    fetchResponse2 = profilerStub_->FetchData(&fetchContext2, fetchRequest2);
    ASSERT_TRUE(fetchResponse2 != nullptr);
    std::thread fetchDataThread2([&]() {
        int ict = 0;
        while (1) {
            FetchDataResponse rp;
            fetchResponse2.get()->Read(&rp);
            for (int i = 0; i < rp.plugin_data().size(); i++) {
                ProfilerPluginData pd = rp.plugin_data(i);
                if (ict == ROUND_COUNT) {
                    return;
                }

                ASSERT_STREQ(pd.name().c_str(), "memory-plugin");

                MemoryData md;
                auto& data = pd.data();
                md.ParseFromArray(data.data(), data.size());
                EXPECT_FALSE(md.processesinfo(0).has_memsummary());

                ict += 1;
            }
        }
    });
    fetchDataThread2.join();
    EXPECT_TRUE(StopPluginSession(sessionId2));
    EXPECT_TRUE(DestroyPluginSession(sessionId2));
    // 销毁会话之后停止心跳发送
    sendHeart2 = false;
    keepSessionThread2.join();

    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);
    StopProcessStub(idlePid_);
    StopProcessStub(requestMemoryPid);

    pluginVec_.clear();
}

/**
 * @tc.name: server
 * @tc.desc: RPC interface performance for cpu_plugin normal test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServicePerformanceTest, DFX_DFR_Hiprofiler_0020, Function | MediumTest | Level1)
{
    ASSERT_TRUE(profilerStub_ != nullptr);
    StartIdleProcess();

    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1); // 睡眠1s确保hiprofilerd进程启动之后再启动hiprofiler_plugins进程
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);

    sleep(1); // 睡眠1s确保hiprofiler_plugins进程可以监控到插件

    Timer timer = {};
    GetCapabilitiesResponse capResponse;
    EXPECT_TRUE(GetPluginCapabilities(ROUND_COUNT, capResponse));
    auto timeCost = timer.ElapsedUs();
    printf("GetCapabilities %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    ASSERT_TRUE(isExistSpecifyPlugin(capResponse, "cpu-plugin"));

    timer.Reset();
    uint32_t sessionId1;
    EXPECT_TRUE(CreatePluginSession(sessionId1, idlePid_));
    timeCost = timer.ElapsedUs();
    printf("CreateSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // KeepSession
    timer.Reset();
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId1));
    timeCost = timer.ElapsedUs();
    printf("KeepSession %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart = true;
    std::thread keepSessionThread1([&]() {
        while (sendHeart) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId1);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    // StartSession
    timer.Reset();
    EXPECT_TRUE(StartPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StartSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // FetchData
    timer.Reset();
    std::vector<CpuData> cpuDataVec1;
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse = nullptr;
    grpc::ClientContext fetchContext;
    FetchDataRequest fetchRequest = FetchDataRequest();
    fetchRequest.set_request_id(0);
    fetchRequest.set_session_id(sessionId1);
    fetchResponse = profilerStub_->FetchData(&fetchContext, fetchRequest);
    ASSERT_TRUE(fetchResponse != nullptr);
    std::thread fetchDataThread1([&]() {
        int ict = 0;
        while (1) {
            FetchDataResponse rp;
            fetchResponse.get()->Read(&rp);
            for (int i = 0; i < rp.plugin_data().size(); i++) {
                ProfilerPluginData pd = rp.plugin_data(i);
                if (ict == ROUND_COUNT) {
                    return;
                }

                ASSERT_STREQ(pd.name().c_str(), "cpu-plugin");

                CpuData cd;
                auto& data = pd.data();
                cd.ParseFromArray(data.data(), data.size());
                cpuDataVec1.push_back(cd);

                ict += 1;
            }
        }
    });
    fetchDataThread1.join();
    timeCost = timer.ElapsedUs();
    printf("FetchData cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, FETCHDATA_TIMEOUT_US);

    // StopSession
    timer.Reset();
    EXPECT_TRUE(StopPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StopSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // DestroySession
    timer.Reset();
    EXPECT_TRUE(DestroyPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("DestroySession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 销毁会话之后停止心跳发送
    sendHeart = false;
    keepSessionThread1.join();

    // 开启活跃进程获取数据
    int cpuActivePid = -1;
    StartActiveProcess(cpuActivePid);
    //睡眠一秒确保子进程开始工作
    sleep(1);
    uint32_t sessionId2;
    EXPECT_TRUE(CreatePluginSession(sessionId2, cpuActivePid));
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId2));

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart2 = true;
    std::thread keepSessionThread2([&]() {
        while (sendHeart2) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId2);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart2) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    EXPECT_TRUE(StartPluginSession(sessionId2));

    // FetchData
    std::vector<CpuData> cpuDataVec2;
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse2 = nullptr;
    grpc::ClientContext fetchContext2;
    FetchDataRequest fetchRequest2 = FetchDataRequest();
    fetchRequest2.set_request_id(0);
    fetchRequest2.set_session_id(sessionId2);
    fetchResponse2 = profilerStub_->FetchData(&fetchContext2, fetchRequest2);
    ASSERT_TRUE(fetchResponse2 != nullptr);
    std::thread fetchDataThread2([&]() {
        int ict = 0;
        while (1) {
            FetchDataResponse rp;
            fetchResponse2.get()->Read(&rp);
            for (int i = 0; i < rp.plugin_data().size(); i++) {
                ProfilerPluginData pd = rp.plugin_data(i);
                if (ict == ROUND_COUNT) {
                    return;
                }

                ASSERT_STREQ(pd.name().c_str(), "cpu-plugin");

                CpuData cd;
                auto& data = pd.data();
                cd.ParseFromArray(data.data(), data.size());
                cpuDataVec2.push_back(cd);

                ict += 1;
            }
        }
    });
    fetchDataThread2.join();
    EXPECT_TRUE(StopPluginSession(sessionId2));
    EXPECT_TRUE(DestroyPluginSession(sessionId2));
    // 销毁会话之后停止心跳发送
    sendHeart2 = false;
    keepSessionThread2.join();

    // 校验进程的cpu占用时间
    for (int i = 0; i < cpuDataVec1.size() && i < cpuDataVec2.size(); i++) {
        EXPECT_LT(cpuDataVec1[i].cpu_usage_info().process_cpu_time_ms(),
            cpuDataVec2[i].cpu_usage_info().process_cpu_time_ms());

        EXPECT_GE(cpuDataVec1[i].cpu_usage_info().process_cpu_time_ms(), 0);
        EXPECT_GE(cpuDataVec2[i].cpu_usage_info().process_cpu_time_ms(), 0);
        EXPECT_GT(cpuDataVec1[i].cpu_usage_info().system_cpu_time_ms(), 0);
        EXPECT_GT(cpuDataVec2[i].cpu_usage_info().system_cpu_time_ms(), 0);
        EXPECT_GT(cpuDataVec1[i].cpu_usage_info().system_boot_time_ms(), 0);
        EXPECT_GT(cpuDataVec2[i].cpu_usage_info().system_boot_time_ms(), 0);
        EXPECT_GE(cpuDataVec1[i].cpu_usage_info().cores().size(), 1);
        EXPECT_GE(cpuDataVec2[i].cpu_usage_info().cores().size(), 1);
        EXPECT_GE(cpuDataVec1[i].thread_info().size(), 1);
        EXPECT_GE(cpuDataVec2[i].thread_info().size(), 1);
        EXPECT_GT(cpuDataVec1[i].thread_info(0).tid(), 0);
        EXPECT_GT(cpuDataVec2[i].thread_info(0).tid(), 0);
        EXPECT_STRNE(cpuDataVec1[i].thread_info(0).thread_name().c_str(), "");
        EXPECT_STRNE(cpuDataVec2[i].thread_info(0).thread_name().c_str(), "");
        EXPECT_LE(cpuDataVec1[i].thread_info(0).thread_state(), ThreadState::THREAD_WAITING);
        EXPECT_LE(cpuDataVec2[i].thread_info(0).thread_state(), ThreadState::THREAD_WAITING);
        EXPECT_GE(cpuDataVec1[i].thread_info(0).thread_cpu_time_ms(), 0);
        EXPECT_GE(cpuDataVec2[i].thread_info(0).thread_cpu_time_ms(), 0);

        if (i > 0) {
            EXPECT_EQ(cpuDataVec1[i].cpu_usage_info().prev_process_cpu_time_ms(),
                cpuDataVec1[i-1].cpu_usage_info().process_cpu_time_ms());
            EXPECT_EQ(cpuDataVec2[i].cpu_usage_info().prev_process_cpu_time_ms(),
                cpuDataVec2[i-1].cpu_usage_info().process_cpu_time_ms());
            EXPECT_EQ(cpuDataVec1[i].cpu_usage_info().prev_system_cpu_time_ms(),
                cpuDataVec1[i-1].cpu_usage_info().system_cpu_time_ms());
            EXPECT_EQ(cpuDataVec2[i].cpu_usage_info().prev_system_cpu_time_ms(),
                cpuDataVec2[i-1].cpu_usage_info().system_cpu_time_ms());
            EXPECT_EQ(cpuDataVec1[i].cpu_usage_info().prev_system_boot_time_ms(),
                cpuDataVec1[i-1].cpu_usage_info().system_boot_time_ms());
            EXPECT_EQ(cpuDataVec2[i].cpu_usage_info().prev_system_boot_time_ms(),
                cpuDataVec2[i-1].cpu_usage_info().system_boot_time_ms());
            EXPECT_EQ(cpuDataVec1[i].thread_info(0).prev_thread_cpu_time_ms(),
                cpuDataVec1[i-1].thread_info(0).thread_cpu_time_ms());
            EXPECT_EQ(cpuDataVec2[i].thread_info(0).prev_thread_cpu_time_ms(),
                cpuDataVec2[i-1].thread_info(0).thread_cpu_time_ms());
        }
    }

    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);
    StopProcessStub(cpuActivePid);
    StopProcessStub(idlePid_);

    pluginVec_.clear();
}

/**
 * @tc.name: server
 * @tc.desc: RPC interface performance for diskio_plugin normal test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServicePerformanceTest, DFX_DFR_Hiprofiler_0030, Function | MediumTest | Level1)
{
    ASSERT_TRUE(profilerStub_ != nullptr);

    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1); // 睡眠1s确保hiprofilerd进程启动之后再启动hiprofiler_plugins进程
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);

    sleep(1); // 睡眠1s确保hiprofiler_plugins进程可以监控到插件

    Timer timer = {};
    GetCapabilitiesResponse capResponse;
    EXPECT_TRUE(GetPluginCapabilities(ROUND_COUNT, capResponse));
    auto timeCost = timer.ElapsedUs();
    printf("GetCapabilities %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    ASSERT_TRUE(isExistSpecifyPlugin(capResponse, "diskio-plugin"));

    timer.Reset();
    uint32_t sessionId1;
    EXPECT_TRUE(CreatePluginSession(sessionId1, 0));
    timeCost = timer.ElapsedUs();
    printf("CreateSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // KeepSession
    timer.Reset();
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId1));
    timeCost = timer.ElapsedUs();
    printf("KeepSession %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart = true;
    std::thread keepSessionThread([&]() {
        while (sendHeart) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId1);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    // StartSession
    timer.Reset();
    EXPECT_TRUE(StartPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StartSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // FetchData
    timer.Reset();
    std::vector<DiskioData> diskioDataVec1;
    std::vector<DiskioData> diskioDataVec2;
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse = nullptr;
    grpc::ClientContext fetchContext;
    FetchDataRequest fetchRequest = FetchDataRequest();
    fetchRequest.set_request_id(0);
    fetchRequest.set_session_id(sessionId1);
    fetchResponse = profilerStub_->FetchData(&fetchContext, fetchRequest);
    ASSERT_TRUE(fetchResponse != nullptr);
    int count = ROUND_COUNT + ROUND_COUNT + 1;
    int diskioPid = -1;
    std::thread fetchDataThread([&]() {
        int ict = 0;
        while (1) {
            FetchDataResponse rp;
            fetchResponse.get()->Read(&rp);
            for (int i = 0; i < rp.plugin_data().size(); i++) {
                ProfilerPluginData pd = rp.plugin_data(i);
                if (ict == count) {
                    return;
                }

                if (ict == ROUND_COUNT) { // 向磁盘写入100k的数据
                    StartTestDiskioProcess(diskioPid);
                    sleep(1);
                    ict += 1;
                    break;
                }

                ASSERT_STREQ(pd.name().c_str(), "diskio-plugin");

                DiskioData dd;
                auto& data = pd.data();
                dd.ParseFromArray(data.data(), data.size());
                if (ict < ROUND_COUNT) {
                    diskioDataVec1.push_back(dd);
                } else {
                    diskioDataVec2.push_back(dd);
                }

                ict += 1;
            }
        }
    });
    fetchDataThread.join();
    timeCost = timer.ElapsedUs();
    printf("FetchData cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, FETCHDATA_TIMEOUT_US);

    // StopSession
    timer.Reset();
    EXPECT_TRUE(StopPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StopSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // DestroySession
    timer.Reset();
    EXPECT_TRUE(DestroyPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("DestroySession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
    // 销毁会话之后停止心跳发送
    sendHeart = false;
    keepSessionThread.join();

    // 验证diskio上报的数据指标
    for (int i = 0; i < diskioDataVec1.size() && i < diskioDataVec2.size(); i++) {
        EXPECT_LE(diskioDataVec1[i].wr_sectors_kb() + WRITE_KB, diskioDataVec2[i].wr_sectors_kb());
        EXPECT_GT(diskioDataVec1[i].wr_sectors_kb(), 0);
        EXPECT_GT(diskioDataVec2[i].wr_sectors_kb(), 0);
        EXPECT_GT(diskioDataVec1[i].rd_sectors_kb(), 0);
        EXPECT_GT(diskioDataVec2[i].rd_sectors_kb(), 0);
    }

    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);
    StopProcessStub(diskioPid);

    pluginVec_.clear();
}

/**
 * @tc.name: server
 * @tc.desc: RPC interface performance for process_plugin normal test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServicePerformanceTest, DFX_DFR_Hiprofiler_0040, Function | MediumTest | Level1)
{
    ASSERT_TRUE(profilerStub_ != nullptr);

    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1); // 睡眠1s确保hiprofilerd进程启动之后再启动hiprofiler_plugins进程
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);

    sleep(1); // 睡眠1s确保hiprofiler_plugins进程可以监控到插件

    Timer timer = {};
    GetCapabilitiesResponse capResponse;
    EXPECT_TRUE(GetPluginCapabilities(ROUND_COUNT, capResponse));
    auto timeCost = timer.ElapsedUs();
    printf("GetCapabilities %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    ASSERT_TRUE(isExistSpecifyPlugin(capResponse, "process-plugin"));

    timer.Reset();
    uint32_t sessionId1;
    EXPECT_TRUE(CreatePluginSession(sessionId1, 0));
    timeCost = timer.ElapsedUs();
    printf("CreateSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // KeepSession
    timer.Reset();
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId1));
    timeCost = timer.ElapsedUs();
    printf("KeepSession %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart = true;
    std::thread keepSessionThread([&]() {
        while (sendHeart) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId1);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    // StartSession
    timer.Reset();
    EXPECT_TRUE(StartPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StartSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // FetchData
    timer.Reset();
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse = nullptr;
    grpc::ClientContext fetchContext;
    FetchDataRequest fetchRequest = FetchDataRequest();
    fetchRequest.set_request_id(0);
    fetchRequest.set_session_id(sessionId1);
    fetchResponse = profilerStub_->FetchData(&fetchContext, fetchRequest);
    ASSERT_TRUE(fetchResponse != nullptr);
    int hiprofilerdPidCount = 0;
    int hiprofilerPluginsPidCount = 0;
    std::thread fetchDataThread([&]() {
        int ict = 0;
        while (1) {
            FetchDataResponse rp;
            fetchResponse.get()->Read(&rp);
            for (int i = 0; i < rp.plugin_data().size(); i++) {
                ProfilerPluginData pd = rp.plugin_data(i);
                if (ict == ROUND_COUNT) {
                    return;
                }

                ASSERT_STREQ(pd.name().c_str(), "process-plugin");

                ProcessData processData;
                auto& data = pd.data();
                processData.ParseFromArray(data.data(), data.size());
                EXPECT_GT(static_cast<int>(processData.processesinfo().size()), 0);
                for (int j = 0; j < processData.processesinfo().size(); j++) {
                    ProcessInfo info = processData.processesinfo(j);
                    if (info.pid() == hiprofilerdPid_) {
                        hiprofilerdPidCount++;
                    } else if (info.pid() == hiprofilerPluginsPid_) {
                        hiprofilerPluginsPidCount++;
                    }
                }

                ict += 1;
            }
        }
    });
    fetchDataThread.join();
    timeCost = timer.ElapsedUs();
    printf("FetchData cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, FETCHDATA_TIMEOUT_US);
    EXPECT_EQ(hiprofilerdPidCount, ROUND_COUNT);
    EXPECT_EQ(hiprofilerPluginsPidCount, ROUND_COUNT);

    // StopSession
    timer.Reset();
    EXPECT_TRUE(StopPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StopSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // DestroySession
    timer.Reset();
    EXPECT_TRUE(DestroyPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("DestroySession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
    // 销毁会话之后停止心跳发送
    sendHeart = false;
    keepSessionThread.join();

    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);

    pluginVec_.clear();
}

/**
 * @tc.name: server
 * @tc.desc: RPC interface performance for hilog_plugin normal test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServicePerformanceTest, DFX_DFR_Hiprofiler_0050, Function | MediumTest | Level1)
{
    ASSERT_TRUE(profilerStub_ != nullptr);

    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1); // 睡眠1s确保hiprofilerd进程启动之后再启动hiprofiler_plugins进程
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);

    sleep(1); // 睡眠1s确保hiprofiler_plugins进程可以监控到插件

    Timer timer = {};
    GetCapabilitiesResponse capResponse;
    EXPECT_TRUE(GetPluginCapabilities(ROUND_COUNT, capResponse));
    auto timeCost = timer.ElapsedUs();
    printf("GetCapabilities %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    ASSERT_TRUE(isExistSpecifyPlugin(capResponse, "hilog-plugin"));

    timer.Reset();
    uint32_t sessionId1;
    EXPECT_TRUE(CreatePluginSession(sessionId1, 0, true));
    timeCost = timer.ElapsedUs();
    printf("CreateSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // KeepSession
    timer.Reset();
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId1));
    timeCost = timer.ElapsedUs();
    printf("KeepSession %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart = true;
    std::thread keepSessionThread([&]() {
        while (sendHeart) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId1);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    // StartSession
    timer.Reset();
    EXPECT_TRUE(StartPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StartSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // FetchData
    timer.Reset();
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse = nullptr;
    grpc::ClientContext fetchContext;
    FetchDataRequest fetchRequest = FetchDataRequest();
    fetchRequest.set_request_id(0);
    fetchRequest.set_session_id(sessionId1);
    fetchResponse = profilerStub_->FetchData(&fetchContext, fetchRequest);
    ASSERT_TRUE(fetchResponse != nullptr);
    FetchDataResponse rp;
    fetchResponse.get()->Read(&rp);
    for (int i = 0; i < rp.plugin_data().size() && i < ROUND_COUNT; i++) {
        ProfilerPluginData pd = rp.plugin_data(i);
        HilogInfo hilogInfo;
        auto& data = pd.data();
        hilogInfo.ParseFromArray(data.data(), data.size());
        EXPECT_GT(static_cast<int>(hilogInfo.info().size()), 0);
        EXPECT_TRUE(hilogInfo.info(0).has_detail());
        if (hilogInfo.info(0).detail().pid() > 0) {
            EXPECT_STRNE(hilogInfo.info(0).context().c_str(), "");
        }
    }
    timeCost = timer.ElapsedUs();
    printf("FetchData cost %ldus.\n", timeCost);

    // StopSession
    timer.Reset();
    EXPECT_TRUE(StopPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StopSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // DestroySession
    timer.Reset();
    EXPECT_TRUE(DestroyPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("DestroySession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
    // 销毁会话之后停止心跳发送
    sendHeart = false;
    keepSessionThread.join();

    // 验证hilog离线模式是否生成hilogResultFile离线文件
    uint32_t sessionId2;
    EXPECT_TRUE(CreatePluginSession(sessionId2, 0, false));
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId2));

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart2 = true;
    std::thread keepSessionThread2([&]() {
        while (sendHeart2) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId2);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart2) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    EXPECT_TRUE(StartPluginSession(sessionId2));
    usleep(TIMEOUT_US);
    EXPECT_EQ(access(HILOG_RESULT_FILE.c_str(), F_OK), 0);
    EXPECT_TRUE(StopPluginSession(sessionId2));
    EXPECT_TRUE(DestroyPluginSession(sessionId2));
    // 销毁会话之后停止心跳发送
    sendHeart2 = false;
    keepSessionThread2.join();

    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);

    pluginVec_.clear();
}

/**
 * @tc.name: server
 * @tc.desc: RPC interface performance for network_plugin normal test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServicePerformanceTest, DFX_DFR_Hiprofiler_0060, Function | MediumTest | Level1)
{
    ASSERT_TRUE(profilerStub_ != nullptr);

    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1); // 睡眠1s确保hiprofilerd进程启动之后再启动hiprofiler_plugins进程
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);

    sleep(1); // 睡眠1s确保hiprofiler_plugins进程可以监控到插件

    Timer timer = {};
    GetCapabilitiesResponse capResponse;
    EXPECT_TRUE(GetPluginCapabilities(ROUND_COUNT, capResponse));
    auto timeCost = timer.ElapsedUs();
    printf("GetCapabilities %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    ASSERT_TRUE(isExistSpecifyPlugin(capResponse, "network-plugin"));

    timer.Reset();
    uint32_t sessionId1;
    EXPECT_TRUE(CreatePluginSession(sessionId1, 0));
    timeCost = timer.ElapsedUs();
    printf("CreateSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // KeepSession
    timer.Reset();
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId1));
    timeCost = timer.ElapsedUs();
    printf("KeepSession %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart = true;
    std::thread keepSessionThread([&]() {
        while (sendHeart) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId1);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    // StartSession
    timer.Reset();
    EXPECT_TRUE(StartPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StartSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // FetchData network_plugin在开发板上没有节点，不上报数据
    timer.Reset();
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse = nullptr;
    grpc::ClientContext fetchContext;
    FetchDataRequest fetchRequest = FetchDataRequest();
    fetchRequest.set_request_id(0);
    fetchRequest.set_session_id(sessionId1);
    fetchResponse = profilerStub_->FetchData(&fetchContext, fetchRequest);
    ASSERT_TRUE(fetchResponse != nullptr);
    bool recvData = true;
    std::thread fetchDataThread([&]() {
        while (recvData) {
            FetchDataResponse rp;
            fetchResponse.get()->Read(&rp);
            EXPECT_EQ(rp.plugin_data().size(), 0);
        }
    });
    usleep(TIMEOUT_US);

    // StopSession
    timer.Reset();
    EXPECT_TRUE(StopPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("StopSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    recvData = false;
    fetchDataThread.join();

    // DestroySession
    timer.Reset();
    EXPECT_TRUE(DestroyPluginSession(sessionId1));
    timeCost = timer.ElapsedUs();
    printf("DestroySession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
    // 销毁会话之后停止心跳发送
    sendHeart = false;
    keepSessionThread.join();

    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);

    pluginVec_.clear();
}

/**
 * @tc.name: server
 * @tc.desc: RPC interface performance unusual test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServicePerformanceTest, DFX_DFR_Hiprofiler_0070, Function | MediumTest | Level1)
{
    ASSERT_TRUE(profilerStub_ != nullptr);

    uint32_t sessionId;
    GetCapabilitiesResponse capResponse1;
    EXPECT_FALSE(GetPluginCapabilities(1, capResponse1));
    EXPECT_FALSE(CreatePluginSession(sessionId, 1));
    EXPECT_FALSE(KeepPluginSession(1, sessionId));
    EXPECT_FALSE(StartPluginSession(sessionId));
    EXPECT_FALSE(StartPluginSession(sessionId));
    EXPECT_FALSE(DestroyPluginSession(sessionId));

    StartServerStub(DEFAULT_HIPROFILERD_PATH);
    sleep(1); // 睡眠1s确保hiprofilerd进程启动之后再启动hiprofiler_plugins进程
    StartServerStub(DEFAULT_HIPROFILER_PLUGINS_PATH);

    sleep(1); // 睡眠1s确保hiprofiler_plugins进程可以监控到插件

    Timer timer = {};
    GetCapabilitiesResponse capResponse;
    EXPECT_TRUE(GetPluginCapabilities(1, capResponse));
    auto timeCost = timer.ElapsedUs();
    printf("UnusualTest: GetCapabilities 1 time cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    ASSERT_TRUE(isExistSpecifyPlugin(capResponse, "cpu-plugin"));

    timer.Reset();
    EXPECT_TRUE(CreatePluginSession(sessionId, 1));
    timeCost = timer.ElapsedUs();
    printf("UnusualTest: CreateSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // KeepSession,5s超时心跳
    timer.Reset();
    EXPECT_TRUE(KeepPluginSession(1, sessionId));
    timeCost = timer.ElapsedUs();
    printf("UnusualTest: KeepSession 1 time cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 睡眠5s，确保seesionId已失效
    usleep(KEEP_SESSION_TIME_US);

    // StartSession
    timer.Reset();
    EXPECT_FALSE(StartPluginSession(sessionId));
    timeCost = timer.ElapsedUs();
    printf("UnusualTest: StartSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // FetchData
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse = nullptr;
    grpc::ClientContext fetchContext;
    FetchDataRequest fetchRequest = FetchDataRequest();
    fetchRequest.set_request_id(0);
    fetchRequest.set_session_id(sessionId);
    fetchResponse = profilerStub_->FetchData(&fetchContext, fetchRequest);
    ASSERT_TRUE(fetchResponse != nullptr);
    FetchDataResponse rp;
    fetchResponse.get()->Read(&rp);
    EXPECT_EQ(rp.plugin_data().size(), 0);

    // StopSession
    timer.Reset();
    EXPECT_FALSE(StopPluginSession(sessionId));
    timeCost = timer.ElapsedUs();
    printf("UnusualTest: StopSession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // DestroySession
    timer.Reset();
    EXPECT_FALSE(DestroyPluginSession(sessionId));
    timeCost = timer.ElapsedUs();
    printf("UnusualTest: DestroySession cost %ldus.\n", timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    // 验证心跳正常，pid为1的cpu插件数据上报
    GetCapabilitiesResponse capResponse2;
    EXPECT_TRUE(GetPluginCapabilities(ROUND_COUNT, capResponse2));

    uint32_t sessionId1;
    EXPECT_TRUE(CreatePluginSession(sessionId1, 1));

    // KeepSession
    EXPECT_TRUE(KeepPluginSession(ROUND_COUNT, sessionId1));

    // 开启每隔4s发一次心跳的线程，确保seesionid不会失效
    bool sendHeart = true;
    std::thread keepSessionThread1([&]() {
        while (sendHeart) {
            KeepSessionRequest request;
            request.set_request_id(0);
            request.set_session_id(sessionId1);
            request.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext context;
            KeepSessionResponse ksResponse;
            grpc::Status status = profilerStub_->KeepSession(&context, request, &ksResponse);
            if (sendHeart) {
                EXPECT_TRUE(status.ok());
            }
            usleep(KEEP_SESSION_SLEEP_US);
        }
    });

    // StartSession
    EXPECT_TRUE(StartPluginSession(sessionId1));

    // FetchData
    std::vector<CpuData> cpuDataVec1;
    std::unique_ptr<grpc::ClientReader<FetchDataResponse>> fetchResponse1 = nullptr;
    grpc::ClientContext fetchContext2;
    FetchDataRequest fetchRequest1 = FetchDataRequest();
    fetchRequest1.set_request_id(0);
    fetchRequest1.set_session_id(sessionId1);
    fetchResponse1 = profilerStub_->FetchData(&fetchContext2, fetchRequest1);
    ASSERT_TRUE(fetchResponse1 != nullptr);
    std::thread fetchDataThread1([&]() {
        int ict = 0;
        while (1) {
            FetchDataResponse rp;
            fetchResponse1.get()->Read(&rp);
            for (int i = 0; i < rp.plugin_data().size(); i++) {
                ProfilerPluginData pd = rp.plugin_data(i);
                if (ict == ROUND_COUNT) {
                    return;
                }

                ASSERT_STREQ(pd.name().c_str(), "cpu-plugin");
                ict += 1;
            }
        }
    });
    fetchDataThread1.join();

    // StopSession
    EXPECT_TRUE(StopPluginSession(sessionId1));

    // DestroySession
    EXPECT_TRUE(DestroyPluginSession(sessionId1));

    // 销毁会话之后停止心跳发送
    sendHeart = false;
    keepSessionThread1.join();

    StopProcessStub(hiprofilerPluginsPid_);
    StopProcessStub(hiprofilerdPid_);

    pluginVec_.clear();
}
}

#pragma clang optimize on
