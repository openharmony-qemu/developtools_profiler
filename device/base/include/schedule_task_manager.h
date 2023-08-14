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

#ifndef OHOS_PROFILER_SCHEDULE_TASK_MANAGER_H
#define OHOS_PROFILER_SCHEDULE_TASK_MANAGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include "logging.h"
#include "nocopyable.h"

class ScheduleTaskManager {
public:
    static ScheduleTaskManager& GetInstance();

    using ms = std::chrono::milliseconds;

    bool ScheduleTask(const std::string& name,
                      const std::function<void(void)>& callback,
                      const std::chrono::milliseconds& repeatInterval);

    bool ScheduleTask(const std::string& name,
                      const std::function<void(void)>& callback,
                      const std::chrono::milliseconds& repeatInterval,
                      std::chrono::milliseconds initialDelay);

    bool UnscheduleTask(const std::string& name);

    void Shutdown();

    ScheduleTaskManager();
    ~ScheduleTaskManager();

private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    struct Task {
        std::string name;
        std::function<void(void)> callback;
        std::chrono::milliseconds repeatInterval;
        std::chrono::milliseconds initialDelay;
        TimePoint nextRunTime;
    };
    using SharedTask = STD_PTR(shared, Task);
    using WeakTask = STD_PTR(weak, Task);

    static std::chrono::milliseconds NormalizeInterval(std::chrono::milliseconds interval);

    void DumpTask(const SharedTask& task);

    void ScheduleThread();

    WeakTask TakeFront();

private:
    mutable std::mutex taskMutex_;
    std::condition_variable taskCv_;
    std::atomic<bool> runScheduleThread_ = true;
    std::multimap<TimePoint, WeakTask> timeMap_;          // schedule list
    std::unordered_map<std::string, SharedTask> taskMap_; // task details
    std::thread scheduleThread_;

    DISALLOW_COPY_AND_MOVE(ScheduleTaskManager);
};

#endif // !OHOS_PROFILER_SCHEDULE_TASK_MANAGER_H