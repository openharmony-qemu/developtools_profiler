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
#include <ctime>
#include <malloc.h>
#include <parameters.h>
#include <unistd.h>

#include "application_context.h"
#include "context.h"
#include "cpu_collector.h"
#include "directory_ex.h"
#include "dump_usage.h"
#include "file_ex.h"
#include "hidebug_native_interface.h"
#include "hilog/log.h"
#include "iservice_registry.h"
#include "memory_collector.h"
#include "napi_hidebug_init.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"
#include "refbase.h"
#include "storage_acl.h"
#include "system_ability_definition.h"
#include "napi_hidebug_gc.h"

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
const std::string DEFAULT_FILENAME = "undefined";
const std::string JSON_FILE = ".json";
const std::string HEAPSNAPSHOT_FILE = ".heapsnapshot";
const std::string KEY_HIVIEW_USER_TYPE = "const.logsystem.versiontype";
const std::string KEY_HIVIEW_DEVELOP_TYPE = "persist.hiview.leak_detector";
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

static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

static bool CreateFile(const std::string &path)
{
    if (access(path.c_str(), F_OK) == 0) {
        if (access(path.c_str(), W_OK) == 0) {
            return true;
        }
        return false;
    }
    const mode_t defaultMode = S_IRUSR | S_IWUSR | S_IRGRP; // -rw-r-----
    int fd = creat(path.c_str(), defaultMode);
    if (fd == -1) {
        HILOG_ERROR(LOG_CORE, "file create failed, errno = %{public}d", errno);
        return false;
    } else {
        close(fd);
        return true;
    }
}

static bool IsLegalPath(const std::string& path)
{
    if (path.find("./") != std::string::npos ||
        path.find("../") != std::string::npos) {
        return false;
    }
    return true;
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

static std::string GetFileNameParam(napi_env env, napi_callback_info info)
{
    size_t argc = ONE_VALUE_LIMIT;
    napi_value argv[ONE_VALUE_LIMIT] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc != ONE_VALUE_LIMIT) {
        HILOG_ERROR(LOG_CORE, "invalid number = %{public}d of params.", ONE_VALUE_LIMIT);
        return DEFAULT_FILENAME;
    }
    if (!MatchValueType(env, argv[ARRAY_INDEX_FIRST], napi_string)) {
        HILOG_ERROR(LOG_CORE, "Type error, should be string type!");
        return DEFAULT_FILENAME;
    }
    size_t bufLen = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &bufLen);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "Get input filename param length failed.");
        return DEFAULT_FILENAME;
    }
    const int bufMax = 128;
    if (bufLen > bufMax || bufLen == 0) {
        HILOG_ERROR(LOG_CORE, "input filename param length is illegal.");
        return DEFAULT_FILENAME;
    }
    char buf[bufLen + 1];
    napi_get_value_string_utf8(env, argv[0], buf, bufLen + 1, &bufLen);
    std::string fileName = buf;
    return fileName;
}

static bool GetTheOnlyStringParam(napi_env env, napi_callback_info info, std::string &fileName)
{
    size_t argc = ONE_VALUE_LIMIT;
    napi_value argv[ONE_VALUE_LIMIT] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc != ONE_VALUE_LIMIT) {
        HILOG_ERROR(LOG_CORE, "invalid number = %{public}d of params.", ONE_VALUE_LIMIT);
        return false;
    }
    if (!MatchValueType(env, argv[ARRAY_INDEX_FIRST], napi_string)) {
        HILOG_ERROR(LOG_CORE, "Type error, should be string type!");
        return false;
    }
    size_t bufLen = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &bufLen);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "Get input filename param length failed.");
        return false;
    }
    const int bufMax = 128;
    if (bufLen > bufMax || bufLen == 0) {
        HILOG_ERROR(LOG_CORE, "input filename param length is illegal.");
        return false;
    }
    char buf[bufLen + 1];
    napi_get_value_string_utf8(env, argv[0], buf, bufLen + 1, &bufLen);
    fileName = buf;
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

static napi_value CreateUndefined(napi_env env)
{
    napi_value res = nullptr;
    napi_get_undefined(env, &res);
    return res;
}

static napi_value CreateErrorMessage(napi_env env, std::string msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, (char *)msg.data(), msg.size(), &message);
    napi_create_error(env, nullptr, message, &result);
    return result;
}

napi_value StartProfiling(napi_env env, napi_callback_info info)
{
    std::string fileName = GetFileNameParam(env, info);
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return CreateErrorMessage(env, "Get ApplicationContext failed.");
    }
    std::string filesDir = context->GetFilesDir();
    if (filesDir.empty()) {
        return CreateErrorMessage(env, "Get App files dir failed.");
    }
    std::string filePath = filesDir + SLASH_STR + fileName + JSON_FILE;
    if (!IsLegalPath(filePath)) {
        return CreateErrorMessage(env, "input fileName is illegal.");
    }
    if (!CreateFile(filePath)) {
        return CreateErrorMessage(env, "file created failed.");
    }
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->StartCpuProfiler(filePath);
    return CreateUndefined(env);
}

napi_value StartJsCpuProfiling(napi_env env, napi_callback_info info)
{
    std::string fileName;
    if (!GetTheOnlyStringParam(env, info, fileName)) {
        std::string paramErrorMessage = "Invalid parameter, require a string parameter.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    HILOG_INFO(LOG_CORE, "filename: %{public}s.", fileName.c_str());
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return CreateErrorMessage(env, "Get ApplicationContext failed.");
    }
    std::string filesDir = context->GetFilesDir();
    if (filesDir.empty()) {
        return CreateErrorMessage(env, "Get App files dir failed.");
    }
    std::string filePath = filesDir + SLASH_STR + fileName + JSON_FILE;
    if (!IsLegalPath(filePath)) {
        return CreateErrorMessage(env, "input fileName is illegal.");
    }
    if (!CreateFile(filePath)) {
        return CreateErrorMessage(env, "file created failed.");
    }
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->StartCpuProfiler(filePath);
    return CreateUndefined(env);
}

napi_value StopProfiling(napi_env env, napi_callback_info info)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->StopCpuProfiler();
    return CreateUndefined(env);
}

napi_value StopJsCpuProfiling(napi_env env, napi_callback_info info)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->StopCpuProfiler();
    return CreateUndefined(env);
}

napi_value DumpHeapData(napi_env env, napi_callback_info info)
{
    std::string fileName = GetFileNameParam(env, info);
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return CreateErrorMessage(env, "Get ApplicationContext failed.");
    }
    std::string filesDir = context->GetFilesDir();
    if (filesDir.empty()) {
        return CreateErrorMessage(env, "Get App files dir failed.");
    }
    std::string filePath = filesDir + SLASH_STR + fileName + HEAPSNAPSHOT_FILE;
    if (!IsLegalPath(filePath)) {
        return CreateErrorMessage(env, "input fileName is illegal.");
    }
    if (!CreateFile(filePath)) {
        return CreateErrorMessage(env, "file created failed.");
    }
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->DumpHeapSnapshot(filePath, true, DumpFormat::JSON, false, true);
    return CreateUndefined(env);
}

napi_value DumpJsHeapData(napi_env env, napi_callback_info info)
{
    std::string fileName;
    if (!GetTheOnlyStringParam(env, info, fileName)) {
        std::string paramErrorMessage = "Invalid parameter, require a string parameter.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    HILOG_ERROR(LOG_CORE, "filename: %{public}s.", fileName.c_str());
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return CreateErrorMessage(env, "Get ApplicationContext failed.");
    }
    std::string filesDir = context->GetFilesDir();
    if (filesDir.empty()) {
        return CreateErrorMessage(env, "Get App files dir failed.");
    }
    std::string filePath = filesDir + SLASH_STR + fileName + HEAPSNAPSHOT_FILE;
    if (!IsLegalPath(filePath)) {
        return CreateErrorMessage(env, "input fileName is illegal.");
    }
    if (!CreateFile(filePath)) {
        return CreateErrorMessage(env, "file created failed.");
    }
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->DumpHeapSnapshot(filePath, true, DumpFormat::JSON, false, true);
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
    struct mallinfo mi = mallinfo();
    napi_value nativeHeapAllocatedSize;
    napi_create_bigint_uint64(env, uint64_t(mi.uordblks), &nativeHeapAllocatedSize);
    return nativeHeapAllocatedSize;
}

napi_value GetNativeHeapFreeSize(napi_env env, napi_callback_info info)
{
    struct mallinfo mi = mallinfo();
    napi_value nativeHeapFreeSize;
    napi_create_bigint_uint64(env, uint64_t(mi.fordblks), &nativeHeapFreeSize);
    return nativeHeapFreeSize;
}

static napi_value GetServiceDump(napi_env env, napi_callback_info info)
{
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
    auto cpuUsageOptional = HidebugNativeInterface::CreateInstance()->GetSystemCpuUsage();
    if (!cpuUsageOptional.has_value()) {
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
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    if (engine == nullptr) {
        return CreateUndefined(env);
    }

    napi_value vMMemoryInfo;
    napi_create_object(env, &vMMemoryInfo);

    napi_value totalHeap;
    uint64_t totalHeapValue = engine->GetHeapTotalSize();
    totalHeapValue = totalHeapValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, totalHeapValue, &totalHeap);
    napi_set_named_property(env, vMMemoryInfo, "totalHeap", totalHeap);

    napi_value heapUsed;
    uint64_t heapUsedValue = engine->GetHeapUsedSize();
    heapUsedValue = heapUsedValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, heapUsedValue, &heapUsed);
    napi_set_named_property(env, vMMemoryInfo, "heapUsed", heapUsed);

    napi_value allArraySize;
    uint64_t allArraySizeValue = engine->GetArrayBufferSize();
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
    napi_value result;
    auto nativeInterface = HidebugNativeInterface::CreateInstance();
    if (!nativeInterface) {
        return CreateUndefined(env);
    }
    std::map<uint32_t, double> threadMap = nativeInterface->GetAppThreadCpuUsage();
    ConvertThreadCpuUsageMapToJs(env, result, threadMap);
    return result;
}

napi_value GetAppMemoryLimit(napi_env env, napi_callback_info info)
{
    napi_value appMemoryLimit;
    napi_create_object(env, &appMemoryLimit);
    auto nativeInterface = HidebugNativeInterface::CreateInstance();
    if (!nativeInterface) {
        return CreateUndefined(env);
    }

    auto memoryLimit = nativeInterface->GetAppMemoryLimit();
    if (!memoryLimit) {
        return CreateUndefined(env);
    }
    napi_value rssLimit;
    napi_create_bigint_uint64(env, memoryLimit->rssLimit, &rssLimit);
    napi_set_named_property(env, appMemoryLimit, "rssLimit", rssLimit);

    napi_value vssLimit;
    napi_create_bigint_uint64(env, memoryLimit->vssLimit, &vssLimit);
    napi_set_named_property(env, appMemoryLimit, "vssLimit", vssLimit);

    NativeEngine *engine = reinterpret_cast<NativeEngine *>(env);
    if (engine == nullptr) {
        return CreateUndefined(env);
    }
    napi_value vmHeapLimit;
    uint64_t vmHeapLimitValue = engine->GetHeapLimitSize();
    vmHeapLimitValue = vmHeapLimitValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, vmHeapLimitValue, &vmHeapLimit);
    napi_set_named_property(env, appMemoryLimit, "vmHeapLimit", vmHeapLimit);

    napi_value vmTotalHeapSize;
    uint64_t vmTotalHeapSizeValue = engine->GetProcessHeapLimitSize();
    vmTotalHeapSizeValue = vmTotalHeapSizeValue >> BYTE_2_KB_SHIFT_BITS;
    napi_create_bigint_uint64(env, vmTotalHeapSizeValue, &vmTotalHeapSize);
    napi_set_named_property(env, appMemoryLimit, "vmTotalHeapSize", vmTotalHeapSize);

    return appMemoryLimit;
}

napi_value GetAppNativeMemInfo(napi_env env, napi_callback_info info)
{
    auto nativeInterface = HidebugNativeInterface::CreateInstance();
    if (!nativeInterface) {
        return CreateUndefined(env);
    }

    auto nativeMemInfo = nativeInterface->GetAppNativeMemInfo();
    if (!nativeMemInfo) {
        return CreateUndefined(env);
    }

    napi_value memInfo;
    napi_create_object(env, &memInfo);

    napi_value pss;
    napi_create_bigint_uint64(env, nativeMemInfo->pss, &pss);
    napi_set_named_property(env, memInfo, "pss", pss);

    napi_value rss;
    napi_create_bigint_uint64(env, nativeMemInfo->rss, &rss);
    napi_set_named_property(env, memInfo, "rss", rss);

    napi_value sharedDirty;
    napi_create_bigint_uint64(env, nativeMemInfo->sharedDirty, &sharedDirty);
    napi_set_named_property(env, memInfo, "sharedDirty", sharedDirty);

    napi_value privateDirty;
    napi_create_bigint_uint64(env, nativeMemInfo->privateDirty, &privateDirty);
    napi_set_named_property(env, memInfo, "privateDirty", privateDirty);

    napi_value sharedClean;
    napi_create_bigint_uint64(env, nativeMemInfo->sharedClean, &sharedClean);
    napi_set_named_property(env, memInfo, "sharedClean", sharedClean);

    napi_value privateClean;
    napi_create_bigint_uint64(env, nativeMemInfo->privateClean, &privateClean);
    napi_set_named_property(env, memInfo, "privateClean", privateClean);

    napi_value vss;
    napi_create_bigint_uint64(env, nativeMemInfo->vss, &vss);
    napi_set_named_property(env, memInfo, "vss", vss);

    return memInfo;
}

napi_value GetSystemMemInfo(napi_env env, napi_callback_info info)
{
    auto nativeInterface = HidebugNativeInterface::CreateInstance();
    if (!nativeInterface) {
        return CreateUndefined(env);
    }

    auto systemMemInfo = nativeInterface->GetSystemMemInfo();
    if (!systemMemInfo) {
        return CreateUndefined(env);
    }

    napi_value sysMemInfo;
    napi_create_object(env, &sysMemInfo);

    napi_value totalMem;
    napi_create_bigint_uint64(env, systemMemInfo->memTotal, &totalMem);
    napi_set_named_property(env, sysMemInfo, "totalMem", totalMem);

    napi_value freeMem;
    napi_create_bigint_uint64(env, systemMemInfo->memFree, &freeMem);
    napi_set_named_property(env, sysMemInfo, "freeMem", freeMem);

    napi_value availableMem;
    napi_create_bigint_uint64(env, systemMemInfo->memAvailable, &availableMem);
    napi_set_named_property(env, sysMemInfo, "availableMem", availableMem);

    return sysMemInfo;
}

napi_value StartAppTraceCapture(napi_env env, napi_callback_info info)
{
    napi_value result;
    uint32_t traceFlag = 0;
    uint32_t limitSize = 0;
    std::vector<uint64_t> tags;
    if (!GetTraceParam(env, info, traceFlag, limitSize, tags)) {
        std::string paramErrorMessage = "Invalid argument";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
    }
    uint64_t tag = std::accumulate(tags.begin(), tags.end(), 0ull, [](uint64_t a, uint64_t b) { return a | b; });
    std::string file;
    auto nativeInterface = HidebugNativeInterface::CreateInstance();
    if (!nativeInterface) {
        std::string errorMessage = "The status of the trace is abnormal";
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), errorMessage.c_str());
        return CreateUndefined(env);
    }
    auto ret = nativeInterface->StartAppTraceCapture(tag, traceFlag, limitSize, file);
    if (ret == HIDEBUG_SUCCESS) {
        napi_create_string_utf8(env, file.c_str(), NAPI_AUTO_LENGTH, &result);
        return result;
    }
    if (ret == HIDEBUG_INVALID_ARGUMENT) {
        std::string errorMessage = "Invalid argument";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), errorMessage.c_str());
    }
    if (ret == HIDEBUG_TRACE_CAPTURED_ALREADY) {
        std::string errorMessage = "Capture trace already enabled.";
        napi_throw_error(env, std::to_string(ErrorCode::HAVA_ALREADY_TRACE).c_str(), errorMessage.c_str());
    }
    if (ret == HIDEBUG_NO_PERMISSION) {
        std::string errorMessage = "No write permission on the file.";
        napi_throw_error(env, std::to_string(ErrorCode::WITHOUT_WRITE_PERMISSON).c_str(), errorMessage.c_str());
    }
    std::string errorMessage = "Abnormal trace status.";
    napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), errorMessage.c_str());
    return CreateUndefined(env);
}

napi_value StopAppTraceCapture(napi_env env, napi_callback_info info)
{
    auto nativeInterface = HidebugNativeInterface::CreateInstance();
    if (!nativeInterface) {
        return CreateUndefined(env);
    }
    auto ret = nativeInterface->StopAppTraceCapture();
    if (ret == HIDEBUG_TRACE_ABNORMAL) {
        std::string errorMessage = "The status of the trace is abnormal";
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), errorMessage.c_str());
    }
    if (ret == HIDEBUG_NO_TRACE_RUNNING) {
        std::string errorMessage = "No capture trace running";
        napi_throw_error(env, std::to_string(ErrorCode::NO_CAPTURE_TRACE_RUNNING).c_str(), errorMessage.c_str());
    }
    return CreateUndefined(env);
}

napi_value GetVMRuntimeStats(napi_env env, napi_callback_info info)
{
    napi_value vmRunTimeStats;
    napi_create_object(env, &vmRunTimeStats);
    for (const auto &[k, v] : GC::vmGcMap_) {
        napi_set_named_property(env, vmRunTimeStats, k.c_str(), v(env));
    }
    return vmRunTimeStats;
}

napi_value GetVMRuntimeStat(napi_env env, napi_callback_info info)
{
    std::string param;
    if (!GetTheOnlyStringParam(env, info, param)) {
        std::string paramErrorMessage = "Invalid parameter, a string parameter required.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    if (GC::vmGcMap_.find(param) == GC::vmGcMap_.end()) {
        std::string paramErrorMessage = "Invalid parameter, unknown property.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
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
    if (napi_get_value_int32(env, argv[SECOND_POS], &value) != napi_ok || value < 0) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. Input value error.");
        return false;
    }
    if (!JudgeValueRange(type, value)) {
        HILOG_ERROR(LOG_CORE, "GetAppResourceLimitParam failed. The value range is invalid.");
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
    constexpr mode_t defaultLogDirMode = 0x0770;
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
    if (!CheckVersionType("beta", KEY_HIVIEW_USER_TYPE) &&
        !CheckVersionType("enable", KEY_HIVIEW_DEVELOP_TYPE)) {
        HILOG_ERROR(LOG_CORE, "SetAppResourceLimit failed. Not developer options or beta versions");
        return CreateUndefined(env);
    }
    std::string type = "";
    int32_t value = 0;
    bool enabledDebugLog = false;
    if (!GetAppResourceLimitParam(env, info, type, value, enabledDebugLog)) {
        return CreateUndefined(env);
    }
    if (type == "js_heap") { // js_heap set value
        NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
        engine->SetJsDumpThresholds(value);
    }
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!abilityManager) {
        return CreateUndefined(env);
    }
    sptr<IRemoteObject> remoteObject = abilityManager->CheckSystemAbility(DFX_SYS_HIVIEW_ABILITY_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR(LOG_CORE, "SetAppResourceLimit failed. No this system ability.");
        std::string idErrorMessage = "system ability is not exist.";
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_STATUS_ABNORMAL).c_str(), idErrorMessage.c_str());
        return CreateUndefined(env);
    }
    auto result = HidebugNativeInterface::CreateInstance()->GetMemoryLeakResource(type, value, enabledDebugLog);
    if (result == MemoryState::MEMORY_FAILED) {
        return CreateUndefined(env);
    }
    CreateSanBoxDir();
    return CreateUndefined(env);
}

napi_value IsDebugState(napi_env env, napi_callback_info info)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    if (engine == nullptr) {
        return CreateUndefined(env);
    }

    bool debugState = engine->GetIsDebugModeEnabled() ||
        HidebugNativeInterface::CreateInstance()->IsDebuggerConnected();

    napi_value result = nullptr;
    napi_get_boolean(env, debugState, &result);
    return result;
}

napi_value DeclareHiDebugInterface(napi_env env, napi_value exports)
{
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
        DECLARE_NAPI_FUNCTION("isDebugState", IsDebugState)
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
