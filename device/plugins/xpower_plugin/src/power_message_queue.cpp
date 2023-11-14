/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#include "power_message_queue.h"

#include <dlfcn.h>
#include <unistd.h>

#include "securec.h"


PowerMessageQueue::PowerMessageQueue(size_t maxSize)
{
    maxSize_ = maxSize;
}

PowerMessageQueue::~PowerMessageQueue() {}

bool PowerMessageQueue::IsShutDown()
{
    return this->shutDown_;
}

void PowerMessageQueue::ShutDown()
{
    std::unique_lock<std::mutex> lock(mutex_);
    dataQueue_.clear();
    this->shutDown_ = true;
    fullCon_.notify_all();
    emptyCon_.notify_all();
}

bool PowerMessageQueue::WaitAndPop(std::shared_ptr<PowerOptimizeData> &value, const std::chrono::microseconds realTime)
{
    // the relative timeout rel_time expires
    std::unique_lock<std::mutex> lock(mutex_);
    if (IsShutDown()) {
        return false;
    }
    if (emptyCon_.wait_for(lock, realTime, [&] { return !dataQueue_.empty(); })) {
        value = dataQueue_.front();
        dataQueue_.pop_front();
    } else {
        return false;
    }
    lock.unlock();
    fullCon_.notify_one();
    return true;
}

bool PowerMessageQueue::WaitAndPopBatch(std::vector<std::shared_ptr<PowerOptimizeData>> &array,
                                        const std::chrono::microseconds realTime, size_t batchCount)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (IsShutDown()) {
        return false;
    }
    uint32_t queueSize = 0;
    if (emptyCon_.wait_for(lock, realTime, [&] { return !dataQueue_.empty(); })) {
        queueSize = dataQueue_.size();
        size_t resultSize = queueSize > batchCount ? batchCount : queueSize;
        for (size_t i = 0; i < resultSize; i++) {
            std::shared_ptr<PowerOptimizeData> result = dataQueue_.front();
            dataQueue_.pop_front();
            array[i] = result;
        }
    } else {
        return false;
    }
    lock.unlock();
    fullCon_.notify_one();
    return true;
}

void PowerMessageQueue::PushBack(std::shared_ptr<PowerOptimizeData> &item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    while (dataQueue_.size() >= maxSize_) {
        fullCon_.wait(mlock);
    }
    dataQueue_.push_back(item);
    mlock.unlock();         // unlock before notificiation to minimize mutex con
    emptyCon_.notify_one(); // notify one waiting thread
}

size_t PowerMessageQueue::Size()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    size_t size = dataQueue_.size();
    mlock.unlock();
    return size;
}

bool PowerMessageQueue::Empty()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    bool isEmpty = dataQueue_.empty();
    mlock.unlock();
    return isEmpty;
}
