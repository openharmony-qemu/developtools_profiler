/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hidebug_native_interface.h"

#include <chrono>
#include <memory>
#include <vector>
#include <unistd.h>

#include "dump_usage.h"
#include "hilog/log.h"


namespace OHOS {
namespace HiviewDFX {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_Native_Interface"

class HidebugNativeInterfaceImpl : public HidebugNativeInterface {
public:
    HidebugNativeInterfaceImpl() = default;
    HidebugNativeInterfaceImpl(const HidebugNativeInterfaceImpl&) = delete;
    HidebugNativeInterfaceImpl& operator =(const HidebugNativeInterfaceImpl&) = delete;
    double GetCpuUsage() override;
    std::map<uint32_t, double> GetAppThreadCpuUsage() override;
    std::string StartAppTraceCapture(uint64_t tags, uint32_t flag, uint32_t limitsize) override;
    void StopAppTraceCapture() override;
};

std::unique_ptr<HidebugNativeInterface> HidebugNativeInterface::CreateInstance()
{
    return std::make_unique<HidebugNativeInterfaceImpl>();
}

double HidebugNativeInterfaceImpl::GetCpuUsage()
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    pid_t pid = getpid();
    float tmpCpuUsage = dumpUsage->GetCpuUsage(pid);
    double cpuUsage = static_cast<double>(tmpCpuUsage);
    return cpuUsage;
}

std::map<uint32_t, double> HidebugNativeInterfaceImpl::GetAppThreadCpuUsage()
{
    return {};
}

std::string HidebugNativeInterfaceImpl::StartAppTraceCapture(uint64_t tags, uint32_t flag, uint32_t limitsize)
{
    return "";
}

void HidebugNativeInterfaceImpl::StopAppTraceCapture()
{
}

}
}

