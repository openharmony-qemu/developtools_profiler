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

#ifndef GPU_DATA_PLUGIN_H
#define GPU_DATA_PLUGIN_H

#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include "gpu_plugin_config.pb.h"
#include "gpu_plugin_result.pb.h"
#include "logging.h"
#include "plugin_module_api.h"

enum ErrorType {
    RET_NULL_ADDR,
    RET_IVALID_PID,
    RET_TGID_VALUE_NULL,
    RET_FAIL = -1,
    RET_SUCC = 0,
};



class GpuDataPlugin {
public:
    GpuDataPlugin();
    ~GpuDataPlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Report(uint8_t* data, uint32_t dataSize);
    int ReportOptimize(RandomWriteCtx* randomWrite);
    int Stop();
    // for UT
    void SetPath(std::string path)
    {
        path_ = path;
    }

private:
    int32_t ReadFile(std::string& fileName);

    template <typename T> void WriteGpuDataInfo(T& gpuData);


private:
    /* data */
    GpuConfig protoConfig_;
    void* buffer_ = nullptr;
    std::string path_ = "/sys/class/devfreq/gpufreq/gpu_scene_aware/utilisation";
    int32_t err_ = -1;

    int pid_ = -1;
};

#endif
