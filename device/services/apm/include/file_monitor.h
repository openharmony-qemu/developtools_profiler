/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <cstdint>
#include <string>
#include "nocopyable.h"
#include "i_semaphore.h"

class FileMonitor {
public:
    using FileFilter = std::function<bool(std::string& filename)>;
    FileMonitor(const std::string& listenDir, FileFilter filter);
    ~FileMonitor();
    bool Start();
    void Finalize();
    std::string GetResultFile(int32_t waitMs);

private:
    bool Init();
    void Run();
    bool ProcessEvent();

    int inotifyFd_ {-1};
    int wd_ {-1};
    std::thread thread_;
    std::atomic<bool> running_ {false};
    std::atomic<bool> started_ {false};
    std::string listenDir_;
    std::string resultFile_;
    FileFilter filter_;
    SemaphorePtr sem_ {nullptr};

    DISALLOW_COPY_AND_MOVE(FileMonitor);
};

#endif // FileMonitor