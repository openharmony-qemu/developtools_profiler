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
#include <cstdio>

#include "common.h"
#ifdef LITE_PROTO
#include "common_types_lite.pb.h"
#else
#include "common_types.pb.h"
#endif
#include "logging.h"

using CharPtr = std::unique_ptr<char>::pointer;
using ConstCharPtr = std::unique_ptr<const char>::pointer;

namespace {
constexpr int MB_TO_BYTE = (1024 * 1024);
constexpr int GB_TO_BYTE = (1024 * 1024 * 1024);
constexpr int SPLIT_FILE_MIN_SIZE = 200;    // split file min size
constexpr int SPLIT_FILE_DEFAULT_NUM = 10;  // split file default num
} // namespace

TraceFileWriter::TraceFileWriter(const std::string& path) : TraceFileWriter(path, false, 0, 0) {}

TraceFileWriter::TraceFileWriter(int32_t fd) : fd_(fd)
{
    if (write(fd_, &header_, sizeof(header_)) != sizeof(header_)) {
        PROFILER_LOG_ERROR(LOG_CORE, "write initial header failed!, error: %s", strerror(errno));
    }
    isWriteFd_ = true;
    (void)FlushStream();
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

#ifdef LITE_PROTO
void TraceFileWriter::WriteStandalonePluginData(
    const std::string &pluginName, const std::string &data,
    const std::string &pluginVersion)
{
    LITE::ProfilerPluginData pluginData;
    pluginData.set_name(pluginName);
    pluginData.set_data(data);
    if (!pluginVersion.empty()) {
        pluginData.set_version(pluginVersion);
        pluginData.set_status(0);

        struct timespec ts = { 0, 0 };
        clock_gettime(CLOCK_REALTIME, &ts);
        pluginData.set_tv_sec(ts.tv_sec);
        pluginData.set_tv_nsec(ts.tv_nsec);
        pluginData.set_clock_id(LITE::ProfilerPluginData::CLOCKID_REALTIME);
    }

    std::vector<char> msgData(pluginData.ByteSizeLong());
    if (pluginData.SerializeToArray(msgData.data(), msgData.size()) <= 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s StandalonePluginData SerializeToArray failed!", pluginName.c_str());
    }

    Write(msgData.data(), msgData.size());
}
#else
void TraceFileWriter::WriteStandalonePluginData(
    const std::string &pluginName, const std::string &data,
    const std::string &pluginVersion)
{
    ProfilerPluginData pluginData;
    pluginData.set_name(pluginName);
    pluginData.set_data(data);
    if (!pluginVersion.empty()) {
        pluginData.set_version(pluginVersion);
        pluginData.set_status(0);

        struct timespec ts = { 0, 0 };
        clock_gettime(CLOCK_REALTIME, &ts);
        pluginData.set_tv_sec(ts.tv_sec);
        pluginData.set_tv_nsec(ts.tv_nsec);
        pluginData.set_clock_id(ProfilerPluginData::CLOCKID_REALTIME);
    }

    std::vector<char> msgData(pluginData.ByteSizeLong());
    if (pluginData.SerializeToArray(msgData.data(), msgData.size()) <= 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s StandalonePluginData SerializeToArray failed!", pluginName.c_str());
    }

    Write(msgData.data(), msgData.size());
}
#endif
void TraceFileWriter::SetTimeStamp()
{
    header_.data_.boottime = headerDataTime_.boottime;
    header_.data_.realtime = headerDataTime_.realtime;
    header_.data_.realtimeCoarse = headerDataTime_.realtimeCoarse;
    header_.data_.monotonic = headerDataTime_.monotonic;
    header_.data_.monotonicCoarse = headerDataTime_.monotonicCoarse;
    header_.data_.monotonicRaw = headerDataTime_.monotonicRaw;
    header_.data_.durationNs = headerDataTime_.durationNs;
}

void TraceFileWriter::SetTimeSource()
{
    constexpr uint64_t nanoSeconds = 1000000000;
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    headerDataTime_.boottime = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_REALTIME, &ts);
    headerDataTime_.realtime = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    headerDataTime_.realtimeCoarse = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC, &ts);
    headerDataTime_.monotonic = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    headerDataTime_.monotonicCoarse = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    headerDataTime_.monotonicRaw = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec);
}

void TraceFileWriter::SetDurationTime()
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    constexpr uint64_t nanoSeconds = 1000000000;
    auto currBoottime = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds + static_cast<uint64_t>(ts.tv_nsec);
    headerDataTime_.durationNs = currBoottime - headerDataTime_.boottime;
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

    stream_ = std::ofstream(path_, std::ios_base::out | std::ios_base::binary);
    CHECK_TRUE(stream_.is_open(), false, "open %s failed, %d!", path_.c_str(), errno);

    // write initial header, makes file write position move forward
    helper_ = {};
    header_ = {};
    stream_.write(reinterpret_cast<CharPtr>(&header_), sizeof(header_));
    CHECK_TRUE(stream_, false, "write initial header to %s failed!", path_.c_str());
    dataSize_ = header_.HEADER_SIZE;
    PROFILER_LOG_INFO(LOG_CORE, "write file(%s) header end", path_.c_str());
    return true;
}

// delete first split file if split file num over max
void TraceFileWriter::DeleteOldSplitFile()
{
    if (splitFilePaths_.size() <= splitFileMaxNum_) {
        PROFILER_LOG_INFO(LOG_CORE, "splitFilePaths_ size %zu, no need to delete.", splitFilePaths_.size());
        return;
    }

    std::string splitFilePath = splitFilePaths_.front();
    int ret = unlink(splitFilePath.c_str());
    PROFILER_LOG_INFO(LOG_CORE, "DeleteOldSplitFile remove %s return %d. ", splitFilePath.c_str(), ret);
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
    if (isWriteFd_) {
        auto ret = write(fd_, reinterpret_cast<CharPtr>(&dataLen), sizeof(dataLen));
        CHECK_TRUE(ret == sizeof(dataLen), 0, "write raw buffer data failed!");
        ret = write(fd_, data, size);
        CHECK_TRUE(ret == static_cast<long>(size), 0, "write raw buffer data failed!, dataSize: %zu, errno: %s",
                   size, strerror(errno));
    } else {
        CHECK_TRUE(stream_.is_open(), 0, "binary file %s not open or open failed!", path_.c_str());

        // write 4B data length.
        stream_.write(reinterpret_cast<CharPtr>(&dataLen), sizeof(dataLen));
        CHECK_TRUE(stream_, 0, "binary file %s write raw buffer size failed!", path_.c_str());
        CHECK_TRUE(helper_.AddSegment(reinterpret_cast<uint8_t*>(&dataLen), sizeof(dataLen)),
            0, "Add payload for size %u FAILED!", dataLen);

        // write data bytes
        stream_.write(reinterpret_cast<ConstCharPtr>(data), size);
        CHECK_TRUE(stream_, 0, "binary file %s write raw buffer data failed!", path_.c_str());
    }

    CHECK_TRUE(helper_.AddSegment(reinterpret_cast<uint8_t*>(const_cast<void*>(data)), size),
        0, "Add payload for data bytes %zu FAILED!", size);

    long nbytes = sizeof(dataLen) + size;
    writeBytes_ += nbytes;
    ++writeCount_;
    return nbytes;
}

long TraceFileWriter::WriteStandalonePluginFile(const std::string &file,
                                                const std::string &name,
                                                const std::string &version,
                                                DataType type)
{
    CHECK_TRUE(stream_.is_open(), 0, "binary file %s not open or open failed!", path_.c_str());

    std::ifstream fsFile {}; // read data from file
    fsFile.open(file, std::ios_base::in | std::ios_base::binary);
    if (!fsFile.good()) {
        PROFILER_LOG_ERROR(LOG_CORE, "open file(%s) failed: %d", file.c_str(), fsFile.rdstate());
        return 0;
    }

    TraceFileHeader header {};
    fsFile.seekg(0, std::ios_base::end);
    uint64_t fileSize = static_cast<uint64_t>(fsFile.tellg());
    header.data_.length += fileSize;
    size_t size = name.size();
    if (size > 0) {
        if (size > PLUGIN_MODULE_NAME_MAX) {
            PROFILER_LOG_ERROR(LOG_CORE, "standalonePluginName(%s) size(%zu) is greater than %d!",
                name.c_str(), size, PLUGIN_MODULE_NAME_MAX);
        } else if (strncpy_s(header.data_.standalonePluginName, PLUGIN_MODULE_NAME_MAX, name.c_str(), size) != EOK) {
            PROFILER_LOG_ERROR(LOG_CORE, "strncpy_s standalonePluginName(%s) error!", name.c_str());
        }
    }
    size = version.size();
    if (size > 0) {
        if (size > PLUGIN_MODULE_VERSION_MAX) {
            PROFILER_LOG_ERROR(LOG_CORE, "pluginVersion(%s) size(%zu) is greater than %d!",
                version.c_str(), size, PLUGIN_MODULE_VERSION_MAX);
        } else if (strncpy_s(header.data_.pluginVersion, PLUGIN_MODULE_VERSION_MAX, version.c_str(), size) != EOK) {
            PROFILER_LOG_ERROR(LOG_CORE, "strncpy_s pluginVersion(%s) error!", version.c_str());
        }
    }
    header.data_.dataType = type;
    stream_.write(reinterpret_cast<char*>(&header), sizeof(header));

    constexpr uint64_t readBufSize = 4 * 1024 * 1024;
    std::vector<char> readBuf(readBufSize);
    uint64_t readSize = 0;
    fsFile.seekg(0);
    while ((readSize = std::min(readBufSize, fileSize)) > 0) {
        fsFile.read(readBuf.data(), readSize);
        stream_.write(readBuf.data(), readSize);

        fileSize -= readSize;
        writeBytes_ += readSize;
        ++writeCount_;
    }

    fsFile.close();
    return fileSize;
}

bool TraceFileWriter::IsSplitFile(uint32_t size)
{
    dataSize_ += sizeof(uint32_t) + size;
    if (dataSize_ >= splitFileMaxSize_) {
        PROFILER_LOG_INFO(LOG_CORE, "need to split the file(%s), data size:%d, size: %d, splitFileMaxSize_:%d",
            path_.c_str(), dataSize_, size, splitFileMaxSize_);

        // update old file header
        SetDurationTime();
        Finish();
        if (stream_.is_open()) {
            stream_.close();
        }
        fileNum_++;

        // write header of the new file
        if (!WriteHeader()) {
            return false;
        }
        SetTimeSource();

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
    SetTimeStamp(); // add timestamp in header

    if (isWriteFd_) {
        if (lseek(fd_, 0, SEEK_SET) == -1) {
            return false;
        }
        auto ret = write(fd_, &header_, sizeof(header_));
        CHECK_TRUE(ret == sizeof(header_), false, "write initial header failed!, error: %s", strerror(errno));
    } else {
        long long filePos = stream_.tellp();
        if (filePos == -1) { // -1 :file not open or error
            return false;
        }
        // move write position to begin of file
        CHECK_TRUE(stream_.is_open(), false, "binary file %s not open or open failed!", path_.c_str());
        stream_.seekp(0);
        CHECK_TRUE(stream_, false, "seek write position to head for %s failed!", path_.c_str());

        // write final header
        stream_.write(reinterpret_cast<CharPtr>(&header_), sizeof(header_));
        stream_.seekp(filePos);
        CHECK_TRUE(stream_, false, "write final header to %s failed!", path_.c_str());
        CHECK_TRUE(stream_.flush(), false, "binary file %s flush failed!", path_.c_str());
        PROFILER_LOG_DEBUG(LOG_CORE, "Finish: %s, bytes: %" PRIu64 ", count: %" PRIu64, path_.c_str(), writeBytes_,
                   writeCount_);
    }
    return true;
}

bool TraceFileWriter::Flush()
{
    return FlushStream();
}

bool TraceFileWriter::FlushStream()
{
    if (!isWriteFd_) {
        CHECK_TRUE(stream_.is_open(), false, "binary file %s not open or open failed!", path_.c_str());
        CHECK_TRUE(stream_.flush(), false, "binary file %s flush failed!", path_.c_str());
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "flush: %s, bytes: %" PRIu64 ", count: %" PRIu64, path_.c_str(),
                       writeBytes_, writeCount_);
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
        PROFILER_LOG_ERROR(LOG_CORE, "LogDiskUsage() return %d, path:%s, msg:%s",
                           ret, diskPath.c_str(), errorMsg.c_str());
        return;
    }

    unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
        static_cast<unsigned long long>(diskInfo.f_bfree);
    unsigned long long totalSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
        static_cast<unsigned long long>(diskInfo.f_blocks);
    float freePercent = static_cast<float>(freeSize) / static_cast<float>(totalSize);
    uint32_t freeSizeGb = freeSize / GB_TO_BYTE;
    // 100: in terms of percentage
    PROFILER_LOG_INFO(LOG_CORE, "LogDiskUsage() freePercent:%.1f, freeSizeGb:%u", freePercent * 100, freeSizeGb);
}