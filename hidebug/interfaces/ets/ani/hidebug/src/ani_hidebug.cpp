/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include <algorithm>
#include <cerrno>
#include <codecvt>
#include <fstream>
#include <string>
#include <memory>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include <malloc.h>
#include <parameters.h>
#include <unistd.h>

#include "ani_util.h"
#include "application_context.h"
#include "context.h"
#include "cpu_collector.h"
#include "directory_ex.h"
#include "dump_usage.h"
#include "error_code.h"
#include "file_ex.h"
#include "hiappevent_util.h"
#include "hidebug_native_interface.h"
#include "hidebug_util.h"
#include "hilog/log.h"
#include "iservice_registry.h"
#include "memory_collector.h"
#include "refbase.h"
#include "static_core/plugins/ets/dfx/heap_helpers.h"
#include "static_core/runtime/tooling/inspector/debugger_arkapi.h"
#include "storage_acl.h"
#include "system_ability_definition.h"

using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_ANI"
constexpr int BYTE_2_KB_SHIFT_BITS = 10;
constexpr int MAX_TAGS_ARRAY_LENGTH = 40;
constexpr auto SLASH_STR = "/";
constexpr auto JSON_FILE = ".json";

static ani_vm* g_currentVm = nullptr;

std::map<std::string, uint64_t (*)(ani_vm*)> GetVmGcMap()
{
    return {
                {"ark.gc.gc-count", ark::dfx::GetGCCount},
                {"ark.gc.gc-time", ark::dfx::GetGCDuration},
                {"ark.gc.gc-bytes-allocated", ark::dfx::GetAccumulatedAllocateSize},
                {"ark.gc.gc-bytes-freed", ark::dfx::GetAccumulatedFreeSize},
                {"ark.gc.fullgc-longtime-count", ark::dfx::GetFullGCLongTimeCount}
            };
}

bool IsArrayForAniValue(ani_env *env, ani_object param, ani_int &arraySize)
{
    ani_boolean isArray = ANI_FALSE;
    ani_class cls = nullptr;
    ani_static_method isArrayMethod = nullptr;
    if (env->FindClass("escompat.Array", &cls) != ANI_OK ||
        env->Class_FindStaticMethod(cls, "isArray", "C{std.core.Object}:z", &isArrayMethod) != ANI_OK ||
        env->Class_CallStaticMethod_Boolean(cls, isArrayMethod, &isArray, param) != ANI_OK ||
        isArray == ANI_FALSE) {
        return false;
    }

    ani_double length = 0;
    if (env->Object_GetPropertyByName_Double(param, "length", &length) != ANI_OK) {
        return false;
    }
    arraySize = static_cast<ani_int>(length);
    return true;
}

bool GetDumpParam(ani_env *env, ani_object argsAni, std::vector<std::u16string> &args)
{
    ani_int arraySize = 0;
    if (!IsArrayForAniValue(env, argsAni, arraySize)) {
        HILOG_ERROR(LOG_CORE, "Get input args failed.");
        return false;
    }
    for (ani_int i = 0; i < arraySize; i++) {
        ani_ref aniValue = nullptr;
        if (env->Object_CallMethodByName_Ref(argsAni, "$_get", "i:C{std.core.Object}", &aniValue, i) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get_element -> Get input args failed.");
            return false;
        }
        std::string strValue;
        if (AniUtil::ParseAniString(env, static_cast<ani_string>(aniValue), strValue) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get_value -> Get input args failed.");
            return false;
        }
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> strCnv;
        args.push_back(strCnv.from_bytes(strValue));
    }
    return true;
}

bool GetTraceParam(ani_env *env, ani_array_long tagsAni, std::vector<uint64_t> &tags)
{
    ani_size arraySize = 0;
    if (env->Array_GetLength(static_cast<ani_array>(tagsAni), &arraySize) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Get input tags size failed.");
        return false;
    }
    if (arraySize > static_cast<ani_size>(MAX_TAGS_ARRAY_LENGTH)) {
        HILOG_ERROR(LOG_CORE, "The length of tags array exceeds the limit.");
        return false;
    }
    ani_long *aniValues = new ani_long[arraySize];
    if (env->Array_GetRegion_Long(tagsAni, 0, arraySize, aniValues) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Get input tags value failed.");
        delete []aniValues;
        return false;
    }
    for (ani_size i = 0; i < arraySize; i++) {
        tags.push_back(static_cast<uint64_t>(aniValues[i]));
    }
    delete []aniValues;
    return true;
}

ani_object GetPss(ani_env *env)
{
    ani_object pss = nullptr;
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (collector != nullptr) {
        int pid = getprocpid();
        auto collectResult = collector->CollectProcessMemory(pid);
        uint64_t pssInfo = static_cast<uint64_t>(collectResult.data.pss) +
            static_cast<uint64_t>(collectResult.data.swapPss);
        AniUtil::ToAniBigInt(env, pssInfo, pss);
    } else {
        AniUtil::ToAniBigInt(env, 0, pss);
    }
    return pss;
}

ani_object GetSharedDirty(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getSharedDirty");
    ani_object sharedDirty = nullptr;
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (collector != nullptr) {
        int pid = getprocpid();
        auto collectResult = collector->CollectProcessMemory(pid);
        AniUtil::ToAniBigInt(env, static_cast<uint64_t>(collectResult.data.sharedDirty), sharedDirty);
    } else {
        AniUtil::ToAniBigInt(env, 0, sharedDirty);
    }
    return sharedDirty;
}

ani_object GetPrivateDirty(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getPrivateDirty");
    ani_object privateDirtyValue = nullptr;
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (collector != nullptr) {
        pid_t pid = getprocpid();
        auto collectResult = collector->CollectProcessMemory(pid);
        AniUtil::ToAniBigInt(env, static_cast<uint64_t>(collectResult.data.privateDirty), privateDirtyValue);
    } else {
        AniUtil::ToAniBigInt(env, 0, privateDirtyValue);
    }
    return privateDirtyValue;
}

ani_double GetCpuUsage(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getCpuUsage");
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    pid_t pid = getprocpid();
    double cpuUsage = dumpUsage->GetCpuUsage(pid);
    ani_double cpuUsageValue = static_cast<ani_double>(cpuUsage);
    return cpuUsageValue;
}

ani_object GetNativeHeapSize(ani_env *env)
{
    struct mallinfo mi = mallinfo();
    ani_object nativeHeapSize = nullptr;
    AniUtil::ToAniBigInt(env, uint64_t(mi.uordblks + mi.fordblks), nativeHeapSize);
    return nativeHeapSize;
}

ani_object GetNativeHeapAllocatedSize(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getNativeHeapAllocatedSize");
    struct mallinfo mi = mallinfo();
    ani_object nativeHeapAllocatedSize = nullptr;
    AniUtil::ToAniBigInt(env, uint64_t(mi.uordblks), nativeHeapAllocatedSize);
    return nativeHeapAllocatedSize;
}

ani_object GetNativeHeapFreeSize(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getNativeHeapFreeSize");
    struct mallinfo mi = mallinfo();
    ani_object nativeHeapFreeSize = nullptr;
    AniUtil::ToAniBigInt(env, uint64_t(mi.fordblks), nativeHeapFreeSize);
    return nativeHeapFreeSize;
}

void GetServiceDump(ani_env *env,
    ani_int serviceIdAni, ani_int fdAni, ani_object argsAni)
{
    ApiInvokeRecorder apiInvokeRecorder("getServiceDump");
    int serviceAbilityId = static_cast<int>(serviceIdAni);
    int fd = static_cast<int>(fdAni);
    std::vector<std::u16string> args;
    if (!GetDumpParam(env, argsAni, args)) {
        std::string paramErrorMessage = "The parameter check failed.";
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::PARAMETER_ERROR);
        return;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!sam) {
        return;
    }
    sptr<IRemoteObject> sa = sam->CheckSystemAbility(serviceAbilityId);
    if (sa == nullptr) {
        HILOG_ERROR(LOG_CORE, "no this system ability.");
        std::string idErrorMessage = "ServiceId invalid. The system ability does not exist.";
        AniUtil::ThrowErrorMessage(env, idErrorMessage, ErrorCode::SYSTEM_ABILITY_NOT_FOUND);
        return;
    }
    int dumpResult = sa->Dump(fd, args);
    HILOG_INFO(LOG_CORE, "Dump result: %{public}d", dumpResult);
}

ani_object GetVss(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getVss");
    ani_object vss = nullptr;
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (collector != nullptr) {
        pid_t pid = getprocpid();
        auto collectResult = collector->CollectProcessVss(pid);
        uint64_t vssInfo = collectResult.data;
        AniUtil::ToAniBigInt(env, vssInfo, vss);
    } else {
        AniUtil::ToAniBigInt(env, 0, vss);
    }
    return vss;
}

ani_double GetSystemCpuUsage(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getSystemCpuUsage");
    auto cpuUsageOptional = HidebugNativeInterface::GetInstance().GetSystemCpuUsage();
    if (!cpuUsageOptional) {
        std::string paramErrorMessage = "The status of the system CPU usage is abnormal.";
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return 0;
    }
    ani_double retMsg = static_cast<ani_double>(cpuUsageOptional.value());
    return retMsg;
}

ani_object ConvertThreadCpuUsageToEts(ani_env *env, ani_class cls, uint32_t threadIdValue, double cpuUsageValue)
{
    ani_method ctorMethod = nullptr;
    ani_object obj = nullptr;
    if (env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &obj) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    env->Object_SetPropertyByName_Long(obj, "threadId", static_cast<ani_long>(threadIdValue));
    AniUtil::SetNamedPropertyNumber(env, obj, "cpuUsage", cpuUsageValue);
    return obj;
}

ani_array_ref ConvertThreadCpuUsageMapToEts(ani_env *env, const std::map<uint32_t, double> &threadMap)
{
    ani_class cls = nullptr;
    ani_size aniSize = static_cast<ani_size>(threadMap.size());
    ani_array_ref result = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.ThreadCpuUsageImpl", &cls) != ANI_OK ||
        env->Array_New_Ref(static_cast<ani_type>(cls), aniSize, nullptr, &result) != ANI_OK) {
        return result;
    }
    ani_size idx = 0;
    for (const auto[threadId, cpuUsage] : threadMap) {
        ani_object obj = ConvertThreadCpuUsageToEts(env, cls, threadId, cpuUsage);
        env->Array_Set_Ref(result, idx, static_cast<ani_ref>(obj));
        idx++;
    }
    return result;
}

ani_array GetAppThreadCpuUsage(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppThreadCpuUsage");
    ani_array result = nullptr;
    std::map<uint32_t, double> threadMap = HidebugNativeInterface::GetInstance().GetAppThreadCpuUsage();
    return ConvertThreadCpuUsageMapToEts(env, threadMap);
}

ani_object GetAppNativeMemInfo(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppNativeMemInfo");
    HiDebug_NativeMemInfo nativeMemInfo{};
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo();
    if (nativeMemInfoOption) {
        nativeMemInfo = nativeMemInfoOption.value();
    }

    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object memInfo = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.NativeMemInfoImpl", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &memInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "pss", nativeMemInfo.pss);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "rss", nativeMemInfo.rss);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "sharedDirty", nativeMemInfo.sharedDirty);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "privateDirty", nativeMemInfo.privateDirty);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "sharedClean", nativeMemInfo.sharedClean);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "privateClean", nativeMemInfo.privateClean);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "vss", nativeMemInfo.vss);
    return memInfo;
}

ani_object GetSystemMemInfo(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getSystemMemInfo");
    SysMemory systemMemInfo{};
    auto systemMemInfoOption = HidebugNativeInterface::GetInstance().GetSystemMemInfo();
    if (systemMemInfoOption) {
        systemMemInfo = systemMemInfoOption.value();
    }

    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object sysMemInfo = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.SystemMemInfoImpl", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &sysMemInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }

    AniUtil::SetNamedPropertyBigInt(env, sysMemInfo, "totalMem", systemMemInfo.memTotal);
    AniUtil::SetNamedPropertyBigInt(env, sysMemInfo, "freeMem", systemMemInfo.memFree);
    AniUtil::SetNamedPropertyBigInt(env, sysMemInfo, "availableMem", systemMemInfo.memAvailable);
    return sysMemInfo;
}

ani_string StartAppTraceCapture(ani_env *env,
    ani_array_long tagsAni, ani_enum_item flagAni, ani_int limitSizeAni)
{
    ApiInvokeRecorder apiInvokeRecorder("startAppTraceCapture");
    ani_string result = nullptr;
    uint32_t traceFlag = 0;
    uint32_t limitSize = static_cast<uint32_t>(limitSizeAni);
    std::vector<uint64_t> tags;
    if (AniUtil::ParseAniEnum(env, flagAni, traceFlag) != ANI_OK || !GetTraceParam(env, tagsAni, tags)) {
        std::string paramErrorMessage = "Invalid argument";
        ani_object undefinedResult = AniUtil::CreateUndefined(env);
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::PARAMETER_ERROR);
        return nullptr;
    }
    uint64_t tag = std::accumulate(tags.begin(), tags.end(), 0ull, [](uint64_t a, uint64_t b) { return a | b; });
    std::string file;
    auto ret = HidebugNativeInterface::GetInstance().StartAppTraceCapture(tag, traceFlag, limitSize, file);
    if (ret == HIDEBUG_SUCCESS) {
        env->String_NewUTF8(file.c_str(), file.size(), &result);
        return result;
    }
    if (ret == HIDEBUG_INVALID_ARGUMENT) {
        std::string errorMessage = "Invalid argument";
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::PARAMETER_ERROR);
    }
    if (ret == HIDEBUG_TRACE_CAPTURED_ALREADY) {
        std::string errorMessage = "Capture trace already enabled.";
        apiInvokeRecorder.SetErrorCode(ErrorCode::HAVA_ALREADY_TRACE);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::HAVA_ALREADY_TRACE);
    }
    if (ret == HIDEBUG_NO_PERMISSION) {
        std::string errorMessage = "No write permission on the file.";
        apiInvokeRecorder.SetErrorCode(ErrorCode::WITHOUT_WRITE_PERMISSON);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::WITHOUT_WRITE_PERMISSON);
    }
    std::string errorMessage = "Abnormal trace status.";
    apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
    AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::SYSTEM_STATUS_ABNORMAL);
    return nullptr;
}

ani_boolean IsDebugState(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("isDebugState");
    bool debugState = ark::ArkDebugNativeAPI::IsDebugModeEnabled() ||
        HidebugNativeInterface::GetInstance().IsDebuggerConnected();
    ani_boolean result = static_cast<ani_boolean>(debugState);
    return result;
}

void StopAppTraceCapture(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("stopAppTraceCapture");
    auto ret = HidebugNativeInterface::GetInstance().StopAppTraceCapture();
    if (ret == HIDEBUG_TRACE_ABNORMAL) {
        std::string errorMessage = "The status of the trace is abnormal";
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return;
    }
    if (ret == HIDEBUG_NO_TRACE_RUNNING) {
        std::string errorMessage = "No capture trace running";
        apiInvokeRecorder.SetErrorCode(ErrorCode::NO_CAPTURE_TRACE_RUNNING);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::NO_CAPTURE_TRACE_RUNNING);
        return;
    }
}

ani_int GetGraphicsMemorySync(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getGraphicsMemorySync");
    std::optional<int32_t> result = HidebugNativeInterface::GetInstance().GetGraphicsMemory();
    if (result) {
        return static_cast<ani_int>(result.value());
    }
    constexpr const char* errMsg = "Failed to get the application memory due to a remote exception";
    AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::SYSTEM_STATUS_ABNORMAL);
    apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
    return 0;
}

bool GetTheOnlyStringParam(ani_env *env, ani_string filenameAni, std::string &fileName)
{
    if (AniUtil::ParseAniString(env, filenameAni, fileName) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Failed to parse ani_string filename");
        return false;
    }
    size_t bufLen = fileName.size();
    const int bufMax = 128;
    if (bufLen > bufMax || bufLen == 0) {
        HILOG_ERROR(LOG_CORE, "input filename param length is illegal.");
        return false;
    }
    return true;
}

ani_object GetAppVMMemoryInfo(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppVMMemoryInfo");
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object vMMemoryInfo = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.VMMemoryInfoImpl", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &vMMemoryInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }

    uint64_t totalHeapValue = ark::dfx::GetHeapTotalSize(g_currentVm);
    totalHeapValue = totalHeapValue >> BYTE_2_KB_SHIFT_BITS;
    AniUtil::SetNamedPropertyBigInt(env, vMMemoryInfo, "totalHeap", totalHeapValue);

    uint64_t heapUsedValue = ark::dfx::GetHeapUsedSize(g_currentVm);
    heapUsedValue = heapUsedValue >> BYTE_2_KB_SHIFT_BITS;
    AniUtil::SetNamedPropertyBigInt(env, vMMemoryInfo, "heapUsed", heapUsedValue);

    uint64_t allArraySizeValue = ark::dfx::GetArrayBufferSize(g_currentVm);
    allArraySizeValue = allArraySizeValue >> BYTE_2_KB_SHIFT_BITS;
    AniUtil::SetNamedPropertyBigInt(env, vMMemoryInfo, "allArraySize", allArraySizeValue);

    return vMMemoryInfo;
}

ani_object GetAppMemoryLimit(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppMemoryLimit");
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object appMemoryLimit = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.MemoryLimitImpl", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &appMemoryLimit) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    MemoryLimit memoryLimit{};
    auto memoryLimitOption = HidebugNativeInterface::GetInstance().GetAppMemoryLimit();
    if (memoryLimitOption) {
        memoryLimit = memoryLimitOption.value();
    }
    AniUtil::SetNamedPropertyBigInt(env, appMemoryLimit, "rssLimit", memoryLimit.rssLimit);
    AniUtil::SetNamedPropertyBigInt(env, appMemoryLimit, "vssLimit", memoryLimit.vssLimit);
    uint64_t vmHeapLimitValue = ark::dfx::GetHeapLimitSize(g_currentVm);
    vmHeapLimitValue = vmHeapLimitValue >> BYTE_2_KB_SHIFT_BITS;
    AniUtil::SetNamedPropertyBigInt(env, appMemoryLimit, "vmHeapLimit", vmHeapLimitValue);
    uint64_t vmTotalHeapSizeValue = ark::dfx::GetProcessHeapLimitSize(g_currentVm);
    vmTotalHeapSizeValue = vmTotalHeapSizeValue >> BYTE_2_KB_SHIFT_BITS;
    AniUtil::SetNamedPropertyBigInt(env, appMemoryLimit, "vmTotalHeapSize", vmTotalHeapSizeValue);
    return appMemoryLimit;
}

ani_object GetVMRuntimeStats(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getVMRuntimeStats");
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object vmRunTimeStats = nullptr;
    if (env->FindClass("escompat.Record", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", nullptr, &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &vmRunTimeStats, nullptr) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to init record.");
        return AniUtil::CreateUndefined(env);
    }
    ani_method setMethod = nullptr;
    if (env->Class_FindMethod(cls, "$_set", nullptr, &setMethod) != ANI_OK) {
        return vmRunTimeStats;
    }
    ani_class longClass = nullptr;
    env->FindClass("std.core.Long", &longClass);
    ani_method longCtor = nullptr;
    env->Class_FindMethod(longClass, "<ctor>", "l:", &longCtor);
    auto vmGcMap = GetVmGcMap();
    for (const auto &[k, targetFunction] : vmGcMap) {
        ani_string aniKey = nullptr;
        if (ANI_OK != env->String_NewUTF8(k.c_str(), k.size(), &aniKey)) {
            HILOG_ERROR(LOG_CORE, "create new value string failed");
        }
        ani_object longObject = nullptr;
        env->Object_New(longClass, longCtor, &longObject, static_cast<ani_long>(targetFunction(g_currentVm)));
        env->Object_CallMethod_Void(vmRunTimeStats, setMethod, aniKey, longObject);
    }
    return vmRunTimeStats;
}

ani_long GetVMRuntimeStat(ani_env *env, ani_string itemAni)
{
    ApiInvokeRecorder apiInvokeRecorder("getVMRuntimeStat");
    std::string param;
    if (!GetTheOnlyStringParam(env, itemAni, param)) {
        std::string paramErrorMessage = "Invalid parameter, a string parameter required.";
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::PARAMETER_ERROR);
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        return 0;
    }
    auto vmGcMap = GetVmGcMap();
    auto target = vmGcMap.find(param);
    if (target == vmGcMap.end()) {
        std::string paramErrorMessage = "Invalid parameter, unknown property.";
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::PARAMETER_ERROR);
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        return 0;
    }
    return static_cast<ani_long>((target->second)(g_currentVm));
}

std::string StartCpuProfiler(const std::string& fileName)
{
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return "Get ApplicationContext failed.";
    }
    std::string filesDir = context->GetFilesDir();
    if (filesDir.empty()) {
        return "Get App files dir failed.";
    }
    std::string filePath = filesDir + SLASH_STR + fileName + JSON_FILE;
    if (!IsLegalPath(filePath)) {
        return "input fileName is illegal.";
    }
    if (!CreateFile(filePath)) {
        return "file created failed.";
    }
    if (!ark::ArkDebugNativeAPI::StartProfiling(filePath)) {
        return "failed start profiler.";
    }
    return "";
}

void StartJsCpuProfiling(ani_env *env, ani_string fileNameAni)
{
    ApiInvokeRecorder apiInvokeRecorder("startJsCpuProfiling");
    std::string fileName;
    if (!GetTheOnlyStringParam(env, fileNameAni, fileName)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        std::string paramErrorMessage = "Invalid parameter, require a string parameter.";
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::PARAMETER_ERROR);
        return;
    }
    HILOG_INFO(LOG_CORE, "filename: %{public}s.", fileName.c_str());
    auto errMsg = StartCpuProfiler(fileName);
    if (!errMsg.empty()) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return;
    }
}

void StopJsCpuProfiling(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("stopJsCpuProfiling");
    if (!ark::ArkDebugNativeAPI::StopProfiling()) {
        HILOG_ERROR(LOG_CORE, "failed stopJsCpuProfiling");
    }
}
}
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    g_currentVm = vm;
    ani_env *env = nullptr;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    ani_namespace nameSpace = nullptr;
    if (ANI_OK != env->FindNamespace("@ohos.hidebug.hidebug", &nameSpace)) {
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"getAppVMMemoryInfo", ":C{@ohos.hidebug.hidebug.VMMemoryInfo}",
            reinterpret_cast<void *>(GetAppVMMemoryInfo)},
        ani_native_function {"getAppMemoryLimit", ":C{@ohos.hidebug.hidebug.MemoryLimit}",
            reinterpret_cast<void *>(GetAppMemoryLimit)},
        ani_native_function {"getVMRuntimeStats", ":C{escompat.Record}", reinterpret_cast<void *>(GetVMRuntimeStats)},
        ani_native_function {"getVMRuntimeStat", "C{std.core.String}:l", reinterpret_cast<void *>(GetVMRuntimeStat)},
        ani_native_function {"getPss", ":C{escompat.BigInt}", reinterpret_cast<void *>(GetPss)},
        ani_native_function {"getSharedDirty", ":C{escompat.BigInt}", reinterpret_cast<void *>(GetSharedDirty)},
        ani_native_function {"getPrivateDirty", ":C{escompat.BigInt}", reinterpret_cast<void *>(GetPrivateDirty)},
        ani_native_function {"getCpuUsage", ":d", reinterpret_cast<void *>(GetCpuUsage)},
        ani_native_function {"getServiceDump", "iiC{escompat.Array}:", reinterpret_cast<void *>(GetServiceDump)},
        ani_native_function {"getNativeHeapSize", ":C{escompat.BigInt}", reinterpret_cast<void *>(GetNativeHeapSize)},
        ani_native_function {"getNativeHeapAllocatedSize", ":C{escompat.BigInt}",
            reinterpret_cast<void *>(GetNativeHeapAllocatedSize)},
        ani_native_function {"getNativeHeapFreeSize", ":C{escompat.BigInt}",
            reinterpret_cast<void *>(GetNativeHeapFreeSize)},
        ani_native_function {"getVss", ":C{escompat.BigInt}", reinterpret_cast<void *>(GetVss)},
        ani_native_function {"getAppThreadCpuUsage", ":C{escompat.Array}",
            reinterpret_cast<void *>(GetAppThreadCpuUsage)},
        ani_native_function {"getSystemCpuUsage", ":d", reinterpret_cast<void *>(GetSystemCpuUsage)},
        ani_native_function {"getAppNativeMemInfo", ":C{@ohos.hidebug.hidebug.NativeMemInfo}",
            reinterpret_cast<void *>(GetAppNativeMemInfo)},
        ani_native_function {"getSystemMemInfo", ":C{@ohos.hidebug.hidebug.SystemMemInfo}",
            reinterpret_cast<void *>(GetSystemMemInfo)},
        ani_native_function {"startAppTraceCapture",
            "C{escompat.Array}C{@ohos.hidebug.hidebug.TraceFlag}i:C{std.core.String}",
            reinterpret_cast<void *>(StartAppTraceCapture)},
        ani_native_function {"isDebugState", ":z", reinterpret_cast<void *>(IsDebugState)},
        ani_native_function {"stopAppTraceCapture", ":", reinterpret_cast<void *>(StopAppTraceCapture)},
        ani_native_function {"startJsCpuProfiling", "C{std.core.String}:",
            reinterpret_cast<void *>(StartJsCpuProfiling)},
        ani_native_function {"stopJsCpuProfiling", ":", reinterpret_cast<void *>(StopJsCpuProfiling)},
        ani_native_function {"getGraphicsMemorySync", ":i", reinterpret_cast<void *>(GetGraphicsMemorySync)},
    };
    auto retCode = env->Namespace_BindNativeFunctions(nameSpace, methods.data(), methods.size());
    if (ANI_OK != retCode) {
        return retCode;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
