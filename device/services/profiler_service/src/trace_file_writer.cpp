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
#include <fcntl.h>
#include <memory>
#include <sys/mman.h>
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
constexpr size_t DEFULT_PAGES = 32 * 256; // 32M
constexpr int PIECE_HEAD_LEN = 4;
} // namespace

TraceFileWriter::TraceFileWriter(const std::string& path) : TraceFileWriter(path, false, 0, 0) {}

TraceFileWriter::TraceFileWriter(int32_t fd) : fd_(fd)
{
    writeCtx_.write = this;
    writeCtx_.ctx.getMemory = [](RandomWriteCtx* ctx, uint32_t size, uint8_t** memory, uint32_t* offset) -> bool {
        TraceFileWriterCtx* writeCtx = reinterpret_cast<TraceFileWriterCtx*>(ctx);
        return writeCtx->write->GetMemory(size, memory, offset);
    };
    writeCtx_.ctx.seek = [](RandomWriteCtx* ctx, uint32_t offset) -> bool {
        TraceFileWriterCtx* writeCtx = reinterpret_cast<TraceFileWriterCtx*>(ctx);
        return writeCtx->write->Seek(offset);
    };

    CHECK_TRUE(fd_ != 0, NO_RETVAL, "only-nmd mode, no need to use TraceFileWriter");
    if (write(fd_, &header_, sizeof(header_)) != sizeof(header_)) {
        PROFILER_LOG_ERROR(LOG_CORE, "write initial header failed!, error: %s", strerror(errno));
        return;
    }
    (void)FlushStream();
    fileWriteLength_ = sizeof(header_);
    mapOffset_ = sizeof(header_);
    messageWriteOffset_ = PIECE_HEAD_LEN;
    pageSize_ = static_cast<size_t>(sysconf(_SC_PAGE_SIZE));
    fileLength_ = DEFULT_PAGES * pageSize_;

    if (fallocate(fd_, 0, 0, fileLength_) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "fallocate file(%zu) failed, error: %s", fileLength_, strerror(errno));
        return;
    }

    fileMapAddr_ = mmap(nullptr, fileLength_, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd_, 0);
    if (fileMapAddr_ == MAP_FAILED) {
        PROFILER_LOG_ERROR(LOG_CORE, "mmap file(%d) failed, error: %s", fd_, strerror(errno));
        return;
    }
    mmapFileLength_ = fileLength_;
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
    CHECK_TRUE(fd_ != 0, NO_RETVAL, "only-nmd mode, no need to use TraceFileWriter");
    (void)FlushStream();
    if (stream_.is_open()) {
        stream_.close();
    }
    if (fileMapAddr_ != MAP_FAILED) {
        munmap(fileMapAddr_, mmapFileLength_);
    }
}

std::string TraceFileWriter::Path() const
{
    return path_;
}

bool TraceFileWriter::SetPluginConfig(const void* data, size_t size)
{
    CHECK_TRUE(fd_ != 0, false, "SetPluginConfig, nmd mode no need to use TraceFileWriter");
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
    CHECK_TRUE(fd_ != 0, NO_RETVAL, "WriteStandalonePluginData, nmd mode no need to use TraceFileWriter");
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
#endif

void TraceFileWriter::SetTimeStamp()
{
    CHECK_TRUE(fd_ != 0, NO_RETVAL, "SetTimeStamp, nmd mode no need to use TraceFileWriter");
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
    CHECK_TRUE(fd_ != 0, NO_RETVAL, "SetTimeSource, nmd mode no need to use TraceFileWriter");
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
    CHECK_TRUE(fd_ != 0, NO_RETVAL, "SetDurationTime, nmd mode no need to use TraceFileWriter");
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    constexpr uint64_t nanoSeconds = 1000000000;
    auto currBoottime = static_cast<uint64_t>(ts.tv_sec) * nanoSeconds + static_cast<uint64_t>(ts.tv_nsec);
    headerDataTime_.durationNs = currBoottime - headerDataTime_.boottime;
}

bool TraceFileWriter::WriteHeader()
{
    CHECK_TRUE(fd_ != 0, false, "WriteHeader, nmd-only mode no need to use TraceFileWriter");
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
    CHECK_TRUE(fd_ != 0, 0, "Write, nmd-only mode no need to use TraceFileWriter");
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

    uint64_t nbytes = sizeof(dataLen) + size;
    writeBytes_ += nbytes;
    ++writeCount_;
    return nbytes;
}

long TraceFileWriter::WriteStandalonePluginFile(const std::string &file,
                                                const std::string &name,
                                                const std::string &version,
                                                DataType type)
{
    CHECK_TRUE(fd_ != 0, 0, "WriteStandalonePluginFile, no need to use TraceFileWriter");
    CHECK_TRUE(stream_.is_open(), 0, "binary file %s not open or open failed!", path_.c_str());
    auto retFile = COMMON::CheckNotExistsFilePath(file);
    if (!retFile.first) {
        PROFILER_LOG_INFO(LOG_CORE, "%s:check file path %s fail", __func__, file.c_str());
        return 0;
    }
    std::ifstream fsFile {}; // read data from file
    fsFile.open(retFile.second, std::ios_base::in | std::ios_base::binary);
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
    CHECK_TRUE(fd_ != 0, false, "IsSplitFile, nmd-only mode no need to use TraceFileWriter");
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
    CHECK_TRUE(fd_ != 0, 0, "Write, nmd-only mode no need to use TraceFileWriter");
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
    CHECK_TRUE(fd_ != 0, false, "Finish(), nmd-only mode no need to use TraceFileWriter");
    // update header info
    helper_.Update(header_);
    SetTimeStamp(); // add timestamp in header

    if (fd_ != -1) {
        if (lseek(fd_, 0, SEEK_SET) == -1) {
            return false;
        }
        auto ret = write(fd_, &header_, sizeof(header_));
        CHECK_TRUE(ret == sizeof(header_), false, "write initial header failed!, error: %s", strerror(errno));
        CHECK_TRUE(ftruncate(fd_, fileWriteLength_) == 0, false, "ftruncate(%u) failed, error: %s",
            fileWriteLength_, strerror(errno));
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

bool TraceFileWriter::UpdateSaFileHeader()
{
    CHECK_TRUE(fd_ != 0, false, "Finish(), nmd-only mode no need to use TraceFileWriter");
    SetDurationTime();
    // update header info
    helper_.Update(header_);
    SetTimeStamp(); // add timestamp in header
    // get position of file pointer
    int offsetCur = lseek(fd_, 0, SEEK_CUR);
    if (offsetCur == -1) {
        return false;
    }
    if (lseek(fd_, 0, SEEK_SET) == -1) {
        return false;
    }
    auto ret = write(fd_, &header_, sizeof(header_));
    CHECK_TRUE(ret == sizeof(header_), false, "write initial header failed!, error: %s", strerror(errno));
    CHECK_TRUE(fileMapAddr_ != MAP_FAILED, false, "UpdateSaFileHeader() fileMapAddr not mapped!");
    // restore
    if (lseek(fd_, offsetCur, SEEK_SET) == -1) {
        return false;
    }
    if ((msync(fileMapAddr_, mmapFileLength_, MS_SYNC)) == -1) {
        PROFILER_LOG_INFO(LOG_CORE, "msync failed, error: %s", strerror(errno));
    }
    return true;
}

bool TraceFileWriter::Flush()
{
    CHECK_TRUE(fd_ != 0, false, "Finish(), nmd-only mode no need to use TraceFileWriter");
    return FlushStream();
}

bool TraceFileWriter::FlushStream()
{
    if (fd_ == -1) {
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
    float freePercent = 0;
    if (totalSize != 0) {
        freePercent = static_cast<float>(freeSize) / static_cast<float>(totalSize);
    }
    uint32_t freeSizeGb = freeSize / GB_TO_BYTE;
    // 100: in terms of percentage
    PROFILER_LOG_INFO(LOG_CORE, "LogDiskUsage() freePercent:%.1f, freeSizeGb:%u", freePercent * 100, freeSizeGb);
}

bool TraceFileWriter::RemapFile()
{
    CHECK_TRUE(fileMapAddr_ != MAP_FAILED, false, "RemapFile() fileMapAddr not mapped!");
    if (munmap(fileMapAddr_, mmapFileLength_) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "munmap file(%zu) failed, error: %s", fileLength_, strerror(errno));
        return false;
    }

    // file size increased by DEFULT_PAGES * pageSize_(32M)
    size_t newLength = fileLength_ + DEFULT_PAGES * pageSize_;
    if (fallocate(fd_, 0, fileLength_, DEFULT_PAGES * pageSize_) != 0) {
        fileMapAddr_ = MAP_FAILED;
        PROFILER_LOG_ERROR(LOG_CORE, "fallocate file(%zu) failed, error: %s", newLength, strerror(errno));
        return false;
    }

    size_t remapPos = fileWriteLength_ & ~(pageSize_ - 1);
    if (newLength <= remapPos) {
        fileMapAddr_ = MAP_FAILED;
        PROFILER_LOG_ERROR(LOG_CORE, "RemapFile failed, newLength is less than remapPos");
        return false;
    }
    fileMapAddr_ = mmap(nullptr, newLength - remapPos, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_POPULATE,
                        fd_, remapPos);
    if (fileMapAddr_ == MAP_FAILED) {
        PROFILER_LOG_ERROR(LOG_CORE, "remap(%zu:%zu) data file failed, error: %s", newLength, remapPos,
                           strerror(errno));
        return false;
    }
    mmapFileLength_ = newLength - remapPos;
    mapOffset_ = fileWriteLength_ - remapPos;
    fileLength_ = newLength;
    PROFILER_LOG_INFO(LOG_CORE, "remap(%zu:%zu) data file(%zu) sucess", remapPos, mapOffset_, fileLength_);
    return true;
}

bool TraceFileWriter::GetMemory(uint32_t size, uint8_t** memory, uint32_t* offset)
{
    while ((fileWriteLength_ + PIECE_HEAD_LEN + messageWriteOffset_ + size) >= fileLength_) {
        if ((fileLength_ + DEFULT_PAGES * pageSize_) > GB_TO_BYTE) {
            return false;
        }
        
        if (!RemapFile()) {
            return false;
        }
    }
    if (fileMapAddr_ == MAP_FAILED) {
        return false;
    }
    *memory = &reinterpret_cast<uint8_t*>(fileMapAddr_)[mapOffset_ + messageWriteOffset_];
    *offset = messageWriteOffset_;
    return true;
}

bool TraceFileWriter::Seek(uint32_t offset)
{
    messageWriteOffset_ = offset;
    return true;
}

void TraceFileWriter::FinishReport(int32_t size)
{
    if (fileMapAddr_ == MAP_FAILED) {
        return;
    }
    if (size <= 0) {
        return;
    }
    auto realSize = PIECE_HEAD_LEN + size;
    CHECK_TRUE(helper_.AddSegment(nullptr, realSize), NO_RETVAL, "AddSegment(%d) failed, error: %s",
        realSize, strerror(errno));
    if (mapOffset_ + PIECE_HEAD_LEN > mmapFileLength_) {
        PROFILER_LOG_ERROR(LOG_CORE, "FinishReport (%zu:%zu) data file failed", mapOffset_, fileLength_);
        return;
    }
    // write data length
    *(reinterpret_cast<int*>((&reinterpret_cast<uint8_t*>(fileMapAddr_)[mapOffset_]))) = size;
    mapOffset_ += static_cast<uint64_t>(realSize);
    fileWriteLength_ += static_cast<uint64_t>(realSize);
    writeBytes_ += static_cast<uint64_t>(realSize);
    ++writeCount_;
}

void TraceFileWriter::ResetPos()
{
    messageWriteOffset_ = PIECE_HEAD_LEN;
}