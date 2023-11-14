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

#ifndef POWER_MESSAGE_QUEUE_H
#define POWER_MESSAGE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

#include "logging.h"
#include "xpower_common.h"


class PowerMessageQueue {
public:
    explicit PowerMessageQueue(size_t maxSize);
    ~PowerMessageQueue();
    bool WaitAndPop(std::shared_ptr<PowerOptimizeData> &value, const std::chrono::microseconds rel_time);
    bool WaitAndPopBatch(std::vector<std::shared_ptr<PowerOptimizeData>> &array,
                         const std::chrono::microseconds realTime, size_t batchCount);
    void PushBack(std::shared_ptr<PowerOptimizeData> &item);
    void ShutDown();
    size_t Size();
    bool Empty();
    bool IsShutDown();

private:
    std::deque<std::shared_ptr<PowerOptimizeData>> dataQueue_;
    std::mutex mutex_;
    std::condition_variable fullCon_;
    std::condition_variable emptyCon_;
    bool shutDown_ = false;
    size_t maxSize_;
};
#endif // POWER_MESSAGE_QUEUE_H
