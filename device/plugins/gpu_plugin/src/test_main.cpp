/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#include "gpu_plugin_config.pb.h"
#include "gpu_plugin_result.pb.h"
#include "plugin_module_api.h"

namespace {
    int g_testCount = 10;
}

int main(int agrc, char* agrv[])
{
    GpuConfig protoConfig;
    PluginModuleStruct* gpuPlugin;
    void* handle = dlopen("./libgpudataplugin.z.so", RTLD_LAZY);
    if (handle == nullptr) {
        std::cout << "test:dlopen err: " << dlerror() << std::endl;
        return 0;
    }
    std::cout << "test:handle = " << handle << std::endl;
    gpuPlugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    std::cout << "test:name = " << gpuPlugin->name << std::endl;
    std::cout << "test:buffer size = " << gpuPlugin->resultBufferSizeHint << std::endl;

    // Serialize config
    int configLength = protoConfig.ByteSizeLong();
    std::vector<uint8_t> configBuffer(configLength);
    int ret = protoConfig.SerializeToArray(configBuffer.data(), configBuffer.size());
    std::cout << "test:configLength = " << configLength << std::endl;
    std::cout << "test:serialize success start plugin ret = " << ret << std::endl;

    // Start
    std::vector<uint8_t> dataBuffer(gpuPlugin->resultBufferSizeHint);
    gpuPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength);
    while (g_testCount--) {
        int len = gpuPlugin->callbacks->onPluginReportResult(dataBuffer.data(),
                                                             gpuPlugin->resultBufferSizeHint);
        std::cout << "test:filler buffer length = " << len << std::endl;

        if (len > 0) {
            GpuData gpuData;
            gpuData.ParseFromArray(dataBuffer.data(), len);
            std::cout << "test:ParseFromArray length = " << len << std::endl;
            std::cout << "gpu_utilisation:" << gpuData.gpu_utilisation() << std::endl;
        }

        std::cout << "test:sleep...................." << std::endl;
        sleep(1);
    }
    gpuPlugin->callbacks->onPluginSessionStop();
    dlclose(handle);

    return 0;
}