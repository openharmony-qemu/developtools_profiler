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

#ifndef STREAM_PLUGIN_H
#define STREAM_PLUGIN_H

#include "logging.h"
#include "plugin_module_api.h"
#include "stream_plugin_config.pb.h"
#include "stream_plugin_result.pb.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

class StreamPlugin {
public:
    StreamPlugin();
    ~StreamPlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Stop();

    int SetWriter(WriterStruct* writer);
    void Loop(void);

private:
    StreamConfig protoConfig_;
    std::vector<char> buffer_;
    WriterStruct* resultWriter_ = nullptr;

    std::mutex mutex_;
    std::thread writeThread_;
    std::atomic<bool> running_ = true;
};

#endif // STREAM_PLUGIN_H