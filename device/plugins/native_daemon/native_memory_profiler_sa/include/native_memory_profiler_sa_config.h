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

#ifndef NATIVE_MEMORY_PROFILER_SA_CONFIG_H
#define NATIVE_MEMORY_PROFILER_SA_CONFIG_H

#include "parcel.h"

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr uint32_t DEFAULT_DURATION = 20;
constexpr uint8_t  DEFAULT_STACK_DAPTH = 30;
}
class NativeMemoryProfilerSaConfig : public Parcelable {
public:
    bool Marshalling(Parcel& parcel) const override;
    static bool Unmarshalling(Parcel& parcel, std::shared_ptr<NativeMemoryProfilerSaConfig> config);
    static void PrintConfig(std::shared_ptr<NativeMemoryProfilerSaConfig>& config);

public:
    int32_t pid_{0};
    std::string filePath_;
    uint32_t duration_{DEFAULT_DURATION}; // second
    int32_t filterSize_{0};
    uint32_t shareMemorySize_{0};
    std::string processName_;
    uint8_t maxStackDepth_{DEFAULT_STACK_DAPTH};
    bool mallocDisable_{false};
    bool mmapDisable_{false};
    bool freeStackData_{false};
    bool munmapStackData_{false};
    uint32_t mallocFreeMatchingInterval_{0};
    uint32_t mallocFreeMatchingCnt_{0};
    bool stringCompressed_{true};
    bool fpUnwind_{true};
    bool blocked_{true};
    bool recordAccurately_{true};
    bool startupMode_{false};
    bool memtraceEnable_{false};
    bool offlineSymbolization_{true};
    bool callframeCompress_{true};
    uint32_t statisticsInterval_{0};
    clockid_t clockId_{CLOCK_REALTIME};
    uint32_t sampleInterval_{0};
    bool responseLibraryMode_{false};
    bool printNmd_{false};
    uint32_t nmdPid_{0};
    uint32_t nmdType_{0};
    int32_t jsStackReport_{0};
    uint8_t maxJsStackDepth_{0};
    std::string filterNapiName_;
};
} // namespace OHOS::Developtools::NativeDaemon
#endif // NATIVE_MEMORY_PROFILER_SA_CONFIG_H