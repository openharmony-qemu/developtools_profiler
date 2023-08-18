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

#include "schedule_task_manager.h"

#include <ctime>
#include <iostream>
#include <mutex>
#include <pthread.h>

#include "logging.h"
#include "securec.h"

namespace {
constexpr std::chrono::milliseconds POLL_INTERVAL = std::chrono::milliseconds(5000);
constexpr std::chrono::milliseconds MIN_REPEAT_INTERVAL = std::chrono::milliseconds(10);
constexpr std::chrono::milliseconds ZERO_INTERVAL = std::chrono::milliseconds(0);
} // namespace

ScheduleTaskManager& ScheduleTaskManager::GetInstance()
{
    static ScheduleTaskManager instance;
    return instance;
}

ScheduleTaskManager::ScheduleTaskManager()
{
    runScheduleThread_ = true;
    scheduleThread_ = std::thread(&ScheduleTaskManager::ScheduleThread, this);
}

ScheduleTaskManager::~ScheduleTaskManager()
{
    Shutdown();
}

void ScheduleTaskManager::Shutdown()
{
    bool expect = true;
    if (!runScheduleThread_.compare_exchange_strong(expect, false)) {
        return;
    }
    taskCv_.notify_one();
    if (scheduleThread_.joinable()) {
        scheduleThread_.join();
    }
    taskMap_.clear();
    timeMap_.clear();
}

std::chrono::milliseconds ScheduleTaskManager::NormalizeInterval(std::chrono::milliseconds interval)
{
    if (interval <= ZERO_INTERVAL) {
        return ZERO_INTERVAL;
    }
    if (interval < MIN_REPEAT_INTERVAL) {
        return MIN_REPEAT_INTERVAL;
    }
    return interval / MIN_REPEAT_INTERVAL * MIN_REPEAT_INTERVAL;
}

bool ScheduleTaskManager::ScheduleTask(const std::string& name,
                                       const std::function<void(void)>& callback,
                                       const std::chrono::milliseconds& repeatInterval)
{
    return ScheduleTask(name, callback, repeatInterval, repeatInterval);
}

bool ScheduleTaskManager::ScheduleTask(const std::string& name,
                                       const std::function<void(void)>& callback,
                                       const std::chrono::milliseconds& repeatInterval,
                                       std::chrono::milliseconds initialDelay)
{
    auto currentTime = Clock::now();
    auto task = std::make_shared<Task>();

    task->name = name;
    task->callback = callback;
    task->initialDelay = initialDelay;
    task->repeatInterval = NormalizeInterval(repeatInterval);
    task->nextRunTime = currentTime + initialDelay;

    std::lock_guard<std::mutex> guard(taskMutex_);
    CHECK_TRUE(taskMap_.count(name) <= 0, false, "task name %s already exists!", name.c_str());

    taskMap_[name] = task;
    timeMap_.insert(std::make_pair(task->nextRunTime, task));
    taskCv_.notify_one();

    DumpTask(task);
    HILOG_DEBUG(LOG_CORE, "add schedule %s done, total: %zu", name.c_str(), taskMap_.size());
    return true;
}

bool ScheduleTaskManager::UnscheduleTask(const std::string& name)
{
    std::unique_lock<std::mutex> lck(taskMutex_);
    HILOG_DEBUG(LOG_CORE, "del schedule %s start, total: %zu", name.c_str(), taskMap_.size());
    auto it = taskMap_.find(name);
    if (it != taskMap_.end()) {
        taskMap_.erase(it);
        HILOG_DEBUG(LOG_CORE, "del schedule %s done, remain: %zu", name.c_str(), taskMap_.size());
        return true;
    }
    HILOG_DEBUG(LOG_CORE, "del schedule %s pass, total: %zu", name.c_str(), taskMap_.size());
    return false;
}

ScheduleTaskManager::WeakTask ScheduleTaskManager::TakeFront()
{
    std::unique_lock<std::mutex> lck(taskMutex_);

    // thread wait until task insert or shutdown
    while (timeMap_.empty() && runScheduleThread_) {
        taskCv_.wait_for(lck, POLL_INTERVAL);
    }

    if (!runScheduleThread_) {
        return {};
    }

    auto task = timeMap_.begin()->second;
    timeMap_.erase(timeMap_.begin());
    return task;
}

void ScheduleTaskManager::DumpTask(const SharedTask& task) {}

void ScheduleTaskManager::ScheduleThread()
{
    pthread_setname_np(pthread_self(), "SchedTaskMgr");
    while (runScheduleThread_) {
        // take front task from task queue
        WeakTask weakTask = TakeFront();
        if (!runScheduleThread_) {
            break;
        }

        TimePoint targetTime;
        {
            auto taskTime = weakTask.lock(); // promote to shared_ptr
            if (!taskTime) {
                // task cancelled with UnschduleTask or not a repeat task
                HILOG_INFO(LOG_CORE, "front task cacelled or not repeat task");
                continue;
            }
            targetTime = taskTime->nextRunTime;
        }

        // delay to target time
        auto currentTime = Clock::now();
        if (targetTime >= currentTime) {
            std::this_thread::sleep_for(targetTime - currentTime);
        }

        auto taskRepeat = weakTask.lock();
        if (!taskRepeat) {
            // task cancelled with UnschduleTask
            HILOG_INFO(LOG_CORE, "front task cacelled");
            continue;
        }

        // call task callback
        DumpTask(taskRepeat);
        taskRepeat->callback();
        taskRepeat->nextRunTime = targetTime + taskRepeat->repeatInterval;

        if (taskRepeat->repeatInterval.count() != 0) {
            // repeat task, re-insert task to timeMap
            std::unique_lock<std::mutex> guard(taskMutex_);
            timeMap_.insert(std::make_pair(taskRepeat->nextRunTime, taskRepeat));
        } else {
            // not a repeat task.
            std::unique_lock<std::mutex> guard(taskMutex_);
            taskMap_.erase(taskRepeat->name);
        }
    }
}
