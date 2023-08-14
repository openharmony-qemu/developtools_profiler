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
#ifndef EPOLL_EVENT_POLLER_H
#define EPOLL_EVENT_POLLER_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "nocopyable.h"

class EpollEventPoller {
public:
    explicit EpollEventPoller(int timeOut);
    ~EpollEventPoller();

    using OnReadableCallback = std::function<void(void)>;
    using OnWritableCallback = std::function<void(void)>;

    bool AddFileDescriptor(int fd, const OnReadableCallback& onReadable, const OnWritableCallback& onWritable = {});

    bool RemoveFileDescriptor(int fd);

    bool Init();
    bool Start();
    bool Stop();
    bool Finalize();

private:
    enum State {
        INITIAL,
        INITIED,
        STARTED,
    };
    static constexpr int INVALID_FD = -1;
    struct EventContext {
        int fd = INVALID_FD;
        OnReadableCallback onReadable;
        OnWritableCallback onWritable;
    };
    using EventContextPtr = std::shared_ptr<EventContext>;

    void Run();
    bool UpdateEvent(int op, const EventContextPtr& ctx);
    void HandleEvent(uint32_t events, const EventContext& ctx);

    bool AddContextLocked(const EventContextPtr& ctx);
    bool RemoveContextLocked(const EventContextPtr& ctx);

    void OnNotify();
    bool Notify(uint64_t value = 1);

private:
    std::mutex mutex_;
    std::mutex vecMutex_;
    std::thread pollThread_;
    int timeOut_ = 0;
    int epollFd_ = INVALID_FD;
    int eventFd_ = INVALID_FD;
    std::atomic<int> state_ = INITIAL;
    std::atomic<bool> running_ = false;
    std::vector<int> fileDescriptors_;
    std::unordered_map<int, EventContextPtr> context_;

    DISALLOW_COPY_AND_MOVE(EpollEventPoller);
};
#endif // EPOLL_EVENT_POLLER_H
