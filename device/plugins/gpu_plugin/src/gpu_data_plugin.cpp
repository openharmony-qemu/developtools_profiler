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
#include "common.h"
#include "gpu_plugin_result.pbencoder.h"

namespace {
using namespace OHOS::Developtools::Profiler;
const std::string GPU_PATH = "/sys/class/devfreq/gpufreq/gpu_scene_aware/utilisation";
} // namespace

int GpuDataPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, RET_FAIL,
               "%s:parseFromArray failed!", __func__);

    if (protoConfig_.pid() > 0) {
        pid_ = protoConfig_.pid();
    }

    file_.open(GPU_PATH);
    auto args = GetCmdArgs(protoConfig_);
    if (!file_.is_open()) {
        int ret = COMMON::PluginWriteToHisysevent("gpu_plugin", "sh", args, RET_FAIL, "failed");
        PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to open(%s), hisysevent report gpu_plugin ret: %d",
            __func__, GPU_PATH.c_str(), ret);
        return RET_FAIL;
    }

    int ret = COMMON::PluginWriteToHisysevent("gpu_plugin", "sh", args, RET_SUCC, "success");
    PROFILER_LOG_INFO(LOG_CORE, "%s:start success! hisysevent report gpu_plugin result: %d", __func__, ret);
    return RET_SUCC;
}

std::string GpuDataPlugin::GetCmdArgs(GpuConfig traceConfig)
{
    std::string args;
    args += "pid: " + std::to_string(traceConfig.pid()) + ", report_gpu_info: ";
    args += (traceConfig.report_gpu_info() ? "true" : "false");
    return args;
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
    file_.close();
    PROFILER_LOG_INFO(LOG_CORE, "%s:stop success!", __func__);
    return 0;
}

int GpuDataPlugin::ReadFile()
{
    file_.clear();
    file_.seekg(0);
    std::string line;
    std::getline(file_, line);
    if (line == "") {
        return RET_FAIL;
    }
    for (char charac : line) {
        if (!isdigit(charac)) {
            PROFILER_LOG_ERROR(LOG_CORE, "invalid file content for (%s)", GPU_PATH.c_str());
            return RET_FAIL;
        }
    }
    return stoi(line);
}

template <typename T> void GpuDataPlugin::WriteGpuDataInfo(T& gpuData)
{
    int ret = ReadFile();
    if (ret == RET_FAIL) {
        return;
    }

    constexpr uint64_t nanoSeconds = 1000000000;
    struct timespec ts;
    int result = clock_gettime(CLOCK_BOOTTIME, &ts);
    if (result == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "clock_gettime failed");
        return;
    }
    uint64_t boottime = (static_cast<uint64_t>(ts.tv_sec) * nanoSeconds +
        static_cast<uint64_t>(ts.tv_nsec)) / 1000000;
    gpuData.set_boottime(boottime);
    gpuData.set_gpu_utilisation(static_cast<uint64_t>(ret));
}
