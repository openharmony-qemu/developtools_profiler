/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#ifndef HOOK_RECORD_FACTORY_H
#define HOOK_RECORD_FACTORY_H
#include <memory>
#include <string>
#include <deque>
#include "hook_common.h"
#include "hook_record.h"

namespace OHOS::Developtools::NativeDaemon {
constexpr int RAW_DATA_CACHE_INIT_SIZE = 40000;
constexpr int HOOK_RECORD_CACHE_INIT_SIZE = 20000;
constexpr int RAW_DATA_CACHE_MAX_SIZE = 400000;
constexpr int HOOK_RECORD_CACHE_MAX_SIZE = 200000;

class HookRecordFactory {
public:
    HookRecordFactory() = delete;
    HookRecordFactory(NativeHookConfig hookConfig);
    std::shared_ptr<HookRecord> GetHookRecord(const int8_t sharedMemData[], uint32_t size, bool storeData = true);
    void SaveHookRecord(HookRecordPtr hookRecord);

    template <typename T>
    std::shared_ptr<T> GetRecordFromCache(std::deque<std::shared_ptr<T>>& dataCache, int16_t type);
    std::shared_ptr<RawStack> GetRawStackFromCache();
    void SaveRawStack(RawStackPtr rawStack);
    std::shared_ptr<HookRecord> CreateStackRecord(uint16_t type, RawStackPtr rawStack);
    void SaveRecordToCache(uint16_t type, HookRecordPtr hookRecord);
    void InitRawStack(RawStackPtr rawStack, const int8_t sharedMemData[],
                      uint32_t size, bool storeData);

private:
    NativeHookConfig hookConfig_;
    std::deque<std::shared_ptr<RawStack>> rawStackCache_;
    std::deque<std::shared_ptr<HookRecord>> freeRecordSimpCache_;
    std::deque<std::shared_ptr<HookRecord>> freeRecordCache_;
    std::deque<std::shared_ptr<HookRecord>> mallocRecordCache_;
    std::deque<std::shared_ptr<HookRecord>> mmapRecordCache_;
    std::deque<std::shared_ptr<HookRecord>> munmapRecordCache_;
    std::deque<std::shared_ptr<HookRecord>> jsRecordCache_;
    std::deque<std::shared_ptr<HookRecord>> arkTsFreeRecordCache_;
    std::deque<std::shared_ptr<HookRecord>> arkTsMallocRecordCache_;
    std::mutex rawStackMutex_;
    std::mutex recordMutex_;
};
}
#endif //HOOK_RECORD_FACTORY_H