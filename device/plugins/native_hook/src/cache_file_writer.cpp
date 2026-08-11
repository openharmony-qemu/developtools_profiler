/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include "cache_file_writer.h"

#include <algorithm>
#include <chrono>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include "logging.h"
#include "securec.h"

namespace {
constexpr size_t WRITE_CHUNK_SIZE = 4 * 1024 * 1024;
constexpr uint64_t MAX_CACHE_FILE_SIZE = 96 * 1024 * 1024;
constexpr auto WAIT_NEXT_CACHE_FILE_INTERVAL = std::chrono::milliseconds(5);
constexpr auto WAIT_NEXT_CACHE_FILE_TIMEOUT = std::chrono::seconds(3);
constexpr const char* CACHE_VMA_NAME = "hooknativesmb";
}

using namespace OHOS::Developtools::NativeDaemon;

CacheFileWriter::CacheFileWriter(uint32_t cacheMemorySize)
{
    if (cacheMemorySize < MIN_FILE_CACHE_MEMORY_SIZE || cacheMemorySize > MAX_FILE_CACHE_MEMORY_SIZE) {
        memoryCacheDataSizeLimit_ = DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES;
    } else {
        memoryCacheDataSizeLimit_ = cacheMemorySize * MB_TO_BYTES;
    }
    uint32_t extraSize = sizeof(StackRawData) + sizeof(uint32_t);
    memoryCacheCapacity_ = memoryCacheDataSizeLimit_ + extraSize;
    memoryCache_ = AllocateMmapCache(memoryCacheCapacity_);
}

uint8_t* CacheFileWriter::AllocateMmapCache(uint32_t capacity)
{
    void* base = mmap(nullptr, capacity, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) {
        return nullptr;
    }
    (void)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, reinterpret_cast<uintptr_t>(base), capacity,
                reinterpret_cast<uintptr_t>(CACHE_VMA_NAME));
    return reinterpret_cast<uint8_t*>(base);
}

void CacheFileWriter::ReleaseMmapCache()
{
    if (memoryCache_ != nullptr) {
        munmap(memoryCache_, memoryCacheCapacity_);
        memoryCache_ = nullptr;
        memoryCacheCapacity_ = 0;
        memoryCacheSize_ = 0;
    }
}

CacheFileWriter::~CacheFileWriter()
{
    if (!isCacheFileEnded_ && (memoryCacheSize_ > 0 || currentFileSize_ > 0)) {
        SetCacheFileWriterEnd();
    } else {
        Flush();
    }
    CloseCacheFile(currentFileFd_);
    for (auto& fd : pendingFileFds_) {
        CloseCacheFile(fd);
    }
    ReleaseMmapCache();
}

void CacheFileWriter::Prepare(FileEventCallback callback)
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    fileEventCallback_ = std::move(callback);
    initialized_ = true;
}

void CacheFileWriter::EnterStopMode()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    stopModeEnabled_ = true;
}

bool CacheFileWriter::AddCacheFile(uint16_t fileIndex, int fd)
{
    if (fd < 0 || fileIndex >= FILE_CACHE_FD_COUNT) {
        return false;
    }
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (pendingFileFds_[fileIndex] >= 0 || currentFileIndex_ == fileIndex) {
        return false;
    }
    pendingFileFds_[fileIndex] = fd;
    return true;
}

bool CacheFileWriter::Write(const void* data, size_t size)
{
    if (data == nullptr || size == 0 || size > memoryCacheDataSizeLimit_ || !initialized_ || isCacheFileEnded_ ||
        memoryCache_ == nullptr) {
        return false;
    }
    uint32_t totalSize = size;
    size_t incomingSize = sizeof(totalSize) + size;
    std::lock_guard<std::mutex> lock(writeMutex_);
    
    if (!stopModeEnabled_ && ShouldApplyBackpressure()) {
        ApplyBackpressureDelay();
    }
    
    if (!EnsureWritableCacheFile()) {
        return false;
    }
    if (memoryCacheSize_ + incomingSize >= memoryCacheDataSizeLimit_ && !FlushMemoryCacheToFile()) {
        return false;
    }
    size_t originalSize = memoryCacheSize_;
    if (!WriteToMemoryCache(&totalSize, sizeof(totalSize)) || !WriteToMemoryCache(data, size)) {
        memoryCacheSize_ = originalSize;
        return false;
    }
    return true;
}

bool CacheFileWriter::WriteWithPayload(const void* data, size_t size, const void* payload, size_t payloadSize)
{
    if (data == nullptr || size == 0 || size > memoryCacheDataSizeLimit_ || payloadSize > memoryCacheDataSizeLimit_ ||
        !initialized_ || isCacheFileEnded_ || memoryCache_ == nullptr) {
        return false;
    }
    uint32_t totalSize = size + payloadSize;
    size_t incomingSize = sizeof(totalSize) + size + payloadSize;
    std::lock_guard<std::mutex> lock(writeMutex_);
    
    if (!stopModeEnabled_ && ShouldApplyBackpressure()) {
        ApplyBackpressureDelay();
    }
    
    if (!EnsureWritableCacheFile()) {
        return false;
    }
    if (memoryCacheSize_ + incomingSize >= memoryCacheDataSizeLimit_ && !FlushMemoryCacheToFile()) {
        return false;
    }
    size_t originalSize = memoryCacheSize_;
    bool ok = WriteToMemoryCache(&totalSize, sizeof(totalSize)) && WriteToMemoryCache(data, size);
    if (payload != nullptr && payloadSize > 0) {
        ok = ok && WriteToMemoryCache(payload, payloadSize);
    }
    if (!ok) {
        memoryCacheSize_ = originalSize;
        return false;
    }
    return true;
}

bool CacheFileWriter::WriteWithExtraData(const void* data, size_t size, const void* payload, size_t payloadSize,
    const void* firstExtra, size_t firstExtraSize, const void* secondExtra, size_t secondExtraSize)
{
    if (payload == nullptr) {
        payloadSize = 0;
    }
    if (firstExtra == nullptr) {
        firstExtraSize = 0;
    }
    if (secondExtra == nullptr) {
        secondExtraSize = 0;
    }
    uint32_t totalSize = size + payloadSize + firstExtraSize + secondExtraSize;
    if (data == nullptr || size == 0 || totalSize > memoryCacheDataSizeLimit_ || !initialized_ || isCacheFileEnded_ ||
        memoryCache_ == nullptr) {
        return false;
    }
    size_t incomingSize = sizeof(totalSize) + totalSize;
    std::lock_guard<std::mutex> lock(writeMutex_);
    
    if (!stopModeEnabled_ && ShouldApplyBackpressure()) {
        ApplyBackpressureDelay();
    }
    
    if (!EnsureWritableCacheFile()) {
        return false;
    }
    if (memoryCacheSize_ + incomingSize >= memoryCacheDataSizeLimit_ && !FlushMemoryCacheToFile()) {
        return false;
    }
    size_t originalSize = memoryCacheSize_;
    bool ok = WriteToMemoryCache(&totalSize, sizeof(totalSize)) && WriteToMemoryCache(data, size);
    if (payloadSize > 0) {
        ok = ok && WriteToMemoryCache(payload, payloadSize);
    }
    if (firstExtraSize > 0) {
        ok = ok && WriteToMemoryCache(firstExtra, firstExtraSize);
    }
    if (secondExtraSize > 0) {
        ok = ok && WriteToMemoryCache(secondExtra, secondExtraSize);
    }
    if (!ok) {
        memoryCacheSize_ = originalSize;
        return false;
    }
    return true;
}

void CacheFileWriter::SetCacheFileWriterEnd()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (isCacheFileEnded_) {
        return;
    }
    isCacheFileEnded_ = true;
    if (memoryCacheSize_ > 0) {
        (void)FlushMemoryCacheToFile();
        return;
    }
    if (currentFileSize_ > 0) {
        (void)ReturnCurrentCacheFile();
    } else {
        CloseCacheFile(currentFileFd_);
        currentFileIndex_ = INVALID_CACHE_FILE_INDEX;
    }
    (void)NotifyStopEvent();
}

bool CacheFileWriter::WriteToMemoryCache(const void* data, size_t size)
{
    if (size == 0) {
        return true;
    }
    if (memoryCache_ == nullptr || data == nullptr ||
        memoryCacheSize_ + size > memoryCacheCapacity_) {
        return false;
    }
    if (memcpy_s(memoryCache_ + memoryCacheSize_, memoryCacheCapacity_ - memoryCacheSize_, data, size) != EOK) {
        return false;
    }
    memoryCacheSize_ += size;
    return true;
}

bool CacheFileWriter::EnsureWritableCacheFile()
{
    return currentFileFd_ >= 0 || SwitchToNextCacheFile();
}

bool CacheFileWriter::FlushEndedCacheFile()
{
    bool ret = WriteToFile(memoryCache_, memoryCacheSize_);
    if (!ret) {
        return false;
    }
    memoryCacheSize_ = 0;
    if (currentFileSize_ > 0 && !ReturnCurrentCacheFile()) {
        return false;
    }
    return NotifyStopEvent();
}

bool CacheFileWriter::NotifyStopEvent()
{
    return NotifyFileEvent(INVALID_CACHE_FILE_INDEX, CACHE_FILE_STOP, -1);
}

bool CacheFileWriter::ReturnCurrentCacheFile()
{
    if (fsync(currentFileFd_) != 0) {
        return false;
    }
    if (!NotifyFileEvent(currentFileIndex_, CACHE_FILE_READY, currentFileFd_)) {
        return false;
    }
    CloseCacheFile(currentFileFd_);
    currentFileIndex_ = INVALID_CACHE_FILE_INDEX;
    currentFileSize_ = 0;
    return true;
}

bool CacheFileWriter::SwitchToNextCacheFile()
{
    for (uint16_t count = 0; count < FILE_CACHE_FD_COUNT; ++count) {
        uint16_t fileIndex = (nextFileIndex_ + count) % FILE_CACHE_FD_COUNT;
        if (pendingFileFds_[fileIndex] < 0) {
            continue;
        }
        currentFileFd_ = pendingFileFds_[fileIndex];
        pendingFileFds_[fileIndex] = -1;
        currentFileIndex_ = fileIndex;
        currentFileSize_ = 0;
        nextFileIndex_ = (fileIndex + 1) % FILE_CACHE_FD_COUNT;
        return true;
    }
    return false;
}

bool CacheFileWriter::WaitForNextCacheFile()
{
    auto start = std::chrono::steady_clock::now();
    while (!stopModeEnabled_ &&
        std::chrono::steady_clock::now() - start < WAIT_NEXT_CACHE_FILE_TIMEOUT) {
        if (SwitchToNextCacheFile()) {
            return true;
        }
        std::this_thread::sleep_for(WAIT_NEXT_CACHE_FILE_INTERVAL);
    }
    return SwitchToNextCacheFile();
}

bool CacheFileWriter::WriteToFile(const uint8_t* data, size_t size)
{
    if (size == 0) {
        return true;
    }
    if (currentFileFd_ < 0) {
        return false;
    }
    size_t remaining = size;
    size_t offset = 0;
    while (remaining > 0) {
        size_t writeSize = std::min(remaining, WRITE_CHUNK_SIZE);
        ssize_t written = write(currentFileFd_, data + offset, writeSize);
        if (written < 0 || static_cast<size_t>(written) != writeSize) {
            return false;
        }
        offset += writeSize;
        remaining -= writeSize;
    }
    currentFileSize_ += size;
    return true;
}

bool CacheFileWriter::FlushMemoryCacheToFile()
{
    if (memoryCacheSize_ == 0) {
        return true;
    }
    if (!EnsureWritableCacheFile()) {
        return false;
    }
    if (isCacheFileEnded_) {
        return FlushEndedCacheFile();
    }
    if (!stopModeEnabled_ && currentFileSize_ + memoryCacheSize_ >= MAX_CACHE_FILE_SIZE) {
        if (!ReturnCurrentCacheFile() || !WaitForNextCacheFile()) {
            return false;
        }
    }
    bool ret = WriteToFile(memoryCache_, memoryCacheSize_);
    if (ret) {
        memoryCacheSize_ = 0;
    }
    return ret;
}

bool CacheFileWriter::Flush()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    return FlushMemoryCacheToFile();
}

bool CacheFileWriter::NotifyFileEvent(uint16_t fileIndex, uint16_t eventType, int fd)
{
    if (fileEventCallback_ == nullptr) {
        return true;
    }
    return fileEventCallback_(fileIndex, eventType, fd);
}

void CacheFileWriter::CloseCacheFile(int& fd)
{
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

void CacheFileWriter::UpdateBackpressureState(const BackpressureStateMsg& state)
{
    currentBackpressureDelay_ = state.recommendedDelayNs;
}

bool CacheFileWriter::ShouldApplyBackpressure()
{
    if (stopModeEnabled_ || isCacheFileEnded_) {
        return false;
    }
    if (currentBackpressureDelay_.load() == 0) {
        return false;
    }

    delayCount_++;
    if (delayCount_ == UINT64_MAX) {
        delayCount_ = 0;
    }
    if (delayCount_ % BACKPRESSURE_SAMPLE_SIZE != 0) {
        return false;
    }
    return true;
}

void CacheFileWriter::ApplyBackpressureDelay()
{
    uint32_t delayNs = currentBackpressureDelay_.load();
    if (delayNs == 0) {
        return;
    }
    AdaptiveWait(delayNs);
}

void CacheFileWriter::AdaptiveWait(uint32_t delayNs)
{
    if (delayNs == 0) {
        return;
    }
    
    if (delayNs <= BACKPRESSURE_HIGH_DELAY_NS) {
        auto start = std::chrono::steady_clock::now();
        auto target = start + std::chrono::nanoseconds(delayNs);
        // ns/us级别延时需要忙等待
        while (std::chrono::steady_clock::now() < target) {
            if (currentBackpressureDelay_.load() == 0 || stopModeEnabled_ || isCacheFileEnded_) {
                break;
            }
            // volatile屏障避免编译优化
            for (volatile int i = 0; i < BACKPRESSURE_VOLATILE_SIZE; ++i);
        }
    }
}
