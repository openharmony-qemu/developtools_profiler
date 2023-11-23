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
    RET_NULL_ADDR = -2,
    RET_FAIL = -1,
    RET_SUCC = 0,
};

class GpuDataPlugin {
public:
    GpuDataPlugin() = default;
    ~GpuDataPlugin() = default;
    int Start(const uint8_t* configData, uint32_t configSize);
    int Report(uint8_t* data, uint32_t dataSize);
    int ReportOptimize(RandomWriteCtx* randomWrite);
    int Stop();

private:
    int ReadFile();
    template <typename T> void WriteGpuDataInfo(T& gpuData);

private:
    GpuConfig protoConfig_;
    int pid_ = -1;
    std::ifstream file_;
};

#endif
