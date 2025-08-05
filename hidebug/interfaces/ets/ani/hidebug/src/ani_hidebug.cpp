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
#include <codecvt>
#include <string>
#include <memory>
#include <malloc.h>
#include <parameters.h>

#include "ani_util.h"
#include "application_context.h"
#include "context.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hiappevent_util.h"
#include "hidebug_native_interface.h"
#include "hilog/log.h"
#include "iservice_registry.h"
#include "refbase.h"
#include "system_ability_definition.h"

using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_ANI"

constexpr int MAX_TAGS_ARRAY_LENGTH = 40;
enum ErrorCode {
    PERMISSION_ERROR = 201,
    PARAMETER_ERROR = 401,
    VERSION_ERROR = 801,
    SYSTEM_ABILITY_NOT_FOUND = 11400101,
    HAVA_ALREADY_TRACE = 11400102,
    WITHOUT_WRITE_PERMISSON = 11400103,
    SYSTEM_STATUS_ABNORMAL = 11400104,
    NO_CAPTURE_TRACE_RUNNING = 11400105,
};
}

static bool IsArrayForAniValue(ani_env *env, ani_object param, ani_int &arraySize)
{
    ani_boolean isArray = ANI_FALSE;
    ani_class cls = nullptr;
    ani_static_method isArrayMethod = nullptr;
    if (env->FindClass("Lescompat/Array;", &cls) != ANI_OK ||
        env->Class_FindStaticMethod(cls, "isArray", "Lstd/core/Object;:Z", &isArrayMethod) != ANI_OK ||
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

static bool GetDumpParam(ani_env *env, ani_object argsAni, std::vector<std::u16string> &args)
{
    ani_int arraySize = 0;
    if (!IsArrayForAniValue(env, argsAni, arraySize)) {
        HILOG_ERROR(LOG_CORE, "Get input args failed.");
        return false;
    }
    for (ani_int i = 0; i < arraySize; i++) {
        ani_ref aniValue = nullptr;
        if (env->Object_CallMethodByName_Ref(argsAni, "$_get", "I:Lstd/core/Object;", &aniValue, i) != ANI_OK) {
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

static bool GetTraceParam(ani_env *env, ani_array_double tagsAni, std::vector<uint64_t> &tags)
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
    ani_double *aniValues = new ani_double[arraySize];
    if (env->Array_GetRegion_Double(tagsAni, 0, arraySize, aniValues) != ANI_OK) {
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
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    AniUtil::ToAniBigInt(env, nativeMemInfoOption ? static_cast<uint64_t>(nativeMemInfoOption->pss) : 0, pss);
    return pss;
}

ani_object GetSharedDirty(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getSharedDirty");
    ani_object sharedDirty = nullptr;
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    AniUtil::ToAniBigInt(env,
        nativeMemInfoOption ? static_cast<uint64_t>(nativeMemInfoOption->sharedDirty) : 0, sharedDirty);
    return sharedDirty;
}

ani_object GetPrivateDirty(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getPrivateDirty");
    ani_object privateDirtyValue = nullptr;
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    AniUtil::ToAniBigInt(env, nativeMemInfoOption ? nativeMemInfoOption->privateDirty : 0, privateDirtyValue);
    return privateDirtyValue;
}

ani_double GetCpuUsage(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getCpuUsage");
    return static_cast<ani_double>(HidebugNativeInterface::GetInstance().GetCpuUsage());
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

static void GetServiceDump(ani_env *env,
    ani_double serviceIdAni, ani_double fdAni, ani_object argsAni)
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
    auto vssInfoOption = HidebugNativeInterface::GetInstance().GetVss();
    AniUtil::ToAniBigInt(env, vssInfoOption ? vssInfoOption.value() : 0, vss);
    return vss;
}

static ani_double GetSystemCpuUsage(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getSystemCpuUsage");
    auto cpuUsageOptional = HidebugNativeInterface::GetInstance().GetSystemCpuUsage();
    if (!cpuUsageOptional) {
        std::string paramErrorMessage = "The status of the system CPU usage is abnormal.";
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return 0;
    }
    return static_cast<ani_double>(cpuUsageOptional.value());
}

static ani_object ConvertThreadCpuUsageToEts(ani_env *env, ani_class cls, uint32_t threadIdValue, double cpuUsageValue)
{
    ani_method ctorMethod = nullptr;
    ani_object obj = nullptr;
    if (env->Class_FindMethod(cls, "<ctor>", ":V", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &obj) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    AniUtil::SetNamedPropertyNumber(env, obj, "threadId", static_cast<double>(threadIdValue));
    AniUtil::SetNamedPropertyNumber(env, obj, "cpuUsage", cpuUsageValue);
    return obj;
}

static ani_array_ref ConvertThreadCpuUsageMapToEts(ani_env *env, const std::map<uint32_t, double> &threadMap)
{
    ani_class cls = nullptr;
    ani_size aniSize = static_cast<ani_size>(threadMap.size());
    ani_array_ref result = nullptr;
    if (env->FindClass("L@ohos/hidebug/hidebug/ThreadCpuUsageImpl;", &cls) != ANI_OK ||
        env->Array_New_Ref(static_cast<ani_type>(cls), aniSize, nullptr, &result) != ANI_OK) {
        return result;
    }
    ani_size idx = 0;
    for (const auto& [threadId, cpuUsage] : threadMap) {
        ani_object obj = ConvertThreadCpuUsageToEts(env, cls, threadId, cpuUsage);
        env->Array_Set_Ref(result, idx, static_cast<ani_ref>(obj));
        idx++;
    }
    return result;
}

ani_array GetAppThreadCpuUsage(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppThreadCpuUsage");
    std::map<uint32_t, double> threadMap = HidebugNativeInterface::GetInstance().GetAppThreadCpuUsage();
    return ConvertThreadCpuUsageMapToEts(env, threadMap);
}

ani_object GetAppNativeMemInfo(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppNativeMemInfo");
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo();
    if (!nativeMemInfoOption) {
        nativeMemInfoOption.emplace();
    }

    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object memInfo = nullptr;
    if (env->FindClass("L@ohos/hidebug/hidebug/NativeMemInfoImpl;", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":V", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &memInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "pss", nativeMemInfoOption->pss);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "rss", nativeMemInfoOption->rss);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "sharedDirty", nativeMemInfoOption->sharedDirty);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "privateDirty", nativeMemInfoOption->privateDirty);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "sharedClean", nativeMemInfoOption->sharedClean);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "privateClean", nativeMemInfoOption->privateClean);
    AniUtil::SetNamedPropertyBigInt(env, memInfo, "vss", nativeMemInfoOption->vss);
    return memInfo;
}

ani_object GetSystemMemInfo(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getSystemMemInfo");
    auto sysMemOption = HidebugNativeInterface::GetInstance().GetSystemMemInfo();
    if (!sysMemOption) {
        sysMemOption.emplace();
    }

    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object sysMemInfo = nullptr;
    if (env->FindClass("L@ohos/hidebug/hidebug/SystemMemInfoImpl;", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":V", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &sysMemInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }

    AniUtil::SetNamedPropertyBigInt(env, sysMemInfo, "totalMem", static_cast<uint64_t>(sysMemOption->totalMem));
    AniUtil::SetNamedPropertyBigInt(env, sysMemInfo, "freeMem", static_cast<uint64_t>(sysMemOption->freeMem));
    AniUtil::SetNamedPropertyBigInt(env, sysMemInfo, "availableMem", static_cast<uint64_t>(sysMemOption->availableMem));
    return sysMemInfo;
}

ani_string StartAppTraceCapture(ani_env *env,
    ani_array_double tagsAni, ani_enum_item flagAni, ani_double limitSizeAni)
{
    ApiInvokeRecorder apiInvokeRecorder("startAppTraceCapture");
    ani_string result = nullptr;
    uint32_t traceFlag = 0;
    uint32_t limitSize = static_cast<uint32_t>(limitSizeAni);
    std::vector<uint64_t> tags;
    if (AniUtil::ParseAniEnum(env, flagAni, traceFlag) != ANI_OK || !GetTraceParam(env, tagsAni, tags)) {
        std::string paramErrorMessage = "Invalid argument";
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::PARAMETER_ERROR);
        return nullptr;
    }
    uint64_t tag = std::accumulate(tags.begin(), tags.end(), 0ull, [](uint64_t a, uint64_t b) { return a | b; });
    std::string file;
    auto ret = HidebugNativeInterface::GetInstance().StartAppTraceCapture(tag, traceFlag, limitSize, file);
    if (ret == TRACE_SUCCESS) {
        env->String_NewUTF8(file.c_str(), file.size(), &result);
        return result;
    }
    if (ret == TRACE_INVALID_ARGUMENT) {
        std::string errorMessage = "Invalid argument";
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::PARAMETER_ERROR);
    }
    if (ret == TRACE_CAPTURED_ALREADY) {
        std::string errorMessage = "Capture trace already enabled.";
        apiInvokeRecorder.SetErrorCode(ErrorCode::HAVA_ALREADY_TRACE);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::HAVA_ALREADY_TRACE);
    }
    if (ret == TRACE_NO_PERMISSION) {
        std::string errorMessage = "No write permission on the file.";
        apiInvokeRecorder.SetErrorCode(ErrorCode::WITHOUT_WRITE_PERMISSON);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::WITHOUT_WRITE_PERMISSON);
    }
    std::string errorMessage = "Abnormal trace status.";
    apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
    AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::SYSTEM_STATUS_ABNORMAL);
    return nullptr;
}

void StopAppTraceCapture(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("stopAppTraceCapture");
    auto ret = HidebugNativeInterface::GetInstance().StopAppTraceCapture();
    if (ret == TRACE_ABNORMAL) {
        std::string errorMessage = "The status of the trace is abnormal";
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return;
    }
    if (ret == NO_TRACE_RUNNING) {
        std::string errorMessage = "No capture trace running";
        apiInvokeRecorder.SetErrorCode(ErrorCode::NO_CAPTURE_TRACE_RUNNING);
        AniUtil::ThrowErrorMessage(env, errorMessage, ErrorCode::NO_CAPTURE_TRACE_RUNNING);
        return;
    }
}

ani_double GetGraphicsMemorySync(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getGraphicsMemorySync");
    std::optional<int32_t> result = HidebugNativeInterface::GetInstance().GetGraphicsMemory();
    if (result) {
        return static_cast<ani_double>(result.value());
    }
    constexpr const char* errMsg = "Failed to get the application memory due to a remote exception";
    AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::SYSTEM_STATUS_ABNORMAL);
    apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
    return 0;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env = nullptr;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    ani_namespace nameSpace = nullptr;
    if (ANI_OK != env->FindNamespace("L@ohos/hidebug/hidebug;", &nameSpace)) {
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"getPss", ":Lescompat/BigInt;", reinterpret_cast<void *>(GetPss)},
        ani_native_function {"getSharedDirty", ":Lescompat/BigInt;", reinterpret_cast<void *>(GetSharedDirty)},
        ani_native_function {"getPrivateDirty", ":Lescompat/BigInt;", reinterpret_cast<void *>(GetPrivateDirty)},
        ani_native_function {"getCpuUsage", ":D", reinterpret_cast<void *>(GetCpuUsage)},
        ani_native_function {"getServiceDump", "DDLescompat/Array;:V", reinterpret_cast<void *>(GetServiceDump)},
        ani_native_function {"getNativeHeapSize", ":Lescompat/BigInt;", reinterpret_cast<void *>(GetNativeHeapSize)},
        ani_native_function {"getNativeHeapAllocatedSize", ":Lescompat/BigInt;",
            reinterpret_cast<void *>(GetNativeHeapAllocatedSize)},
        ani_native_function {"getNativeHeapFreeSize", ":Lescompat/BigInt;",
            reinterpret_cast<void *>(GetNativeHeapFreeSize)},
        ani_native_function {"getVss", ":Lescompat/BigInt;", reinterpret_cast<void *>(GetVss)},
        ani_native_function {"getAppThreadCpuUsage", ":[L@ohos/hidebug/hidebug/ThreadCpuUsage;",
            reinterpret_cast<void *>(GetAppThreadCpuUsage)},
        ani_native_function {"getSystemCpuUsage", ":D", reinterpret_cast<void *>(GetSystemCpuUsage)},
        ani_native_function {"getAppNativeMemInfo", ":L@ohos/hidebug/hidebug/NativeMemInfo;",
            reinterpret_cast<void *>(GetAppNativeMemInfo)},
        ani_native_function {"getSystemMemInfo", ":L@ohos/hidebug/hidebug/SystemMemInfo;",
            reinterpret_cast<void *>(GetSystemMemInfo)},
        ani_native_function {"startAppTraceCapture", "[DL@ohos/hidebug/hidebug/TraceFlag;D:Lstd/core/String;",
            reinterpret_cast<void *>(StartAppTraceCapture)},
        ani_native_function {"stopAppTraceCapture", ":V", reinterpret_cast<void *>(StopAppTraceCapture)},
        ani_native_function {"getGraphicsMemorySync", ":D", reinterpret_cast<void *>(GetGraphicsMemorySync)},
    };
    if (ANI_OK != env->Namespace_BindNativeFunctions(nameSpace, methods.data(), methods.size())) {
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
