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
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include "hook_common.h"
#include "logging.h"

namespace {
    constexpr size_t WRITE_CHUNK_SIZE = 4 * 1024 * 1024; // 4MB
    constexpr uint64_t MAX_CACHE_FILE_SIZE = 96 * 1024 * 1024; // 96MB
    constexpr double MIN_DISK_USAGE_LIMIT_PERCENT = 0.15; // 15%
    const std::string CACHE_FILE_PATH = "/data/storage/el2/base/cache/";
    const std::string CACHE_FILE_PREFIX = "native_hook_cache_";
}

using namespace OHOS::Developtools::NativeDaemon;

CacheFileWriter::CacheFileWriter(int eventFd, uint32_t cacheMemorySize)
{
    cacheFileDir_ = CACHE_FILE_PATH;
    CleanOldCacheFiles();
    eventNotifier_ = EventNotifier::CreateWithFd(eventFd);
    if (cacheMemorySize < MIN_FILE_CACHE_MEMORY_SIZE || cacheMemorySize > MAX_FILE_CACHE_MEMORY_SIZE) {
        cacheMemorySize_ = DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES;
    } else {
        cacheMemorySize_ = cacheMemorySize * MB_TO_BYTES;
    }
    uint32_t extraSize = sizeof(StackRawData) + sizeof(uint32_t);
    memoryCache_.reserve(cacheMemorySize_ + extraSize);
}

CacheFileWriter::~CacheFileWriter()
{
    Flush();
    if (currentFileFd_ != -1) {
        close(currentFileFd_);
        currentFileFd_ = -1;
    }
    eventNotifier_ = nullptr;
}

void CacheFileWriter::Prepare(FileEventCallback callback)
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    fileEventCallback_ = callback;
    initialized_ = CreateNewCacheFile();
}

bool CacheFileWriter::Write(const void* data, size_t size)
{
    if (data == nullptr || size == 0 || size > cacheMemorySize_ || !initialized_ || isTotalFileSizeLimit_) {
        return false;
    }
    uint32_t totalSize = size;
    size_t incomingSize = sizeof(totalSize) + size;

    std::lock_guard<std::mutex> lock(writeMutex_);
    if (memoryCache_.size() + incomingSize >= cacheMemorySize_) {
        if (!FlushMemoryCacheToFile()) {
            return false;
        }
    }
    WriteToMemoryCache(&totalSize, sizeof(totalSize));
    WriteToMemoryCache(data, size);
    return true;
}

bool CacheFileWriter::WriteWithPayload(const void* data, size_t size, const void* payload, size_t payloadSize)
{
    if (data == nullptr || size == 0 || size > cacheMemorySize_ ||
        payloadSize > cacheMemorySize_ || !initialized_ || isTotalFileSizeLimit_) {
        return false;
    }
    uint32_t totalSize = size + payloadSize;
    size_t incomingSize = sizeof(totalSize) + size + payloadSize;

    std::lock_guard<std::mutex> lock(writeMutex_);
    if (memoryCache_.size() + incomingSize >= cacheMemorySize_) {
        if (!FlushMemoryCacheToFile()) {
            return false;
        }
    }
    WriteToMemoryCache(&totalSize, sizeof(totalSize));
    WriteToMemoryCache(data, size);
    if (payload != nullptr && payloadSize > 0) {
        WriteToMemoryCache(payload, payloadSize);
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
    FlushMemoryCacheToFile();
}

void CacheFileWriter::WriteToMemoryCache(const void* data, size_t size)
{
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(data);
    memoryCache_.insert(memoryCache_.end(), buffer, buffer + size);
}

std::string CacheFileWriter::GetCacheFilePath(uint16_t index) const
{
    time_t now = time(nullptr);
    tm tm;
    const int timeLength = 64;
    char timeBuffer[timeLength] = {0};
    std::string filePath = cacheFileDir_;

    if (localtime_r(&now, &tm) == nullptr || strftime(timeBuffer, sizeof(timeBuffer), "%Y_%m_%d_%H_%M_%S", &tm) == 0) {
        filePath += CACHE_FILE_PREFIX + std::to_string(index) + ".bin";
        return filePath;
    }
    std::string timestamp = timeBuffer;
    filePath += CACHE_FILE_PREFIX + timestamp + "_" + std::to_string(index) + ".bin";
    return filePath;
}

bool CacheFileWriter::CheckIsDiskUsageExceedLimit()
{
    struct statvfs diskInfo;
    if (statvfs(cacheFileDir_.c_str(), &diskInfo) != 0) {
        return true;
    }

    if (diskInfo.f_blocks > 0) {
        double freePercent = static_cast<double>(diskInfo.f_bfree) / static_cast<double>(diskInfo.f_blocks);
        if (freePercent > MIN_DISK_USAGE_LIMIT_PERCENT) {
            return false;
        }
    }
    return true;
}

void CacheFileWriter::HandleFileCacheEnd()
{
    (void)WriteToFile(memoryCache_.data(), memoryCache_.size());
    fsync(currentFileFd_);
    // Notify native_daemon that file is ready to read
    NotifyFileEvent(currentFileIndex_, CACHE_FILE_READY);
    NotifyFileEvent(currentFileIndex_, CACHE_FILE_STOP);
    memoryCache_.clear();
    if (currentFileFd_ != -1) {
        close(currentFileFd_);
        currentFileFd_ = -1;
    }
}

void CacheFileWriter::HandleFileSizeExceedLimit()
{
    isTotalFileSizeLimit_ = true;
    StackRawData appendEndData = {{{{0}}}};
    appendEndData.type = END_MSG;
    uint32_t totalSize = sizeof(appendEndData);
    WriteToMemoryCache(&totalSize, sizeof(totalSize));
    WriteToMemoryCache(&appendEndData, sizeof(appendEndData));
    isCacheFileEnded_ = true;
    HandleFileCacheEnd();
}

bool CacheFileWriter::CreateNewCacheFile()
{
    if (CheckIsDiskUsageExceedLimit()) {
        HandleFileSizeExceedLimit();
        return false;
    }
    if (initialized_) {
        fsync(currentFileFd_);
        // Notify native_daemon that file is ready to read
        NotifyFileEvent(currentFileIndex_, CACHE_FILE_READY);
    }

    if (currentFileFd_ != -1) {
        close(currentFileFd_);
        currentFileFd_ = -1;
    }

    currentFileIndex_++;
    cacheFilePath_ = GetCacheFilePath(currentFileIndex_);
    currentFileFd_ = open(cacheFilePath_.c_str(), O_RDWR | O_CREAT | O_TRUNC, static_cast<mode_t>(0644));
    if (currentFileFd_ < 0) {
        return false;
    }
    unlink(cacheFilePath_.c_str());

    // Notify native_daemon that a new cache file created
    NotifyFileEvent(currentFileIndex_, CACHE_FILE_CREATED);
    totalBytesWritten_ += currentFileSize_;
    currentFileSize_ = 0;

    return true;
}

bool CacheFileWriter::WriteToFile(const uint8_t* data, size_t size)
{
    if (currentFileFd_ < 0) {
        return false;
    }

    size_t remaining = size;
    size_t offset = 0;
    while (remaining > 0) {
        size_t writeSize = std::min(remaining, WRITE_CHUNK_SIZE);
        ssize_t written = write(currentFileFd_, data + offset, writeSize);
        if (written < 0) {
            return false;
        }
        if (static_cast<size_t>(written) != writeSize) {
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
    if (memoryCache_.size() == 0) {
        return true;
    }

    if (currentFileFd_ < 0) {
        return false;
    }

    if (isCacheFileEnded_) {
        HandleFileCacheEnd();
        return true;
    }

    if (currentFileSize_ + memoryCache_.size() >= MAX_CACHE_FILE_SIZE) {
        if (!CreateNewCacheFile()) {
            return false;
        }
    }
    bool ret = WriteToFile(memoryCache_.data(), memoryCache_.size());
    if (ret) {
        memoryCache_.clear();
    }
    return ret;
}

bool CacheFileWriter::Flush()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    return FlushMemoryCacheToFile();
}

void CacheFileWriter::NotifyFileEvent(uint16_t fileIndex, uint16_t eventType)
{
    switch (eventType) {
        case CACHE_FILE_CREATED: {
            if (fileEventCallback_ != nullptr) {
                fileEventCallback_(fileIndex, eventType, currentFileFd_);
            }
            break;
        }
        case CACHE_FILE_READY: {
            if (eventNotifier_ != nullptr) {
                uint64_t fileCnt = 1;
                eventNotifier_->Post(fileCnt);
            }
            break;
        }
        case CACHE_FILE_STOP: {
            if (eventNotifier_ != nullptr) {
                uint64_t fileCnt = 1;
                eventNotifier_->Post(fileCnt);
            }
            break;
        }
        default:
            break;
    }
}

bool CacheFileWriter::CleanOldCacheFiles()
{
    DIR* dir = opendir(cacheFileDir_.c_str());
    if (dir == nullptr) {
        return false;
    }

    const char* prefix = CACHE_FILE_PREFIX.c_str();
    const char* suffix = ".bin";
    size_t prefixLen = strlen(prefix);
    size_t suffixLen = strlen(suffix);

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // Check if file start with CACHE_FILE_PREFIX and end with ".bin"
        size_t len = strlen(entry->d_name);
        bool isFileMatched = (len > prefixLen + suffixLen &&
            strncmp(entry->d_name, prefix, prefixLen) == 0 &&
            strcmp(entry->d_name + len - suffixLen, suffix) == 0);
        if (!isFileMatched) {
            continue;
        }
        std::string fileName(entry->d_name);
        std::string fullPath = cacheFileDir_ + fileName;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            unlink(fullPath.c_str());
        }
    }
    closedir(dir);
    return true;
}