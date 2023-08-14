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
#include "std_semaphore.h"

StdSemaphore::StdSemaphore(unsigned int value) : value_(value) {}

StdSemaphore::~StdSemaphore() {}

bool StdSemaphore::Wait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (value_ == 0) {
        condVar_.wait(lock);
    }
    --value_;
    return true;
}

bool StdSemaphore::TryWait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return TryWaitLocked();
}

bool StdSemaphore::TryWaitLocked()
{
    if (value_ == 0) {
        return false;
    }
    --value_;
    return true;
}

bool StdSemaphore::TimedWait(int seconds, int nanoSeconds)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (value_ == 0) {
        auto timePoint = std::chrono::steady_clock::now();
        timePoint += std::chrono::seconds(seconds);
        timePoint += std::chrono::nanoseconds(nanoSeconds);
        condVar_.wait_until(lock, timePoint);
    }
    return TryWaitLocked();
}

bool StdSemaphore::Post()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ++value_;
    }
    condVar_.notify_all();
    return true;
}

unsigned StdSemaphore::Value() const
{
    return value_;
}

SemaphorePtr StdSemaphoreFactory::Create(unsigned int value)
{
    return std::make_shared<StdSemaphore>(value);
}
