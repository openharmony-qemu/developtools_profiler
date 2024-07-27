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

using BatchNativeHookDataPtr = STD_PTR(shared, BatchNativeHookData);

class StackDataRepeater {
public:
    struct RawStack {
        std::unique_ptr<uint8_t[]> baseStackData = nullptr; // save the shared memory data
        BaseStackRawData* stackConext = nullptr; // points to the foundation type data
        union {
            uint8_t* stackData;
            const char* jsStackData = nullptr;
        };
        uint8_t* data = nullptr; // fp mode data is ip, dwarf mode data is regs
        uint32_t stackSize = 0;
        uint8_t fpDepth = 0; // fp mode fpDepth is ip depth, dwarf mode is invalid
        bool reportFlag = false;
        bool reduceStackFlag = false;
    };

    explicit StackDataRepeater(size_t maxSize);
    ~StackDataRepeater();
    bool PutRawStack(const std::shared_ptr<RawStack>& rawData, bool isRecordAccurately);
    std::shared_ptr<RawStack> TakeRawData(uint32_t during, clockid_t clockId, uint32_t batchCount,
        std::shared_ptr<RawStack> batchRawStack[], uint32_t statInterval, bool& isTimeOut);
    void Close();
    void Reset();
    size_t Size();

private:
    std::mutex mutex_;
    std::condition_variable slotCondVar_;
    std::condition_variable itemCondVar_;
    std::deque<std::shared_ptr<RawStack>> rawDataQueue_;
    std::unordered_map<void *, std::shared_ptr<RawStack>> mallocMap_ = {};
    size_t maxSize_;
    uint64_t reducedStackCount_;
    std::atomic_bool closed_;

    DISALLOW_COPY_AND_MOVE(StackDataRepeater);
};

using RawStackPtr = STD_PTR(shared, StackDataRepeater::RawStack);

using StackDataRepeaterPtr = STD_PTR(shared, StackDataRepeater);

#endif // STACK_DATA_REPEATER_H