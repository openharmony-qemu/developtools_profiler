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
#include <atomic>
#include <chrono>
#include <codecvt>
#include <condition_variable>
#include <cinttypes>
#include <cstring>
#include <malloc.h>
#include <memory>
#include <mutex>
#include <parameters.h>
#include <sys/statvfs.h>
#include <string>
#include <thread>
#include "ani_util.h"
#include "application_context.h"
#include "context.h"
#include "directory_ex.h"
#include "error_code.h"
#include "faultlogger_client.h"
#include "file_ex.h"
#include "hiappevent_util.h"
#include "hisysevent.h"
#include "hidebug_native_interface.h"
#include "hidebug_util.h"
#include "hilog/log.h"
#include "iservice_registry.h"
#include "refbase.h"
#include "heap_helpers.h"
#include "debugger_arkapi.h"
#include "dfx_jsnapi.h"
#include "system_ability_definition.h"

#ifdef ENABLE_API_METRICS
#include "histogram_plugin_macros.h"
#endif

using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_ANI"
constexpr int BYTE_2_KB_SHIFT_BITS = 10;
constexpr int MAX_TAGS_ARRAY_LENGTH = 40;
constexpr int MAX_WAIT_TIME_SECOND = 30; // 30s
constexpr auto SLASH_STR = "/";
constexpr auto JSON_FILE = ".json";
constexpr auto HEAPSNAPSHOT_FILE = ".heapsnapshot";
constexpr auto RAW_HEAP_RECORD_FILE = "rawheap";
constexpr auto RAW_HEAP_FILE_EXT = ".rawheap";
constexpr auto HIDEBUG_DUMP_QUOTA = "user.hidebugdump.quota";
constexpr auto DUMP_MAX_COUNT = "persist.hiview.hidebugdump.maxcount";
constexpr auto PROCESS_DUMP_MAX_COUNT = "persist.hiview.hidebugdump.process.maxcount";
constexpr int64_t MS_TO_NS = 1000 * 1000;
constexpr int64_t S_TO_NS = MS_TO_NS * 1000;
constexpr int QUOTA_VALUE_LENGTH = 256;
constexpr int RAW_HEAP_RESULT_ARRAY_SIZE = 2;

constexpr std::pair<TraceErrorCode, const char*> TraceErrorCodeMap[] = {
    {TraceErrorCode::TRACE_INVALID_ARGUMENT, "Invalid parameter."},
    {TraceErrorCode::TRACE_ABNORMAL, "Remote service exception."},
    {TraceErrorCode::TRACE_STORAGE_LIMIT, "Trace storage limit reached."},
    {TraceErrorCode::RESOURCE_UNAVAILABLE, "Resource unavailable."},
};

constexpr std::pair<int, const char*> DumpRawHeapErrCodeMap[] = {
    {ErrorCode::PARAMETER_ERROR, "Invalid parameter."},
    {DumpRawHeapErrors::REPEAT_DUMPING, "It is currently dumping. Please wait until the dumping is completed "
    "before making another call."},
    {DumpRawHeapErrors::FAILED_CREATE_FILE, "Failed to create dump file."},
    {DumpRawHeapErrors::LOW_DISK_SPACE, "Disk remaining space too low."},
    {DumpRawHeapErrors::QUOTA_EXCEEDED, "The number of available invocations for the interface is insufficient. "
    "Please wait until the number of available invocations for the interface is reset."},
    {DumpRawHeapErrors::FORK_FAILED, "Fork operation failed."},
    {DumpRawHeapErrors::FAILED_WAIT_CHILD_PROCESS_FINISHED, "Failed to wait for the child process to finish."},
    {DumpRawHeapErrors::TIMEOUT_WAIT_CHILD_PROCESS_FINISHED, "Timeout while waiting for the child process to finish."}
};

bool ResolveTraceErrorCode(ani_env *env, const TraceErrorCode errCode)
{
    if (errCode == TraceErrorCode::TRACE_SUCCESS) {
        return true;
    }
    for (const auto &[code, msg] : TraceErrorCodeMap) {
        if (code == errCode) {
            AniUtil::ThrowErrorMessage(env, msg, errCode);
            return false;
        }
    }
    return false;
}

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

struct GwpAsanParams {
    bool alwaysEnabled = false;
    bool isRecover = false;
    int32_t sampleRate = 2500;
    int32_t maxSimutaneousAllocations = 1000;
    int32_t duration = 7;
};

struct RequestTraceParam {
    std::string identifier;
    uint32_t bufferSize = 0;
    uint32_t duration = 0;
};

bool GetDumpParam(ani_env *env, ani_array argsAni, std::vector<std::u16string> &args, std::string* errMsg)
{
    ani_size arraySize = 0;
    if (env->Array_GetLength(argsAni, &arraySize) != ANI_OK) {
        if (errMsg != nullptr) { *errMsg = "Invalid parameter, get input args size failed."; }
        HILOG_ERROR(LOG_CORE, "Get input args size failed.");
        return false;
    }
    for (ani_size i = 0; i < arraySize; i++) {
        ani_ref aniValue = nullptr;
        if (env->Object_CallMethodByName_Ref(argsAni, "$_get", "i:Y", &aniValue, i) != ANI_OK) {
            if (errMsg != nullptr) { *errMsg = "Invalid parameter, get input args element failed."; }
            HILOG_ERROR(LOG_CORE, "get_element -> Get input args failed.");
            return false;
        }
        std::string strValue;
        if (AniUtil::ParseAniString(env, static_cast<ani_string>(aniValue), strValue) != ANI_OK) {
            if (errMsg != nullptr) { *errMsg = "Invalid parameter, parse input args string failed."; }
            HILOG_ERROR(LOG_CORE, "get_value -> Get input args failed.");
            return false;
        }
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> strCnv;
        args.push_back(strCnv.from_bytes(strValue));
    }
    return true;
}

bool GetTraceParam(ani_env *env, ani_array tagsAni, std::vector<uint64_t> &tags, std::string* errMsg)
{
    ani_size arraySize = 0;
    if (env->Array_GetLength(static_cast<ani_array>(tagsAni), &arraySize) != ANI_OK) {
        if (errMsg != nullptr) { *errMsg = "Invalid parameter, get input tags size failed."; }
        HILOG_ERROR(LOG_CORE, "Get input tags size failed.");
        return false;
    }
    if (arraySize > static_cast<ani_size>(MAX_TAGS_ARRAY_LENGTH)) {
        if (errMsg != nullptr) { *errMsg = "Invalid parameter, the length of tags array exceeds the limit."; }
        HILOG_ERROR(LOG_CORE, "The length of tags array exceeds the limit.");
        return false;
    }
    ani_class longClass {};
    ani_method toLong {};
    ani_status status = env->FindClass("std.core.Long", &longClass);
    if (status != ANI_OK) {
        if (errMsg != nullptr) { *errMsg = "Invalid parameter, find class std.core.Long failed."; }
        HILOG_ERROR(LOG_CORE, "Get input tags value failed.");
        return false;
    }
    status = env->Class_FindMethod(longClass, "toLong", ":l", &toLong);
    for (ani_size i = 0; i < arraySize; i++) {
        ani_ref longRef {};
        ani_long longValue {};
        status = env->Array_Get(tagsAni, i, &longRef);
        if (status != ANI_OK) {
            if (errMsg != nullptr) { *errMsg = "Invalid parameter, get tags array element failed."; }
            HILOG_ERROR(LOG_CORE, "Array_Get failed, status: %{public}d", status);
            return false;
        }
        status = env->Object_CallMethod_Long(static_cast<ani_object>(longRef), toLong, &longValue);
        if (status != ANI_OK) {
            if (errMsg != nullptr) { *errMsg = "Invalid parameter, invoke toLong failed."; }
            HILOG_ERROR(LOG_CORE, "Invoke toLong, status: %{public}d", status);
            return false;
        }
        tags.push_back(static_cast<uint64_t>(longValue));
    }
    return true;
}

bool GetRequestTraceParam(ani_env *env, ani_string identifierAni, ani_int bufferSizeAni, ani_int durationAni,
    RequestTraceParam &param, std::string* errMsg)
{
    if (AniUtil::ParseAniString(env, identifierAni, param.identifier) != ANI_OK) {
        if (errMsg != nullptr) { *errMsg = "Invalid parameter, get input identifier failed."; }
        HILOG_ERROR(LOG_CORE, "Get input identifier failed.");
        return false;
    }
    param.bufferSize = static_cast<uint32_t>(bufferSizeAni);
    param.duration = static_cast<uint32_t>(durationAni);
    return true;
}

ani_long GetPss(ani_env *env)
{
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    return nativeMemInfoOption ? static_cast<ani_long>(nativeMemInfoOption->pss) : 0;
}

ani_long GetSharedDirty(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getSharedDirty");
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    return nativeMemInfoOption ? static_cast<ani_long>(nativeMemInfoOption->sharedDirty) : 0;
}

ani_long GetPrivateDirty(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getPrivateDirty");
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    return nativeMemInfoOption ? static_cast<ani_long>(nativeMemInfoOption->privateDirty) : 0;
}

ani_double GetCpuUsage(ani_env *env)
{
#ifdef ENABLE_API_METRICS
    HISTOGRAM_BOOLEAN("PerformanceAnalysisKit.ApiCall.getCpuUsage", 1);
#endif
    return static_cast<ani_double>(HidebugNativeInterface::GetInstance().GetCpuUsage());
}

ani_long GetNativeHeapSize(ani_env *env)
{
    struct mallinfo mi = mallinfo();
    return static_cast<ani_long>(mi.uordblks + mi.fordblks);
}

ani_long GetNativeHeapAllocatedSize(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getNativeHeapAllocatedSize");
    struct mallinfo mi = mallinfo();
    return static_cast<ani_long>(mi.uordblks);
}

ani_long GetNativeHeapFreeSize(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getNativeHeapFreeSize");
    struct mallinfo mi = mallinfo();
    return static_cast<ani_long>(mi.fordblks);
}

void GetServiceDump(ani_env *env,
    ani_int serviceIdAni, ani_int fdAni, ani_array argsAni)
{
    ApiInvokeRecorder apiInvokeRecorder("getServiceDump");
    int serviceAbilityId = static_cast<int>(serviceIdAni);
    int fd = static_cast<int>(fdAni);
    std::vector<std::u16string> args;
    std::string errMsg;
    if (!GetDumpParam(env, argsAni, args, &errMsg)) {
        AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::PARAMETER_ERROR);
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

ani_long GetVss(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getVss");
    auto vssInfoOption = HidebugNativeInterface::GetInstance().GetVss();
    return vssInfoOption ? static_cast<ani_long>(vssInfoOption.value()) : 0;
}

ani_double GetSystemCpuUsage(ani_env *env)
{
#ifdef ENABLE_API_METRICS
    HISTOGRAM_BOOLEAN("PerformanceAnalysisKit.ApiCall.getSystemCpuUsage", 1);
#endif
    auto cpuUsageOptional = HidebugNativeInterface::GetInstance().GetSystemCpuUsage();
    if (!cpuUsageOptional) {
        std::string paramErrorMessage = "The status of the system CPU usage is abnormal.";
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return 0;
    }
    return static_cast<ani_double>(cpuUsageOptional.value());
}

ani_object ConvertThreadCpuUsageToEts(ani_env *env, ani_class cls,
    uint32_t threadIdValue, double cpuUsageValue, const std::string &threadNameValue)
{
    ani_method ctorMethod = nullptr;
    ani_object obj = nullptr;
    if (env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &obj) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    env->Object_SetPropertyByName_Long(obj, "threadId", static_cast<ani_long>(threadIdValue));
    AniUtil::SetNamedPropertyNumber(env, obj, "cpuUsage", cpuUsageValue);
    AniUtil::SetNamedPropertyString(env, obj, "threadName", threadNameValue);
    return obj;
}

ani_array ConvertThreadCpuUsageListToEts(ani_env *env, const std::vector<ThreadCpuUsageInfo> &threadCpuUsageList)
{
    ani_class cls = nullptr;
    ani_size aniSize = static_cast<ani_size>(threadCpuUsageList.size());
    ani_array result = nullptr;
    ani_ref undefinedRef = AniUtil::CreateUndefined(env);
    if (env->FindClass("@ohos.hidebug.hidebug.ThreadCpuUsageImpl", &cls) != ANI_OK ||
        env->Array_New(aniSize, undefinedRef, &result) != ANI_OK) {
        return result;
    }
    ani_size idx = 0;
    for (const auto& item : threadCpuUsageList) {
        ani_object obj = ConvertThreadCpuUsageToEts(env, cls, item.tid, item.cpuUsage, item.threadName);
        env->Array_Set(result, idx, static_cast<ani_ref>(obj));
        idx++;
    }
    return result;
}

ani_array GetAppThreadCpuUsage(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppThreadCpuUsage");
    auto threadCpuUsageList = HidebugNativeInterface::GetInstance().GetAppThreadCpuUsage();
    return ConvertThreadCpuUsageListToEts(env, threadCpuUsageList);
}

ani_object GetAppNativeMemInfoAdapter(ani_env *env, ani_boolean forceRefresh)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppNativeMemInfo");
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(!forceRefresh);
    if (!nativeMemInfoOption) {
        nativeMemInfoOption.emplace();
    }

    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object memInfo = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.NativeMemInfoAdapter", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &memInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    env->Object_SetPropertyByName_Long(memInfo, "pss",
        static_cast<ani_long>(nativeMemInfoOption->pss));
    env->Object_SetPropertyByName_Long(memInfo, "rss",
        static_cast<ani_long>(nativeMemInfoOption->rss));
    env->Object_SetPropertyByName_Long(memInfo, "sharedDirty",
        static_cast<ani_long>(nativeMemInfoOption->sharedDirty));
    env->Object_SetPropertyByName_Long(memInfo, "privateDirty",
        static_cast<ani_long>(nativeMemInfoOption->privateDirty));
    env->Object_SetPropertyByName_Long(memInfo, "sharedClean",
        static_cast<ani_long>(nativeMemInfoOption->sharedClean));
    env->Object_SetPropertyByName_Long(memInfo, "sharedClean",
        static_cast<ani_long>(nativeMemInfoOption->sharedClean));
    env->Object_SetPropertyByName_Long(memInfo, "privateClean",
        static_cast<ani_long>(nativeMemInfoOption->privateClean));
    env->Object_SetPropertyByName_Long(memInfo, "vss", static_cast<ani_long>(nativeMemInfoOption->vss));
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
    if (env->FindClass("@ohos.hidebug.hidebug.SystemMemInfoAdapter", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &sysMemInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    env->Object_SetPropertyByName_Long(sysMemInfo, "totalMem", static_cast<ani_long>(sysMemOption->totalMem));
    env->Object_SetPropertyByName_Long(sysMemInfo, "freeMem", static_cast<ani_long>(sysMemOption->freeMem));
    env->Object_SetPropertyByName_Long(sysMemInfo, "availableMem", static_cast<ani_long>(sysMemOption->availableMem));
    return sysMemInfo;
}

ani_string StartAppTraceCapture(ani_env *env,
    ani_array tagsAni, ani_enum_item flagAni, ani_int limitSizeAni)
{
    ApiInvokeRecorder apiInvokeRecorder("startAppTraceCapture");
    ani_string result = nullptr;
    uint32_t traceFlag = 0;
    uint32_t limitSize = static_cast<uint32_t>(limitSizeAni);
    std::vector<uint64_t> tags;
    std::string errMsg;
    bool paramOk = false;
    if (AniUtil::ParseAniEnum(env, flagAni, traceFlag) != ANI_OK) {
        errMsg = "Invalid parameter, parse trace flag failed.";
    } else if (!GetTraceParam(env, tagsAni, tags, &errMsg)) {
        HILOG_ERROR(LOG_CORE, "%{public}s", errMsg.c_str());
    } else {
        paramOk = true;
    }
    if (!paramOk) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::PARAMETER_ERROR);
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

ani_string RequestTraceAdapter(ani_env *env, ani_string identifierAni, ani_int bufferSizeAni,
    ani_int durationAni, ani_int reservedAni)
{
    ApiInvokeRecorder apiInvokeRecorder("requestTrace");
    (void)reservedAni;
    RequestTraceParam param;
    std::string errMsg;
    if (!GetRequestTraceParam(env, identifierAni, bufferSizeAni, durationAni, param, &errMsg)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::PARAMETER_ERROR);
        return nullptr;
    }
    std::mutex mutex;
    std::condition_variable cond;
    bool hasFinished = false;
    TraceErrorCode callbackRetCode = TraceErrorCode::TRACE_ABNORMAL;
    std::string tracePath;
    auto task = [&mutex, &cond, &hasFinished, &callbackRetCode, &tracePath](
        TraceErrorCode callbackRet, const std::string& path) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            callbackRetCode = callbackRet;
            tracePath = path;
            hasFinished = true;
        }
        cond.notify_all();
    };
    TraceErrorCode retCode = HidebugNativeInterface::GetInstance().RequestTrace(
        param.identifier, param.bufferSize, param.duration, task);
    if (retCode == TraceErrorCode::TRACE_SUCCESS) {
        std::unique_lock<std::mutex> lock(mutex);
        bool waitRet = cond.wait_for(lock, std::chrono::seconds(MAX_WAIT_TIME_SECOND), [&hasFinished] {
            return hasFinished;
        });
        if (!waitRet) {
            retCode = TraceErrorCode::TRACE_ABNORMAL;
            HILOG_ERROR(LOG_CORE, "wait callback timeout");
        } else {
            retCode = callbackRetCode;
        }
    }
    if (!ResolveTraceErrorCode(env, retCode)) {
        apiInvokeRecorder.SetErrorCode(retCode);
        return nullptr;
    }
    ani_string retValue = nullptr;
    if (env->String_NewUTF8(tracePath.c_str(), tracePath.size(), &retValue) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Create requestTrace return string failed.");
        return nullptr;
    }
    return retValue;
}

ani_object GetGraphicsMemoryAdapter(ani_env *env, ani_int interval)
{
    ApiInvokeRecorder apiInvokeRecorder("getGraphicsMemory");
    do {
        auto result = HidebugNativeInterface::GetInstance().GetGraphicsMemorySummary(static_cast<uint32_t>(interval));
        if (!result) {
            break;
        }
        ani_class cls = nullptr;
        ani_method ctorMethod = nullptr;
        ani_object graphicMemorySummary = nullptr;
        if (env->FindClass("@ohos.hidebug.hidebug.GraphicsMemorySummaryImpl", &cls) != ANI_OK ||
            env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
            env->Object_New(cls, ctorMethod, &graphicMemorySummary) != ANI_OK) {
            break;
        }
        env->Object_SetPropertyByName_Int(graphicMemorySummary, "gl", result->gl);
        env->Object_SetPropertyByName_Int(graphicMemorySummary, "graph", result->graph);
        return graphicMemorySummary;
    } while (false);
    constexpr const char* errMsg = "Failed to get the application memory due to a remote exception";
    AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::SYSTEM_STATUS_ABNORMAL);
    apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
    return AniUtil::CreateUndefined(env);
}

bool GetTheOnlyStringParam(ani_env *env, ani_string filenameAni, std::string &fileName, std::string* errMsg)
{
    if (AniUtil::ParseAniString(env, filenameAni, fileName) != ANI_OK) {
        if (errMsg != nullptr) { *errMsg = "Invalid parameter, failed to parse ani_string filename."; }
        HILOG_ERROR(LOG_CORE, "Failed to parse ani_string filename");
        return false;
    }
    size_t bufLen = fileName.size();
    const int bufMax = 128;
    if (bufLen > bufMax || bufLen == 0) {
        if (errMsg != nullptr) { *errMsg = "Invalid parameter, input filename param length is illegal."; }
        HILOG_ERROR(LOG_CORE, "input filename param length is illegal.");
        return false;
    }
    return true;
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
    std::string paramErrMsg;
    if (!GetTheOnlyStringParam(env, fileNameAni, fileName, &paramErrMsg)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, paramErrMsg, ErrorCode::PARAMETER_ERROR);
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

ani_object GetAppVMMemoryInfo(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppVMMemoryInfo");
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object vMMemoryInfo = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.VMMemoryInfoAdapter", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &vMMemoryInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    ani_vm* vm = AniUtil::GetAniVm(env);
    auto totalHeapValue = static_cast<int64_t>(ark::dfx::GetHeapTotalSize(vm) >> BYTE_2_KB_SHIFT_BITS);
    env->Object_SetPropertyByName_Long(vMMemoryInfo, "totalHeap", totalHeapValue);
    auto heapUsedValue = static_cast<int64_t>(ark::dfx::GetHeapUsedSize(vm) >> BYTE_2_KB_SHIFT_BITS);
    env->Object_SetPropertyByName_Long(vMMemoryInfo, "heapUsed", heapUsedValue);
    auto allArraySizeValue = static_cast<int64_t>(ark::dfx::GetArrayBufferSize(vm) >> BYTE_2_KB_SHIFT_BITS);
    env->Object_SetPropertyByName_Long(vMMemoryInfo, "allArraySize", allArraySizeValue);
    return vMMemoryInfo;
}

ani_long GetAppVMObjectUsedSize(ani_env *env)
{
    ani_vm* vm = AniUtil::GetAniVm(env);
    return static_cast<ani_long>(ark::dfx::GetHeapUsedSize(vm) >> BYTE_2_KB_SHIFT_BITS);
}

ani_object GetAppMemoryLimit(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppMemoryLimit");
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object appMemoryLimit = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.MemoryLimitImplAdapter", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &appMemoryLimit) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    auto memoryLimitOption = HidebugNativeInterface::GetInstance().GetAppMemoryLimit();
    if (!memoryLimitOption) {
        memoryLimitOption.emplace();
    }
    env->Object_SetPropertyByName_Long(appMemoryLimit, "rssLimit", static_cast<ani_long>(memoryLimitOption->rssLimit));
    env->Object_SetPropertyByName_Long(appMemoryLimit, "vssLimit", static_cast<ani_long>(memoryLimitOption->vssLimit));
    ani_vm* vm = AniUtil::GetAniVm(env);
    int64_t vmHeapLimitValue = static_cast<int64_t>(ark::dfx::GetHeapLimitSize(vm) >> BYTE_2_KB_SHIFT_BITS);
    env->Object_SetPropertyByName_Long(appMemoryLimit, "vmHeapLimit", vmHeapLimitValue);
    int64_t vmTotalHeapSizeValue = static_cast<int64_t>(ark::dfx::GetProcessHeapLimitSize(vm) >> BYTE_2_KB_SHIFT_BITS);
    env->Object_SetPropertyByName_Long(appMemoryLimit, "vmTotalHeapSize", vmTotalHeapSizeValue);
    return appMemoryLimit;
}

ani_object GetVMRuntimeStats(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getVMRuntimeStats");
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object vmRunTimeStats = nullptr;
    if (env->FindClass("std.core.Record", &cls) != ANI_OK ||
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
    ani_vm* vm = AniUtil::GetAniVm(env);
    for (const auto &[k, targetFunction] : vmGcMap) {
        ani_string aniKey = nullptr;
        if (ANI_OK != env->String_NewUTF8(k.c_str(), k.size(), &aniKey)) {
            HILOG_ERROR(LOG_CORE, "create new value string failed");
        }
        ani_object longObject = nullptr;
        env->Object_New(longClass, longCtor, &longObject, static_cast<ani_long>(targetFunction(vm)));
        env->Object_CallMethod_Void(vmRunTimeStats, setMethod, aniKey, longObject);
    }
    return vmRunTimeStats;
}

ani_long GetVMRuntimeStat(ani_env *env, ani_string itemAni)
{
    ApiInvokeRecorder apiInvokeRecorder("getVMRuntimeStat");
    std::string param;
    std::string errMsg;
    if (!GetTheOnlyStringParam(env, itemAni, param, &errMsg)) {
        AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::PARAMETER_ERROR);
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
    return static_cast<ani_long>((target->second)(AniUtil::GetAniVm(env)));
}

bool ParseGwpAsanOptions(ani_env *env, ani_object optionsObj, GwpAsanParams& gwpAsanParams)
{
    if (!AniUtil::ParseBoolProperty(env, optionsObj, "alwaysEnabled", gwpAsanParams.alwaysEnabled)) {
        return false;
    }

    if (!AniUtil::ParseBoolProperty(env, optionsObj, "isRecover", gwpAsanParams.isRecover)) {
        return false;
    }

    if (!AniUtil::ParseIntProperty(env, optionsObj, "sampleRate", gwpAsanParams.sampleRate)) {
        return false;
    }

    if (!AniUtil::ParseIntProperty(env, optionsObj, "maxSimutaneousAllocations",
        gwpAsanParams.maxSimutaneousAllocations)) {
        return false;
    }

    if (gwpAsanParams.sampleRate <= 0 || gwpAsanParams.maxSimutaneousAllocations <= 0) {
        HILOG_ERROR(LOG_CORE, "sampleRate or maxSimutaneousAllocations must be greater than 0");
        return false;
    }

    HILOG_DEBUG(LOG_CORE, "Parse GwpAsanOptions success, alwaysEnabled: %{public}d, sampleRate: %{public}d,"
        " maxSimutaneousAllocations: %{public}d, isRecover: %{public}d",
        gwpAsanParams.alwaysEnabled, gwpAsanParams.sampleRate,
        gwpAsanParams.maxSimutaneousAllocations, gwpAsanParams.isRecover);
    return true;
}

bool UpdateGwpAsanParams(ani_env *env, ani_object optionsObj, ani_object durationObj,
    GwpAsanParams& gwpAsanParams, std::string& paramErrorMessage)
{
    ani_boolean isUndefined;
    env->Reference_IsUndefined(durationObj, &isUndefined);
    if (isUndefined == ANI_FALSE) {
        ani_int durationAni;
        env->Object_CallMethodByName_Int(durationObj, "toInt", ":i", &durationAni);
        if (durationAni <= 0) {
            paramErrorMessage = "Invalid parameter, duration must be greater than 0.";
            return false;
        }
        gwpAsanParams.duration = static_cast<int32_t>(durationAni);
    }

    env->Reference_IsUndefined(optionsObj, &isUndefined);
    if (isUndefined == ANI_FALSE) {
        if (!ParseGwpAsanOptions(env, optionsObj, gwpAsanParams)) {
            paramErrorMessage = "Invalid parameter, parse gwpAsanOptions error.";
            return false;
        }
    }
    return true;
}

void EnableGwpAsanGrayscaleAni(ani_env *env, ani_object optionsObj, ani_object durationObj)
{
    ApiInvokeRecorder apiInvokeRecorder("enableGwpAsanGrayscale");
    GwpAsanParams gwpAsanParams;
    std::string paramErrorMessage;
    if (!UpdateGwpAsanParams(env, optionsObj, durationObj, gwpAsanParams, paramErrorMessage)) {
        HILOG_ERROR(LOG_CORE, "EnableGwpAsanGrayscale failed. Invalid params!");
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::PARAMETER_ERROR);
        return;
    }
    if (!EnableGwpAsanGrayscale(gwpAsanParams.alwaysEnabled, gwpAsanParams.sampleRate,
        gwpAsanParams.maxSimutaneousAllocations, gwpAsanParams.duration, gwpAsanParams.isRecover)) {
        HILOG_ERROR(LOG_CORE, "Failed to enable gwp asan grayscale!");
        apiInvokeRecorder.SetErrorCode(ErrorCode::OVER_ENABLE_LIMIT);
        paramErrorMessage = "The number of GWP-ASAN applications of this device overflowed after last boot.";
        AniUtil::ThrowErrorMessage(env, paramErrorMessage, ErrorCode::OVER_ENABLE_LIMIT);
        return;
    }
    return;
}

void DisableGwpAsanGrayscaleAni(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("disableGwpAsanGrayscale");
    DisableGwpAsanGrayscale();
    return;
}

ani_int GetGwpAsanGrayscaleStateAni(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getGwpAsanGrayscaleState");
    auto result = GetGwpAsanGrayscaleState();
    ani_int ret = static_cast<ani_int>(result);
    return ret;
}

bool DumpLimitEnable()
{
    return !IsDeveloperOptionsEnabled() && !IsDebuggableHap() && !IsBetaVersion();
}

bool InitRawHeapFile(std::string& fileName)
{
    std::string filePath = GetProcessDir(DirectoryType::CACHE);
    if (filePath.empty()) {
        return false;
    }
    fileName = filePath + SLASH_STR + RAW_HEAP_RECORD_FILE;
    if (OHOS::FileExists(fileName)) {
        return true;
    }
    constexpr unsigned fileMode = 0700;
    return CreateDirectory(fileName, fileMode);
}

std::vector<std::string> LoadDumpRecords()
{
    std::vector<std::string> records;
    std::string rawHeapFile;
    if (!InitRawHeapFile(rawHeapFile)) {
        return records;
    }
    std::string quota;
    if (!GetXAttr(rawHeapFile, HIDEBUG_DUMP_QUOTA, quota, QUOTA_VALUE_LENGTH)) {
        return records;
    }
    const int64_t currentTime = GetRealNanoSecondsTimestamp();
    constexpr int64_t timeout = 24 * 60 * 60 * S_TO_NS;
    const int64_t validTime = currentTime - timeout;
    return SplitStr(quota, ',', [validTime, currentTime](std::string& record) {
        if (record.empty()) {
            return false;
        }
        constexpr int decBase = 10;
        int64_t num = std::strtoll(record.c_str(), nullptr, decBase);
        return num > validTime && num < currentTime;
    });
}

bool InsertDumpRecord()
{
    std::vector<std::string> records = LoadDumpRecords();
    records.emplace_back(std::to_string(GetRealNanoSecondsTimestamp()));
    std::string recordStr;
    for (const auto& record : records) {
        recordStr += (record + ",");
    }
    std::string rawHeapFile;
    if (!InitRawHeapFile(rawHeapFile)) {
        return false;
    }
    return SetXAttr(rawHeapFile, HIDEBUG_DUMP_QUOTA, recordStr);
}

bool ReportDumpSuccess()
{
    int32_t ret = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK,
        "ARK_STATS_DUMP",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "TYPE", "hidebugDump");
    if (ret != 0) {
        HILOG_ERROR(LOG_CORE, "failed to report dump success ret %{public}d.", ret);
    }
    return true;
}

bool CheckDeviceQuota()
{
    return OHOS::system::GetIntParameter(DUMP_MAX_COUNT, 0) > 0;
}

bool CheckProcessQuota()
{
    int limit = OHOS::system::GetIntParameter(PROCESS_DUMP_MAX_COUNT, 0);
    if (limit <= 0) {
        return false;
    }
    auto size = LoadDumpRecords().size();
    if (size >= static_cast<size_t>(limit)) {
        return false;
    }
    return true;
}

bool CheckDumpDiskSpace()
{
    struct statvfs stat{};
    std::string appDir = GetProcessDir(DirectoryType::FILE);
    if (statvfs(appDir.c_str(), &stat) != 0) {
        return false;
    }
    constexpr uint64_t dumpNeedSpace = 30ULL * 1024 * 1024 * 1024;
    uint64_t freeSize = stat.f_bsize * stat.f_bfree;
    if (freeSize <= dumpNeedSpace) {
        HILOG_ERROR(LOG_CORE, "disk space is not enough, remains %{public}" PRIu64 "B.", freeSize);
        return false;
    }
    return true;
}

std::string GenerateDumpFile(int64_t currentTime)
{
    std::string filesDir = GetProcessDir(DirectoryType::FILE);
    if (filesDir.empty()) {
        return "";
    }
    return filesDir + SLASH_STR + "hidebug-jsheap-" + std::to_string(getpid()) + "-" + std::to_string(gettid()) +
        "-" + std::to_string(currentTime / MS_TO_NS) + RAW_HEAP_FILE_EXT;
}

std::string GenerateStaticDumpFile(int64_t currentTime)
{
    std::string filesDir = GetProcessDir(DirectoryType::FILE);
    if (filesDir.empty()) {
        return "";
    }
    return filesDir + SLASH_STR + "hidebug-static_jsheap-" + std::to_string(getpid()) +
        "-" + std::to_string(currentTime / MS_TO_NS) + RAW_HEAP_FILE_EXT;
}

bool AppendMetaData(const std::string& newPath)
{
    auto rawHeapFileSize = static_cast<uint32_t>(GetFileSize(newPath));
    if (rawHeapFileSize == 0) {
        HILOG_ERROR(LOG_CORE, "%{public}s is not existed or empty.", newPath.c_str());
        return false;
    }
    constexpr auto metaDataPath = "/system/lib64/module/arkcompiler/metadata.json";
    auto metaDataFileSize = static_cast<uint32_t>(GetFileSize(metaDataPath));
    if (metaDataFileSize == 0) {
        HILOG_ERROR(LOG_CORE, "%{public}s is not existed or empty.", metaDataPath);
        return false;
    }
    auto targetFile = SmartFile::OpenFile(newPath, "ab");
    if (targetFile == nullptr) {
        return false;
    }
    auto metaData = SmartFile::OpenFile(metaDataPath, "rb");
    if (metaData == nullptr) {
        return false;
    }
    constexpr auto buffSize = 1024;
    char buffer[buffSize];
    size_t bytesRead;
    while ((bytesRead = metaData->Read(buffer, 1, buffSize)) > 0) {
        if (!targetFile->Write(buffer, 1, bytesRead)) {
            return false;
        }
    }
    return targetFile->Write(&rawHeapFileSize, sizeof(rawHeapFileSize), 1) &&
        targetFile->Write(&metaDataFileSize, sizeof(metaDataFileSize), 1);
}

int TransferRetCodeToErrorCode(int retCode)
{
    constexpr int dumpSuccess = 0;
    constexpr int forkFailed = 1;
    constexpr int failedToWait = 2;
    constexpr int waitTimeOut = 3;
    switch (retCode) {
        case dumpSuccess:
            return 0;
        case forkFailed:
            return DumpRawHeapErrors::FORK_FAILED;
        case failedToWait:
            return DumpRawHeapErrors::FAILED_WAIT_CHILD_PROCESS_FINISHED;
        case waitTimeOut:
            return DumpRawHeapErrors::TIMEOUT_WAIT_CHILD_PROCESS_FINISHED;
        default:
            HILOG_ERROR(LOG_CORE, "unknown retCode %{public}d.", retCode);
            return DumpRawHeapErrors::TIMEOUT_WAIT_CHILD_PROCESS_FINISHED;
    }
}
}

static ani_status BindGwpMethods(ani_env *env, ani_namespace nameSpace)
{
    std::array gwpMethods = {
        ani_native_function {"enableGwpAsanGrayscale", nullptr,
            reinterpret_cast<void *>(EnableGwpAsanGrayscaleAni)},
        ani_native_function {"disableGwpAsanGrayscale", ":",
            reinterpret_cast<void *>(DisableGwpAsanGrayscaleAni)},
        ani_native_function {"getGwpAsanGrayscaleState", ":i",
            reinterpret_cast<void *>(GetGwpAsanGrayscaleStateAni)},
    };
    return env->Namespace_BindNativeFunctions(nameSpace, gwpMethods.data(), gwpMethods.size());
}

ani_object GetRssInfoAdapter(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getRssInfoAdapter");
    auto rssInfoOption = HidebugNativeInterface::GetInstance().GetRssInfo();
    if (!rssInfoOption) {
        constexpr const char* errMsg = "Failed to get the application memory due to a remote exception";
        AniUtil::ThrowErrorMessage(env, errMsg, ErrorCode::SYSTEM_STATUS_ABNORMAL);
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return AniUtil::CreateUndefined(env);
    }

    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_object rssInfo = nullptr;
    if (env->FindClass("@ohos.hidebug.hidebug.RssInfoAdapter", &cls) != ANI_OK ||
        env->Class_FindMethod(cls, "<ctor>", ":", &ctorMethod) != ANI_OK ||
        env->Object_New(cls, ctorMethod, &rssInfo) != ANI_OK) {
        return AniUtil::CreateUndefined(env);
    }
    env->Object_SetPropertyByName_Long(rssInfo, "rss", static_cast<ani_long>(rssInfoOption->rss));
    env->Object_SetPropertyByName_Long(rssInfo, "swapRss", static_cast<ani_long>(rssInfoOption->swapRss));
    return rssInfo;
}

ani_string GetAppRunningUniqueId(ani_env *env)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppRunningUniqueId");
    const char* id = HidebugNativeInterface::GetInstance().GetAppRunningUniqueId();
    std::string idStr = (id != nullptr) ? std::string(id) : "";
    ani_string result = nullptr;
    env->String_NewUTF8(idStr.c_str(), idStr.size(), &result);
    return result;
}

void DumpJsHeapDataAdapter(ani_env *env, ani_string fileNameAni, ani_boolean needClean)
{
    ApiInvokeRecorder apiInvokeRecorder("dumpJsHeapData");
    std::string fileName;
    std::string paramErrMsg;
    if (!GetTheOnlyStringParam(env, fileNameAni, fileName, &paramErrMsg)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, paramErrMsg, ErrorCode::PARAMETER_ERROR);
        return;
    }
    std::string filesDir = GetProcessDir(DirectoryType::FILE);
    if (filesDir.empty()) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        AniUtil::ThrowErrorMessage(env, "Get App files dir failed.", ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return;
    }
    std::string filePath = filesDir + SLASH_STR + fileName + HEAPSNAPSHOT_FILE;
    if (!IsLegalPath(filePath)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        AniUtil::ThrowErrorMessage(env, "input fileName is illegal.", ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return;
    }
    if (!CreateFile(filePath)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        AniUtil::ThrowErrorMessage(env, "file created failed.", ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return;
    }
    panda::DumpSnapShotOption dumpOption;
    dumpOption.dumpFormat = panda::DumpFormat::JSON;
    dumpOption.isVmMode = true;
    dumpOption.isPrivate = false;
    dumpOption.captureNumericValue = true;
    dumpOption.isFullGC = true;
    dumpOption.isSync = true;
    dumpOption.isClearNodeIdCache = needClean == ANI_TRUE;
    dumpOption.languageEnv = panda::ecmascript::LanguageEnv::HYBRID;
    if (!panda::DFXJSNApi::DumpHybridHeapSnapshot(filePath, dumpOption)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        AniUtil::ThrowErrorMessage(env, "dump heap snapshot failed.", ErrorCode::SYSTEM_STATUS_ABNORMAL);
    }
}

// 重复调用保护计数器（dump 期间拒绝并发调用）
static std::atomic<int> g_dumpingCount{0};

static ani_vm *g_vm = nullptr;

constexpr ani_size LOCAL_SCOPE_REF_CAPACITY = 4096;

class AniThreadEnvGuard {
public:
    explicit AniThreadEnvGuard(ani_vm *vm) : vm_(vm)
    {
        if (vm_ == nullptr) {
            return;
        }
        isTemporary = (vm_->GetEnv(ANI_VERSION_1, &env_) != ANI_OK);
        if (isTemporary) {
            if (vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env_) != ANI_OK) {
                HILOG_ERROR(LOG_CORE, "AniThreadEnvGuard: AttachCurrentThread failed");
                env_ = nullptr;
                return;
            }
            env_->CreateLocalScope(LOCAL_SCOPE_REF_CAPACITY);
        }
    }
    ~AniThreadEnvGuard()
    {
        if (isTemporary && env_ != nullptr) {
            env_->DestroyLocalScope();
            vm_->DetachCurrentThread();
        }
    }
    ani_env *GetEnv() const { return env_; }

private:
    ani_vm *vm_;
    ani_env *env_ = nullptr;
    bool isTemporary = false;
};

static ani_error CreateBusinessError(ani_env *env, int32_t errCode, const std::string &msg)
{
    ani_class cls {};
    constexpr auto businessErrorClassName = "@ohos.base.BusinessError";
    if (env->FindClass(businessErrorClassName, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "CreateBusinessError: find class %{public}s failed", businessErrorClassName);
        return nullptr;
    }
    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", ":", &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "CreateBusinessError: find BusinessError ctor failed");
        return nullptr;
    }
    ani_object error {};
    if (env->Object_New(cls, ctor, &error) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "CreateBusinessError: new BusinessError object failed");
        return nullptr;
    }
    env->Object_SetPropertyByName_Int(error, "code_", static_cast<ani_int>(errCode));
    ani_string messageRef {};
    if (env->String_NewUTF8(msg.c_str(), msg.size(), &messageRef) == ANI_OK) {
        env->Object_SetPropertyByName_Ref(error, "message", static_cast<ani_ref>(messageRef));
    }
    return static_cast<ani_error>(error);
}

static std::string GetDumpRawHeapErrMsg(int32_t errCode)
{
    for (const auto &[code, msg] : DumpRawHeapErrCodeMap) {
        if (code == errCode) {
            return msg;
        }
    }
    return "dumpJsRawHeapData failed";
}

int32_t CheckDumpJsRawHeapPreconditions(ApiInvokeRecorder &recorder)
{
    if (g_dumpingCount.fetch_add(1) != 0) {
        HILOG_ERROR(LOG_CORE, "DumpRawHeap is already running, reject repeated request.");
        recorder.SetErrorCode(DumpRawHeapErrors::REPEAT_DUMPING);
        return DumpRawHeapErrors::REPEAT_DUMPING;
    }
    if (!CheckDumpDiskSpace()) {
        recorder.SetErrorCode(DumpRawHeapErrors::LOW_DISK_SPACE);
        return DumpRawHeapErrors::LOW_DISK_SPACE;
    }
    if (DumpLimitEnable() && (!CheckDeviceQuota() || !CheckProcessQuota())) {
        recorder.SetErrorCode(DumpRawHeapErrors::QUOTA_EXCEEDED);
        return DumpRawHeapErrors::QUOTA_EXCEEDED;
    }
    return 0;
}

struct DumpFilePaths {
    std::string dynamicPath;
    std::string staticPath;
};

DumpFilePaths PrepareDumpFile(ApiInvokeRecorder &recorder, int32_t &errCode)
{
    errCode = 0;
    int64_t currentTime = GetRealNanoSecondsTimestamp();
    DumpFilePaths paths;
    paths.dynamicPath = GenerateDumpFile(currentTime);
    paths.staticPath = GenerateStaticDumpFile(currentTime);
    if (paths.dynamicPath.empty() || paths.staticPath.empty()) {
        recorder.SetErrorCode(DumpRawHeapErrors::FAILED_CREATE_FILE);
        errCode = DumpRawHeapErrors::FAILED_CREATE_FILE;
        g_dumpingCount.fetch_sub(1);
        return {};
    }
    if (!CreateFile(paths.dynamicPath) || !CreateFile(paths.staticPath)) {
        recorder.SetErrorCode(DumpRawHeapErrors::FAILED_CREATE_FILE);
        errCode = DumpRawHeapErrors::FAILED_CREATE_FILE;
        g_dumpingCount.fetch_sub(1);
        return {};
    }
    return paths;
}

struct DumpHeapParams {
    bool isGc;
    bool isNeedClean;
    bool isProcDump;
};

void PostDumpSuccess(const std::string& filePath)
{
    if (!AppendMetaData(filePath)) {
        HILOG_ERROR(LOG_CORE, "failed to append metadata to dump file.");
    }
    if (DumpLimitEnable()) {
        if (!InsertDumpRecord()) {
            HILOG_ERROR(LOG_CORE, "failed to insert process dump record.");
        }
        if (!ReportDumpSuccess()) {
            HILOG_ERROR(LOG_CORE, "failed to report dump success event.");
        }
    }
}

struct DumpRawHeapContext {
    ani_resolver deferred;
    std::string dynamicFilePath;
    std::string staticFilePath;
    DumpHeapParams params;
    std::shared_ptr<ApiInvokeRecorder> recorder;
};

static void OnDumpHeapSnapshotDone(std::shared_ptr<DumpRawHeapContext> ctx, uint8_t retCode)
{
    AniThreadEnvGuard guard(g_vm);
    ani_env *cbEnv = guard.GetEnv();
    if (cbEnv == nullptr) {
        HILOG_ERROR(LOG_CORE, "OnDumpHeapSnapshotDone: get ani_env failed");
        ctx->recorder->SetErrorCode(DumpRawHeapErrors::TIMEOUT_WAIT_CHILD_PROCESS_FINISHED);
        g_dumpingCount.fetch_sub(1);
        return;
    }
    if (retCode != 0) {
        int32_t errCode = TransferRetCodeToErrorCode(retCode);
        ctx->recorder->SetErrorCode(errCode);
        remove(ctx->dynamicFilePath.c_str());
        remove(ctx->staticFilePath.c_str());
        g_dumpingCount.fetch_sub(1);
        ani_error err = CreateBusinessError(cbEnv, errCode, GetDumpRawHeapErrMsg(errCode));
        cbEnv->PromiseResolver_Reject(ctx->deferred, err);
        return;
    }
    PostDumpSuccess(ctx->dynamicFilePath);
    ani_string dynamicResult = nullptr;
    ani_string staticResult = nullptr;
    if (cbEnv->String_NewUTF8(ctx->dynamicFilePath.c_str(), ctx->dynamicFilePath.size(), &dynamicResult) != ANI_OK ||
        cbEnv->String_NewUTF8(ctx->staticFilePath.c_str(), ctx->staticFilePath.size(), &staticResult) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "OnDumpHeapSnapshotDone: String_NewUTF8 failed");
        g_dumpingCount.fetch_sub(1);
        ani_error err = CreateBusinessError(cbEnv, ErrorCode::SYSTEM_STATUS_ABNORMAL,
            "Create dump file path string failed");
        cbEnv->PromiseResolver_Reject(ctx->deferred, err);
        return;
    }
    ani_array arr = nullptr;
    ani_ref initElem = cbEnv->GetUndefined(&initElem) == ANI_OK ? initElem : nullptr;
    if (cbEnv->Array_New(RAW_HEAP_RESULT_ARRAY_SIZE, initElem, &arr) == ANI_OK) {
        cbEnv->Array_Set(arr, 0, static_cast<ani_ref>(dynamicResult));
        cbEnv->Array_Set(arr, 1, static_cast<ani_ref>(staticResult));
        g_dumpingCount.fetch_sub(1);
        cbEnv->PromiseResolver_Resolve(ctx->deferred, static_cast<ani_ref>(arr));
        return;
    }
    HILOG_ERROR(LOG_CORE, "OnDumpHeapSnapshotDone: Array_New failed, reject");
    g_dumpingCount.fetch_sub(1);
    ani_error err = CreateBusinessError(cbEnv, ErrorCode::SYSTEM_STATUS_ABNORMAL,
        "Failed to create result array");
    cbEnv->PromiseResolver_Reject(ctx->deferred, err);
}

ani_object DumpJsRawHeapDataAdapter(ani_env *env, ani_boolean needGC, ani_boolean needClean, ani_boolean processDump)
{
    auto apiInvokeRecorder = std::make_shared<ApiInvokeRecorder>("dumpJsRawHeapData");
    ani_resolver deferred = nullptr;
    ani_object promise = nullptr;
    if (env->Promise_New(&deferred, &promise) != ANI_OK || deferred == nullptr || promise == nullptr) {
        HILOG_ERROR(LOG_CORE, "DumpJsRawHeapDataAdapter: Promise_New failed");
        AniUtil::ThrowErrorMessage(env, "Promise_New failed", ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return nullptr;
    }
    int32_t preErrCode = CheckDumpJsRawHeapPreconditions(*apiInvokeRecorder);
    if (preErrCode != 0) {
        g_dumpingCount.fetch_sub(1);
        ani_error err = CreateBusinessError(env, preErrCode, GetDumpRawHeapErrMsg(preErrCode));
        env->PromiseResolver_Reject(deferred, err);
        return promise;
    }
    int32_t prepErrCode = 0;
    DumpFilePaths paths = PrepareDumpFile(*apiInvokeRecorder, prepErrCode);
    if (prepErrCode != 0) {
        ani_error err = CreateBusinessError(env, prepErrCode, GetDumpRawHeapErrMsg(prepErrCode));
        env->PromiseResolver_Reject(deferred, err);
        return promise;
    }
    auto ctx = std::make_shared<DumpRawHeapContext>();
    ctx->deferred = deferred;
    ctx->dynamicFilePath = paths.dynamicPath;
    ctx->staticFilePath = paths.staticPath;
    ctx->params = {needGC == ANI_TRUE, needClean == ANI_TRUE, processDump == ANI_TRUE};
    ctx->recorder = apiInvokeRecorder;

    auto callback = [ctx](uint8_t code) { OnDumpHeapSnapshotDone(ctx, code); };
    panda::DumpSnapShotOption rawDumpOption;
    rawDumpOption.dumpFormat = panda::DumpFormat::BINARY;
    rawDumpOption.isVmMode = true;
    rawDumpOption.isPrivate = false;
    rawDumpOption.isFullGC = ctx->params.isGc;
    rawDumpOption.isSync = false;
    rawDumpOption.isClearNodeIdCache = ctx->params.isNeedClean;
    rawDumpOption.isProcDump = ctx->params.isProcDump;
    rawDumpOption.languageEnv = panda::ecmascript::LanguageEnv::HYBRID;
    if (!panda::DFXJSNApi::DumpHybridRawHeapSnapshot(ctx->dynamicFilePath, ctx->staticFilePath,
        rawDumpOption, callback)) {
        HILOG_ERROR(LOG_CORE, "DumpHybridRawHeapSnapshot failed.");
        // 1 = DumpHeapSnapshotStatus::FORK_FAILED (enum in heap_profiler.h, cannot include due to platform headers)
        callback(1);
    }
    return promise;
}

void SetJsRawHeapTrimLevelAdapter([[maybe_unused]] ani_env *env, ani_int level)
{
    ApiInvokeRecorder apiInvokeRecorder("setJsRawHeapTrimLevel");
    if (level < 0 || level > 1) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        AniUtil::ThrowErrorMessage(env, "Param type error, invalid level.", ErrorCode::PARAMETER_ERROR);
        return;
    }
    panda::DFXJSNApi::SetJsRawHeapCropLevel(static_cast<panda::CropLevel>(level));
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    g_vm = vm;
    ani_env *env = nullptr;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    ani_namespace nameSpace = nullptr;
    if (ANI_OK != env->FindNamespace("@ohos.hidebug.hidebug", &nameSpace)) {
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"getAppVMMemoryInfoAdapter", ":C{@ohos.hidebug.hidebug.VMMemoryInfoAdapter}",
            reinterpret_cast<void *>(GetAppVMMemoryInfo)},
        ani_native_function {"getAppVMObjectUsedSizeAdapter", ":l", reinterpret_cast<void *>(GetAppVMObjectUsedSize)},
        ani_native_function {"getMemoryLimitImplAdapter", ":C{@ohos.hidebug.hidebug.MemoryLimitImplAdapter}",
            reinterpret_cast<void *>(GetAppMemoryLimit)},
        ani_native_function {"getVMRuntimeStats", ":C{std.core.Record}", reinterpret_cast<void *>(GetVMRuntimeStats)},
        ani_native_function {"getVMRuntimeStat", "C{std.core.String}:l", reinterpret_cast<void *>(GetVMRuntimeStat)},
        ani_native_function {"getPssAdapter", ":l", reinterpret_cast<void *>(GetPss)},
        ani_native_function {"getSharedDirtyAdapter", ":l", reinterpret_cast<void *>(GetSharedDirty)},
        ani_native_function {"getPrivateDirtyAdapter", ":l", reinterpret_cast<void *>(GetPrivateDirty)},
        ani_native_function {"getCpuUsage", ":d", reinterpret_cast<void *>(GetCpuUsage)},
        ani_native_function {"getServiceDump", "iiC{std.core.Array}:", reinterpret_cast<void *>(GetServiceDump)},
        ani_native_function {"getNativeHeapSizeAdapter", ":l", reinterpret_cast<void *>(GetNativeHeapSize)},
        ani_native_function {"getNativeHeapAllocatedSizeAdapter", ":l",
            reinterpret_cast<void *>(GetNativeHeapAllocatedSize)},
        ani_native_function {"getNativeHeapFreeSizeAdapter", ":l",
            reinterpret_cast<void *>(GetNativeHeapFreeSize)},
        ani_native_function {"getVssAdapter", ":l", reinterpret_cast<void *>(GetVss)},
        ani_native_function {"getAppRunningUniqueId", ":C{std.core.String}",
            reinterpret_cast<void *>(GetAppRunningUniqueId)},
        ani_native_function {"getAppThreadCpuUsage", ":C{std.core.Array}",
            reinterpret_cast<void *>(GetAppThreadCpuUsage)},
        ani_native_function {"getSystemCpuUsage", ":d", reinterpret_cast<void *>(GetSystemCpuUsage)},
        ani_native_function {"getAppNativeMemInfoAdapter", "z:C{@ohos.hidebug.hidebug.NativeMemInfoAdapter}",
            reinterpret_cast<void *>(GetAppNativeMemInfoAdapter)},
        ani_native_function {"getSystemMemInfoAdapter", ":C{@ohos.hidebug.hidebug.SystemMemInfoAdapter}",
            reinterpret_cast<void *>(GetSystemMemInfo)},
        ani_native_function {"startAppTraceCapture",
            "C{std.core.Array}C{@ohos.hidebug.hidebug.TraceFlag}i:C{std.core.String}",
            reinterpret_cast<void *>(StartAppTraceCapture)},
        ani_native_function {"requestTraceAdapter", "C{std.core.String}iii:C{std.core.String}",
            reinterpret_cast<void *>(RequestTraceAdapter)},
        ani_native_function {"isDebugState", ":z", reinterpret_cast<void *>(IsDebugState)},
        ani_native_function {"stopAppTraceCapture", ":", reinterpret_cast<void *>(StopAppTraceCapture)},
        ani_native_function {"startJsCpuProfiling", "C{std.core.String}:",
            reinterpret_cast<void *>(StartJsCpuProfiling)},
        ani_native_function {"stopJsCpuProfiling", ":", reinterpret_cast<void *>(StopJsCpuProfiling)},
        ani_native_function {"getGraphicsMemoryAdapter", "i:C{@ohos.hidebug.hidebug.GraphicsMemorySummary}",
            reinterpret_cast<void *>(GetGraphicsMemoryAdapter)},
        ani_native_function {"getRssInfoAdapter", ":C{@ohos.hidebug.hidebug.RssInfoAdapter}",
            reinterpret_cast<void *>(GetRssInfoAdapter)},
        ani_native_function {"dumpJsHeapDataAdapter", "C{std.core.String}z:",
            reinterpret_cast<void *>(DumpJsHeapDataAdapter)},
        ani_native_function {"dumpJsRawHeapDataAdapter", "zzz:C{std.core.Promise}",
            reinterpret_cast<void *>(DumpJsRawHeapDataAdapter)},
        ani_native_function {"setJsRawHeapTrimLevelAdapter", "i:",
            reinterpret_cast<void *>(SetJsRawHeapTrimLevelAdapter)},
    };
    auto retCode = env->Namespace_BindNativeFunctions(nameSpace, methods.data(), methods.size());
    if (ANI_OK != retCode) {
        return retCode;
    }
    retCode = BindGwpMethods(env, nameSpace);
    if (ANI_OK != retCode) {
        return retCode;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
