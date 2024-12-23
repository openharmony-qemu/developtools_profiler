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
#include <numeric>
#include "common.h"
#include "gpu_plugin_result.pbencoder.h"

namespace {
using namespace OHOS::Developtools::Profiler;
const std::string GPU_PATH = "/sys/class/devfreq/gpufreq/gpu_scene_aware/utilisation";
const int RET_COUNT = 50;
const uint32_t SAMPLE_INTERVAL = 10;
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
    CHECK_NOTNULL(resultWriter_, -1, "GPUDataPlugin: Writer is no set!");
    CHECK_NOTNULL(resultWriter_->write, -1, "GPUDataPlugin: Writer.write is no set!");
    CHECK_NOTNULL(resultWriter_->flush, -1, "GPUDataPlugin: Writer.flush is no set!");
    running_ = true;
    writeThread_ = std::thread([this] { this->ReadGpuData(); });

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

void GpuDataPlugin::ReadGpuData()
{
    PROFILER_LOG_INFO(LOG_CORE, "GPUDataPlugin: Read GPU data start");
    CHECK_NOTNULL(resultWriter_, NO_RETVAL, "%s: resultWriter_ is nullptr", __func__);
    typedef std::chrono::system_clock ClockType;
    typedef std::chrono::time_point<ClockType> TimePointType;
    TimePointType currTime = ClockType::now();
    TimePointType awaitTimePoint = currTime;
    std::condition_variable waitCv;
    std::mutex waitMut;
    std::vector<std::pair<uint64_t, int>> vectGpuData = {};
    uint32_t nCount = 0;
    while (running_) {
        currTime = ClockType::now();
        std::unique_lock<std::mutex> lock(waitMut);
        if (currTime <= awaitTimePoint) {
            if (waitCv.wait_until(lock, awaitTimePoint, [this]() { return !running_; })) {
                if (!running_) {
                    break;
                }
            }
        }
        if (!running_) {
            break;
        }
        int retGpu = ReadFile();
        if (retGpu != RET_FAIL) {
            uint64_t bootTime = GetBootTime();
            vectGpuData.push_back({bootTime, retGpu});
            nCount++;
        }
        if (nCount == RET_COUNT) {
            FlushGpuData(vectGpuData);
            nCount = 0;
            vectGpuData.clear();
        }
        awaitTimePoint = awaitTimePoint + std::chrono::milliseconds(SAMPLE_INTERVAL);
    }
    if (vectGpuData.size() > 0) {
        FlushGpuData(vectGpuData);
    }
    PROFILER_LOG_INFO(LOG_CORE, "GPUDataPlugin: Read data end");
}

void GpuDataPlugin::FlushGpuData(std::vector<std::pair<uint64_t, int>>& vectGpuData)
{
    if (resultWriter_->isProtobufSerialize) {
        GpuData dataProto;
        WriteGpuDataInfoExt(dataProto, vectGpuData);
        if (dataProto.ByteSizeLong() > 0) {
            buffer_.resize(dataProto.ByteSizeLong());
            dataProto.SerializeToArray(buffer_.data(), buffer_.size());
            resultWriter_->write(resultWriter_, buffer_.data(), buffer_.size());
            resultWriter_->flush(resultWriter_);
        }
    } else {
        ProtoEncoder::GpuData dataProto(resultWriter_->startReport(resultWriter_));
        WriteGpuDataInfoExt(dataProto, vectGpuData);
        int messageLen = dataProto.Finish();
        resultWriter_->finishReport(resultWriter_, messageLen);
        resultWriter_->flush(resultWriter_);
    }
}

int GpuDataPlugin::Stop()
{
    running_ = false;
    if (writeThread_.joinable()) {
        writeThread_.join();
    }
    PROFILER_LOG_INFO(LOG_CORE, "GPUDataPlugin:stop thread success!");
    file_.close();
    PROFILER_LOG_INFO(LOG_CORE, "GPUDataPlugin: stop success!");
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

uint64_t GpuDataPlugin::GetBootTime()
{
    constexpr uint64_t nanoSeconds = 1000000000;
    struct timespec ts;
    int result = clock_gettime(CLOCK_BOOTTIME, &ts);
    if (result == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "clock_gettime failed");
        return 0;
    }
    uint64_t bootTime = (static_cast<uint64_t>(ts.tv_sec) * nanoSeconds + static_cast<uint64_t>(ts.tv_nsec)) / 1000000;
    return bootTime;
}

template <typename T>
void GpuDataPlugin::WriteGpuDataInfoExt(T& gpuData, std::vector<std::pair<uint64_t, int>>& vectGpuData)
{
    gpuData.set_boottime(0);
    gpuData.set_gpu_utilisation(0);
    for (auto& gpuDataInfo : vectGpuData) {
        auto* gpuDataExt = gpuData.add_gpu_data_array();
        gpuDataExt->set_boottime(gpuDataInfo.first);
        gpuDataExt->set_gpu_utilisation(gpuDataInfo.second);
    }
}

int GpuDataPlugin::SetWriter(WriterStruct* writer)
{
    resultWriter_ = writer;
    return 0;
}
