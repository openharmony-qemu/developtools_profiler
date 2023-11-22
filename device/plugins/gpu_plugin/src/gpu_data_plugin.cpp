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
const std::string GPU_PATH = "/sys/class/devfreq/gpufreq/gpu_scene_aware/utilisation";
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

    file.open(GPU_PATH);
    if (!file.is_open()) {
        HILOG_ERROR(LOG_CORE, "%s:failed to open(%s)", __func__, GPU_PATH.c_str());
        return RET_FAIL;
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
    file.close();
    HILOG_INFO(LOG_CORE, "%s:stop success!", __func__);
    return 0;
}

uint64_t GpuDataPlugin::ReadFile()
{
    file.clear();
    file.seekg(0);
    std::string line;
    std::getline(file, line);
    for (char charac : line) {
        if (!isdigit(charac)) {
            HILOG_ERROR(LOG_CORE, "invalid file content for (%s)", GPU_PATH.c_str());
            return -1;
        }
    }
    return stoi(line);
}


template <typename T> void GpuDataPlugin::WriteGpuDataInfo(T& gpuData)
{
    // write system info
    uint64_t ret = ReadFile();
    if (ret == (uint64_t)-1) {
        return;
    }

    constexpr uint64_t nanoSeconds = 1000000000;
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    uint64_t boottime = (static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec)) / 1000000;
    gpuData.set_boottime(boottime);
    gpuData.set_gpu_utilisation(ret);
}
