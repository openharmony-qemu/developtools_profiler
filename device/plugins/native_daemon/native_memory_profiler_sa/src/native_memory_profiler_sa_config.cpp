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
#include <vector>
namespace OHOS::Developtools::NativeDaemon {
namespace {
bool WriteStringVector(Parcel& parcel, const std::vector<std::string>& values)
{
    WRITEUINT32(parcel, static_cast<uint32_t>(values.size()));
    for (const auto& value : values) {
        WRITESTRING(parcel, value);
    }
    return true;
}

bool ReadStringVector(Parcel& parcel, std::vector<std::string>& values)
{
    uint32_t size = 0;
    READUINT32(parcel, size);
    if (size > MAX_STRING_VECTOR_SIZE) {
        PROFILER_LOG_ERROR(LOG_CORE, "ReadStringVector: size %u exceeds limit %u", size, MAX_STRING_VECTOR_SIZE);
        return false;
    }
    values.clear();
    values.reserve(size);
    for (uint32_t i = 0; i < size; ++i) {
        std::string value;
        READSTRING(parcel, value);
        values.emplace_back(value);
    }
    return true;
}
}

static bool WriteGpuRange(Parcel& parcel, const std::vector<std::vector<uint64_t>>& gpuRange)
{
    for (size_t i = 0; i < gpuRange.size(); i++) {
        for (size_t j = 0; j < gpuRange[i].size(); j++) {
            WRITEUINT64(parcel, gpuRange[i][j]);
        }
    }
    return true;
}

static bool ReadGpuRange(Parcel& parcel, std::shared_ptr<NativeMemoryProfilerSaConfig> config)
{
    std::vector<std::vector<uint64_t>> gpuRange;
    for (size_t i = 0; i < GPU_TYPE_SIZE; i++) {
        std::vector<uint64_t> vec;
        for (size_t j = 0; j < GPU_RANGE_SIZE * 2; j++) { // 2: the count of array
            uint64_t val = 0;
            READUINT64(parcel, val);
            vec.push_back(val);
        }
        gpuRange.push_back(vec);
    }
    std::swap(config->gpuRange_, gpuRange);
    return true;
}

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
    WRITEBOOL(parcel, asyncStackEnable_);
    WRITEUINT64(parcel, asyncFlag_);
    WRITEBOOL(parcel, nestedAsyncStackEnable_);
    WRITEUINT64(parcel, traceMask_);
    WriteGpuRange(parcel, gpuRange_);
    return MarshallingExt(parcel);
}

bool NativeMemoryProfilerSaConfig::MarshallingExt(Parcel& parcel) const
{
    WRITEBOOL(parcel, offlineSymbolization_);
    WRITEBOOL(parcel, callframeCompress_);
    WRITEUINT32(parcel, statisticsInterval_);
    WRITEINT32(parcel, clockId_);
    WRITEUINT32(parcel, sampleInterval_);
    WRITEBOOL(parcel, responseLibraryMode_);
    WRITEBOOL(parcel, printNmd_);
    WRITEBOOL(parcel, printNmdOnly_);
    WRITEBOOL(parcel, hookstandalone_);
    WRITEBOOL(parcel, saveFile_);
    WRITEBOOL(parcel, extendProcesses_);
    WRITESTRING(parcel, fileName_);
    WRITEUINT32(parcel, nmdPid_);
    WRITEUINT32(parcel, nmdType_);
    WRITEINT32(parcel, jsStackReport_);
    WRITEUINT8(parcel, maxJsStackDepth_);
    WRITESTRING(parcel, filterNapiName_);
    WRITESTRING(parcel, targetSoName_);
    if (!WriteStringVector(parcel, filterLibList_)) {
        return false;
    }
    WRITEUINT32(parcel, largestSize_);
    WRITEUINT32(parcel, secondLargestSize_);
    WRITEUINT32(parcel, maxGrowthSize_);
    WRITEUINT32(parcel, uniqueStackTableSize_);
    WRITESTRING(parcel, typeFileName_);
    WRITEUINT32(parcel, startType_);
    WRITEUINT32(parcel, gwpAsanHoldTime_);
    WRITEUINT32(parcel, gwpAsanMaxEntries_);
    WRITEUINT32(parcel, gwpAsanMaxStackDepth_);
    WRITEBOOL(parcel, allocatorSamplingEnable_);
    WRITEBOOL(parcel, nativeOrphanSnapshotsEnable_);
    WRITEUINT32(parcel, memLeakFormat_);
    WRITEUINT32(parcel, memLeakLimit_);
    WRITEUINT32(parcel, asyncNestingDepth_);
    WRITEUINT32(parcel, asyncTaskStackDepth_);
    WRITEBOOL(parcel, arkuiMemTraceEnable_);
    WRITEBOOL(parcel, discardDestroyedTraces_);
    WRITESTRING(parcel, bundleName_);
    WRITEBOOL(parcel, compositeHeapEnable_);
    return true;
}

bool NativeMemoryProfilerSaConfig::Unmarshalling(Parcel& parcel,
    std::shared_ptr<NativeMemoryProfilerSaConfig> config)
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
    READBOOL(parcel, config->asyncStackEnable_);
    READUINT64(parcel, config->asyncFlag_);
    READBOOL(parcel, config->nestedAsyncStackEnable_);
    READUINT64(parcel, config->traceMask_);
    ReadGpuRange(parcel, config);
    return UnmarshallingExt(parcel, config);
}

bool NativeMemoryProfilerSaConfig::UnmarshallingExt(Parcel& parcel,
    std::shared_ptr<NativeMemoryProfilerSaConfig> config)
{
    READBOOL(parcel, config->offlineSymbolization_);
    READBOOL(parcel, config->callframeCompress_);
    READUINT32(parcel, config->statisticsInterval_);
    READINT32(parcel, config->clockId_);
    READUINT32(parcel, config->sampleInterval_);
    READBOOL(parcel, config->responseLibraryMode_);
    READBOOL(parcel, config->printNmd_);
    READBOOL(parcel, config->printNmdOnly_);
    READBOOL(parcel, config->hookstandalone_);
    READBOOL(parcel, config->saveFile_);
    READBOOL(parcel, config->extendProcesses_);
    READSTRING(parcel, config->fileName_);
    READUINT32(parcel, config->nmdPid_);
    READUINT32(parcel, config->nmdType_);
    READINT32(parcel, config->jsStackReport_);
    READUINT8(parcel, config->maxJsStackDepth_);
    READSTRING(parcel, config->filterNapiName_);
    READSTRING(parcel, config->targetSoName_);
    if (!ReadStringVector(parcel, config->filterLibList_)) {
        return false;
    }
    READUINT32(parcel, config->largestSize_);
    READUINT32(parcel, config->secondLargestSize_);
    READUINT32(parcel, config->maxGrowthSize_);
    READUINT32(parcel, config->uniqueStackTableSize_);
    READSTRING(parcel, config->typeFileName_);
    READUINT32(parcel, config->startType_);
    READUINT32(parcel, config->gwpAsanHoldTime_);
    READUINT32(parcel, config->gwpAsanMaxEntries_);
    READUINT32(parcel, config->gwpAsanMaxStackDepth_);
    READBOOL(parcel, config->allocatorSamplingEnable_);
    READBOOL(parcel, config->nativeOrphanSnapshotsEnable_);
    READUINT32(parcel, config->memLeakFormat_);
    READUINT32(parcel, config->memLeakLimit_);
    READUINT32(parcel, config->asyncNestingDepth_);
    READUINT32(parcel, config->asyncTaskStackDepth_);
    READBOOL(parcel, config->arkuiMemTraceEnable_);
    READBOOL(parcel, config->discardDestroyedTraces_);
    READSTRING(parcel, config->bundleName_);
    READBOOL(parcel, config->compositeHeapEnable_);
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
        "startupMode: %d, memtraceEnable: %d, asyncStackEnable: %d, asyncFlag: %llu,"
        " nestedAsyncStackEnable: %d, asyncNestingDepth: %u, asyncTaskStackDepth: %u, arkuiMemTraceEnable: %d,"
        " discardDestroyedTraces: %d, traceMask: %llu, offlineSymbolization: %d, callframeCompress: %d,"
        "statisticsInterval: %d, clockId: %d, targetSoName: %s, filterLibListSize: %zu",
        config->startupMode_, config->memtraceEnable_, config->asyncStackEnable_,
        static_cast<unsigned long long>(config->asyncFlag_), config->nestedAsyncStackEnable_,
        config->asyncNestingDepth_, config->asyncTaskStackDepth_, config->arkuiMemTraceEnable_,
        config->discardDestroyedTraces_,
        static_cast<unsigned long long>(config->traceMask_), config->offlineSymbolization_,
        config->callframeCompress_, config->statisticsInterval_, config->clockId_, config->targetSoName_.c_str(),
        config->filterLibList_.size());
    PROFILER_LOG_DEBUG(LOG_CORE,
        "jsStackReport: %d, maxJsStackDepth_: %d, filterNapiName_: %s",
        config->jsStackReport_, config->maxJsStackDepth_, config->filterNapiName_.c_str());
    PROFILER_LOG_DEBUG(LOG_CORE, "uniqueStackTableSize: %u", config->uniqueStackTableSize_);
    PROFILER_LOG_DEBUG(LOG_CORE, "startType: %d", config->startType_);
    PROFILER_LOG_DEBUG(LOG_CORE, "memLeakEnable: %d, memLeakFormat_: %u, limit:%u",
        config->nativeOrphanSnapshotsEnable_, config->memLeakFormat_, config->memLeakLimit_);
    PROFILER_LOG_DEBUG(LOG_CORE, "compositeHeapEnable: %d", config->compositeHeapEnable_);
}
} // namespace OHOS::Developtools::NativeDaemon
