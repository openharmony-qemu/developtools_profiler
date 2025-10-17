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
#include <fcntl.h>
#include <mutex>
#include <pthread.h>
#include <cstring>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace {
constexpr int32_t TIME_BASE = 1000; // Time progression rate.
constexpr int32_t FIRST_TIME = 10; // The start time of the first task is 10 nanoseconds.
constexpr int32_t EPOLL_EVENT_MAX = 1024;
} // namespace

ScheduleTaskManager::ScheduleTaskManager()
{
    StartThread();
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
    uint64_t value = 1;
    write(stopFd_, &value, sizeof(value));
    if (scheduleThread_.joinable()) {
        scheduleThread_.join();
    }
    std::unique_lock<std::mutex> lock(mtx_);
    for (const auto& [timerFd, func] : tasks_) {
        close(timerFd);
    }
    close(epollFd_);
    close(stopFd_);
}

int32_t ScheduleTaskManager::ScheduleTask(const std::function<void(void)>& callback, const uint64_t interval, bool once)
{
    int32_t timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "ScheduleTaskManager timerfd create failed");
        return -1;
    }

    std::function<void(void)> func;
    struct itimerspec time;
    if (once) {
        if (interval == 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "the interval parameters of a single execution cannot be 0");
            return -1;
        }
        time.it_value.tv_sec = interval / TIME_BASE;
        time.it_value.tv_nsec = (interval % TIME_BASE) * TIME_BASE * TIME_BASE;
        time.it_interval.tv_sec = 0;
        time.it_interval.tv_nsec = 0;
        func = ([this, timerFd, callback] { this->HandleSingleTask(timerFd, callback); });
    } else {
        time.it_value.tv_sec = 0;
        time.it_value.tv_nsec = FIRST_TIME;
        time.it_interval.tv_sec = interval / TIME_BASE;
        time.it_interval.tv_nsec = (interval % TIME_BASE) * TIME_BASE * TIME_BASE;
        func = callback;
    }

    int32_t ret = timerfd_settime(timerFd, 0, &time, NULL);
    if (ret == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "ScheduleTaskManager timerfd settime failed");
        return -1;
    }

    struct epoll_event evt;
    evt.data.fd = timerFd;
    evt.events = EPOLLIN;
    epoll_ctl(epollFd_, EPOLL_CTL_ADD, timerFd, &evt);
    std::unique_lock<std::mutex> lock(mtx_);
    tasks_[timerFd] = std::move(func);
    return timerFd;
}

bool ScheduleTaskManager::UnscheduleTask(const int32_t timerFd)
{
    return DeleteTask(timerFd);
}

bool ScheduleTaskManager::DeleteTask(const int32_t timerFd)
{
    std::unique_lock<std::mutex> lock(mtx_);
    if (auto iter = tasks_.find(timerFd); iter != tasks_.end()) {
        tasks_.erase(timerFd);
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, timerFd, NULL);
        close(timerFd);
        return true;
    }
    return false;
}

void ScheduleTaskManager::ScheduleThread()
{
    pthread_setname_np(pthread_self(), "SchedTaskMgr");
    while (runScheduleThread_) {
        if (!HandleEpollEvents()) {
            return;
        }
    }
}

bool ScheduleTaskManager::HandleEpollEvents()
{
    struct epoll_event events[EPOLL_EVENT_MAX];
    int32_t nfd = epoll_wait(epollFd_, events, EPOLL_EVENT_MAX, -1);
    if (nfd > 0) {
        for (int32_t i = 0; i < nfd; ++i) {
            if (events[i].data.fd == stopFd_) {
                return false;
            }
            HandleSingleEvent(events[i]);
        }
    }
    return true;
}

bool ScheduleTaskManager::HandleSingleEvent(const epoll_event& event)
{
    if (event.data.fd == stopFd_) {
        return false;
    }
    std::unique_lock<std::mutex> lock(mtx_);
    uint64_t exp;
    auto it = tasks_.find(event.data.fd);
    if (it == tasks_.end()) {
        PROFILER_LOG_WARN(LOG_CORE, "ScheduleTaskManager timerfd:%d not found", event.data.fd);
        return false;
    }
    int32_t ret = read(event.data.fd, &exp, sizeof(uint64_t));
    if (ret != sizeof(uint64_t)) {
        PROFILER_LOG_WARN(LOG_CORE, "ScheduleTaskManager timerfd:%d read failed", event.data.fd);
        return false;
    }
    if (it->second == nullptr) {
        return false;
    }
    auto funcTask = it->second;
    lock.unlock();
    funcTask();
    return true;
}

void ScheduleTaskManager::HandleSingleTask(int32_t fd, std::function<void(void)> callback)
{
    callback();
    UnscheduleTask(fd);
}

void ScheduleTaskManager::StartThread()
{
    epollFd_ = epoll_create(0);
    stopFd_ = eventfd(0, EFD_NONBLOCK); // Specifically designed for stopping epoll_wait.
    struct epoll_event evt;
    evt.data.fd = stopFd_;
    evt.events = EPOLLIN;
    epoll_ctl(epollFd_, EPOLL_CTL_ADD, stopFd_, &evt);
    scheduleThread_ = std::thread([this] { this->ScheduleThread(); });
}