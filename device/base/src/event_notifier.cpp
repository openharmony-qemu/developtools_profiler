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
#include "event_notifier.h"

#include <cerrno>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include "logging.h"

#ifndef EFD_SEMAPHORE
#define EFD_SEMAPHORE 1
#endif

EventNotifierPtr EventNotifier::Create(unsigned int initValue, unsigned int mask)
{
    return std::make_shared<EventNotifier>(initValue, mask);
}

EventNotifierPtr EventNotifier::CreateWithFd(int fd)
{
    return std::make_shared<EventNotifier>(fd);
}

EventNotifier::EventNotifier(unsigned int initValue, unsigned int mask) : fd_(-1), flags_(O_CLOEXEC)
{
    if (mask & NONBLOCK) {
        flags_ |= O_NONBLOCK;
    }
    if (mask & SEMAPHORE) {
        flags_ |= EFD_SEMAPHORE;
    }
    fd_ = eventfd(initValue, flags_);
    CHECK_TRUE(fd_ >= 0, NO_RETVAL, "create eventfd FAILED, %d", errno);
    HILOG_DEBUG(LOG_CORE, "EventNotifier create eventfd %d done!", fd_);
}

EventNotifier::EventNotifier(int fd) : fd_(fd), flags_(0)
{
    int flags = fcntl(fd_, F_GETFL);
    CHECK_TRUE(flags >= 0, NO_RETVAL, "get flags of fd %d FAILED, %d", fd, errno);
    HILOG_DEBUG(LOG_CORE, "EventNotifier bind eventfd %d done!", fd_);
}

EventNotifier::~EventNotifier()
{
    HILOG_DEBUG(LOG_CORE, "EventNotifier close eventfd %d", fd_);
    close(fd_);
}

int EventNotifier::GetFd() const
{
    return fd_;
}

uint64_t EventNotifier::Take() const
{
    uint64_t value = UINT64_MAX;
    int retval = TEMP_FAILURE_RETRY(read(fd_, &value, sizeof(value)));
    CHECK_TRUE(retval == sizeof(value), 0, "read value from eventfd %d failed, %d!", fd_, errno);
    return value;
}

bool EventNotifier::Post(uint64_t value) const
{
    int retval = TEMP_FAILURE_RETRY(write(fd_, &value, sizeof(value)));
    CHECK_TRUE(retval == sizeof(value), false, "write value to eventfd %d failed, %d!", fd_, errno);
    return true;
}
