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
#include <unistd.h>

#include "cpu_plugin_config.pb.h"
#include "cpu_plugin_result.pb.h"
#include "logging.h"
#include "plugin_module_api.h"

namespace {
int g_testCount = 100;
constexpr int CONSUME_CPU_SLEEP_TIME = 500 * 1000;
constexpr int PROCESS_SLEEP_TIME = 10;
#if defined(__LP64__)
const std::string SO_PATH = "/system/lib64/libcpudataplugin.z.so";
#else
const std::string SO_PATH = "/system/lib/libcpudataplugin.z.so";
#endif
} // namespace

static void Report(PluginModuleStruct*& cpuPlugin, std::vector<uint8_t>& dataBuffer)
{
    while (g_testCount--) {
        int len = cpuPlugin->callbacks->onPluginReportResult(dataBuffer.data(), cpuPlugin->resultBufferSizeHint);
        std::cout << "test:filler buffer length = " << len << std::endl;

        if (len > 0) {
            CpuData cpuData;
            cpuData.ParseFromArray(dataBuffer.data(), len);
            std::cout << "test:ParseFromArray length = " << len << std::endl;

            CpuUsageInfo cpuUsageInfo = cpuData.cpu_usage_info();
            std::cout << "prev_process_cpu_time_ms:" << cpuUsageInfo.prev_process_cpu_time_ms() << std::endl;
            std::cout << "prev_system_cpu_time_ms:" << cpuUsageInfo.prev_system_cpu_time_ms() << std::endl;
            std::cout << "prev_system_boot_time_ms:" << cpuUsageInfo.prev_system_boot_time_ms() << std::endl;
            std::cout << "process_cpu_time_ms:" << cpuUsageInfo.process_cpu_time_ms() << std::endl;
            std::cout << "system_cpu_time_ms:" << cpuUsageInfo.system_cpu_time_ms() << std::endl;
            std::cout << "system_boot_time_ms:" << cpuUsageInfo.system_boot_time_ms() << std::endl;

            for (int i = 0; i < cpuUsageInfo.cores_size(); i++) {
                CpuCoreUsageInfo cpuCoreUsageInfo = cpuUsageInfo.cores()[i];
                std::cout << "cpu_core:" << cpuCoreUsageInfo.cpu_core() << std::endl;
                std::cout << "prev_system_cpu_time_ms:" << cpuCoreUsageInfo.prev_system_cpu_time_ms() << std::endl;
                std::cout << "prev_system_boot_time_ms:" << cpuCoreUsageInfo.prev_system_boot_time_ms() << std::endl;
                std::cout << "system_cpu_time_ms:" << cpuCoreUsageInfo.system_cpu_time_ms() << std::endl;
                std::cout << "system_boot_time_ms:" << cpuCoreUsageInfo.system_boot_time_ms() << std::endl;
            }

            for (int i = 0; i < cpuData.thread_info_size(); i++) {
                ThreadInfo threadInfo = cpuData.thread_info()[i];
                std::cout << "tid : " << threadInfo.tid() << std::endl;
                std::cout << "thread_name : " << threadInfo.thread_name() << std::endl;
                std::cout << "thread_state : " << threadInfo.thread_state() << std::endl;
                std::cout << "prev_thread_cpu_time_ms : " << threadInfo.prev_thread_cpu_time_ms() << std::endl;
                std::cout << "thread_cpu_time_ms : " << threadInfo.thread_cpu_time_ms() << std::endl;
            }
        }

        std::cout << "test:sleep...................." << std::endl;
        usleep(CONSUME_CPU_SLEEP_TIME);
    }
}

int main(int agrc, char* agrv[])
{
    bool isConsumeCpu = false;
    for (int i = 1; i < agrc; i++) {
        isConsumeCpu = atoi(agrv[i]);
    }

    if (isConsumeCpu || agrc == 1) {
        CpuConfig protoConfig;
        void* handle = dlopen(SO_PATH.c_str(), RTLD_LAZY);
        if (handle == nullptr) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "test:dlopen err, errno(%d:%s)", errno, buf);
            return 0;
        }

        PluginModuleStruct* cpuPlugin = (PluginModuleStruct*)dlsym(handle, "g_pluginModule");
        if (cpuPlugin == nullptr) {
            dlclose(handle);
            return 0;
        }

        // Serialize config
        int pid = getpid();
        protoConfig.set_pid(pid);
        int configLength = protoConfig.ByteSizeLong();
        std::vector<uint8_t> configBuffer(configLength);
        protoConfig.SerializeToArray(configBuffer.data(), configLength);

        // run plugin
        std::vector<uint8_t> dataBuffer(cpuPlugin->resultBufferSizeHint);
        cpuPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength);
        if (agrc == 1) {
            Report(cpuPlugin, dataBuffer);
        } else {
            // 循环上报数据消耗cpu
            while (1) {
                int len = cpuPlugin->callbacks->onPluginReportResult(dataBuffer.data(),
                    cpuPlugin->resultBufferSizeHint);
                if (len > 0) {
                    CpuData cpuData;
                    cpuData.ParseFromArray(dataBuffer.data(), len);
                }
                const int interval = 100000;
                usleep(interval);
            }
        }
        cpuPlugin->callbacks->onPluginSessionStop();
        dlclose(handle);
    }

    sleep(PROCESS_SLEEP_TIME);
    return 0;
}
