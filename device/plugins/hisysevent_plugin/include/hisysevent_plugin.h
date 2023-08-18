/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HISYSEVENT_PLUGIN_H
#define HISYSEVENT_PLUGIN_H

#include "hisysevent_plugin_config.pb.h"
#include "hisysevent_plugin_result.pb.h"
#include "common.h"
#include "logging.h"
#include "plugin_module_api.h"

#include <atomic>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

class HisyseventPlugin {
public:
    HisyseventPlugin();
    ~HisyseventPlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Stop();
    void Run(void);

    int SetWriter(WriterStruct* writer);

private:
    std::string GetFullCmd();
    bool InitHisyseventCmd();

    bool ParseSyseventLineInfo(const char* data, size_t len, HisyseventInfo& dataProto);

    bool WriteResult(const HisyseventInfo& dataProto);

private:
    std::atomic<uint64_t> id_;
    std::vector<std::string> fullCmd_;

    std::thread workThread_;
    std::atomic<bool> running_ = true;

    std::unique_ptr<FILE, std::function<int (FILE*)>> fp_;

    HisyseventConfig protoConfig_;
    std::vector<char> protoBuffer_;
    WriterStruct* resultWriter_ = nullptr;

    volatile pid_t childPid_ = -1;
    int pipeFds_[2] = {-1, -1};
};
#endif // !HISYSEVENT_PLUGIN_H
