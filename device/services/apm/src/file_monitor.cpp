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
#include "file_monitor.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include "securec.h"
#include "logging.h"

namespace {
constexpr uint32_t MAX_BUF_SIZE = 1024;
constexpr int32_t TIME_BASE = 1000;
constexpr int32_t WAIT_MONITOR_THREAD_RUN_SEC = 2;
} //namespace

FileMonitor::FileMonitor(const std::string& listenDir, FileFilter filter) : listenDir_(listenDir), filter_(filter)
{
    if (!Init()) {
        PROFILER_LOG_ERROR(LOG_CORE, "Init file monitor fail");
    }
}

FileMonitor::~FileMonitor()
{
    Finalize();
}

bool FileMonitor::Start()
{
    CHECK_TRUE(running_.load(), false, "file monitor not running");
    started_.store(true);
    return true;
}

void FileMonitor::Finalize()
{
    started_.store(false);
    running_.store(false);
    if (thread_.joinable()) {
        thread_.join();
    }
    if (inotifyFd_ > 0) {
        if (wd_ > 0) {
            inotify_rm_watch(inotifyFd_, wd_);
            wd_ = -1;
        }
        close(inotifyFd_);
        inotifyFd_ = -1;
    }
}

std::string FileMonitor::GetResultFile(int32_t waitMs)
{
    CHECK_NOTNULL(sem_, "", "sem_ is NULL");
    if (waitMs <= 0) {
        CHECK_TRUE(sem_->Wait(), "", "get result file timeout");
    } else {
        CHECK_TRUE(sem_->TimedWait(waitMs / TIME_BASE, (waitMs % TIME_BASE) * TIME_BASE * TIME_BASE), "",
            "get result file timeout for %d ms", waitMs);
    }
    return resultFile_;
}

bool FileMonitor::Init()
{
    CHECK_TRUE(filter_, false, "invalid filter");
    char realPath[PATH_MAX] = {0};
    if (realpath(listenDir_.c_str(), realPath) == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "realpath(%s) failed, errno: %s", listenDir_.c_str(), strerror(errno));
        return false;
    }
    sem_ = GetSemaphoreFactory().Create(0);
    CHECK_NOTNULL(sem_, false, "create sem fail");
    inotifyFd_ = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    CHECK_TRUE(inotifyFd_ >= 0, false, "init inotify fail, errno:%s", strerror(errno));
    wd_ = inotify_add_watch(inotifyFd_, realPath, IN_CLOSE_WRITE);
    if (wd_ < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "add watch fail, errno: %s", strerror(errno));
        close(inotifyFd_);
        inotifyFd_ = -1;
        return false;
    }
    thread_ = std::thread([this] { this->Run(); });
    CHECK_TRUE(sem_->TimedWait(WAIT_MONITOR_THREAD_RUN_SEC), false, "wati thread run timeout");
    PROFILER_LOG_DEBUG(LOG_CORE, "init file monitor success");
    return true;
}

void FileMonitor::Run()
{
    struct timeval time;
    pthread_setname_np(pthread_self(), "FileMonitor");
    running_.store(true);
    sem_->Post();
    PROFILER_LOG_DEBUG(LOG_CORE, "file monitor running");
    while (running_.load()) {
        fd_set rFds;
        FD_ZERO(&rFds);
        FD_SET(inotifyFd_, &rFds);
        time.tv_sec = 1;
        time.tv_usec = 0;
        int ret = select(inotifyFd_ + 1, &rFds, nullptr, nullptr, &time);
        if (ret > 0 && started_.load() && ProcessEvent()) {
            PROFILER_LOG_DEBUG(LOG_CORE, "process target event success, exit monitor");
            running_.store(false);
        }
    }
}

bool FileMonitor::ProcessEvent()
{
    if (!filter_ || sem_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "invalid state");
        return false;
    }
    char filenameBuf[PATH_MAX] = {0};
    const struct inotify_event *event = nullptr;
    char buffer[MAX_BUF_SIZE] = {'\0'};
    char *ptr = nullptr;

    ssize_t readLength = read(inotifyFd_, buffer, MAX_BUF_SIZE);
    if (readLength <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "read event fail, err: %s", strerror(errno));
        return false;
    }
    for (ptr = buffer; ptr < buffer + readLength; ptr += sizeof(struct inotify_event) + event->len) {
        event = reinterpret_cast<const struct inotify_event *>(ptr);
        if (event->wd != wd_ || event->mask != IN_CLOSE_WRITE) {
            continue;
        }
        if (strncpy_s(filenameBuf, PATH_MAX, event->name, event->len) != EOK) {
            PROFILER_LOG_ERROR(LOG_CORE, "strncpy_s fail");
            continue;
        }
        std::string filename = filenameBuf;
        if (!filter_(filename)) {
            continue;
        }
        PROFILER_LOG_DEBUG(LOG_CORE, "notify file write done:%s", event->name);
        resultFile_ = event->name;
        sem_->Post();
        (void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
        return true;
    }
    (void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
    return false;
}