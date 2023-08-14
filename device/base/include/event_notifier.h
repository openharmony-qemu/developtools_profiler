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
#ifndef EVENT_NOTIFIER_H
#define EVENT_NOTIFIER_H

#include <cstdint>
#include <memory>
#include "nocopyable.h"

class EventNotifier;

using EventNotifierPtr = std::shared_ptr<EventNotifier>;

class EventNotifier {
public:
    enum {
        NONBLOCK = (1u << 0),
        SEMAPHORE = (1u << 1),
    };

    static EventNotifierPtr Create(unsigned int initValue = 0, unsigned int mask = 0);
    static EventNotifierPtr CreateWithFd(int fd);

    int GetFd() const;

    uint64_t Take() const;
    bool Post(uint64_t value) const;

    EventNotifier(unsigned int initValue, unsigned int mask);
    EventNotifier(int fd);
    ~EventNotifier();

private:
    int fd_;
    unsigned int flags_;

    DISALLOW_COPY_AND_MOVE(EventNotifier);
};

#endif // EVENT_NOTIFIER_H