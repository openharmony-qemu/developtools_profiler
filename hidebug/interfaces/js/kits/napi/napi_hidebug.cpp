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

#include <codecvt>
#include <string>
#include <malloc.h>
#include <parameters.h>

#include "application_context.h"
#include "context.h"
#include "directory_ex.h"
#include "faultlogger_client.h"
#include "file_ex.h"
#include "hiappevent_util.h"
#include "hidebug_native_interface.h"
#include "hidebug_util.h"
#include "hilog/log.h"
#include "iservice_registry.h"
#include "napi_hidebug_dump.h"
#include "napi_hidebug_init.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"
#include "refbase.h"
#include "system_ability_definition.h"
#include "napi_hidebug_vm.h"
#include "napi_util.h"
#include "error_code.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_NAPI"
constexpr int REMOVE_NAPI_WRAP_PARAM_COUNT = 2;
constexpr int NAME_LEN = 128;
constexpr int BYTE_2_KB_SHIFT_BITS = 10;
constexpr int FIRST_POS = 0;
constexpr int SECOND_POS = 1;
constexpr int THIRD_POS = 2;
constexpr int TRIM_LEVEL_MAX = 1;
constexpr int ENABLE_ARGS_NO_PARAMS = 0;
constexpr int ENABLE_ARGS_ONE_PARAMS = 1;
constexpr int ENABLE_ARGS_TWO_PARAMS = 2;
constexpr double DEFAULT_SAMPLE_RATE = 2500;
constexpr double DEFAULT_MAX_SIMUTANEOUS_ALLOCATIONS = 1000;
constexpr int DEFAULT_DURATION = 7;
constexpr auto SLASH_STR = "/";
constexpr auto JSON_FILE = ".json";
constexpr auto KEY_HIVIEW_DEVELOP_TYPE = "persist.hiview.leak_detector";

struct GwpAsanParams {
    bool alwaysEnabled = false;
    double sampleRate = DEFAULT_SAMPLE_RATE;
    double maxSimutaneousAllocations = DEFAULT_MAX_SIMUTANEOUS_ALLOCATIONS;
    int32_t duration = DEFAULT_DURATION;
};
}

static bool GetDumpParam(napi_env env, napi_callback_info info,
    int& serviceId, int& fd, std::vector<std::u16string>& args)
{
    constexpr int valueNum = 3;
    size_t argc = valueNum;
    napi_value argv[valueNum] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc != valueNum) {
        HILOG_ERROR(LOG_CORE, "invalid number = %{public}zu of params.", argc);
        return false;
    }
    if (!GetNapiInt32Value(env, argv[FIRST_POS], serviceId)) {
        HILOG_ERROR(LOG_CORE, "Get input serviceId failed.");
        return false;
    }
    if (!GetNapiInt32Value(env, argv[SECOND_POS], fd)) {
        HILOG_ERROR(LOG_CORE, "Get input fd failed.");
        return false;
    }
    uint32_t arraySize = 0;
    if (!GetNapiArrayLength(env, argv[THIRD_POS], arraySize)) {
        HILOG_ERROR(LOG_CORE, "Get input args failed.");
        return false;
    }
    for (uint32_t i = 0; i < arraySize; i++) {
        napi_value jsValue = nullptr;
        if (napi_get_element(env, argv[THIRD_POS], i, &jsValue) != napi_ok) {
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
    constexpr int valueNum = 3;
    size_t argc = valueNum;
    napi_value argv[valueNum] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != valueNum) {
        HILOG_ERROR(LOG_CORE, "invalid numbers of params!");
        return false;
    }
    uint32_t arraySize = 0;
    if (!GetNapiArrayLength(env, argv[FIRST_POS], arraySize)) {
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
    if (!GetNapiUint32Value(env, argv[SECOND_POS], traceFlag)) {
        HILOG_ERROR(LOG_CORE, "Get input traceFlag failed.");
        return false;
    }
    if (!GetNapiUint32Value(env, argv[THIRD_POS], limitSize)) {
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
    napi_value retValue = nullptr;
    auto memInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    napi_create_bigint_uint64(env, memInfoOption ? static_cast<uint64_t>(memInfoOption->pss) : 0, &retValue);
    return retValue;
}

napi_value GetSharedDirty(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getSharedDirty");
    napi_value retValue = nullptr;
    auto memInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    napi_create_bigint_uint64(env, memInfoOption ? static_cast<uint64_t>(memInfoOption->sharedDirty) : 0, &retValue);
    return retValue;
}

napi_value GetPrivateDirty(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getPrivateDirty");
    napi_value retValue = nullptr;
    auto memInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    napi_create_bigint_uint64(env, memInfoOption ? static_cast<uint64_t>(memInfoOption->privateDirty) : 0, &retValue);
    return retValue;
}

napi_value GetCpuUsage(napi_env env, napi_callback_info info)
{
    constexpr uint32_t reportTimeOutSec = 5 * 60;
    constexpr uint32_t limitSize = 100;
    static MultipleRecordReporter multipleRecordReporter(reportTimeOutSec, limitSize);
    ApiInvokeRecorder apiInvokeRecorder("getCpuUsage", multipleRecordReporter);
    double cpuUsage = HidebugNativeInterface::GetInstance().GetCpuUsage();
    napi_value retValue = nullptr;
    napi_create_double(env, cpuUsage, &retValue);
    return retValue;
}

napi_value GetNativeHeapSize(napi_env env, napi_callback_info info)
{
    struct mallinfo mi = mallinfo();
    napi_value retValue = nullptr;
    napi_create_bigint_uint64(env, uint64_t(mi.uordblks + mi.fordblks), &retValue);
    return retValue;
}

napi_value GetNativeHeapAllocatedSize(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getNativeHeapAllocatedSize");
    struct mallinfo mi = mallinfo();
    napi_value retValue = nullptr;
    napi_create_bigint_uint64(env, uint64_t(mi.uordblks), &retValue);
    return retValue;
}

napi_value GetNativeHeapFreeSize(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getNativeHeapFreeSize");
    struct mallinfo mi = mallinfo();
    napi_value retValue = nullptr;
    napi_create_bigint_uint64(env, uint64_t(mi.fordblks), &retValue);
    return retValue;
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
    napi_value retValue = nullptr;
    auto vssInfoOption = HidebugNativeInterface::GetInstance().GetVss();
    napi_create_bigint_uint64(env, vssInfoOption ? vssInfoOption.value() : 0, &retValue);
    return retValue;
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
        (!MatchValueType(env, argv[FIRST_POS], napi_object) ||
        !MatchValueType(env, argv[SECOND_POS], napi_boolean))) {
        HILOG_DEBUG(LOG_CORE, "RemoveNapiWrap Failed to parse parameters, argc %{public}d", (int)argc);
        std::string paramErrorMessage = "The parameter check failed.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }

    // remove jsObj's wrap
    auto jsObj = argv[FIRST_POS];
    void *nativePtr = nullptr;
    napi_remove_wrap(env, jsObj, (void **)&nativePtr);

    // remove jsObj's properties wrap
    bool needRemoveProperty = false;
    napi_get_value_bool(env, argv[SECOND_POS], &needRemoveProperty);
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

    napi_value threadId = nullptr;
    napi_create_uint32(env, threadIdValue, &threadId);
    napi_set_named_property(env, result, "threadId", threadId);

    napi_value cpuUsage = nullptr;
    napi_create_double(env, cpuUsageValue, &cpuUsage);
    napi_set_named_property(env, result, "cpuUsage", cpuUsage);
}

static void ConvertThreadCpuUsageMapToJs(napi_env env, napi_value &result, const std::map<uint32_t, double> &threadMap)
{
    napi_create_array(env, &result);
    size_t idx = 0;
    for (const auto& [threadId, cpuUsage] : threadMap) {
        napi_value obj = nullptr;
        ConvertThreadCpuUsageToJs(env, obj, threadId, cpuUsage);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

napi_value GetAppThreadCpuUsage(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppThreadCpuUsage");
    napi_value result = nullptr;
    std::map<uint32_t, double> threadMap = HidebugNativeInterface::GetInstance().GetAppThreadCpuUsage();
    ConvertThreadCpuUsageMapToJs(env, result, threadMap);
    return result;
}

napi_value GetAppMemoryLimit(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppMemoryLimit");
    napi_value appMemoryLimit = nullptr;
    napi_create_object(env, &appMemoryLimit);
    auto memoryLimitOption = HidebugNativeInterface::GetInstance().GetAppMemoryLimit();
    if (!memoryLimitOption) {
        memoryLimitOption.emplace();
    }
    napi_value rssLimit = nullptr;
    napi_create_bigint_uint64(env, memoryLimitOption->rssLimit, &rssLimit);
    napi_set_named_property(env, appMemoryLimit, "rssLimit", rssLimit);

    napi_value vssLimit = nullptr;
    napi_create_bigint_uint64(env, memoryLimitOption->vssLimit, &vssLimit);
    napi_set_named_property(env, appMemoryLimit, "vssLimit", vssLimit);
    uint64_t vmHeapLimitValue{0};
    uint64_t vmTotalHeapSizeValue{0};
    NativeEngine *engine = reinterpret_cast<NativeEngine *>(env);
    if (engine) {
        vmHeapLimitValue = engine->GetHeapLimitSize();
        vmTotalHeapSizeValue = engine->GetProcessHeapLimitSize();
    }
    napi_value vmHeapLimit = nullptr;
    vmHeapLimitValue = vmHeapLimitValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, vmHeapLimitValue, &vmHeapLimit);
    napi_set_named_property(env, appMemoryLimit, "vmHeapLimit", vmHeapLimit);
    napi_value vmTotalHeapSize = nullptr;
    vmTotalHeapSizeValue = vmTotalHeapSizeValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, vmTotalHeapSizeValue, &vmTotalHeapSize);
    napi_set_named_property(env, appMemoryLimit, "vmTotalHeapSize", vmTotalHeapSize);
    return appMemoryLimit;
}

napi_value ConvertToJsNativeMemInfo(napi_env env, const NativeMemInfo& nativeMemInfo)
{
    napi_value memInfo = nullptr;
    napi_create_object(env, &memInfo);
    napi_value pss = nullptr;
    napi_create_bigint_uint64(env, nativeMemInfo.pss, &pss);
    napi_set_named_property(env, memInfo, "pss", pss);

    napi_value rss = nullptr;
    napi_create_bigint_uint64(env, nativeMemInfo.rss, &rss);
    napi_set_named_property(env, memInfo, "rss", rss);

    napi_value sharedDirty = nullptr;
    napi_create_bigint_uint64(env, nativeMemInfo.sharedDirty, &sharedDirty);
    napi_set_named_property(env, memInfo, "sharedDirty", sharedDirty);

    napi_value privateDirty = nullptr;
    napi_create_bigint_uint64(env, nativeMemInfo.privateDirty, &privateDirty);
    napi_set_named_property(env, memInfo, "privateDirty", privateDirty);

    napi_value sharedClean = nullptr;
    napi_create_bigint_uint64(env, nativeMemInfo.sharedClean, &sharedClean);
    napi_set_named_property(env, memInfo, "sharedClean", sharedClean);

    napi_value privateClean = nullptr;
    napi_create_bigint_uint64(env, nativeMemInfo.privateClean, &privateClean);
    napi_set_named_property(env, memInfo, "privateClean", privateClean);

    napi_value vss = nullptr;
    napi_create_bigint_uint64(env, nativeMemInfo.vss, &vss);
    napi_set_named_property(env, memInfo, "vss", vss);
    return memInfo;
}

napi_value GetAppNativeMemInfo(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppNativeMemInfo");
    auto memInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    return memInfoOption ? ConvertToJsNativeMemInfo(env, *memInfoOption) : ConvertToJsNativeMemInfo(env, {});
}

napi_value GetAppNativeMemInfoWithCache(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getAppNativeMemInfoWithCache");
    constexpr auto paramNum = 1;
    size_t argc = paramNum;
    napi_value argv = nullptr ;
    napi_get_cb_info(env, info, &argc, &argv, nullptr, nullptr);
    bool forceRefresh = false;
    if (argc > paramNum || (argc == paramNum && !OHOS::HiviewDFX::GetNapiBoolValue(env, argv, forceRefresh))) {
        constexpr auto paramErrorMessage = "Invalid argument";
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage);
        return CreateUndefined(env);
    }
    auto memInfoOption = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(!forceRefresh);
    return memInfoOption ? ConvertToJsNativeMemInfo(env, *memInfoOption) : ConvertToJsNativeMemInfo(env, {});
}

class GetAppNativeMemInfoAsyncTask : public AsyncTask {
public:
    GetAppNativeMemInfoAsyncTask() : AsyncTask("getAppNativeMemInfoAsync"),
        apiInvokeRecorder_("getAppNativeMemInfoAsync") {}

protected:
    void Work(napi_env env) override
    {
        result_ = HidebugNativeInterface::GetInstance().GetAppNativeMemInfo(false);
    }

    bool Done(napi_env env, napi_status status, napi_value& ret)  override
    {
        ret = result_ ? ConvertToJsNativeMemInfo(env, result_.value()) : ConvertToJsNativeMemInfo(env, {});
        return true;
    }

private:
    std::optional<NativeMemInfo> result_{};
    ApiInvokeRecorder apiInvokeRecorder_;
};

napi_value GetAppNativeMemInfoAsync(napi_env env, napi_callback_info info)
{
    return AsyncTask::GetPromise<GetAppNativeMemInfoAsyncTask>(env, info);
}

napi_value GetSystemMemInfo(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getSystemMemInfo");
    auto systemMemInfoOption = HidebugNativeInterface::GetInstance().GetSystemMemInfo();
    if (!systemMemInfoOption) {
        systemMemInfoOption.emplace();
    }
    napi_value sysMemInfo = nullptr;
    napi_create_object(env, &sysMemInfo);

    napi_value totalMem = nullptr;
    napi_create_bigint_uint64(env, static_cast<uint64_t>(systemMemInfoOption->totalMem), &totalMem);
    napi_set_named_property(env, sysMemInfo, "totalMem", totalMem);

    napi_value freeMem = nullptr;
    napi_create_bigint_uint64(env, static_cast<uint64_t>(systemMemInfoOption->freeMem), &freeMem);
    napi_set_named_property(env, sysMemInfo, "freeMem", freeMem);

    napi_value availableMem = nullptr;
    napi_create_bigint_uint64(env, static_cast<uint64_t>(systemMemInfoOption->availableMem), &availableMem);
    napi_set_named_property(env, sysMemInfo, "availableMem", availableMem);
    return sysMemInfo;
}

napi_value StartAppTraceCapture(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("startAppTraceCapture");
    napi_value result = nullptr;
    uint32_t traceFlag = 0;
    uint32_t limitSize = 0;
    std::vector<uint64_t> tags;
    if (!GetTraceParam(env, info, traceFlag, limitSize, tags)) {
        constexpr auto paramErrorMessage = "Invalid argument";
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage);
    }
    uint64_t tag = std::accumulate(tags.begin(), tags.end(), 0ull, [](uint64_t a, uint64_t b) { return a | b; });
    std::string file;
    auto ret = HidebugNativeInterface::GetInstance().StartAppTraceCapture(tag, traceFlag, limitSize, file);
    if (ret == TRACE_SUCCESS) {
        napi_create_string_utf8(env, file.c_str(), NAPI_AUTO_LENGTH, &result);
        return result;
    }
    if (ret == TRACE_INVALID_ARGUMENT) {
        constexpr auto errorMessage = "Invalid argument";
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), errorMessage);
        return CreateUndefined(env);
    }
    if (ret == TRACE_CAPTURED_ALREADY) {
        constexpr auto errorMessage = "Capture trace already enabled.";
        apiInvokeRecorder.SetErrorCode(ErrorCode::HAVA_ALREADY_TRACE);
        napi_throw_error(env, std::to_string(ErrorCode::HAVA_ALREADY_TRACE).c_str(), errorMessage);
        return CreateUndefined(env);
    }
    if (ret == TRACE_NO_PERMISSION) {
        constexpr auto errorMessage = "No write permission on the file.";
        apiInvokeRecorder.SetErrorCode(ErrorCode::WITHOUT_WRITE_PERMISSON);
        napi_throw_error(env, std::to_string(ErrorCode::WITHOUT_WRITE_PERMISSON).c_str(), errorMessage);
        return CreateUndefined(env);
    }
    constexpr auto errorMessage = "Abnormal trace status.";
    apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
    napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), errorMessage);
    return CreateUndefined(env);
}

napi_value StopAppTraceCapture(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("stopAppTraceCapture");
    const auto ret = HidebugNativeInterface::GetInstance().StopAppTraceCapture();
    if (ret == TRACE_ABNORMAL) {
        std::string errorMessage = "The status of the trace is abnormal";
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), errorMessage.c_str());
    }
    if (ret == NO_TRACE_RUNNING) {
        std::string errorMessage = "No capture trace running";
        apiInvokeRecorder.SetErrorCode(ErrorCode::NO_CAPTURE_TRACE_RUNNING);
        napi_throw_error(env, std::to_string(ErrorCode::NO_CAPTURE_TRACE_RUNNING).c_str(), errorMessage.c_str());
    }
    return CreateUndefined(env);
}

static bool JudgeValueRange(const std::string &type, int32_t value)
{
    if (type == "pss_memory") {
        constexpr int pssMin = 1024;
        constexpr int pssMax = 4 * 1024 * 1024;
        return value >= pssMin && value <= pssMax;
    }
    if (type == "js_memory") {
        constexpr int jsMin = 85;
        constexpr int jsMax = 95;
        return value >= jsMin && value <= jsMax;
    }
    if (type == "fd_memory") {
        constexpr int fdMin = 10;
        constexpr int fdMax = 10000;
        return value >= fdMin && value <= fdMax;
    }
    if (type == "thread_memory") {
        constexpr int threadMin = 1;
        constexpr int threadMax = 1000;
        return value >= threadMin && value <= threadMax;
    }
    HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Resource type is invalid.");
    return false;
}

static bool GetAppResourceLimitParam(napi_env env, napi_callback_info info, std::string& type,
    int32_t& value, bool& enabledDebugLog)
{
    constexpr int valueNum = 3;
    size_t argc = valueNum;
    napi_value argv[valueNum] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != valueNum) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Invalid numbers of params!");
        return false;
    }
    constexpr int paramMaxLen = 128;
    if (!GetNapiStringValue(env, argv[FIRST_POS], type, paramMaxLen)) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Resource type is invalid.");
        return false;
    }
    if (!GetNapiInt32Value(env, argv[SECOND_POS], value)) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Input value error.");
        return false;
    }
    if (!JudgeValueRange(type, value)) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. The value range is invalid.");
        return false;
    }
    if (!GetNapiBoolValue(env, argv[THIRD_POS], enabledDebugLog)) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Get input enabledDebugLog failed.");
        return false;
    }
    return true;
}

napi_value SetAppResourceLimit(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("setAppResourceLimit");
    CreateResourceLimitDir();
    if (!IsBetaVersion() && !CheckVersionType("enable", KEY_HIVIEW_DEVELOP_TYPE)) {
        HILOG_ERROR(LOG_CORE, "SetAppResourceLimit failed. Not developer options or beta versions");
        apiInvokeRecorder.SetErrorCode(ErrorCode::VERSION_ERROR);
        return CreateUndefined(env);
    }
    std::string type;
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
    if (HidebugNativeInterface::GetInstance().GetMemoryLeakResource(type, value, enabledDebugLog) != NATIVE_SUCCESS) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
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

    bool Done(napi_env env, napi_status status, napi_value& ret) override
    {
        if (result_) {
            napi_create_int32(env, result_.value(), &ret);
            return true;
        }
        apiInvokeRecorder_.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
        constexpr const char* errMsg = "Failed to get the application memory due to a remote exception";
        ret = CreateErrorMessage(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL), errMsg);
        return false;
    }

private:
    std::optional<int> result_{};
    ApiInvokeRecorder apiInvokeRecorder_;
};

napi_value GetGraphicsMemory(napi_env env, napi_callback_info info)
{
    return AsyncTask::GetPromise<GraphicAsyncTask>(env, info);
}


class GraphicsMemorySummaryAsyncTask : public AsyncTask {
public:
    GraphicsMemorySummaryAsyncTask() : AsyncTask("graphicMemorySummaryAsyncTask"),
        apiInvokeRecorder_("getGraphicsMemorySummary") {}
protected:
    napi_value InitAsyncTask(napi_env env, napi_callback_info info) override
    {
        constexpr auto paramNum = 1;
        size_t argc = paramNum;
        napi_value argv = nullptr ;
        napi_get_cb_info(env, info, &argc, &argv, nullptr, nullptr);
        int argInterval = 0;
        if (argc > paramNum || (argc == paramNum && !GetNapiInt32Value(env, argv, argInterval))) {
            constexpr auto paramErrorMessage = "Invalid argument";
            return CreateErrorMessage(env, std::to_string(ErrorCode::PARAMETER_ERROR), paramErrorMessage);
        }
        interval_ = static_cast<uint32_t>(argInterval);
        return nullptr;
    }

    void Work(napi_env env) override
    {
        result_ = HidebugNativeInterface::GetInstance().GetGraphicsMemorySummary(interval_);
    }

    bool Done(napi_env env, napi_status status, napi_value& ret) override
    {
        if (!result_) {
            constexpr auto errMsg = "Failed to get the application memory due to a remote exception";
            apiInvokeRecorder_.SetErrorCode(ErrorCode::SYSTEM_STATUS_ABNORMAL);
            ret = CreateErrorMessage(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL), errMsg);
            return false;
        }
        napi_create_object(env, &ret);
        napi_value gl = nullptr;
        napi_create_int32(env, static_cast<int32_t>(result_->gl), &gl);
        napi_set_named_property(env, ret, "gl", gl);
        napi_value graph = nullptr;
        napi_create_bigint_uint64(env, static_cast<uint64_t>(result_->graph), &graph);
        napi_set_named_property(env, ret, "graph", graph);
        return true;
    }

private:
    std::optional<GraphicsMemorySummary> result_{};
    ApiInvokeRecorder apiInvokeRecorder_;
    uint32_t interval_{0};
};

napi_value GetGraphicsMemorySummary(napi_env env, napi_callback_info info)
{
    return AsyncTask::GetPromise<GraphicsMemorySummaryAsyncTask>(env, info);
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

bool ParseGwpAsanOptions(napi_env env, napi_value value, GwpAsanParams& gwpAsanParams)
{
    if (!MatchValueType(env, value, napi_object)) {
        HILOG_ERROR(LOG_CORE, "The type of options must be GwpAsanOptions.");
        return false;
    }

    //alwaysEnabled?: bool
    napi_value nAlwaysEnabled = nullptr;
    if (GetNapiObjectProperty(env, value, "alwaysEnabled", nAlwaysEnabled)) {
        if (!GetNapiBoolValue(env, nAlwaysEnabled, gwpAsanParams.alwaysEnabled)) {
            HILOG_ERROR(LOG_CORE, "AlwaysEnabled type must be boolean");
            return false;
        }
    }

    //sampleRate?: number
    napi_value nSampleRate = nullptr;
    if (GetNapiObjectProperty(env, value, "sampleRate", nSampleRate)) {
        if (!GetNapiDoubleValue(env, nSampleRate, gwpAsanParams.sampleRate)) {
            HILOG_ERROR(LOG_CORE, "SampleRate type must be a num");
            return false;
        }
    }

    //maxSimutaneousAllocations?: number
    napi_value nMaxSimutaneousAllocations = nullptr;
    if (GetNapiObjectProperty(env, value, "maxSimutaneousAllocations", nMaxSimutaneousAllocations)) {
        if (!GetNapiDoubleValue(env, nMaxSimutaneousAllocations, gwpAsanParams.maxSimutaneousAllocations)) {
            HILOG_ERROR(LOG_CORE, "MaxSimutaneousAllocations type must be a num");
            return false;
        }
    }
    if (gwpAsanParams.sampleRate <= 0 || gwpAsanParams.maxSimutaneousAllocations <= 0) {
        HILOG_ERROR(LOG_CORE, "sampleRate or maxSimutaneousAllocations must be greater than 0");
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "Parse GwpAsanOptions success, alwaysEnabled: %{public}d, sampleRate: %{public}f,"
        " maxSimutaneousAllocations: %{public}f",
        gwpAsanParams.alwaysEnabled, gwpAsanParams.sampleRate, gwpAsanParams.maxSimutaneousAllocations);
    return true;
}

bool UpdateGwpAsanParams(napi_env env, napi_callback_info info, GwpAsanParams& gwpAsanParams,
    std::string& paramErrorMessage)
{
    size_t argc = ENABLE_ARGS_TWO_PARAMS;
    napi_value argv[ENABLE_ARGS_TWO_PARAMS] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    switch (argc) {
        case ENABLE_ARGS_TWO_PARAMS:
            if (!GetNapiInt32Value(env, argv[SECOND_POS], gwpAsanParams.duration) ||
                gwpAsanParams.duration <= 0) {
                paramErrorMessage = "Invalid parameter, set duration must be a num.";
                return false;
            }
            [[fallthrough]];
        case ENABLE_ARGS_ONE_PARAMS:
            if (!ParseGwpAsanOptions(env, argv[FIRST_POS], gwpAsanParams)) {
                paramErrorMessage = "Invalid parameter, parse gwpAsanOptions error.";
                return false;
            }
            [[fallthrough]];
        case ENABLE_ARGS_NO_PARAMS:
            return true;
        default: {
            paramErrorMessage = "Invalid parameter, the number of parameters "
                "cannot exceed two.";
            return false;
        }
    }
}

napi_value EnableGwpAsanGrayscale(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("enableGwpAsanGrayscale");
    GwpAsanParams gwpAsanParams;
    std::string paramErrorMessage;
    if (!UpdateGwpAsanParams(env, info, gwpAsanParams, paramErrorMessage)) {
        HILOG_ERROR(LOG_CORE, "EnableGwpAsanGrayscale failed. Invalid params!");
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(),
            paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    if (!EnableGwpAsanGrayscale(gwpAsanParams.alwaysEnabled, gwpAsanParams.sampleRate,
        gwpAsanParams.maxSimutaneousAllocations, gwpAsanParams.duration)) {
        HILOG_ERROR(LOG_CORE, "Failed to enable gwp asan grayscale!");
        apiInvokeRecorder.SetErrorCode(ErrorCode::OVER_ENABLE_LIMIT);
        paramErrorMessage = "The number of GWP-ASAN applications of this device overflowed after last boot.";
        napi_throw_error(env, std::to_string(ErrorCode::OVER_ENABLE_LIMIT).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    return CreateUndefined(env);
}

napi_value DisableGwpAsanGrayscale(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("disableGwpAsanGrayscale");
    DisableGwpAsanGrayscale();
    return CreateUndefined(env);
}

napi_value GetGwpAsanGrayscaleState(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getGwpAsanGrayscaleState");
    auto result = GetGwpAsanGrayscaleState();
    napi_value ret = nullptr;
    napi_create_uint32(env, result, &ret);
    return ret;
}

static std::string GetJsRawHeapTrimLevelParam(napi_env env, napi_callback_info info, uint32_t &level)
{
    size_t argc = ENABLE_ARGS_ONE_PARAMS;
    napi_value argv = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, nullptr, nullptr);
    std::string paramErrorMessage;
    if (argc != ENABLE_ARGS_ONE_PARAMS) {
        paramErrorMessage = "Param type error, wrong number of parameters.";
        HILOG_ERROR(LOG_CORE, "%{public}s", paramErrorMessage.c_str());
        return paramErrorMessage;
    }
    if (!MatchValueType(env, argv, napi_number)) {
        paramErrorMessage = "Param type error, invalid num.";
        HILOG_ERROR(LOG_CORE, "%{public}s", paramErrorMessage.c_str());
        return paramErrorMessage;
    }
    if (napi_get_value_uint32(env, argv, &level) != napi_ok ||
        level > TRIM_LEVEL_MAX) {
        paramErrorMessage = "Param type error, invalid an unsigned num or "
            "exceeding the maximum value: " + std::to_string(TRIM_LEVEL_MAX);
        HILOG_ERROR(LOG_CORE, "%{public}s", paramErrorMessage.c_str());
        return paramErrorMessage;
    }
    return paramErrorMessage;
}

napi_value SetJsRawHeapTrimLevel(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("setJsRawHeapTrimLevel");
    uint32_t level = 0;
    std::string paramErrorMessage = GetJsRawHeapTrimLevelParam(env, info, level);
    if (!paramErrorMessage.empty()) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(),
            paramErrorMessage.c_str());
        return CreateUndefined(env);
    }

    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    if (engine != nullptr) {
        engine->SetRawHeapTrimLevel(level);
        HILOG_DEBUG(LOG_CORE, "SetRawHeapTrimLevel to level: %{public}d success", level);
    }
    return CreateUndefined(env);
}

static std::string GetProcDumpInSharedOOMParam(napi_env env, napi_callback_info info, bool &enable)
{
    size_t argc = ENABLE_ARGS_ONE_PARAMS;
    napi_value argv = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, nullptr, nullptr);
    std::string paramErrorMessage;
    if (argc != ENABLE_ARGS_ONE_PARAMS) {
        paramErrorMessage = "Param type error, wrong num of parammeters.";
        HILOG_ERROR(LOG_CORE, "%{public}s", paramErrorMessage.c_str());
        return paramErrorMessage;
    }
    if (!MatchValueType(env, argv, napi_boolean)) {
        paramErrorMessage = "Param type error, invalid boolean.";
        HILOG_ERROR(LOG_CORE, "%{public}s", paramErrorMessage.c_str());
        return paramErrorMessage;
    }
    if (napi_get_value_bool(env, argv, &enable) != napi_ok) {
        paramErrorMessage = "Param value error.";
        HILOG_ERROR(LOG_CORE, "%{public}s", paramErrorMessage.c_str());
        return paramErrorMessage;
    }
    return paramErrorMessage;
}

napi_value SetProcDumpInSharedOOM(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("SetProcDumpInSharedOOM");
    bool enable = false;
    std::string paramErrorMessage = GetProcDumpInSharedOOMParam(env, info, enable);
    if (!paramErrorMessage.empty()) {
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(),
            paramErrorMessage.c_str());
        return CreateUndefined(env);
    }

    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    if (engine != nullptr) {
        engine->SetProcDumpInSharedOOM(enable);
        HILOG_DEBUG(LOG_CORE, "SetProcDumpInSharedOOM to enable: %{public}d success", enable);
    }
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
        DECLARE_NAPI_FUNCTION("getAppVMObjectUsedSize", GetAppVMObjectUsedSize),
        DECLARE_NAPI_FUNCTION("getAppThreadCpuUsage", GetAppThreadCpuUsage),
        DECLARE_NAPI_FUNCTION("getSystemCpuUsage", GetSystemCpuUsage),
        DECLARE_NAPI_FUNCTION("getAppMemoryLimit", GetAppMemoryLimit),
        DECLARE_NAPI_FUNCTION("getAppNativeMemInfo", GetAppNativeMemInfo),
        DECLARE_NAPI_FUNCTION("getAppNativeMemInfoWithCache", GetAppNativeMemInfoWithCache),
        DECLARE_NAPI_FUNCTION("getAppNativeMemInfoAsync", GetAppNativeMemInfoAsync),
        DECLARE_NAPI_FUNCTION("getSystemMemInfo", GetSystemMemInfo),
        DECLARE_NAPI_FUNCTION("startAppTraceCapture", StartAppTraceCapture),
        DECLARE_NAPI_FUNCTION("stopAppTraceCapture", StopAppTraceCapture),
        DECLARE_NAPI_FUNCTION("getVMRuntimeStats", GetVMRuntimeStats),
        DECLARE_NAPI_FUNCTION("getVMRuntimeStat", GetVMRuntimeStat),
        DECLARE_NAPI_FUNCTION("setAppResourceLimit", SetAppResourceLimit),
        DECLARE_NAPI_FUNCTION("isDebugState", IsDebugState),
        DECLARE_NAPI_FUNCTION("getGraphicsMemory", GetGraphicsMemory),
        DECLARE_NAPI_FUNCTION("getGraphicsMemorySync", GetGraphicsMemorySync),
        DECLARE_NAPI_FUNCTION("getGraphicsMemorySummary", GetGraphicsMemorySummary),
        DECLARE_NAPI_FUNCTION("dumpJsRawHeapData", DumpJsRawHeapData),
        DECLARE_NAPI_FUNCTION("enableGwpAsanGrayscale", EnableGwpAsanGrayscale),
        DECLARE_NAPI_FUNCTION("disableGwpAsanGrayscale", DisableGwpAsanGrayscale),
        DECLARE_NAPI_FUNCTION("getGwpAsanGrayscaleState", GetGwpAsanGrayscaleState),
        DECLARE_NAPI_FUNCTION("setJsRawHeapTrimLevel", SetJsRawHeapTrimLevel),
        DECLARE_NAPI_FUNCTION("setProcDumpInSharedOOM", SetProcDumpInSharedOOM),
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
