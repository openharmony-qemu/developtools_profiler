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
#include "trace_file_writer.h"

#include <cinttypes>
#include <memory>
#include <sys/statvfs.h>
#include <unistd.h>

#include "common.h"
#include "logging.h"

using CharPtr = std::unique_ptr<char>::pointer;
using ConstCharPtr = std::unique_ptr<const char>::pointer;

namespace {
constexpr int MB_TO_BYTE = (1024 * 1024);
constexpr int GB_TO_BYTE = (1024 * 1024 * 1024);
constexpr int SPLIT_FILE_MIN_SIZE = 200;    // split file min size
constexpr int SPLIT_FILE_DEFAULT_NUM = 10;  // split file default num
} // namespace

TraceFileWriter::TraceFileWriter(const std::string& path) : TraceFileWriter(path, false, 0, 0)
{
}

TraceFileWriter::TraceFileWriter(const std::string& path, bool splitFile, uint32_t splitFileMaxSizeMb,
    uint32_t splitFileMaxNum) : path_(path), isSplitFile_(splitFile)
{
    splitFileMaxSize_ = (splitFileMaxSizeMb < SPLIT_FILE_MIN_SIZE) ? (SPLIT_FILE_MIN_SIZE * MB_TO_BYTE) :
        (splitFileMaxSizeMb * MB_TO_BYTE);
    splitFileMaxNum_ = (splitFileMaxNum == 0) ? SPLIT_FILE_DEFAULT_NUM : splitFileMaxNum;
    oldPath_ = path;
    fileNum_ = 1;

    WriteHeader();
    (void)FlushStream();
}

TraceFileWriter::~TraceFileWriter()
{
    (void)FlushStream();
    if (stream_.is_open()) {
        stream_.close();
    }
}

std::string TraceFileWriter::Path() const
{
    return path_;
}

bool TraceFileWriter::SetPluginConfig(const void* data, size_t size)
{
    if (isSplitFile_) {
        std::vector<char> configVec;
        auto configData = reinterpret_cast<ConstCharPtr>(data);
        configVec.insert(configVec.end(), configData, configData + size);
        pluginConfigsData_.push_back(std::move(configVec));
    }

    Write(data, size);
    return true;
}

void TraceFileWriter::SetTimeStamp()
{
    constexpr uint64_t nanoSeconds = 1000000000;
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    header_.data_.boottime = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_REALTIME, &ts);
    header_.data_.realtime = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    header_.data_.realtimeCoarse = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC, &ts);
    header_.data_.monotonic = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    header_.data_.monotonicCoarse = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    header_.data_.monotonicRaw = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
}

bool TraceFileWriter::WriteHeader()
{
    LogDiskUsage();
    if (isSplitFile_) {
        std::string timeStr = COMMON::GetTimeStr();
        std::size_t pos = oldPath_.find_last_of('.');
        if (pos != std::string::npos) {
            path_ = oldPath_.substr(0, pos) + "_" + timeStr + "_" + std::to_string(fileNum_) +
                oldPath_.substr(pos, oldPath_.size());
        } else {
            path_ = oldPath_ + "_" + timeStr + "_" + std::to_string(fileNum_);
        }
        splitFilePaths_.push(path_);
        DeleteOldSplitFile();
    }

    stream_.open(path_, std::ios_base::out | std::ios_base::binary);
    CHECK_TRUE(stream_.is_open(), false, "open %s failed, %d!", path_.c_str(), errno);

    // write initial header, makes file write position move forward
    helper_ = {};
    header_ = {};
    stream_.write(reinterpret_cast<CharPtr>(&header_), sizeof(header_));
    CHECK_TRUE(stream_, false, "write initial header to %s failed!", path_.c_str());
    dataSize_ = header_.HEADER_SIZE;
    HILOG_INFO(LOG_CORE, "write file(%s) header end", path_.c_str());
    return true;
}

// delete first split file if split file num over max
void TraceFileWriter::DeleteOldSplitFile()
{
    if (splitFilePaths_.size() <= splitFileMaxNum_) {
        HILOG_INFO(LOG_CORE, "splitFilePaths_ size %zu, no need to delete.", splitFilePaths_.size());
        return;
    }

    std::string splitFilePath = splitFilePaths_.front();
    int ret = unlink(splitFilePath.c_str());
    HILOG_INFO(LOG_CORE, "DeleteOldSplitFile remove %s return %d. ", splitFilePath.c_str(), ret);
    splitFilePaths_.pop();
}

long TraceFileWriter::Write(const void* data, size_t size)
{
    if (isSplitFile_ && !isStop_) {
        if (IsSplitFile(size)) {
            return -1;
        }
    }

    uint32_t dataLen = size;
    CHECK_TRUE(stream_.is_open(), 0, "binary file %s not open or open failed!", path_.c_str());

    // write 4B data length.
    stream_.write(reinterpret_cast<CharPtr>(&dataLen), sizeof(dataLen));
    CHECK_TRUE(stream_, 0, "binary file %s write raw buffer size failed!", path_.c_str());
    CHECK_TRUE(helper_.AddSegment(reinterpret_cast<uint8_t*>(&dataLen), sizeof(dataLen)),
        0, "Add payload for size %u FAILED!", dataLen);

    // write data bytes
    stream_.write(reinterpret_cast<ConstCharPtr>(data), size);
    CHECK_TRUE(stream_, 0, "binary file %s write raw buffer data failed!", path_.c_str());
    CHECK_TRUE(helper_.AddSegment(reinterpret_cast<uint8_t*>(const_cast<void*>(data)), size),
        0, "Add payload for data bytes %zu FAILED!", size);

    long nbytes = sizeof(dataLen) + size;
    writeBytes_ += nbytes;
    ++writeCount_;
    return nbytes;
}

bool TraceFileWriter::IsSplitFile(uint32_t size)
{
    dataSize_ += sizeof(uint32_t) + size;
    if (dataSize_ >= splitFileMaxSize_) {
        HILOG_INFO(LOG_CORE, "need to split the file(%s), data size:%d, size: %d, splitFileMaxSize_:%d",
            path_.c_str(), dataSize_, size, splitFileMaxSize_);

        // update old file header
        Finish();
        if (stream_.is_open()) {
            stream_.close();
        }
        fileNum_++;

        // write header of the new file
        if (!WriteHeader()) {
            return false;
        }

        // write the plugin config of the new file
        for (size_t i = 0; i < pluginConfigsData_.size(); i++) {
            Write(pluginConfigsData_[i].data(), pluginConfigsData_[i].size());
        }
        Flush();
        return true;
    }
    return false;
}

long TraceFileWriter::Write(const MessageLite& message)
{
    auto size = message.ByteSizeLong();
    if (isSplitFile_ && !isStop_) {
        if (IsSplitFile(size)) {
            return -1;
        }
    }

    // serialize message to bytes array
    std::vector<char> msgData(size);
    CHECK_TRUE(message.SerializeToArray(msgData.data(), msgData.size()), 0, "SerializeToArray failed!");

    return Write(msgData.data(), msgData.size());
}

bool TraceFileWriter::Finish()
{
    // update header info
    helper_.Update(header_);

    // move write position to begin of file
    CHECK_TRUE(stream_.is_open(), false, "binary file %s not open or open failed!", path_.c_str());
    stream_.seekp(0);
    CHECK_TRUE(stream_, false, "seek write position to head for %s failed!", path_.c_str());

    SetTimeStamp(); // add timestamp in header

    // write final header
    stream_.write(reinterpret_cast<CharPtr>(&header_), sizeof(header_));
    CHECK_TRUE(stream_, false, "write final header to %s failed!", path_.c_str());
    CHECK_TRUE(stream_.flush(), false, "binary file %s flush failed!", path_.c_str());
    return true;
}

bool TraceFileWriter::Flush()
{
    return FlushStream();
}

bool TraceFileWriter::FlushStream()
{
    CHECK_TRUE(stream_.is_open(), false, "binary file %s not open or open failed!", path_.c_str());
    CHECK_TRUE(stream_.flush(), false, "binary file %s flush failed!", path_.c_str());
    HILOG_INFO(LOG_CORE, "flush: %s, bytes: %" PRIu64 ", count: %" PRIu64, path_.c_str(), writeBytes_, writeCount_);
    return true;
}

void TraceFileWriter::SetStopSplitFile(bool isStop)
{
    isStop_ = isStop;
}

void TraceFileWriter::LogDiskUsage()
{
    std::string diskPath = "/data/local/tmp/";
    std::string::size_type pos = oldPath_.find_last_of('/');
    if (pos != std::string::npos) {
        diskPath = oldPath_.substr(0, pos);
    }

    struct statvfs diskInfo;
    int ret = statvfs(diskPath.c_str(), &diskInfo);
    if (ret != 0) {
        std::string errorMsg = COMMON::GetErrorMsg();
        HILOG_ERROR(LOG_CORE, "LogDiskUsage() return %d, path:%s, msg:%s", ret, diskPath.c_str(), errorMsg.c_str());
        return;
    }

    unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
        static_cast<unsigned long long>(diskInfo.f_bfree);
    unsigned long long totalSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
        static_cast<unsigned long long>(diskInfo.f_blocks);
    float freePercent = static_cast<float>(freeSize) / static_cast<float>(totalSize);
    uint32_t freeSizeGb = freeSize / GB_TO_BYTE;
    // 100: in terms of percentage
    HILOG_INFO(LOG_CORE, "LogDiskUsage() freePercent:%.1f, freeSizeGb:%u", freePercent * 100, freeSizeGb);
}