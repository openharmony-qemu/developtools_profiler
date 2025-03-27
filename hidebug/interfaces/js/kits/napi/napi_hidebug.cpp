/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <parameters.h>
#include <unistd.h>

#include "application_context.h"
#include "context.h"
#include "cpu_collector.h"
#include "directory_ex.h"
#include "dump_usage.h"
#include "file_ex.h"
#include "hiappevent_util.h"
#include "hidebug_native_interface.h"
#include "hidebug_util.h"
#include "hilog/log.h"
#include "iservice_registry.h"
#include "memory_collector.h"
#include "napi_hidebug_dump.h"
#include "napi_hidebug_init.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"
#include "refbase.h"
#include "storage_acl.h"
#include "system_ability_definition.h"
#include "napi_hidebug_gc.h"
#include "napi_util.h"
#include "error_code.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_NAPI"
constexpr int ONE_VALUE_LIMIT = 1;
constexpr int ARRAY_INDEX_FIRST = 0;
constexpr int ARRAY_INDEX_SECOND = 1;
constexpr int REMOVE_NAPI_WRAP_PARAM_COUNT = 2;
constexpr int NAME_LEN = 128;
constexpr int BYTE_2_KB_SHIFT_BITS = 10;
constexpr int FIRST_POS = 0;
constexpr int SECOND_POS = 1;
constexpr int THIRD_POS = 2;
constexpr int PSS_MIN = 1024;
constexpr int PSS_MAX = 4 * 1024 * 1024;
constexpr int JS_MIN = 85;
constexpr int JS_MAX = 95;
constexpr int FD_MIN = 10;
constexpr int FD_MAX = 10000;
constexpr int THREAD_MIN = 1;
constexpr int THREAD_MAX = 1000;
const std::string SLASH_STR = "/";
const std::string JSON_FILE = ".json";
const std::string KEY_HIVIEW_DEVELOP_TYPE = "persist.hiview.leak_detector";

const std::unordered_set<std::string> RESOURCE_TYPE_LIST{
    "pss_memory",
    "js_heap",
    "fd",
    "thread"
};
static std::map<std::string, std::pair<int, int>> limitResource = {
    {{"pss_memory", {PSS_MIN, PSS_MAX}}, {"js_heap", {JS_MIN, JS_MAX}},
        {"fd", {FD_MIN, FD_MAX}}, {"thread", {THREAD_MIN, THREAD_MAX}}}
};
}

static bool IsArrayForNapiValue(napi_env env, napi_value param, uint32_t &arraySize)
{
    bool isArray = false;
    arraySize = 0;
    if (napi_is_array(env, param, &isArray) != napi_ok || isArray == false) {
        return false;
    }
    if (napi_get_array_length(env, param, &arraySize) != napi_ok) {
        return false;
    }
    return true;
}

static bool GetDumpParam(napi_env env, napi_callback_info info,
    int& serviceId, int& fd, std::vector<std::u16string>& args)
{
    const int valueNum = 3;
    size_t argc = valueNum;
    napi_value argv[valueNum] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc != valueNum) {
        HILOG_ERROR(LOG_CORE, "invalid number = %{public}d of params.", ONE_VALUE_LIMIT);
        return false;
    }
    int thirdPos = 2;
    if (!MatchValueType(env, argv[0], napi_number) &&
        !MatchValueType(env, argv[1], napi_number) &&
        !MatchValueType(env, argv[thirdPos], napi_object)) {
        HILOG_ERROR(LOG_CORE, "params type error.");
        return false;
    }
    if (napi_get_value_int32(env, argv[0], &serviceId) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "Get input serviceId failed.");
        return false;
    }
    if (napi_get_value_int32(env, argv[1], &fd) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "Get input fd failed.");
        return false;
    }
    uint32_t arraySize = 0;
    if (!IsArrayForNapiValue(env, argv[thirdPos], arraySize)) {
        HILOG_ERROR(LOG_CORE, "Get input args failed.");
        return false;
    }
    for (uint32_t i = 0; i < arraySize; i++) {
        napi_value jsValue = nullptr;
        if (napi_get_element(env, argv[thirdPos], i, &jsValue) != napi_ok) {
            HILOG_ERROR(LOG_CORE, "get_element -> Get input args failed.");
            return false;
        }
        const size_t bufSize = 256;
        size_t bufLen = 0;
        char buf[bufSize] = {0};
        if (napi_get_value_string_utf8(env, jsValue, buf, bufSize - 1, &bufLen) != napi_ok) {
            HILOG_ERROR(LOG_CORE, "get_value -> Get input args failed.");
            return false;
        }
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> strCnv;
        args.push_back(strCnv.from_bytes(buf));
    }
    return true;
}

static bool GetTraceParam(napi_env env, napi_callback_info info,
    uint32_t& traceFlag, uint32_t& limitSize, std::vector<uint64_t>& tags)
{
    const int valueNum = 3;
    size_t argc = valueNum;
    napi_value argv[valueNum] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != valueNum) {
        HILOG_ERROR(LOG_CORE, "invalid numbers of params!");
        return false;
    }
    if (!MatchValueType(env, argv[FIRST_POS], napi_object) &&
        !MatchValueType(env, argv[SECOND_POS], napi_number) &&
        !MatchValueType(env, argv[THIRD_POS], napi_number)) {
        HILOG_ERROR(LOG_CORE, "params type error.");
        return false;
    }
    uint32_t arraySize = 0;
    if (!IsArrayForNapiValue(env, argv[FIRST_POS], arraySize)) {
        HILOG_ERROR(LOG_CORE, "Get input tags failed.");
        return false;
    }
    uint64_t tag = 0;
    bool lossless = true;
    for (uint32_t i = 0; i < arraySize; ++i) {
        napi_value jsValue = nullptr;
        if (napi_get_element(env, argv[FIRST_POS], i, &jsValue) != napi_ok) {
            HILOG_ERROR(LOG_CORE, "get_element -> Get input tags failed.");
            return false;
        }
        if (napi_get_value_bigint_uint64(env, jsValue, &tag, &lossless) != napi_ok) {
            HILOG_ERROR(LOG_CORE, "Get input tags failed.");
            return false;
        }
        tags.push_back(tag);
    }
    if (napi_get_value_uint32(env, argv[SECOND_POS], &traceFlag) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "Get input traceFlag failed.");
        return false;
    }
    if (napi_get_value_uint32(env, argv[THIRD_POS], &limitSize) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "Get input limitSize failed.");
        return false;
    }
    return true;
}

napi_value StartProfiling(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("startProfiling");
    std::string fileName = GetFileNameParam(env, info);
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateErrorMessage(env, "Get ApplicationContext failed.");
    }
    std::string filesDir = context->GetFilesDir();
    if (filesDir.empty()) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateErrorMessage(env, "Get App files dir failed.");
    }
    std::string filePath = filesDir + SLASH_STR + fileName + JSON_FILE;
    if (!IsLegalPath(filePath)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateErrorMessage(env, "input fileName is illegal.");
    }
    if (!CreateFile(filePath)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateErrorMessage(env, "file created failed.");
    }
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->StartCpuProfiler(filePath);
    return CreateUndefined(env);
}

napi_value StartJsCpuProfiling(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("startJsCpuProfiling");
    std::string fileName;
    if (!GetTheOnlyStringParam(env, info, fileName)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        std::string paramErrorMessage = "Invalid parameter, require a string parameter.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    HILOG_INFO(LOG_CORE, "filename: %{public}s.", fileName.c_str());
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateErrorMessage(env, "Get ApplicationContext failed.");
    }
    std::string filesDir = context->GetFilesDir();
    if (filesDir.empty()) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateErrorMessage(env, "Get App files dir failed.");
    }
    std::string filePath = filesDir + SLASH_STR + fileName + JSON_FILE;
    if (!IsLegalPath(filePath)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateErrorMessage(env, "input fileName is illegal.");
    }
    if (!CreateFile(filePath)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateErrorMessage(env, "file created failed.");
    }
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->StartCpuProfiler(filePath);
    return CreateUndefined(env);
}

napi_value StopProfiling(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("stopProfiling");
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->StopCpuProfiler();
    return CreateUndefined(env);
}

napi_value StopJsCpuProfiling(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("stopJsCpuProfiling");
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->StopCpuProfiler();
    return CreateUndefined(env);
}

napi_value GetPss(napi_env env, napi_callback_info info)
{
    napi_value pss;
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (collector != nullptr) {
        int pid = getprocpid();
        auto collectResult = collector->CollectProcessMemory(pid);
        int32_t pssInfo = collectResult.data.pss + collectResult.data.swapPss;
        napi_create_bigint_uint64(env, pssInfo, &pss);
    } else {
        napi_create_bigint_uint64(env, 0, &pss);
    }
    return pss;
}

napi_value GetSharedDirty(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getSharedDirty");
    napi_value sharedDirty;
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (collector != nullptr) {
        int pid = getprocpid();
        auto collectResult = collector->CollectProcessMemory(pid);
        int32_t sharedDirtyInfo = collectResult.data.sharedDirty;
        napi_create_bigint_uint64(env, sharedDirtyInfo, &sharedDirty);
    } else {
        napi_create_bigint_uint64(env, 0, &sharedDirty);
    }
    return sharedDirty;
}

napi_value GetPrivateDirty(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getPrivateDirty");
    napi_value privateDirtyValue;
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (collector != nullptr) {
        pid_t pid = getprocpid();
        auto collectResult = collector->CollectProcessMemory(pid);
        int32_t privateDirty = collectResult.data.privateDirty;
        napi_create_bigint_uint64(env, privateDirty, &privateDirtyValue);
    } else {
        napi_create_bigint_uint64(env, 0, &privateDirtyValue);
    }
    return privateDirtyValue;
}

napi_value GetCpuUsage(napi_env env, napi_callback_info info)
{
    constexpr uint32_t reportTimeOutSec = 5 * 60;
    constexpr uint32_t limitSize = 100;
    static MultipleRecordReporter multipleRecordReporter(reportTimeOutSec, limitSize);
    ApiInvokeRecorder apiInvokeRecorder("getCpuUsage", multipleRecordReporter);
    napi_value cpuUsageValue;
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    pid_t pid = getprocpid();
    double cpuUsage = dumpUsage->GetCpuUsage(pid);
    napi_create_double(env, cpuUsage, &cpuUsageValue);
    return cpuUsageValue;
}

napi_value GetNativeHeapSize(napi_env env, napi_callback_info info)
{
    struct mallinfo mi = mallinfo();
    napi_value nativeHeapSize;
    napi_create_bigint_uint64(env, uint64_t(mi.uordblks + mi.fordblks), &nativeHeapSize);
    return nativeHeapSize;
}

napi_value GetNativeHeapAllocatedSize(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getNativeHeapAllocatedSize");
    struct mallinfo mi = mallinfo();
    napi_value nativeHeapAllocatedSize;
    napi_create_bigint_uint64(env, uint64_t(mi.uordblks), &nativeHeapAllocatedSize);
    return nativeHeapAllocatedSize;
}

napi_value GetNativeHeapFreeSize(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getNativeHeapFreeSize");
    struct mallinfo mi = mallinfo();
    napi_value nativeHeapFreeSize;
    napi_create_bigint_uint64(env, uint64_t(mi.fordblks), &nativeHeapFreeSize);
    return nativeHeapFreeSize;
}

static napi_value GetServiceDump(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getServiceDump");
    int serviceAbilityId = 0;
    int fd = 0;
    std::vector<std::u16string> args;
    if (!GetDumpParam(env, info, serviceAbilityId, fd, args)) {
        std::string paramErrorMessage = "The parameter check failed.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!sam) {
        return CreateUndefined(env);
    }
    sptr<IRemoteObject> sa = sam->CheckSystemAbility(serviceAbilityId);
    if (sa == nullptr) {
        HILOG_ERROR(LOG_CORE, "no this system ability.");
        std::string idErrorMessage = "ServiceId invalid. The system ability does not exist.";
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_ABILITY_NOT_FOUND).c_str(), idErrorMessage.c_str());
        return CreateUndefined(env);
    }
    int dumpResult = sa->Dump(fd, args);
    HILOG_INFO(LOG_CORE, "Dump result: %{public}d", dumpResult);
    return CreateUndefined(env);
}

napi_value GetVss(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getVss");
    napi_value vss;
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    if (collector != nullptr) {
        pid_t pid = getprocpid();
        auto collectResult = collector->CollectProcessVss(pid);
        uint64_t vssInfo = collectResult.data;
        napi_create_bigint_uint64(env, vssInfo, &vss);
    } else {
        napi_create_bigint_uint64(env, 0, &vss);
    }
    return vss;
}

static napi_value GetSystemCpuUsage(napi_env env, napi_callback_info info)
{
    constexpr uint32_t reportTimeOutSec = 5 * 60;
    constexpr uint32_t limitSize = 100;
    static MultipleRecordReporter multipleRecordReporter(reportTimeOutSec, limitSize);
    ApiInvokeRecorder apiInvokeRecorder("getSystemCpuUsage", multipleRecordReporter);
    auto cpuUsageOptional = HidebugNativeInterface::GetInstance().GetSystemCpuUsage();
    if (!cpuUsageOptional) {
        std::string paramErrorMessage = "The status of the system CPU usage is abnormal.";
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    napi_value retMsg = nullptr;
    napi_create_double(env, cpuUsageOptional.value(),  &retMsg);
    return retMsg;
}

static napi_value RemoveNapiWrap(napi_env env, napi_callback_info info)
{
    size_t argc = REMOVE_NAPI_WRAP_PARAM_COUNT;
    napi_value argv[REMOVE_NAPI_WRAP_PARAM_COUNT] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc != REMOVE_NAPI_WRAP_PARAM_COUNT ||
        (!MatchValueType(env, argv[ARRAY_INDEX_FIRST], napi_object) ||
        !MatchValueType(env, argv[ARRAY_INDEX_SECOND], napi_boolean))) {
        HILOG_DEBUG(LOG_CORE, "RemoveNapiWrap Failed to parse parameters, argc %{public}d", (int)argc);
        std::string paramErrorMessage = "The parameter check failed.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }

    // remove jsObj's wrap
    auto jsObj = argv[ARRAY_INDEX_FIRST];
    void *nativePtr = nullptr;
    napi_remove_wrap(env, jsObj, (void **)&nativePtr);

    // remove jsObj's properties wrap
    bool needRemoveProperty = false;
    napi_get_value_bool(env, argv[ARRAY_INDEX_SECOND], &needRemoveProperty);
    if (needRemoveProperty) {
        napi_value allPropertyNames = nullptr;
        napi_object_get_keys(env, jsObj, &allPropertyNames);
        uint32_t nameCount = 0;
        napi_get_array_length(env, allPropertyNames, &nameCount);
        for (size_t i = 0; i < nameCount; ++i) {
            napi_value propertyName = nullptr;
            napi_get_element(env, allPropertyNames, i, &propertyName);
            char name[NAME_LEN] = {0};
            size_t len = 0;
            napi_get_value_string_utf8(env, propertyName, name, NAME_LEN, &len);
            napi_value propertyObj = nullptr;
            napi_get_named_property(env, jsObj, name, &propertyObj);
            napi_remove_wrap(env, propertyObj, (void **)&nativePtr);
        }
    }
    return CreateUndefined(env);
}

napi_value GetAppVMMemoryInfo(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppVMMemoryInfo");
    uint64_t allArraySizeValue{0};
    uint64_t heapUsedValue{0};
    uint64_t totalHeapValue{0};
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    if (engine) {
        allArraySizeValue = engine->GetArrayBufferSize();
        heapUsedValue = engine->GetHeapUsedSize();
        totalHeapValue = engine->GetHeapTotalSize();
    }

    napi_value vMMemoryInfo;
    napi_create_object(env, &vMMemoryInfo);

    napi_value totalHeap;
    totalHeapValue = totalHeapValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, totalHeapValue, &totalHeap);
    napi_set_named_property(env, vMMemoryInfo, "totalHeap", totalHeap);

    napi_value heapUsed;
    heapUsedValue = heapUsedValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, heapUsedValue, &heapUsed);
    napi_set_named_property(env, vMMemoryInfo, "heapUsed", heapUsed);

    napi_value allArraySize;
    allArraySizeValue = allArraySizeValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, allArraySizeValue, &allArraySize);
    napi_set_named_property(env, vMMemoryInfo, "allArraySize", allArraySize);

    return vMMemoryInfo;
}

static void ConvertThreadCpuUsageToJs(napi_env env, napi_value &result, uint32_t threadIdValue, double cpuUsageValue)
{
    napi_create_object(env, &result);

    napi_value threadId;
    napi_create_uint32(env, threadIdValue, &threadId);
    napi_set_named_property(env, result, "threadId", threadId);

    napi_value cpuUsage;
    napi_create_double(env, cpuUsageValue, &cpuUsage);
    napi_set_named_property(env, result, "cpuUsage", cpuUsage);
}

static void ConvertThreadCpuUsageMapToJs(napi_env env, napi_value &result, const std::map<uint32_t, double> &threadMap)
{
    napi_create_array(env, &result);
    size_t idx = 0;
    for (const auto[threadId, cpuUsage] : threadMap) {
        napi_value obj = nullptr;
        ConvertThreadCpuUsageToJs(env, obj, threadId, cpuUsage);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

napi_value GetAppThreadCpuUsage(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppThreadCpuUsage");
    napi_value result;
    std::map<uint32_t, double> threadMap = HidebugNativeInterface::GetInstance().GetAppThreadCpuUsage();
    ConvertThreadCpuUsageMapToJs(env, result, threadMap);
    return result;
}

napi_value GetAppMemoryLimit(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppMemoryLimit");
    napi_value appMemoryLimit;
    napi_create_object(env, &appMemoryLimit);
    MemoryLimit memoryLimit{};
    auto memoryLimitOption = HidebugNativeInterface::GetInstance().GetAppMemoryLimit();
    if (memoryLimitOption) {
        memoryLimit = memoryLimitOption.value();
    }
    napi_value rssLimit;
    napi_create_bigint_uint64(env, memoryLimit.rssLimit, &rssLimit);
    napi_set_named_property(env, appMemoryLimit, "rssLimit", rssLimit);

    napi_value vssLimit;
    napi_create_bigint_uint64(env, memoryLimit.vssLimit, &vssLimit);
    napi_set_named_property(env, appMemoryLimit, "vssLimit", vssLimit);
    uint64_t vmHeapLimitValue{0};
    uint64_t vmTotalHeapSizeValue{0};
    NativeEngine *engine = reinterpret_cast<NativeEngine *>(env);
    if (engine) {
        vmHeapLimitValue = engine->GetHeapLimitSize();
        vmTotalHeapSizeValue = engine->GetProcessHeapLimitSize();
    }
    napi_value vmHeapLimit;
    vmHeapLimitValue = vmHeapLimitValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, vmHeapLimitValue, &vmHeapLimit);
    napi_set_named_property(env, appMemoryLimit, "vmHeapLimit", vmHeapLimit);
    napi_value vmTotalHeapSize;
    vmTotalHeapSizeValue = vmTotalHeapSizeValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, vmTotalHeapSizeValue, &vmTotalHeapSize);
    napi_set_named_property(env, appMemoryLimit, "vmTotalHeapSize", vmTotalHeapSize);
    return appMemoryLimit;
}

napi_value GetAppNativeMemInfo(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppNativeMemInfo");
    HiDebug_NativeMemInfo nativeMemInfo{};
    auto nativeMemInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo();
    if (nativeMemInfoOption) {
        nativeMemInfo = nativeMemInfoOption.value();
    }
    napi_value memInfo;
    napi_create_object(env, &memInfo);

    napi_value pss;
    napi_create_bigint_uint64(env, nativeMemInfo.pss, &pss);
    napi_set_named_property(env, memInfo, "pss", pss);

    napi_value rss;
    napi_create_bigint_uint64(env, nativeMemInfo.rss, &rss);
    napi_set_named_property(env, memInfo, "rss", rss);

    napi_value sharedDirty;
    napi_create_bigint_uint64(env, nativeMemInfo.sharedDirty, &sharedDirty);
    napi_set_named_property(env, memInfo, "sharedDirty", sharedDirty);

    napi_value privateDirty;
    napi_create_bigint_uint64(env, nativeMemInfo.privateDirty, &privateDirty);
    napi_set_named_property(env, memInfo, "privateDirty", privateDirty);

    napi_value sharedClean;
    napi_create_bigint_uint64(env, nativeMemInfo.sharedClean, &sharedClean);
    napi_set_named_property(env, memInfo, "sharedClean", sharedClean);

    napi_value privateClean;
    napi_create_bigint_uint64(env, nativeMemInfo.privateClean, &privateClean);
    napi_set_named_property(env, memInfo, "privateClean", privateClean);

    napi_value vss;
    napi_create_bigint_uint64(env, nativeMemInfo.vss, &vss);
    napi_set_named_property(env, memInfo, "vss", vss);

    return memInfo;
}

napi_value GetSystemMemInfo(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getSystemMemInfo");
    SysMemory systemMemInfo{};
    auto systemMemInfoOption = HidebugNativeInterface::GetInstance().GetSystemMemInfo();
    if (systemMemInfoOption) {
        systemMemInfo = systemMemInfoOption.value();
    }
    napi_value sysMemInfo;
    napi_create_object(env, &sysMemInfo);

    napi_value totalMem;
    napi_create_bigint_uint64(env, systemMemInfo.memTotal, &totalMem);
    napi_set_named_property(env, sysMemInfo, "totalMem", totalMem);

    napi_value freeMem;
    napi_create_bigint_uint64(env, systemMemInfo.memFree, &freeMem);
    napi_set_named_property(env, sysMemInfo, "freeMem", freeMem);

    napi_value availableMem;
    napi_create_bigint_uint64(env, systemMemInfo.memAvailable, &availableMem);
    napi_set_named_property(env, sysMemInfo, "availableMem", availableMem);
    return sysMemInfo;
}

napi_value StartAppTraceCapture(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("startAppTraceCapture");
    napi_value result;
    uint32_t traceFlag = 0;
    uint32_t limitSize = 0;
    std::vector<uint64_t> tags;
    if (!GetTraceParam(env, info, traceFlag, limitSize, tags)) {
        std::string paramErrorMessage = "Invalid argument";
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
    }
    uint64_t tag = std::accumulate(tags.begin(), tags.end(), 0ull, [](uint64_t a, uint64_t b) { return a | b; });
    std::string file;
    auto ret = HidebugNativeInterface::GetInstance().StartAppTraceCapture(tag, traceFlag, limitSize, file);
    if (ret == HIDEBUG_SUCCESS) {
        napi_create_string_utf8(env, file.c_str(), NAPI_AUTO_LENGTH, &result);
        return result;
    }
    if (ret == HIDEBUG_INVALID_ARGUMENT) {
        std::string errorMessage = "Invalid argument";
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), errorMessage.c_str());
    }
    if (ret == HIDEBUG_TRACE_CAPTURED_ALREADY) {
        std::string errorMessage = "Capture trace already enabled.";
        apiInvokeRecorder.SetErrorCode(ErrorCode::HAVA_ALREADY_TRACE);
        napi_throw_error(env, std::to_string(ErrorCode::HAVA_ALREADY_TRACE).c_str(), errorMessage.c_str());
    }
    if (ret == HIDEBUG_NO_PERMISSION) {
        std::string errorMessage = "No write permission on the file.";
        apiInvokeRecorder.SetErrorCode(ErrorCode::WITHOUT_WRITE_PERMISSON);
        napi_throw_error(env, std::to_string(ErrorCode::WITHOUT_WRITE_PERMISSON).c_str(), errorMessage.c_str());
    }
    std::string errorMessage = "Abnormal trace status.";
    apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
    napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), errorMessage.c_str());
    return CreateUndefined(env);
}

napi_value StopAppTraceCapture(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("stopAppTraceCapture");
    auto ret = HidebugNativeInterface::GetInstance().StopAppTraceCapture();
    if (ret == HIDEBUG_TRACE_ABNORMAL) {
        std::string errorMessage = "The status of the trace is abnormal";
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), errorMessage.c_str());
    }
    if (ret == HIDEBUG_NO_TRACE_RUNNING) {
        std::string errorMessage = "No capture trace running";
        apiInvokeRecorder.SetErrorCode(ErrorCode::NO_CAPTURE_TRACE_RUNNING);
        napi_throw_error(env, std::to_string(ErrorCode::NO_CAPTURE_TRACE_RUNNING).c_str(), errorMessage.c_str());
    }
    return CreateUndefined(env);
}

napi_value GetVMRuntimeStats(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getVMRuntimeStats");
    napi_value vmRunTimeStats;
    napi_create_object(env, &vmRunTimeStats);
    for (const auto &[k, v] : GC::vmGcMap_) {
        napi_set_named_property(env, vmRunTimeStats, k.c_str(), v(env));
    }
    return vmRunTimeStats;
}

napi_value GetVMRuntimeStat(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getVMRuntimeStat");
    std::string param;
    if (!GetTheOnlyStringParam(env, info, param)) {
        std::string paramErrorMessage = "Invalid parameter, a string parameter required.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        return CreateUndefined(env);
    }
    if (GC::vmGcMap_.find(param) == GC::vmGcMap_.end()) {
        std::string paramErrorMessage = "Invalid parameter, unknown property.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        return CreateUndefined(env);
    }
    return GC::vmGcMap_.at(param)(env);
}

static bool JudgeValueRange(const std::string &type, int32_t value)
{
    if (limitResource.find(type) != limitResource.end()) {
        auto limitValue = limitResource[type];
        if (value >= limitValue.first && value <= limitValue.second) {
            return true;
        }
    }
    return false;
}

static bool CheckFilenameParamLength(napi_env env, napi_value *argv, size_t &bufLen)
{
    napi_status status = napi_get_value_string_utf8(env, argv[FIRST_POS], nullptr, 0, &bufLen);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Get input filename param length failed.");
        return false;
    }
    const int bufMax = 128;
    if (bufLen > bufMax || bufLen == 0) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. input filename param length is illegal.");
        return false;
    }
    return true;
}

static bool CheckResourceType(napi_env env, napi_value *argv, size_t &bufLen, std::string &type)
{
    std::vector<char> buf(bufLen + 1, 0);
    napi_get_value_string_utf8(env, argv[FIRST_POS], buf.data(), bufLen + 1, &bufLen);
    type = std::string(buf.data());
    if (type.empty()) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Resource type is invalid.");
        return false;
    }
    auto findType = std::find(RESOURCE_TYPE_LIST.begin(), RESOURCE_TYPE_LIST.end(), type);
    if (findType == RESOURCE_TYPE_LIST.end()) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Resource type is invalid.");
        return false;
    }
    return true;
}

static bool CheckInputValue(napi_env env, napi_value *argv, std::string &type, int32_t &value)
{
    if (napi_get_value_int32(env, argv[SECOND_POS], &value) != napi_ok || value < 0) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Input value error.");
        return false;
    }
    if (!JudgeValueRange(type, value)) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. The value range is invalid.");
        return false;
    }
    return true;
}

static bool GetAppResourceLimitParam(napi_env env, napi_callback_info info, std::string& type,
    int32_t& value, bool& enabledDebugLog)
{
    const int valueNum = 3;
    size_t argc = valueNum;
    napi_value argv[valueNum] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != valueNum) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Invalid numbers of params!");
        return false;
    }
    if (!MatchValueType(env, argv[FIRST_POS], napi_string) &&
        !MatchValueType(env, argv[SECOND_POS], napi_number) &&
        !MatchValueType(env, argv[THIRD_POS], napi_boolean)) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam params type error.");
        return false;
    }
    size_t bufLen = 0;
    if (!CheckFilenameParamLength(env, argv, bufLen)) {
        return false;
    }
    if (!CheckResourceType(env, argv, bufLen, type)) {
        return false;
    }
    if (!CheckInputValue(env, argv, type, value)) {
        return false;
    }
    if (napi_get_value_bool(env, argv[THIRD_POS], &enabledDebugLog) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Get input enabledDebugLog failed.");
        return false;
    }
    return true;
}

static bool CreateSanBoxDir()
{
    constexpr mode_t defaultLogDirMode = 0770;
    const std::string reourceLimitDir = "/data/storage/el2/log/resourcelimit/";
    if (!OHOS::FileExists(reourceLimitDir)) {
        OHOS::ForceCreateDirectory(reourceLimitDir);
        OHOS::ChangeModeDirectory(reourceLimitDir, defaultLogDirMode);
    }
    if (OHOS::StorageDaemon::AclSetAccess(reourceLimitDir, "g:1201:rwx") != 0) {
        HILOG_ERROR(LOG_CORE, "CreateSanBoxDir Failed to AclSetAccess");
        return false;
    }
    return true;
}

static bool CheckVersionType(const std::string& type, const std::string& key)
{
    auto versionType = OHOS::system::GetParameter(key, "unknown");
    return (versionType.find(type) != std::string::npos);
}

napi_value SetAppResourceLimit(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("setAppResourceLimit");
    CreateSanBoxDir();
    if (!IsBetaVersion() && !CheckVersionType("enable", KEY_HIVIEW_DEVELOP_TYPE)) {
        HILOG_ERROR(LOG_CORE, "SetAppResourceLimit failed. Not developer options or beta versions");
        apiInvokeRecorder.SetErrorCode(ErrorCode::VERSION_ERROR);
        return CreateUndefined(env);
    }
    std::string type = "";
    int32_t value = 0;
    bool enabledDebugLog = false;
    if (!GetAppResourceLimitParam(env, info, type, value, enabledDebugLog)) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        return CreateUndefined(env);
    }
    if (type == "js_heap") { // js_heap set value
        NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
        engine->SetJsDumpThresholds(value);
    }
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!abilityManager) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_ABILITY_NOT_FOUND);
        return CreateUndefined(env);
    }
    sptr<IRemoteObject> remoteObject = abilityManager->CheckSystemAbility(DFX_SYS_HIVIEW_ABILITY_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR(LOG_CORE, "SetAppResourceLimit failed. No this system ability.");
        std::string idErrorMessage = "system ability is not exist.";
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), idErrorMessage.c_str());
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateUndefined(env);
    }
    auto result = HidebugNativeInterface::GetInstance().GetMemoryLeakResource(type, value, enabledDebugLog);
    if (result == MemoryState::MEMORY_FAILED) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        return CreateUndefined(env);
    }
    return CreateUndefined(env);
}

napi_value IsDebugState(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("isDebugState");
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    bool debugState = (engine && engine->GetIsDebugModeEnabled()) ||
        HidebugNativeInterface::GetInstance().IsDebuggerConnected();
    napi_value result = nullptr;
    napi_get_boolean(env, debugState, &result);
    return result;
}

class GraphicAsyncTask : public AsyncTask {
public:
    GraphicAsyncTask() : AsyncTask("graphicAsyncTask"), apiInvokeRecorder_("getGraphicsMemory") {}

protected:
    void Work(napi_env env) override
    {
        result_ = HidebugNativeInterface::GetInstance().GetGraphicsMemory();
    }

    void Done(napi_env env, napi_status status) override
    {
        if (result_) {
            napi_value ret;
            napi_create_int32(env, result_.value(), &ret);
            napi_resolve_deferred(env, deferred_, ret);
        } else {
            constexpr const char* errMsg = "Failed to get the application memory due to a remote exception";
            apiInvokeRecorder_.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
            napi_reject_deferred(env, deferred_,
                CreateErrorMessage(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL), errMsg));
        }
    }

private:
    std::optional<int> result_{};
    ApiInvokeRecorder apiInvokeRecorder_;
};

napi_value GetGraphicsMemory(napi_env env, napi_callback_info info)
{
    return AsyncTask::GetPromise<GraphicAsyncTask>(env);
}

napi_value GetGraphicsMemorySync(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getGraphicsMemorySync");
    std::optional<int32_t> result = HidebugNativeInterface::GetInstance().GetGraphicsMemory();
    if (result) {
        napi_value ret;
        napi_create_int32(env, result.value(), &ret);
        return ret;
    }
    constexpr const char* errMsg = "Failed to get the application memory due to a remote exception";
    napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), errMsg);
    apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
    return CreateUndefined(env);
}

napi_value DeclareHiDebugInterface(napi_env env, napi_value exports)
{
    ApiRecordReporter::InitProcessor();
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("startProfiling", StartProfiling),
        DECLARE_NAPI_FUNCTION("stopProfiling", StopProfiling),
        DECLARE_NAPI_FUNCTION("dumpHeapData", DumpHeapData),
        DECLARE_NAPI_FUNCTION("startJsCpuProfiling", StartJsCpuProfiling),
        DECLARE_NAPI_FUNCTION("stopJsCpuProfiling", StopJsCpuProfiling),
        DECLARE_NAPI_FUNCTION("dumpJsHeapData", DumpJsHeapData),
        DECLARE_NAPI_FUNCTION("getPss", GetPss),
        DECLARE_NAPI_FUNCTION("getSharedDirty", GetSharedDirty),
        DECLARE_NAPI_FUNCTION("getPrivateDirty", GetPrivateDirty),
        DECLARE_NAPI_FUNCTION("getCpuUsage", GetCpuUsage),
        DECLARE_NAPI_FUNCTION("getServiceDump", GetServiceDump),
        DECLARE_NAPI_FUNCTION("getNativeHeapSize", GetNativeHeapSize),
        DECLARE_NAPI_FUNCTION("getNativeHeapAllocatedSize", GetNativeHeapAllocatedSize),
        DECLARE_NAPI_FUNCTION("getNativeHeapFreeSize", GetNativeHeapFreeSize),
        DECLARE_NAPI_FUNCTION("getVss", GetVss),
        DECLARE_NAPI_FUNCTION("removeNapiWrap", RemoveNapiWrap),
        DECLARE_NAPI_FUNCTION("getAppVMMemoryInfo", GetAppVMMemoryInfo),
        DECLARE_NAPI_FUNCTION("getAppThreadCpuUsage", GetAppThreadCpuUsage),
        DECLARE_NAPI_FUNCTION("getSystemCpuUsage", GetSystemCpuUsage),
        DECLARE_NAPI_FUNCTION("getAppMemoryLimit", GetAppMemoryLimit),
        DECLARE_NAPI_FUNCTION("getAppNativeMemInfo", GetAppNativeMemInfo),
        DECLARE_NAPI_FUNCTION("getSystemMemInfo", GetSystemMemInfo),
        DECLARE_NAPI_FUNCTION("startAppTraceCapture", StartAppTraceCapture),
        DECLARE_NAPI_FUNCTION("stopAppTraceCapture", StopAppTraceCapture),
        DECLARE_NAPI_FUNCTION("getVMRuntimeStats", GetVMRuntimeStats),
        DECLARE_NAPI_FUNCTION("getVMRuntimeStat", GetVMRuntimeStat),
        DECLARE_NAPI_FUNCTION("setAppResourceLimit", SetAppResourceLimit),
        DECLARE_NAPI_FUNCTION("isDebugState", IsDebugState),
        DECLARE_NAPI_FUNCTION("getGraphicsMemory", GetGraphicsMemory),
        DECLARE_NAPI_FUNCTION("getGraphicsMemorySync", GetGraphicsMemorySync),
        DECLARE_NAPI_FUNCTION("dumpJsRawHeapData", DumpJsRawHeapData),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    InitNapiClass(env, exports);
    return exports;
}

static napi_module hidebugModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = HiviewDFX::DeclareHiDebugInterface,
    .nm_modname = "hidebug",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void HiDebugRegisterModule(void)
{
    napi_module_register(&hidebugModule);
}
} // HiviewDFX
} // OHOS
