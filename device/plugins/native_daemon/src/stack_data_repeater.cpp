/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
#include "stack_data_repeater.h"
#include "hook_common.h"

constexpr static uint32_t DEFAULT_SLEEP_TIME_MS = 100; // milliseconds
constexpr static uint32_t DEFAULT_SLEEP_TIME_US = DEFAULT_SLEEP_TIME_MS * 1000;
using namespace OHOS::Developtools::NativeDaemon;

StackDataRepeater::StackDataRepeater(size_t maxSize)
{
    maxSize_ = maxSize;
    closed_ = false;
    reducedStackCount_ = 0;
}

StackDataRepeater::~StackDataRepeater()
{
    Close();
}

size_t StackDataRepeater::Size()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return rawDataQueue_.size();
}

void StackDataRepeater::Reset()
{
    std::unique_lock<std::mutex> lock(mutex_);
    closed_ = false;
}

void StackDataRepeater::Close()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        rawDataQueue_.clear();
        closed_ = true;
    }
    HILOG_INFO(LOG_CORE, "StackDataRepeater Close, reducedStackCount_ : %" PRIx64 " ", reducedStackCount_);
    slotCondVar_.notify_all();
    itemCondVar_.notify_all();
}

bool StackDataRepeater::PutRawStack(const RawStackPtr& rawData, bool isRecordAccurately)
{
    bool needInsert = true;
    std::unique_lock<std::mutex> lock(mutex_);

    if ((rawData == nullptr) && (rawDataQueue_.size() > 0)) {
        HILOG_INFO(LOG_CORE, "no need put nullptr if queue has data, rawDataQueue_.size() = %zu", rawDataQueue_.size());
        return true;
    }
    while (rawDataQueue_.size() >= maxSize_ && !closed_) {
        slotCondVar_.wait(lock);
    }
    if (closed_) {
        return false;
    }

    if (__builtin_expect((rawData != nullptr) && !isRecordAccurately, true)) {
        if (rawData->stackConext->type == FREE_MSG) {
            auto temp = mallocMap_.find(rawData->stackConext->addr);
            // true  : pair of malloc and free matched, both malloc and free will be ignored
            // false : can not match, send free's data anyway
            if (temp != mallocMap_.end()) {
                temp->second->reportFlag = false; // will be ignore later
                mallocMap_.erase(rawData->stackConext->addr);
                needInsert = false;
            }
        } else if (rawData->stackConext->type == MALLOC_MSG) {
            mallocMap_.insert(std::pair<void*, std::shared_ptr<RawStack>>(rawData->stackConext->addr, rawData));
        }
        if (needInsert) {
            rawDataQueue_.push_back(rawData);
        }
    } else {
        rawDataQueue_.push_back(rawData);
    }

    lock.unlock();
    itemCondVar_.notify_one();
    return true;
}

RawStackPtr StackDataRepeater::TakeRawData(uint32_t during, clockid_t clockId, uint32_t batchCount,
    RawStackPtr batchRawStack[])
{
    uint32_t rawDataQueueSize = 0;
    std::unique_lock<std::mutex> lock(mutex_);
    while (rawDataQueue_.empty() && !closed_) {
        itemCondVar_.wait(lock);
    }
    if (closed_) {
        return nullptr;
    }
    RawStackPtr result = nullptr;
    rawDataQueueSize = rawDataQueue_.size();
    int resultSize = rawDataQueueSize > batchCount ? batchCount : rawDataQueueSize;
    bool needReduceStack = rawDataQueueSize >= SPEED_UP_THRESHOLD;
    for (int i = 0; i < resultSize; i++) {
        result = rawDataQueue_.front();
        rawDataQueue_.pop_front();
        batchRawStack[i] = result;
        if ((result != nullptr) && (result->stackConext->type == MALLOC_MSG)) {
            mallocMap_.erase(result->stackConext->addr);
            if (needReduceStack) {
                result->reduceStackFlag = true;
                reducedStackCount_++;
            }
        }
    }

    lock.unlock();
    slotCondVar_.notify_one();
    return result;
}