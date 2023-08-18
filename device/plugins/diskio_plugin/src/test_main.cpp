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

#include "diskio_plugin_config.pb.h"
#include "diskio_plugin_result.pb.h"
#include "plugin_module_api.h"

namespace {
int g_testCount = 10;
const std::string writeFile = "/data/local/tmp/diskio_write_test.txt";
constexpr int BLOCK_LEN = 100 * 1024;
constexpr int SLEEP_TIME = 10;

void IoTest()
{
    // 一次累加16B，直至100KB
    std::string str = "";
    while (str.length() < BLOCK_LEN) {
        str += "this is IO test.";
    }

    // 一次写100K数据，写10次
    int count = 0;
    FILE* writeFp = fopen(writeFile.c_str(), "w");
    if (writeFp == nullptr) {
        printf("fopen() error");
        return;
    }
    while (count < g_testCount) {
        (void)fwrite(const_cast<char*>(str.c_str()), 1, BLOCK_LEN, writeFp);
        fflush(writeFp);
        fsync(fileno(writeFp));
        count++;
    }
    (void)fclose(writeFp);

    // delete file
    std::string command = "rm " + writeFile;
    system(command.c_str());
}
} // namespace

int main(int agrc, char* agrv[])
{
    bool isTestDiskIO = false;
    for (int i = 1; i < agrc; i++) {
        isTestDiskIO = atoi(agrv[i]);
    }

    if (isTestDiskIO) {
        IoTest();
        sleep(SLEEP_TIME);
    } else {
        DiskioConfig protoConfig;
        PluginModuleStruct* diskioPlugin;
        void* handle = dlopen("./libdiskiodataplugin.z.so", RTLD_LAZY);
        if (handle == nullptr) {
            std::cout << "test:dlopen err: " << dlerror() << std::endl;
            return 0;
        }
        std::cout << "test:handle = " << handle << std::endl;
        diskioPlugin = (PluginModuleStruct*)dlsym(handle, "g_pluginModule");
        std::cout << "test:name = " << diskioPlugin->name << std::endl;
        std::cout << "test:buffer size = " << diskioPlugin->resultBufferSizeHint << std::endl;

        // Serialize config
        int configLength = protoConfig.ByteSizeLong();
        std::vector<uint8_t> configBuffer(configLength);
        int ret = protoConfig.SerializeToArray(configBuffer.data(), configBuffer.size());
        std::cout << "test:configLength = " << configLength << std::endl;
        std::cout << "test:serialize success start plugin ret = " << ret << std::endl;

        // Start
        std::vector<uint8_t> dataBuffer(diskioPlugin->resultBufferSizeHint);
        diskioPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength);
        while (g_testCount--) {
            int len = diskioPlugin->callbacks->onPluginReportResult(dataBuffer.data(),
                                                                    diskioPlugin->resultBufferSizeHint);
            std::cout << "test:filler buffer length = " << len << std::endl;

            if (len > 0) {
                DiskioData diskioData;
                diskioData.ParseFromArray(dataBuffer.data(), len);
                std::cout << "test:ParseFromArray length = " << len << std::endl;

                std::cout << "prev_rd_sectors_kb:" << diskioData.prev_rd_sectors_kb() << std::endl;
                std::cout << "prev_wr_sectors_kb:" << diskioData.prev_wr_sectors_kb() << std::endl;
                std::cout << "prev_timestamp.tv_sec:" << diskioData.prev_timestamp().tv_sec() << std::endl;
                std::cout << "prev_timestamp.tv_nsec:" << diskioData.prev_timestamp().tv_nsec() << std::endl;
                std::cout << "rd_sectors_kb:" << diskioData.rd_sectors_kb() << std::endl;
                std::cout << "wr_sectors_kb:" << diskioData.wr_sectors_kb() << std::endl;
                std::cout << "timestamp.tv_sec:" << diskioData.timestamp().tv_sec() << std::endl;
                std::cout << "timestamp.tv_nsec:" << diskioData.timestamp().tv_nsec() << std::endl;
            }

            std::cout << "test:sleep...................." << std::endl;
            sleep(1);
        }
        diskioPlugin->callbacks->onPluginSessionStop();
        dlclose(handle);
    }

    return 0;
}
