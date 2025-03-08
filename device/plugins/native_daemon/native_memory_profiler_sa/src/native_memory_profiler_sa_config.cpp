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

#include "native_memory_profiler_sa_config.h"
#include "define_macro.h"
#include "logging.h"

namespace OHOS::Developtools::NativeDaemon {
bool NativeMemoryProfilerSaConfig::Marshalling(Parcel& parcel) const
{
    WRITEINT32(parcel, pid_);
    WRITESTRING(parcel, filePath_);
    WRITEUINT32(parcel, duration_);
    WRITEINT32(parcel, filterSize_);
    WRITEUINT32(parcel, shareMemorySize_);
    WRITESTRING(parcel, processName_);
    WRITEUINT8(parcel, maxStackDepth_);
    WRITEBOOL(parcel, mallocDisable_);
    WRITEBOOL(parcel, mmapDisable_);
    WRITEBOOL(parcel, freeStackData_);
    WRITEBOOL(parcel, munmapStackData_);
    WRITEUINT32(parcel, mallocFreeMatchingInterval_);
    WRITEUINT32(parcel, mallocFreeMatchingCnt_);
    WRITEBOOL(parcel, stringCompressed_);
    WRITEBOOL(parcel, fpUnwind_);
    WRITEBOOL(parcel, blocked_);
    WRITEBOOL(parcel, recordAccurately_);
    WRITEBOOL(parcel, startupMode_);
    WRITEBOOL(parcel, memtraceEnable_);
    WRITEBOOL(parcel, offlineSymbolization_);
    WRITEBOOL(parcel, callframeCompress_);
    WRITEUINT32(parcel, statisticsInterval_);
    WRITEINT32(parcel, clockId_);
    WRITEUINT32(parcel, sampleInterval_);
    WRITEBOOL(parcel, responseLibraryMode_);
    WRITEBOOL(parcel, printNmd_);
    WRITEBOOL(parcel, printNmdOnly_);
    WRITEUINT32(parcel, nmdPid_);
    WRITEUINT32(parcel, nmdType_);
    WRITEINT32(parcel, jsStackReport_);
    WRITEUINT8(parcel, maxJsStackDepth_);
    WRITESTRING(parcel, filterNapiName_);
    WRITEUINT32(parcel, largestSize_);
    WRITEUINT32(parcel, secondLargestSize_);
    WRITEUINT32(parcel, maxGrowthSize_);
    return true;
}

bool NativeMemoryProfilerSaConfig::Unmarshalling(Parcel& parcel, std::shared_ptr<NativeMemoryProfilerSaConfig> config)
{
    if (config == nullptr) {
        return false;
    }
    READINT32(parcel, config->pid_);
    READSTRING(parcel, config->filePath_);
    READUINT32(parcel, config->duration_);
    READINT32(parcel, config->filterSize_);
    READUINT32(parcel, config->shareMemorySize_);
    READSTRING(parcel, config->processName_);
    READUINT8(parcel, config->maxStackDepth_);
    READBOOL(parcel, config->mallocDisable_);
    READBOOL(parcel, config->mmapDisable_);
    READBOOL(parcel, config->freeStackData_);
    READBOOL(parcel, config->munmapStackData_);
    READUINT32(parcel, config->mallocFreeMatchingInterval_);
    READUINT32(parcel, config->mallocFreeMatchingCnt_);
    READBOOL(parcel, config->stringCompressed_);
    READBOOL(parcel, config->fpUnwind_);
    READBOOL(parcel, config->blocked_);
    READBOOL(parcel, config->recordAccurately_);
    READBOOL(parcel, config->startupMode_);
    READBOOL(parcel, config->memtraceEnable_);
    READBOOL(parcel, config->offlineSymbolization_);
    READBOOL(parcel, config->callframeCompress_);
    READUINT32(parcel, config->statisticsInterval_);
    READINT32(parcel, config->clockId_);
    READUINT32(parcel, config->sampleInterval_);
    READBOOL(parcel, config->responseLibraryMode_);
    READBOOL(parcel, config->printNmd_);
    READBOOL(parcel, config->printNmdOnly_);
    READUINT32(parcel, config->nmdPid_);
    READUINT32(parcel, config->nmdType_);
    READINT32(parcel, config->jsStackReport_);
    READUINT8(parcel, config->maxJsStackDepth_);
    READSTRING(parcel, config->filterNapiName_);
    READUINT32(parcel, config->largestSize_);
    READUINT32(parcel, config->secondLargestSize_);
    READUINT32(parcel, config->maxGrowthSize_);
    PrintConfig(config);
    return true;
}

void NativeMemoryProfilerSaConfig::PrintConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    PROFILER_LOG_DEBUG(LOG_CORE,
        "pid: %d, filePath: %s, duration: %d, filterSize: %d, shareMemorySize: %d, processName: %s",
        config->pid_, config->filePath_.c_str(), config->duration_, config->filterSize_, config->shareMemorySize_,
        config->processName_.c_str());
    PROFILER_LOG_DEBUG(LOG_CORE, "maxStackDepth: %d, mallocDisable: %d, mmapDisable: %d, freeStackData: %d," \
        "munmapStackData: %d",
        config->maxStackDepth_, config->mallocDisable_, config->mmapDisable_, config->freeStackData_,
        config->munmapStackData_);
    PROFILER_LOG_DEBUG(LOG_CORE, "mallocFreeMatchingInterval: %d, mallocFreeMatchingCnt: %d, stringCompressed: %d," \
        "fpUnwind: %d, blocked: %d, recordAccurately: %d",
        config->mallocFreeMatchingInterval_, config->mallocFreeMatchingCnt_, config->stringCompressed_,
        config->fpUnwind_, config->blocked_, config->recordAccurately_);
    PROFILER_LOG_DEBUG(LOG_CORE,
        "startupMode: %d, memtraceEnable: %d, offlineSymbolization: %d, callframeCompress: %d," \
        "statisticsInterval: %d, clockId: %d",
        config->startupMode_, config->memtraceEnable_, config->offlineSymbolization_, config->callframeCompress_,
        config->statisticsInterval_, config->clockId_);
    PROFILER_LOG_DEBUG(LOG_CORE,
        "jsStackReport: %d, maxJsStackDepth_: %d, filterNapiName_: %s",
        config->jsStackReport_, config->maxJsStackDepth_, config->filterNapiName_.c_str());
}
} // namespace OHOS::Developtools::NativeDaemon