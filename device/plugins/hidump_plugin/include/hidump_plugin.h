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

#ifndef HIDUMP_PLUGIN_H
#define HIDUMP_PLUGIN_H

#include "logging.h"
#include "plugin_module_api.h"
#include "hidump_plugin_config.pb.h"
#include "hidump_plugin_result.pb.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include <unistd.h>
#include <sys/wait.h>
#include <csignal>

class HidumpPlugin {
public:
    HidumpPlugin();
    ~HidumpPlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Stop();

    int SetWriter(WriterStruct* writer);
    void Loop(void);

private:
    // for ut
    void SetConfig(HidumpConfig& config);
    int SetTestCmd(const char *test_cmd);
    const char *GetTestCmd();
    char *testCmd_ = nullptr;
private:
    static FILE* CustomPopen(const char* command, const char* type);
    static int CustomPclose(FILE* fp);
    bool ParseHidumpInfo(HidumpInfo& dataProto, char *buf);

    HidumpConfig protoConfig_;
    std::vector<char> buffer_;
    WriterStruct* resultWriter_ = nullptr;
    std::mutex mutex_;
    std::thread writeThread_;
    std::atomic<bool> running_ = true;
    std::unique_ptr<FILE, int (*)(FILE*)> fp_;
};

#endif // HIDUMP_PLUGIN_H