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

class ScheduleTaskManager {
public:
    ScheduleTaskManager();
    ~ScheduleTaskManager();

    /**
     * @brief Set up a scheduled task.
     * @param callback Callback function
     * @param interval Indicates the interval time, measured in milliseconds. If the 'once' parameter is set to false,
     * an 'interval' of 0 means the task will execute immediately once and not repeat thereafter. If the 'once'
     * parameter is set to true, the 'interval' cannot be 0.
     * @param once Indicates whether to execute the task only once.
     * @return If successful, it returns a timerFd greater than 0; if unsuccessful, it returns -1.
     * @note It is crucial to ensure that the code logic executed within the provided callback function
     * does not include any self-termination logic.
     */
    int32_t ScheduleTask(const std::function<void(void)>& callback, const uint64_t interval, bool once = false);

    /**
     * @brief Cancel the scheduled task.
     * @param timerFd The return value of interface ScheduleTask().
     * @return If the cancellation of the scheduled task is successful, it returns true; otherwise, it returns false.
     */
    bool UnscheduleTask(const int32_t timerFd);
    bool UnscheduleTaskLockless(const int32_t timerFd);
    void Shutdown();
    void StartThread();

private:
    void ScheduleThread();
    void HandleSingleTask(int32_t fd, std::function<void(void)> callback);
    bool DeleteTask(const int32_t timerFd);

private:
    std::atomic<bool> runScheduleThread_ = true;
    std::thread scheduleThread_;
    std::unordered_map<int32_t, std::function<void(void)>> tasks_;
    int32_t epollFd_{-1};
    int32_t stopFd_{-1};
    std::mutex mtx_;
};

#endif // !OHOS_PROFILER_SCHEDULE_TASK_MANAGER_H