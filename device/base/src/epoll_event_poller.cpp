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
#include "epoll_event_poller.h"

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "logging.h"

EpollEventPoller::EpollEventPoller(int timeOut) : timeOut_(timeOut), epollFd_(INVALID_FD), eventFd_(INVALID_FD) {}

EpollEventPoller::~EpollEventPoller()
{
    if (state_ == STARTED) {
        HILOG_INFO(LOG_CORE, "need Stop in destructor!");
        Stop();
    }
    if (state_ == INITIED) {
        HILOG_INFO(LOG_CORE, "need Finalize in dtor!");
        Finalize();
    }
}

bool EpollEventPoller::AddFileDescriptor(int fd,
                                         const OnReadableCallback& onReadable,
                                         const OnWritableCallback& onWritable)
{
    auto ctx = std::make_shared<EventContext>();
    CHECK_NOTNULL(ctx, false, "create EventContext FAILED!");
    ctx->fd = fd;
    ctx->onReadable = onReadable;
    ctx->onWritable = onWritable;

    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_TRUE(AddContextLocked(ctx), false, "add context for %d failed!", fd);
    return Notify();
}

bool EpollEventPoller::RemoveFileDescriptor(int fd)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = context_.find(fd);
    CHECK_TRUE(it != context_.end(), false, "fd %d not found in poll set!", fd);

    auto ctx = it->second;
    CHECK_NOTNULL(ctx, false, "ctx null!");
    CHECK_TRUE(RemoveContextLocked(ctx), false, "remove context for %d failed!", fd);
    return Notify();
}

bool EpollEventPoller::AddContextLocked(const EventContextPtr& ctx)
{
    context_[ctx->fd] = ctx;
    return UpdateEvent(EPOLL_CTL_ADD, ctx);
}

bool EpollEventPoller::RemoveContextLocked(const EventContextPtr& ctx)
{
    context_.erase(ctx->fd);
    CHECK_TRUE(UpdateEvent(EPOLL_CTL_DEL, ctx), false, "update fd %d ctx FAILED!", ctx->fd);
    return true;
}

static std::string EpollOpName(int op)
{
    if (op == EPOLL_CTL_ADD) {
        return "ADD";
    }
    if (op == EPOLL_CTL_DEL) {
        return "DEL";
    }
    if (op == EPOLL_CTL_MOD) {
        return "MOD";
    }
    return "";
}

bool EpollEventPoller::UpdateEvent(int op, const EventContextPtr& ctx)
{
    struct epoll_event event = {};
    if (ctx->onReadable) {
        event.events |= EPOLLIN;
    }
    if (ctx->onWritable) {
        event.events |= EPOLLOUT;
    }
    event.data.ptr = ctx.get();

    std::string name = EpollOpName(op).c_str();
    HILOG_DEBUG(LOG_CORE, "poll set %s %d %x start!", name.c_str(), ctx->fd, event.events);
    int retval = epoll_ctl(epollFd_, op, ctx->fd, &event);
    CHECK_TRUE(retval == 0, false, "epoll_ctl %s failed, %d", name.c_str(), errno);
    HILOG_DEBUG(LOG_CORE, "poll set %s %d %x done!", name.c_str(), ctx->fd, event.events);
    return true;
}

void EpollEventPoller::Run()
{
    pthread_setname_np(pthread_self(), "EventPoller");
    std::vector<struct epoll_event> eventVec;
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            eventVec.resize(context_.size());
        }
        int retval = TEMP_FAILURE_RETRY(epoll_wait(epollFd_, eventVec.data(), eventVec.size(), timeOut_));
        CHECK_TRUE(retval >= 0, NO_RETVAL, "epoll_wait failed, %d!", errno);
        if (retval == 0) {
            HILOG_INFO(LOG_CORE, "epoll_wait %dms timeout!", timeOut_);
            continue;
        }
        for (int i = 0; i < retval; i++) {
            auto ctx = reinterpret_cast<EventContext*>(eventVec[i].data.ptr);
            if (ctx != nullptr) {
                HandleEvent(eventVec[i].events, *ctx);
            }
        }
    }
}

void EpollEventPoller::HandleEvent(uint32_t events, const EventContext& ctx)
{
    if (events & EPOLLIN) {
        if (ctx.onReadable) {
            ctx.onReadable();
        }
    } else if (events & EPOLLOUT) {
        if (ctx.onWritable) {
            ctx.onWritable();
        }
    }
}

void EpollEventPoller::OnNotify()
{
    uint64_t value = 0;
    CHECK_TRUE(read(eventFd_, &value, sizeof(value)) == sizeof(value), NO_RETVAL, "read eventfd FAILED!");
    HILOG_DEBUG(LOG_CORE, "OnNotify %llu done!", static_cast<unsigned long long>(value));
}

bool EpollEventPoller::Notify(uint64_t value)
{
    auto nbytes = write(eventFd_, &value, sizeof(value));
    CHECK_TRUE(static_cast<size_t>(nbytes) == sizeof(value), false, "write eventfd FAILED!");
    HILOG_DEBUG(LOG_CORE, "Notify %llu done!", static_cast<unsigned long long>(value));
    return true;
}

bool EpollEventPoller::Init()
{
    HILOG_INFO(LOG_CORE, "Init %d", state_.load());
    CHECK_TRUE(state_ == INITIAL, false, "Init FAIL %d", state_.load());

    int epollFd = epoll_create1(EPOLL_CLOEXEC);
    CHECK_TRUE(epollFd >= 0, false, "epoll_create failed, %d!", errno);
    fileDescriptors_.push_back(epollFd);

    int eventFd = eventfd(0, O_CLOEXEC | O_NONBLOCK);
    CHECK_TRUE(eventFd >= 0, false, "create event fd failed, %d", errno);
    fileDescriptors_.push_back(eventFd);

    auto eventFdCtx = std::make_shared<EventContext>();
    CHECK_NOTNULL(eventFdCtx, false, "create EventContext failed!");
    eventFdCtx->fd = eventFd;
    eventFdCtx->onReadable = std::bind(&EpollEventPoller::OnNotify, this);

    std::unique_lock<std::mutex> lock(mutex_);
    epollFd_ = epollFd;
    eventFd_ = eventFd;
    AddContextLocked(eventFdCtx);
    HILOG_INFO(LOG_CORE, "EpollEventPoller::Init %d done!", state_.load());
    state_ = INITIED;
    return true;
}

bool EpollEventPoller::Finalize()
{
    if (state_ == STARTED) {
        HILOG_INFO(LOG_CORE, "need Stop in Finalize!");
        Stop();
    }

    HILOG_INFO(LOG_CORE, "Finalize %d", state_.load());
    CHECK_TRUE(state_ == INITIED, false, "Finalize FAIL %d", state_.load());

    std::unique_lock<std::mutex> lock(mutex_);
    std::vector<EventContextPtr> contextVec;
    for (auto& ctxPair : context_) {
        contextVec.push_back(ctxPair.second);
    }
    for (auto ctxPtr : contextVec) {
        HILOG_DEBUG(LOG_CORE, "remove context for %d", ctxPtr->fd);
        RemoveContextLocked(ctxPtr);
    }

    for (int fd : fileDescriptors_) {
        close(fd);
    }
    fileDescriptors_.clear();

    epollFd_ = INVALID_FD;
    eventFd_ = INVALID_FD;
    state_ = INITIAL;
    return true;
}

bool EpollEventPoller::Start()
{
    HILOG_INFO(LOG_CORE, "%s %d", __func__, state_.load());
    CHECK_TRUE(state_ == INITIED, false, "Start FAIL %d", state_.load());

    running_ = true;
    pollThread_ = std::thread(&EpollEventPoller::Run, this);
    state_ = STARTED;
    return true;
}

bool EpollEventPoller::Stop()
{
    CHECK_TRUE(state_ == STARTED, false, "Stop FAIL %d", state_.load());

    running_ = false;
    Notify();
    if (pollThread_.joinable()) {
        pollThread_.join();
    }
    state_ = INITIED;
    return true;
}
