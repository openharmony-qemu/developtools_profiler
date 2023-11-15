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

#ifndef HILOG_PLUGIN_H
#define HILOG_PLUGIN_H

#include "file_cache.h"
#include "hilog_plugin_config.pb.h"
#include "hilog_plugin_result.pb.h"
#include "logging.h"
#include "plugin_module_api.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

class HilogPlugin {
public:
    HilogPlugin();
    ~HilogPlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Stop();

    int SetWriter(WriterStruct* writer);
    void Run(void);

private:
    std::string GetPidCmd();
    std::string GetlevelCmd();
    void InitHilogCmd();

    bool OpenLogFile();
    int GetDateTime(char* psDateTime, uint32_t size);

    template <typename T> void ParseLogLineInfo(const char* data, size_t len, T& hilogLineInfo);

    template <typename T> void ParseLogLineData(const char* data, size_t len, T hilogInfoProto);

    template <typename T> bool SetHilogLineDetails(const char* data, T& hilogLineInfo);

    bool TimeStringToNS(const char* data, struct timespec *ts);
    bool FindFirstNum(char** p);
    bool RemoveSpaces(char** p);
    bool FindFirstSpace(char** p);

    bool StringToL(const char* word, long& value);
    // for ut
    void SetConfig(HilogConfig& config)
    {
        protoConfig_ = config;
        return;
    }

    template <typename T> void FlushData(const T hilogLineProto);
    template <typename T> void FlushDataOptimize(const T hilogLineProto);

private:
    HilogConfig protoConfig_;
    std::vector<char> protoBuffer_;
    std::vector<char> dataBuffer_;
    WriterStruct* resultWriter_ = nullptr;
    std::mutex mutex_;
    std::thread workThread_;
    std::atomic<bool> running_ = true;
    std::vector<std::string> fullCmd_;
    std::unique_ptr<FILE, std::function<int (FILE*)>> fp_;
    std::unique_ptr<FileCache> fileCache_ = nullptr;
    int pipeFds_[2] = {-1, -1};
    volatile pid_t childPid_ = -1;
};

#endif // !HILOG_PLUGIN_H