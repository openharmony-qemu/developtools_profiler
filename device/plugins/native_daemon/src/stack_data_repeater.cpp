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

bool StackDataRepeater::PutRawStackArray(std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE>& rawDataArray,
                                         uint32_t batchCount)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if ((rawDataArray.empty()) && (rawDataQueue_.size() > 0)) {
        PROFILER_LOG_INFO(LOG_CORE, "no need put nullptr if queue has data, rawDataQueue_.size() = %zu",
                          rawDataQueue_.size());
        return true;
    }
    while (rawDataQueue_.size() >= maxSize_ && !closed_) {
        slotCondVar_.wait(lock);
    }
    if (closed_) {
        return false;
    }
    for (uint32_t i = 0; i < batchCount; i++) {
        rawDataQueue_.push_back(rawDataArray[i]);
    }
    lock.unlock();
    itemCondVar_.notify_one();
    return true;
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
    PROFILER_LOG_INFO(LOG_CORE, "StackDataRepeater Close, reducedStackCount_ : %" PRIx64 " ", reducedStackCount_);
    slotCondVar_.notify_all();
    itemCondVar_.notify_all();
}

bool StackDataRepeater::PutRawStack(const HookRecordPtr& hookData, bool isRecordAccurately)
{
    bool needInsert = true;
    std::unique_lock<std::mutex> lock(mutex_);

    if ((hookData == nullptr) && (rawDataQueue_.size() > 0)) {
        PROFILER_LOG_INFO(LOG_CORE, "no need put nullptr if queue has data, rawDataQueue_.size() = %zu",
                          rawDataQueue_.size());
        return true;
    }
    while (rawDataQueue_.size() >= maxSize_ && !closed_) {
        slotCondVar_.wait(lock);
    }
    if (closed_) {
        return false;
    }

    if (__builtin_expect((hookData != nullptr) && !isRecordAccurately, true)) {
        if (hookData->GetType() == FREE_MSG || hookData->GetType() == FREE_MSG_SIMP) {
            auto temp = mallocMap_.find(hookData->GetAddr());
            // true  : pair of malloc and free matched, both malloc and free will be ignored
            // false : can not match, send free's data anyway
            if (temp != mallocMap_.end()) {
                temp->second->GetRawStack()->reportFlag = false; // will be ignore later
                mallocMap_.erase(hookData->GetAddr());
                needInsert = false;
            }
        } else if (hookData->GetType() == MALLOC_MSG) {
            mallocMap_.insert(std::pair<uint64_t, std::shared_ptr<HookRecord>>(hookData->GetAddr(), hookData));
        }
        if (needInsert) {
            rawDataQueue_.push_back(hookData);
        }
    } else {
        rawDataQueue_.push_back(hookData);
    }

    lock.unlock();
    itemCondVar_.notify_one();
    return true;
}

HookRecordPtr StackDataRepeater::TakeRawData(uint32_t during, clockid_t clockId, uint32_t batchCount,
                                             HookRecordPtr batchRawStack[], uint32_t statInterval, bool& isTimeOut)
{
    uint32_t rawDataQueueSize = 0;
    std::unique_lock<std::mutex> lock(mutex_);
    if (statInterval > 0 &&
        !itemCondVar_.wait_for(lock, std::chrono::milliseconds(during),
                               [&] { return ((!rawDataQueue_.empty()) || (closed_)); })) {
        if (rawDataQueue_.empty() && !closed_) {
            isTimeOut = true;
            lock.unlock();
            slotCondVar_.notify_one();
            return nullptr;
        }
    } else {
        while (rawDataQueue_.empty() && !closed_) {
            itemCondVar_.wait(lock);
        }
    }
    if (closed_) {
        return nullptr;
    }
    HookRecordPtr hookData = nullptr;
    RawStackPtr rawStack = nullptr;
    rawDataQueueSize = rawDataQueue_.size();
    int resultSize = rawDataQueueSize > batchCount ? batchCount : rawDataQueueSize;
    bool needReduceStack = rawDataQueueSize >= SPEED_UP_THRESHOLD;
    for (int i = 0; i < resultSize; i++) {
        hookData = rawDataQueue_.front();
        rawDataQueue_.pop_front();
        batchRawStack[i] = hookData;
        rawStack = hookData->GetRawStack();
        if ((hookData != nullptr) && (hookData->GetType() == MALLOC_MSG)) {
            mallocMap_.erase(hookData->GetAddr());
            if (needReduceStack) {
                rawStack->reduceStackFlag = true;
                reducedStackCount_++;
            }
        }
    }

    lock.unlock();
    slotCondVar_.notify_one();
    return hookData;
}