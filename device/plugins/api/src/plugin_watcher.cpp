/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugin_watcher.h"

#include <climits>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "logging.h"
#include "plugin_manager.h"
#include "securec.h"

namespace {
constexpr uint32_t MAX_BUF_SIZE = 1024;
} // namespace

PluginWatcher::PluginWatcher(const PluginManagerPtr& pluginManager)
    : inotifyFd_(-1), pluginManager_(pluginManager), runMonitor_(true)
{
    inotifyFd_ = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    if (inotifyFd_ < 0) {
        HILOG_INFO(LOG_CORE, "%s:inotify_init1 failed! inotifyFd_ : %d", __func__, inotifyFd_);
    } else {
        monitorThread_ = std::thread(&PluginWatcher::Monitor, this);
    }
}

PluginWatcher::~PluginWatcher()
{
    runMonitor_ = false;
    for (auto it = wdToDir_.begin(); it != wdToDir_.end(); ++it) {
        inotify_rm_watch(inotifyFd_, it->first);
    }

    if (inotifyFd_ != -1) {
        close(inotifyFd_);
    }
    monitorThread_.join();
}

bool PluginWatcher::ScanPlugins(const std::string& pluginDir)
{
    DIR* dir = nullptr;
    struct dirent* entry = nullptr;
    char fullpath[PATH_MAX + 1] = {0};
    realpath(pluginDir.c_str(), fullpath);
    HILOG_INFO(LOG_CORE, "%s:scan plugin from directory %s", __func__, fullpath);
    dir = opendir(fullpath);
    if (dir == nullptr) {
        return false;
    }
    while (true) {
        entry = readdir(dir);
        if (!entry) {
            HILOG_INFO(LOG_CORE, "%s:readdir finish!", __func__);
            break;
        }
        std::string fileName = entry->d_name;
        if (entry->d_type & DT_REG) {
            size_t pos = fileName.rfind(".so");
            if (pos != std::string::npos && (pos == fileName.length() - strlen(".so"))) {
                OnPluginAdded(std::string(fullpath) + '/' + fileName);
            }
        }
    }
    closedir(dir);
    return true;
}

bool PluginWatcher::WatchPlugins(const std::string& pluginDir)
{
    char fullpath[PATH_MAX + 1] = {0};
    realpath(pluginDir.c_str(), fullpath);

    int wd = inotify_add_watch(inotifyFd_, fullpath, IN_ALL_EVENTS);
    if (wd < 0) {
        HILOG_INFO(LOG_CORE, "%s:inotify_add_watch add directory %s failed!", __func__, pluginDir.c_str());
        return false;
    }
    HILOG_INFO(LOG_CORE, "%s:inotify_add_watch add directory %s success!", __func__, fullpath);
    std::lock_guard<std::mutex> guard(mtx_);
    wdToDir_.insert(std::pair<int, std::string>(wd, std::string(fullpath)));
    return true;
}

bool PluginWatcher::MonitorIsSet()
{
    const struct inotify_event* event = nullptr;
    char buffer[MAX_BUF_SIZE] = {'\0'};
    char* ptr = nullptr;

    ssize_t readLength = read(inotifyFd_, buffer, MAX_BUF_SIZE);
    if (readLength == -1) {
        return false;
    }
    for (ptr = buffer; ptr < buffer + readLength; ptr += sizeof(struct inotify_event) + event->len) {
        event = reinterpret_cast<const struct inotify_event*>(ptr);
        std::unique_lock<std::mutex> guard(mtx_, std::adopt_lock);
        const std::string& pluginDir = wdToDir_[event->wd];
        guard.unlock();
        if (event->mask & IN_ISDIR) {
            continue;
        }
        std::string fileName = event->name;
        size_t pos = fileName.rfind(".so");
        if ((pos == std::string::npos) || (pos != fileName.length() - strlen(".so"))) {
            continue;
        }
        switch (event->mask) {
            case IN_CLOSE_WRITE:
            case IN_MOVED_TO:
                OnPluginAdded(pluginDir + '/' + fileName);
                break;
            case IN_DELETE:
            case IN_MOVED_FROM:
                OnPluginRemoved(pluginDir + '/' + fileName);
                break;
            default:
                break;
        }
    }
    if (memset_s(buffer, sizeof(buffer), 0, sizeof(buffer)) != 0) {
        HILOG_ERROR(LOG_CORE, "%s:memset_s error!", __func__);
    }
    return true;
}

void PluginWatcher::Monitor()
{
    struct timeval time;

    pthread_setname_np(pthread_self(), "PluginWatcher");
    while (runMonitor_) {
        fd_set rFds;
        FD_ZERO(&rFds);
        FD_SET(inotifyFd_, &rFds);
        time.tv_sec = 1;
        time.tv_usec = 0;
        int ret = select(inotifyFd_ + 1, &rFds, nullptr, nullptr, &time);
        if (ret < 0) {
            continue;
        } else if (!ret) {
            continue;
        } else if (FD_ISSET(inotifyFd_, &rFds)) {
            if (!MonitorIsSet()) {
                continue;
            }
        }
    }
}

void PluginWatcher::OnPluginAdded(const std::string& pluginPath)
{
    auto pluginManager = pluginManager_.lock();
    if (pluginManager != nullptr) {
        if (pluginManager->AddPlugin(pluginPath)) {
            HILOG_INFO(LOG_CORE, "%s:plugin %s add success!", __func__, pluginPath.c_str());
        } else {
            HILOG_INFO(LOG_CORE, "%s:pluginPath %s add failed!", __func__, pluginPath.c_str());
        }
    } else {
        HILOG_INFO(LOG_CORE, "%s:weak_ptr pluginManager lock failed!", __func__);
    }
}

void PluginWatcher::OnPluginRemoved(const std::string& pluginPath)
{
    auto pluginManager = pluginManager_.lock();
    if (pluginManager != nullptr) {
        if (pluginManager->RemovePlugin(pluginPath)) {
            HILOG_INFO(LOG_CORE, "%s:pluginPath %s remove success!", __func__, pluginPath.c_str());
        } else {
            HILOG_INFO(LOG_CORE, "%s:pluginPath %s remove failed!", __func__, pluginPath.c_str());
        }
    } else {
        HILOG_INFO(LOG_CORE, "%s:weak_ptr pluginManager lock failed!", __func__);
    }
}