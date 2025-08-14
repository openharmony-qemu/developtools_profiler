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

#include "hidebug_ffi.h"

#include <numeric>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <malloc.h>
#include <codecvt>
#include <string>
#include <vector>
#include <parameters.h>

#include "file_ex.h"
#include "directory_ex.h"
#include "hidebug_native_interface.h"
#include "hilog/log.h"
#include "hidebug_util.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"

namespace OHOS::HiviewDFX {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "CJ_HiDebug"

constexpr auto KEY_HIVIEW_USER_TYPE = "const.logsystem.versiontype";
constexpr auto KEY_HIVIEW_DEVELOP_TYPE = "persist.hiview.leak_detector";

enum ErrorCode {
    MEM_ERROR = 1,
    PERMISSION_ERROR = 201,
    PARAMETER_ERROR = 401,
    VERSION_ERROR = 801,
    SYSTEM_ABILITY_NOT_FOUND = 11400101,
    HAVA_ALREADY_TRACE = 11400102,
    WITHOUT_WRITE_PERMISSON = 11400103,
    SYSTEM_STATUS_ABNORMAL = 11400104,
    NO_CAPTURE_TRACE_RUNNING = 11400105,
};

extern "C" {
    uint64_t FfiHidebugGetPss()
    {
        auto natveMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
        return natveMemInfoOption ? static_cast<uint64_t>(natveMemInfoOption->pss) : 0;
    }

    uint64_t FfiHidebugGetVss()
    {
        auto vssOption = HidebugNativeInterface::GetInstance().GetVss();
        return vssOption ? vssOption.value() : 0;
    }

    uint64_t FfiHidebugGetNativeHeapSize()
    {
        struct mallinfo mi = mallinfo();
        return static_cast<uint64_t>(mi.uordblks + mi.fordblks);
    }

    uint64_t FfiHidebugGetNativeHeapAllocatedSize()
    {
        struct mallinfo mi = mallinfo();
        return static_cast<uint64_t>(mi.uordblks);
    }

    uint64_t FfiHidebugGetNativeHeapFreeSize()
    {
        struct mallinfo mi = mallinfo();
        return static_cast<uint64_t>(mi.fordblks);
    }

    uint64_t FfiHidebugGetSharedDirty()
    {
        auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
        return nativeMemInfoOption ? static_cast<uint64_t>(nativeMemInfoOption->sharedDirty) : 0;
    }

    uint64_t FfiHidebugGetPrivateDirty()
    {
        auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
        return nativeMemInfoOption ? static_cast<uint64_t>(nativeMemInfoOption->privateDirty) : 0;
    }

    double FfiHidebugGetCpuUsage()
    {
        return HidebugNativeInterface::GetInstance().GetCpuUsage();
    }

    double FfiHidebugGetSystemCpuUsage(int32_t &code)
    {
        auto cpuUsageOptional = HidebugNativeInterface::GetInstance().GetSystemCpuUsage();
        if (cpuUsageOptional.has_value()) {
            return cpuUsageOptional.value();
        }
        code = ErrorCode::SYSTEM_STATUS_ABNORMAL;
        return 0;
    }

    ThreadCpuUsageArr FfiHidebugGetAppThreadCpuUsage(int32_t &code)
    {
        ThreadCpuUsageArr arr{ .head = nullptr, .size = 0};
        std::map<uint32_t, double> threadMap = HidebugNativeInterface::GetInstance().GetAppThreadCpuUsage();
        auto size = threadMap.size();
        if (size <= 0) {
            return arr;
        }
        arr.head = static_cast<CThreadCpuUsage *>(malloc(sizeof(CThreadCpuUsage) * size));
        if (arr.head == nullptr) {
            code = ErrorCode::MEM_ERROR;
            return arr;
        }
        size_t idx = 0;
        for (const auto[id, usage] : threadMap) {
            arr.head[idx] = CThreadCpuUsage{ .threadId = id, .cpuUsage = usage };
            idx++;
        }
        arr.size = static_cast<int64_t>(idx);
        return arr;
    }

    CSystemMemInfo FfiHidebugGetSystemMemInfo(int32_t &code)
    {
        CSystemMemInfo info{.totalMem = 0, .freeMem = 0, .availableMem = 0};
        auto systemMemInfo = HidebugNativeInterface::GetInstance().GetSystemMemInfo();
        if (!systemMemInfo) {
            code = ErrorCode::MEM_ERROR;
            return info;
        }
        info.totalMem = static_cast<uint64_t>(systemMemInfo->totalMem);
        info.freeMem = static_cast<uint64_t>(systemMemInfo->freeMem);
        info.availableMem = static_cast<uint64_t>(systemMemInfo->availableMem);
        return info;
    }

    CNativeMemInfo FfiHidebugGetAppNativeMemInfo(int32_t &code)
    {
        CNativeMemInfo info{};
        auto nativeMemInfo = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo();
        if (!nativeMemInfo) {
            code = ErrorCode::MEM_ERROR;
            return info;
        }
        info.pss = nativeMemInfo->pss;
        info.vss = nativeMemInfo->vss;
        info.rss = nativeMemInfo->rss;
        info.sharedDirty = nativeMemInfo->sharedDirty;
        info.privateDirty = nativeMemInfo->privateDirty;
        info.sharedClean = nativeMemInfo->sharedClean;
        info.privateClean = nativeMemInfo->privateClean;
        return info;
    }

    CMemoryLimit FfiHidebugGetAppMemoryLimit(int32_t &code)
    {
        CMemoryLimit limit{.rssLimit = 0, .vssLimit = 0};
        auto memoryLimit = HidebugNativeInterface::GetInstance().GetAppMemoryLimit();
        if (!memoryLimit) {
            code = ErrorCode::MEM_ERROR;
            return limit;
        }
        limit.rssLimit = memoryLimit->rssLimit;
        limit.vssLimit = memoryLimit->vssLimit;
        return limit;
    }

    int32_t FfiHidebugGetServiceDump(int32_t serviceId, int32_t fd, CArrString args)
    {
        sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!sam) {
            return ErrorCode::MEM_ERROR;
        }
        sptr<IRemoteObject> sa = sam->CheckSystemAbility(serviceId);
        if (sa == nullptr) {
            return ErrorCode::SYSTEM_ABILITY_NOT_FOUND;
        }
        std::vector<std::u16string> cargs;
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> strCnv;
        for (int64_t i = 0; i < args.size; i++) {
            cargs.push_back(strCnv.from_bytes(args.head[i]));
        }
        int dumpResult = sa->Dump(fd, cargs);
        HILOG_INFO(LOG_CORE, "Dump result: %{public}d", dumpResult);
        return 0;
    }

    char *FfiHidebugStartAppTraceCapture(CArrUnit tags, int32_t flag, uint32_t limitSize, int32_t &code)
    {
        std::vector<uint64_t> taglist;
        uint64_t *tagPtr = static_cast<uint64_t *>(tags.head);
        for (int64_t i = 0; i < tags.size; i++) {
            taglist.push_back(tagPtr[i]);
        }
        uint64_t tag = std::accumulate(taglist.begin(), taglist.end(), 0ull,
            [](uint64_t a, uint64_t b) { return a | b; });
        std::string file;
        code = HidebugNativeInterface::GetInstance().StartAppTraceCapture(tag, flag, limitSize, file);
        if (code != TRACE_SUCCESS || file.empty()) {
            return nullptr;
        }
        auto len = file.length() + 1;
        char *res = static_cast<char *>(malloc(sizeof(char) * len));
        if (res == nullptr) {
            return nullptr;
        }
        return std::char_traits<char>::copy(res, file.c_str(), len);
    }

    int32_t FfiHidebugStopAppTraceCapture()
    {
        return HidebugNativeInterface::GetInstance().StopAppTraceCapture();
    }

    int32_t FfiHidebugSetAppResourceLimit(const char *type, int32_t value, bool enableDebugLog)
    {
        if (!CheckVersionType("beta", KEY_HIVIEW_USER_TYPE) &&
            !CheckVersionType("enable", KEY_HIVIEW_DEVELOP_TYPE)) {
            HILOG_ERROR(LOG_CORE, "SetAppResourceLimit failed. Not developer options or beta versions");
            return ErrorCode::VERSION_ERROR;
        }
        auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!abilityManager) {
            return ErrorCode::MEM_ERROR;
        }
        sptr<IRemoteObject> remoteObject = abilityManager->CheckSystemAbility(DFX_SYS_HIVIEW_ABILITY_ID);
        if (remoteObject == nullptr) {
            HILOG_ERROR(LOG_CORE, "SetAppResourceLimit failed. No this system ability.");
            return ErrorCode::SYSTEM_STATUS_ABNORMAL;
        }
        if (HidebugNativeInterface::GetInstance().GetMemoryLeakResource(std::string(type), value, enableDebugLog)
            == NATIVE_SUCCESS) {
            CreateResourceLimitDir();
        }
        return 0;
    }

    bool FfiHidebugIsDebugState()
    {
        return HidebugNativeInterface::GetInstance().IsDebuggerConnected();
    }
}
}
