/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include "diskio_data_plugin.h"

#include <ctime>

#include "buffer_splitter.h"

namespace {
constexpr size_t READ_BUFFER_SIZE = 1024 * 16;
} // namespace

DiskioDataPlugin::DiskioDataPlugin()
{
    ioEntry_ = nullptr;
    buffer_ = nullptr;
    path_ = "/proc/vmstat";
    err_ = -1;
    prevRdSectorsKb_ = 0;
    prevWrSectorsKb_ = 0;
    prevTimestamp_.set_tv_sec(0);
    prevTimestamp_.set_tv_nsec(0);
}

DiskioDataPlugin::~DiskioDataPlugin()
{
    HILOG_INFO(LOG_CORE, "%s:~DiskioDataPlugin!", __func__);
    if (buffer_ != nullptr) {
        free(buffer_);
        buffer_ = nullptr;
    }
}

int DiskioDataPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    buffer_ = malloc(READ_BUFFER_SIZE);
    CHECK_NOTNULL(buffer_, RET_FAIL, "%s:malloc buffer_ failed!", __func__);

    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, RET_FAIL,
               "%s:parseFromArray failed!", __func__);

    if (protoConfig_.report_io_stats()) {
        ioEntry_ = std::make_shared<IoStats>(protoConfig_.report_io_stats());
    }
    HILOG_INFO(LOG_CORE, "%s:start success!", __func__);
    return RET_SUCC;
}

int DiskioDataPlugin::Report(uint8_t* data, uint32_t dataSize)
{
    DiskioData dataProto;
    uint32_t length;

    WriteDiskioData(dataProto);

    if (protoConfig_.report_io_stats() && ioEntry_ != nullptr) {
        ioEntry_->GetIoData();
        ioEntry_->PutPluginStatsData(dataProto.mutable_statsdata());
    }

    length = dataProto.ByteSizeLong();
    if (length > dataSize) {
        return -length;
    }
    if (dataProto.SerializeToArray(data, length) > 0) {
        return length;
    }
    return 0;
}

int DiskioDataPlugin::Stop()
{
    if (buffer_ != nullptr) {
        free(buffer_);
        buffer_ = nullptr;
    }
    if (ioEntry_ != nullptr) {
        ioEntry_.reset();
        ioEntry_ = nullptr;
    }
    HILOG_INFO(LOG_CORE, "%s:plugin:stop success!", __func__);
    return 0;
}

int32_t DiskioDataPlugin::ReadFile(std::string& fileName)
{
    int fd = -1;
    ssize_t bytesRead = 0;
    char realPath[PATH_MAX + 1] = {0};
    CHECK_TRUE((fileName.length() < PATH_MAX) && (realpath(fileName.c_str(), realPath) != nullptr), RET_FAIL,
               "%s:path is invalid: %s, errno=%d", __func__, fileName.c_str(), errno);
    fd = open(realPath, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        HILOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno=%d", __func__, fileName.c_str(), errno);
        err_ = errno;
        return RET_FAIL;
    }
    if (buffer_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "%s:empty address, buffer_ is NULL", __func__);
        err_ = RET_NULL_ADDR;
        close(fd);
        return RET_FAIL;
    }
    bytesRead = read(fd, buffer_, READ_BUFFER_SIZE - 1);
    if (bytesRead <= 0) {
        close(fd);
        HILOG_ERROR(LOG_CORE, "%s:failed to read(%s), errno=%d", __func__, fileName.c_str(), errno);
        err_ = errno;
        return RET_FAIL;
    }
    close(fd);

    return bytesRead;
}

void DiskioDataPlugin::SetTimestamp(CollectTimeStamp& prevTimestamp, CollectTimeStamp& timestamp)
{
    timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    timestamp.set_tv_sec(time.tv_sec);
    timestamp.set_tv_nsec(time.tv_nsec);
    prevTimestamp.set_tv_sec(prevTimestamp_.tv_sec());
    prevTimestamp.set_tv_nsec(prevTimestamp_.tv_nsec());
    prevTimestamp_.set_tv_sec(time.tv_sec);
    prevTimestamp_.set_tv_nsec(time.tv_nsec);
}

void DiskioDataPlugin::SetDiskioData(DiskioData& diskioData, const char* pFile, uint32_t fileLen)
{
    BufferSplitter totalbuffer(const_cast<char*>(pFile), fileLen + 1);
    int64_t rd_sectors_kb = 0;
    int64_t wr_sectors_kb = 0;

    do {
        totalbuffer.NextWord(' ');
        std::string curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
        if (strcmp(curWord.c_str(), "pgpgin") == 0) {
            if (!totalbuffer.NextWord('\n')) {
                HILOG_ERROR(LOG_CORE, "%s:failed to get pgpgin, CurWord() = %s", __func__, totalbuffer.CurWord());
                break;
            }
            curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
            rd_sectors_kb = atoi(curWord.c_str());
        } else if (strcmp(curWord.c_str(), "pgpgout") == 0) {
            if (!totalbuffer.NextWord('\n')) {
                HILOG_ERROR(LOG_CORE, "%s:failed to get pgpgout, CurWord() = %s", __func__, totalbuffer.CurWord());
                break;
            }
            curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
            wr_sectors_kb = atoi(curWord.c_str());
        }
    } while (totalbuffer.NextLine());

    // 前一次系统从磁盘调入的总KB数rd1，通过时间间隔t内KB增量计算磁盘读取速率(rd2-rd1)/t
    diskioData.set_prev_rd_sectors_kb(prevRdSectorsKb_);
    // 前一次系统调出到磁盘的总KB数wr1，通过时间间隔t内KB增量计算磁盘写入速率(wr2-wr1)/t
    diskioData.set_prev_wr_sectors_kb(prevWrSectorsKb_);
    diskioData.set_rd_sectors_kb(rd_sectors_kb); // 当前系统从磁盘调入的总KB数rd2
    diskioData.set_wr_sectors_kb(wr_sectors_kb); // 当前系统调出到磁盘的总KB数wr2
    prevRdSectorsKb_ = rd_sectors_kb;
    prevWrSectorsKb_ = wr_sectors_kb;
    auto* prevTimestamp = diskioData.mutable_prev_timestamp();
    auto* timestamp = diskioData.mutable_timestamp();
    SetTimestamp(*prevTimestamp, *timestamp); // 设置前一次时间戳和当前时间戳，以便计算两次获取数据的时间间隔t
}

void DiskioDataPlugin::WriteDiskioData(DiskioData& data)
{
    int32_t ret = ReadFile(path_);
    if (ret == RET_FAIL) {
        return;
    }
    if ((buffer_ == nullptr) || (ret == 0)) {
        return;
    }

    SetDiskioData(data, (char*)buffer_, ret);
}