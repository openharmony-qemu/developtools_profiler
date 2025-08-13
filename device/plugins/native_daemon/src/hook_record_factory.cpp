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

#include "hook_record_factory.h"
#include <memory>
#include <string>
#include "hook_common.h"
#include "logging.h"

namespace OHOS::Developtools::NativeDaemon {

HookRecordFactory::HookRecordFactory(NativeHookConfig hookConfig) : hookConfig_(hookConfig)
{
    const bool isNoDataQueueMode = (hookConfig_.statistics_interval() > 0 && hookConfig_.fp_unwind() &&
                                    hookConfig_.offline_symbolization());
    int sharedMemCount = (hookConfig_.offline_symbolization()) ? SHARED_MEMORY_NUM : 1;
    int rawDataCacheSize = isNoDataQueueMode ? sharedMemCount : RAW_DATA_CACHE_INIT_SIZE;
    int hookRecordCacheSize = isNoDataQueueMode ? sharedMemCount : HOOK_RECORD_CACHE_INIT_SIZE;

    for (int index = 0; index < rawDataCacheSize; ++index) {
        rawStackCache_.emplace_back(std::make_shared<RawStack>());
    }
    for (int index = 0; index < hookRecordCacheSize; ++index) {
        mallocRecordCache_.emplace_back(std::make_shared<MallocRecord>());
        mmapRecordCache_.emplace_back(std::make_shared<MmapRecord>());
        munmapRecordCache_.emplace_back(std::make_shared<MunmapRecord>());
        jsRecordCache_.emplace_back(std::make_shared<JsRecord>());
        if (hookConfig_.statistics_interval() > 0) {
            freeRecordSimpCache_.emplace_back(std::make_shared<FreeRecordSimp>());
        } else {
            freeRecordCache_.emplace_back(std::make_shared<FreeRecord>());
        }
    }
}

HookRecordPtr CreateCacheItem(int16_t type)
{
    switch (type) {
        case MALLOC_MSG:
            return std::make_shared<MallocRecord>();
        case FREE_MSG:
            return std::make_shared<FreeRecord>();
        case FREE_MSG_SIMP:
            return std::make_shared<FreeRecordSimp>();
        case MMAP_MSG:
            return std::make_shared<MmapRecord>();
        case MUNMAP_MSG:
            return std::make_shared<MunmapRecord>();
        case JS_STACK_MSG:
            return std::make_shared<JsRecord>();
        default:
            PROFILER_LOG_ERROR(LOG_CORE, "CreateItem get unexpected type");
            return nullptr;
    }
}

template <typename T>
std::shared_ptr<T> HookRecordFactory::GetRecordFromCache(std::deque<std::shared_ptr<T>>& dataCache, int16_t type)
{
    std::unique_lock<std::mutex> lock(recordMutex_);
    if (!dataCache.empty()) {
        std::shared_ptr<T> cachedData = std::move(dataCache.back());
        dataCache.pop_back();
        return cachedData;
    }
    return CreateCacheItem(type);
}

std::shared_ptr<RawStack> HookRecordFactory::GetRawStackFromCache()
{
    std::unique_lock<std::mutex> lock(rawStackMutex_);
    if (!rawStackCache_.empty()) {
        std::shared_ptr<RawStack> cachedData = std::move(rawStackCache_.back());
        rawStackCache_.pop_back();
        return cachedData;
    }
    lock.unlock();
    return std::make_shared<RawStack>();
}

void HookRecordFactory::SaveRawStack(RawStackPtr rawStack)
{
    std::unique_lock<std::mutex> rawStackLock(rawStackMutex_);
    if (rawStack && (rawStackCache_.size() <= RAW_DATA_CACHE_MAX_SIZE)) {
        rawStackCache_.push_back(std::move(rawStack));
    }
}

void HookRecordFactory::SaveRecordToCache(uint16_t type, HookRecordPtr hookRecord)
{
    switch (type) {
        case FREE_MSG_SIMP:
            if (freeRecordSimpCache_.size() < HOOK_RECORD_CACHE_MAX_SIZE) {
                freeRecordSimpCache_.push_back(hookRecord);
            }
            break;
        case FREE_MSG:
            if (freeRecordCache_.size() < HOOK_RECORD_CACHE_MAX_SIZE) {
                freeRecordCache_.push_back(hookRecord);
            }
            break;
        case MALLOC_MSG:
            if (mallocRecordCache_.size() < HOOK_RECORD_CACHE_MAX_SIZE) {
                mallocRecordCache_.push_back(hookRecord);
            }
            break;
        case MMAP_MSG:
            if (mmapRecordCache_.size() < HOOK_RECORD_CACHE_MAX_SIZE) {
                mmapRecordCache_.push_back(hookRecord);
            }
            break;
        case MUNMAP_MSG:
            if (munmapRecordCache_.size() < HOOK_RECORD_CACHE_MAX_SIZE) {
                munmapRecordCache_.push_back(hookRecord);
            }
            break;
        case JS_STACK_MSG:
            if (jsRecordCache_.size() < HOOK_RECORD_CACHE_MAX_SIZE) {
                jsRecordCache_.push_back(hookRecord);
            }
            break;
        default:
            break;
    }
}

void HookRecordFactory::SaveHookRecord(HookRecordPtr hookRecord)
{
    if (hookRecord == nullptr) {
        return;
    }
    auto rawStack = hookRecord->GetRawStack();
    uint16_t type = hookRecord->GetType();
    hookRecord->Reset();
    SaveRawStack(std::move(rawStack));
    std::unique_lock<std::mutex> lock(recordMutex_);
    SaveRecordToCache(type, std::move(hookRecord));
}

std::shared_ptr<HookRecord> HookRecordFactory::CreateStackRecord(uint16_t type, RawStackPtr rawStack)
{
    switch (type) {
        case MMAP_FILE_PAGE_MSG:
            return std::make_shared<MmapFilePageRecord>(rawStack);
        case MUNMAP_MSG: {
            auto munmapRecord = GetRecordFromCache(munmapRecordCache_, MUNMAP_MSG);
            munmapRecord->rawStack_ = rawStack;
            return munmapRecord;
        }
        case MEMORY_UNUSING_MSG:
            return std::make_shared<MemoryUnusingRecord>(rawStack);
        case MEMORY_USING_MSG:
            return std::make_shared<MemoryUsingRecord>(rawStack);
        case MMAP_FILE_TYPE:
            return std::make_shared<TagRecord>(rawStack);
        case MALLOC_MSG: {
            auto mallocRecord = GetRecordFromCache(mallocRecordCache_, MALLOC_MSG);
            mallocRecord->rawStack_ = rawStack;
            return mallocRecord;
        }
        case MMAP_MSG: {
            auto mmapRecord = GetRecordFromCache(mmapRecordCache_, MMAP_MSG);
            mmapRecord->rawStack_ = rawStack;
            return mmapRecord;
        }
        case FREE_MSG: {
            auto freeRecord = GetRecordFromCache(freeRecordCache_, FREE_MSG);
            freeRecord->rawStack_ = rawStack;
            return freeRecord;
        }
        default:
            PROFILER_LOG_ERROR(LOG_CORE, "GetHookRecord get unknown type");
            return nullptr;
    }
}

void HookRecordFactory::InitRawStack(RawStackPtr rawStack, const int8_t sharedMemData[],
                                     uint32_t size, bool storeData)
{
    if (storeData) {
        rawStack->baseStackData = std::make_unique<uint8_t[]>(size);
        if (memcpy_s(rawStack->baseStackData.get(), size, sharedMemData, size) != EOK) {
            PROFILER_LOG_ERROR(LOG_CORE, "HookRecordFactory memcpy_s raw data failed!");
            return;
        }
        rawStack->stackContext = reinterpret_cast<BaseStackRawData*>(rawStack->baseStackData.get());
        rawStack->data = rawStack->baseStackData.get() + sizeof(BaseStackRawData);
    } else {
        rawStack->stackContext = reinterpret_cast<BaseStackRawData*>(const_cast<int8_t*>(sharedMemData));
        rawStack->data = reinterpret_cast<uint8_t*>(const_cast<int8_t*>(sharedMemData)) + sizeof(BaseStackRawData);
    }
}

std::shared_ptr<HookRecord> HookRecordFactory::GetHookRecord(const int8_t sharedMemData[],
                                                             uint32_t size, bool storeData)
{
    if (size == sizeof(void*)) {
        uint64_t freeAddr = 0;
        if (memcpy_s(&freeAddr, sizeof(freeAddr), sharedMemData, size) != EOK) {
            PROFILER_LOG_ERROR(LOG_CORE, "GetHookRecord memcpy failed");
            return nullptr;
        }
        auto freeRecord = GetRecordFromCache(freeRecordSimpCache_, FREE_MSG_SIMP);
        freeRecord->SetAddr(freeAddr);
        return freeRecord;
    }
    auto rawStack = GetRawStackFromCache();
    InitRawStack(rawStack, sharedMemData, size, storeData);
    uint16_t type = rawStack->stackContext->type;
    switch (type) {
        case NMD_MSG:
            return std::make_shared<NmdRecord>(rawStack);
        case END_MSG:
        case MEMORY_TAG:
        case THREAD_NAME_MSG:
            return std::make_shared<TagRecord>(rawStack);
        case PR_SET_VMA_MSG:
            return std::make_shared<PrSetVmaRecord>(rawStack);
        case JS_STACK_MSG: {
            auto jsRecord = GetRecordFromCache(jsRecordCache_, JS_STACK_MSG);
            jsRecord->rawStack_ = rawStack;
            return jsRecord;
        }
        default:
            break;
    }

    if (hookConfig_.fp_unwind()) {
        rawStack->fpDepth = (size - sizeof(BaseStackRawData)) / sizeof(uint64_t);
    } else {
        size_t rawRealSize = sizeof(BaseStackRawData) + MAX_REG_SIZE * sizeof(char);
        rawStack->stackSize = size - rawRealSize;
        if (rawStack->stackSize > 0) {
            rawStack->stackData = rawStack->baseStackData.get() + rawRealSize;
        }
    }
    return CreateStackRecord(type, rawStack);
}
}