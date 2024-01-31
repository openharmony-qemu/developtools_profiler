/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "include/GPU.h"
#include <iostream>
#include "include/sp_utils.h"
#include "gpu_collector.h"
#include "collect_result.h"
#include "include/sp_log.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> GPU::ItemData()
{
    std::map<std::string, std::string> result;
    int32_t freq = GetGpuFreq();
    float load = GetGpuLoad();
    result["gpuFrequency"] = std::to_string(freq);
    result["gpuLoad"] = std::to_string(load);
    return result;
}

int GPU::GetGpuFreq()
{
    std::shared_ptr<GpuCollector> collector = GpuCollector::Create();
    CollectResult<GpuFreq> result = collector->CollectGpuFrequency();
    LOGI("GpuFrequency: %s", std::to_string(result.data.curFeq).c_str());
    return result.data.curFeq;
}

float GPU::GetGpuLoad()
{
    std::shared_ptr<GpuCollector> collector = GpuCollector::Create();
    CollectResult<SysGpuLoad> result = collector->CollectSysGpuLoad();
    LOGI("SysGpuLoad: %s", std::to_string(result.data.gpuLoad).c_str());
    return float(result.data.gpuLoad);
}
}
}
