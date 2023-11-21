/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#include "gpu_data_plugin.h"
#include <ctime>
#include <vector>
#include "gpu_plugin_result.pbencoder.h"
#include "buffer_splitter.h"

namespace {
using namespace OHOS::Developtools::Profiler;
constexpr size_t READ_BUFFER_SIZE = 1024 * 16;
} // namespace

int GpuDataPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    buffer_ = malloc(READ_BUFFER_SIZE);
    CHECK_NOTNULL(buffer_, RET_FAIL, "%s:malloc buffer_ failed!", __func__);

    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, RET_FAIL,
               "%s:parseFromArray failed!", __func__);

    if (protoConfig_.pid() > 0) {
        pid_ = protoConfig_.pid();
    }
    HILOG_INFO(LOG_CORE, "%s:start success!", __func__);
    return RET_SUCC;
}

int GpuDataPlugin::ReportOptimize(RandomWriteCtx* randomWrite)
{
    ProtoEncoder::GpuData dataProto(randomWrite);
    WriteGpuDataInfo(dataProto);
    int msgSize = dataProto.Finish();
    return msgSize;
}

int GpuDataPlugin::Report(uint8_t* data, uint32_t dataSize)
{
    GpuData dataProto;
    uint32_t length;
    WriteGpuDataInfo(dataProto);

    length = dataProto.ByteSizeLong();
    if (length > dataSize) {
        return -length;
    }
    if (dataProto.SerializeToArray(data, length) > 0) {
        return length;
    }
    return 0;
}

int GpuDataPlugin::Stop()
{
    if (buffer_ != nullptr) {
        free(buffer_);
        buffer_ = nullptr;
    }
    HILOG_INFO(LOG_CORE, "%s:stop success!", __func__);
    return 0;
}

int32_t GpuDataPlugin::ReadFile(std::string& fileName)
{
    int fd = -1;
    ssize_t bytesRead = 0;
    char filePath[PATH_MAX + 1] = {0};
    char realPath[PATH_MAX + 1] = {0};

    if (snprintf_s(filePath, sizeof(filePath), sizeof(filePath) - 1, "%s", fileName.c_str()) < 0) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "snprintf_s(%s) error, errno(%d:%s)", fileName.c_str(), errno, buf);
        return RET_FAIL;
    }
    if (realpath(filePath, realPath) == nullptr) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "realpath(%s) failed, errno(%d:%s)", fileName.c_str(), errno, buf);
        return RET_FAIL;
    }

    fd = open(realPath, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, realPath, errno, buf);
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
        HILOG_ERROR(LOG_CORE, "%s:failed to read(%s), errno=%d", __func__, realPath, errno);
        err_ = errno;
        return RET_FAIL;
    }
    close(fd);

    return bytesRead;
}


template <typename T> void GpuDataPlugin::WriteGpuDataInfo(T& gpuData)
{
    // write system info
    int32_t ret = ReadFile(path_);
    if (ret == RET_FAIL) {
        return;
    }
    if ((buffer_ == nullptr) || (ret == 0)) {
        return;
    }

    constexpr uint64_t nanoSeconds = 1000000000;
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    uint64_t boottime = (static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec)) / 1000000;
    gpuData.set_boottime(boottime);

    const char* pFile = reinterpret_cast<char*>(buffer_);
    BufferSplitter totalbuffer(const_cast<char*>(pFile), ret + 1);
    std::string curWord = std::string(totalbuffer.CurLine(), totalbuffer.CurLineSize());
    int utilisation = atoi(curWord.c_str());
    gpuData.set_gpu_utilisation(utilisation);
}
