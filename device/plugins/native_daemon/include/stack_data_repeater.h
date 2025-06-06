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
#ifndef STACK_DATA_REPEATER_H
#define STACK_DATA_REPEATER_H

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include "logging.h"
#include "nocopyable.h"
#include "native_hook_result.pb.h"
#include "hook_common.h"
#include "utilities.h"
#include "hook_record.h"

namespace OHOS::Developtools::NativeDaemon {

using BatchNativeHookDataPtr = STD_PTR(shared, BatchNativeHookData);
constexpr const int32_t CACHE_ARRAY_SIZE = 10;

class StackDataRepeater {
public:
    explicit StackDataRepeater(size_t maxSize);
    ~StackDataRepeater();
    bool PutRawStack(const std::shared_ptr<HookRecord>& rawData, bool isRecordAccurately);
    bool PutRawStackArray(std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE>& rawDataArray, uint32_t batchCount);
    std::shared_ptr<HookRecord> TakeRawData(uint32_t during, clockid_t clockId, uint32_t batchCount,
        std::shared_ptr<HookRecord> batchRawStack[], uint32_t statInterval, bool& isTimeOut);
    void Close();
    void Reset();
    size_t Size();
    void ClearCache()
    {
        std::unique_lock<std::mutex> lock(cacheMutex_);
        rawDataCacheQueue_.clear();
    }

private:
    std::mutex mutex_;
    std::mutex cacheMutex_;
    std::deque<std::shared_ptr<HookRecord>> rawDataCacheQueue_;
    std::condition_variable slotCondVar_;
    std::condition_variable itemCondVar_;
    std::deque<std::shared_ptr<HookRecord>> rawDataQueue_;
    std::unordered_map<uint64_t, std::shared_ptr<HookRecord>> mallocMap_ = {};
    size_t maxSize_;
    uint64_t reducedStackCount_;
    std::atomic_bool closed_;

    DISALLOW_COPY_AND_MOVE(StackDataRepeater);
};

using StackDataRepeaterPtr = STD_PTR(shared, StackDataRepeater);
}
#endif // STACK_DATA_REPEATER_H