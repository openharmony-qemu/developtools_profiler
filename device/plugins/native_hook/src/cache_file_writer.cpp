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

#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include "logging.h"
#include "securec.h"

namespace {
constexpr auto WAIT_NEXT_CACHE_FILE_INTERVAL = std::chrono::milliseconds(5);
constexpr auto WAIT_NEXT_CACHE_FILE_TIMEOUT = std::chrono::seconds(3);
}

using namespace OHOS::Developtools::NativeDaemon;

// aligns with daemon GetRecordHead limit, a record larger than this is rejected by daemon parsing
constexpr uint64_t MAX_SINGLE_RECORD_SIZE = DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES;

CacheFileWriter::CacheFileWriter(uint32_t cacheMemorySize)
{
    (void)cacheMemorySize;
}

CacheFileWriter::~CacheFileWriter()
{
    if (!isCacheFileEnded_ && writeOffset_ > DATA_SIZE_HEADER_LEN) {
        SetCacheFileWriterEnd();
    } else {
        Flush();
    }
    CloseCacheFile(currentFileFd_);
    for (auto& fd : pendingFileFds_) {
        CloseCacheFile(fd);
    }
    UnmapMemory();
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

// caller must hold writeMutex_; backpressure, writable file, flush and zero-size guard before writing
bool CacheFileWriter::PrepareRecordWrite(uint32_t totalSize, size_t incomingSize)
{
    if (!stopModeEnabled_ && ShouldApplyBackpressure()) {
        ApplyBackpressureDelay();
    }
    if (!EnsureWritableCacheFile()) {
        return false;
    }
    if (writeOffset_ + incomingSize > mmapSize_ && !FlushMemoryCacheToFile()) {
        return false;
    }
    if (totalSize == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "CacheFileWriter zero totalSize before write, totalSize=%u tid=%ld",
            totalSize, syscall(SYS_gettid));
        return false;
    }
    return true;
}

// caller must hold writeMutex_; rollback on failure or count the committed record
bool CacheFileWriter::FinishRecordWrite(bool ok, uint32_t originalOffset, size_t incomingSize)
{
    if (!ok) {
        PROFILER_LOG_WARN(LOG_CORE, "CacheFileWriter rollback, originalOffset=%u incoming=%zu tid=%ld",
            originalOffset, incomingSize, syscall(SYS_gettid));
        writeOffset_ = originalOffset;
        return false;
    }
    fileRecordCount_++;
    return true;
}

bool CacheFileWriter::Write(const void* data, size_t size)
{
    if (data == nullptr || size == 0 || size > MAX_SINGLE_RECORD_SIZE || !initialized_ || isCacheFileEnded_) {
        return false;
    }
    uint32_t totalSize = size;
    size_t incomingSize = sizeof(totalSize) + size;
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (!PrepareRecordWrite(totalSize, incomingSize)) {
        return false;
    }
    uint32_t originalOffset = writeOffset_;
    bool ok = WriteToMemoryCache(&totalSize, sizeof(totalSize)) && WriteToMemoryCache(data, size);
    return FinishRecordWrite(ok, originalOffset, incomingSize);
}

bool CacheFileWriter::WriteWithPayload(const void* data, size_t size, const void* payload, size_t payloadSize)
{
    if (data == nullptr || size == 0 || size + payloadSize > MAX_SINGLE_RECORD_SIZE ||
        !initialized_ || isCacheFileEnded_) {
        return false;
    }
    uint32_t totalSize = size + payloadSize;
    size_t incomingSize = sizeof(totalSize) + size + payloadSize;
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (!PrepareRecordWrite(totalSize, incomingSize)) {
        return false;
    }
    uint32_t originalOffset = writeOffset_;
    bool ok = WriteToMemoryCache(&totalSize, sizeof(totalSize)) && WriteToMemoryCache(data, size);
    if (payload != nullptr && payloadSize > 0) {
        ok = ok && WriteToMemoryCache(payload, payloadSize);
    }
    return FinishRecordWrite(ok, originalOffset, incomingSize);
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
    if (data == nullptr || size == 0 || totalSize > MAX_SINGLE_RECORD_SIZE || !initialized_ || isCacheFileEnded_) {
        return false;
    }
    size_t incomingSize = sizeof(totalSize) + totalSize;
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (!PrepareRecordWrite(totalSize, incomingSize)) {
        return false;
    }
    uint32_t originalOffset = writeOffset_;
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
    return FinishRecordWrite(ok, originalOffset, incomingSize);
}

void CacheFileWriter::SetCacheFileWriterEnd()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (isCacheFileEnded_) {
        return;
    }
    isCacheFileEnded_ = true;
    if (writeOffset_ > DATA_SIZE_HEADER_LEN) {
        (void)FlushMemoryCacheToFile();
        return;
    }
    UnmapMemory();
    if (currentFileFd_ >= 0) {
        (void)ReturnCurrentCacheFile();
    }
    (void)NotifyStopEvent();
}

bool CacheFileWriter::MapFileToMemory(int fd)
{
    if (fd < 0) {
        return false;
    }
    int fdFlags = fcntl(fd, F_GETFL);
    if (fdFlags < 0 || (static_cast<unsigned int>(fdFlags) & O_ACCMODE) != O_RDWR) {
        PROFILER_LOG_ERROR(LOG_CORE, "MapFileToMemory fd not O_RDWR, fd=%d errno=%d", fd, errno);
        return false;
    }
    if (fallocate(fd, 0, 0, static_cast<off_t>(MMAP_FILE_SIZE)) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "MapFileToMemory fallocate failed, fd=%d errno=%d", fd, errno);
        return false;
    }
    // bypass libc mmap via syscall: plain mmap goes through musl malloc dispatch and gets
    // captured by hook_mmap as a spurious 64MB MMAP_MSG polluting application memory statistics
#if defined(__NR_mmap)
    void* addr = reinterpret_cast<void*>(syscall(__NR_mmap, nullptr, MMAP_FILE_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
#else
    // arm32 musl only defines __NR_mmap2 whose offset is in page units (0 here)
    void* addr = reinterpret_cast<void*>(syscall(__NR_mmap2, nullptr, MMAP_FILE_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
#endif
    if (addr == reinterpret_cast<void*>(-1)) {  // syscall returns -1 on error, same as MAP_FAILED
        PROFILER_LOG_ERROR(LOG_CORE, "MapFileToMemory mmap failed, fd=%d errno=%d", fd, errno);
        return false;
    }
    mmapAddr_ = static_cast<uint8_t*>(addr);
    mmapSize_ = MMAP_FILE_SIZE;
    writeOffset_ = DATA_SIZE_HEADER_LEN;
    fileRecordCount_ = 0;
    PROFILER_LOG_INFO(LOG_CORE, "CacheFileWriter mmap done, addr=%p size=%u fd=%d tid=%ld",
        static_cast<void*>(mmapAddr_), mmapSize_, fd, syscall(SYS_gettid));
    uint32_t dataSize = 0;
    if (memcpy_s(mmapAddr_, mmapSize_, &dataSize, DATA_SIZE_HEADER_LEN) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "MapFileToMemory init data size header failed");
        UnmapMemory();
        return false;
    }
    return true;
}

void CacheFileWriter::UnmapMemory()
{
    if (mmapAddr_ != nullptr && mmapSize_ > 0) {
        void* addr = static_cast<void*>(mmapAddr_);
        uint32_t size = mmapSize_;
        // bypass libc munmap via syscall to avoid hook_munmap capture, same as mmap above
        syscall(SYS_munmap, mmapAddr_, mmapSize_);
        PROFILER_LOG_INFO(LOG_CORE, "CacheFileWriter munmap done, addr=%p size=%u tid=%ld",
            addr, size, syscall(SYS_gettid));
    }
    mmapAddr_ = nullptr;
    mmapSize_ = 0;
    writeOffset_ = 0;
}

void CacheFileWriter::UpdateDataSizeHeader()
{
    if (mmapAddr_ == nullptr || mmapSize_ < DATA_SIZE_HEADER_LEN || writeOffset_ < DATA_SIZE_HEADER_LEN) {
        return;
    }
    uint32_t dataSize = writeOffset_ - DATA_SIZE_HEADER_LEN;
    if (memcpy_s(mmapAddr_, DATA_SIZE_HEADER_LEN, &dataSize, DATA_SIZE_HEADER_LEN) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "UpdateDataSizeHeader memcpy_s failed");
    }
}

bool CacheFileWriter::WriteToMemoryCache(const void* data, size_t size)
{
    if (size == 0) {
        return true;
    }
    if (mmapAddr_ == nullptr || data == nullptr || writeOffset_ + size > mmapSize_) {
        return false;
    }
    if (memcpy_s(mmapAddr_ + writeOffset_, mmapSize_ - writeOffset_, data, size) != EOK) {
        return false;
    }
    writeOffset_ += size;
    return true;
}

bool CacheFileWriter::EnsureWritableCacheFile()
{
    return (currentFileFd_ >= 0 && mmapAddr_ != nullptr) || SwitchToNextCacheFile();
}

bool CacheFileWriter::FlushEndedCacheFile()
{
    if (mmapAddr_ == nullptr) {
        return NotifyStopEvent();
    }
    UpdateDataSizeHeader();
    PROFILER_LOG_INFO(LOG_CORE,
        "CacheFileWriter flush ended file, datasize=%u writeOffset=%u recordCount=%u fd=%d tid=%ld",
        static_cast<uint32_t>(writeOffset_ - DATA_SIZE_HEADER_LEN),
        writeOffset_,
        fileRecordCount_, currentFileFd_, syscall(SYS_gettid));
    if (msync(mmapAddr_, mmapSize_, MS_SYNC) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "FlushEndedCacheFile msync failed, errno=%d", errno);
        return false;
    }
    UnmapMemory();
    if (currentFileFd_ >= 0 && !ReturnCurrentCacheFile()) {
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
    if (currentFileFd_ < 0) {
        return false;
    }
    if (!NotifyFileEvent(currentFileIndex_, CACHE_FILE_READY, currentFileFd_)) {
        return false;
    }
    CloseCacheFile(currentFileFd_);
    currentFileIndex_ = INVALID_CACHE_FILE_INDEX;
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
        nextFileIndex_ = (fileIndex + 1) % FILE_CACHE_FD_COUNT;
        if (!MapFileToMemory(currentFileFd_)) {
            CloseCacheFile(currentFileFd_);
            currentFileIndex_ = INVALID_CACHE_FILE_INDEX;
            continue;
        }
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

bool CacheFileWriter::FlushMemoryCacheToFile()
{
    if (writeOffset_ <= DATA_SIZE_HEADER_LEN) {
        return true;
    }
    if (!EnsureWritableCacheFile()) {
        return false;
    }
    if (isCacheFileEnded_) {
        return FlushEndedCacheFile();
    }
    UpdateDataSizeHeader();
    PROFILER_LOG_INFO(LOG_CORE,
        "CacheFileWriter flush file, datasize=%u writeOffset=%u recordCount=%u fd=%d tid=%ld",
        static_cast<uint32_t>(writeOffset_ - DATA_SIZE_HEADER_LEN),
        writeOffset_,
        fileRecordCount_, currentFileFd_, syscall(SYS_gettid));
    if (msync(mmapAddr_, mmapSize_, MS_SYNC) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "FlushMemoryCacheToFile msync failed, errno=%d", errno);
        return false;
    }
    UnmapMemory();
    if (!ReturnCurrentCacheFile() || !WaitForNextCacheFile()) {
        return false;
    }
    return true;
}

bool CacheFileWriter::Flush()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (mmapAddr_ == nullptr || writeOffset_ <= DATA_SIZE_HEADER_LEN) {
        return true;
    }
    if (msync(mmapAddr_, mmapSize_, MS_SYNC) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "Flush msync failed, errno=%d", errno);
        return false;
    }
    return true;
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
        syscall(SYS_close, fd);
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
