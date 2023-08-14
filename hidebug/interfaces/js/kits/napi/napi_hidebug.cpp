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
#include <unistd.h>

#include "application_context.h"
#include "context.h"
#include "directory_ex.h"
#include "dump_usage.h"
#include "hilog/log.h"
#include "iservice_registry.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"
#include "refbase.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D0A, "HiDebug_NAPI" };
constexpr int ONE_VALUE_LIMIT = 1;
constexpr int ARRAY_INDEX_FIRST = 0;
const std::string SLASH_STR = "/";
const std::string DEFAULT_FILENAME = "undefined";
const std::string JSON_FILE = ".json";
const std::string HEAPSNAPSHOT_FILE = ".heapsnapshot";
enum ErrorCode {
    PERMISSION_ERROR = 201,
    PARAMETER_ERROR = 401,
    VERSION_ERROR = 801,
    SYSTEM_ABILITY_NOT_FOUND = 11400101
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
        HiLog::Error(LABEL, "file create failed, errno = %{public}d", errno);
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
        HiLog::Error(LABEL, "invalid number = %{public}d of params.", ONE_VALUE_LIMIT);
        return false;
    }
    int thirdPos = 2;
    if (!MatchValueType(env, argv[0], napi_number) &&
        !MatchValueType(env, argv[1], napi_number) &&
        !MatchValueType(env, argv[thirdPos], napi_object)) {
        HiLog::Error(LABEL, "params type error.");
        return false;
    }
    if (napi_get_value_int32(env, argv[0], &serviceId) != napi_ok) {
        HiLog::Error(LABEL, "Get input serviceId failed.");
        return false;
    }
    if (napi_get_value_int32(env, argv[1], &fd) != napi_ok) {
        HiLog::Error(LABEL, "Get input fd failed.");
        return false;
    }
    uint32_t arraySize = 0;
    if (!IsArrayForNapiValue(env, argv[thirdPos], arraySize)) {
        HiLog::Error(LABEL, "Get input args failed.");
        return false;
    }
    for (uint32_t i = 0; i < arraySize; i++) {
        napi_value jsValue = nullptr;
        if (napi_get_element(env, argv[thirdPos], i, &jsValue) != napi_ok) {
            HiLog::Error(LABEL, "get_element -> Get input args failed.");
            return false;
        }
        const size_t bufSize = 256;
        size_t bufLen = 0;
        char buf[bufSize] = {0};
        if (napi_get_value_string_utf8(env, jsValue, buf, bufSize - 1, &bufLen) != napi_ok) {
            HiLog::Error(LABEL, "get_value -> Get input args failed.");
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
        HiLog::Error(LABEL, "invalid number = %{public}d of params.", ONE_VALUE_LIMIT);
        return DEFAULT_FILENAME;
    }
    if (!MatchValueType(env, argv[ARRAY_INDEX_FIRST], napi_string)) {
        HiLog::Error(LABEL, "Type error, should be string type!");
        return DEFAULT_FILENAME;
    }
    size_t bufLen = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &bufLen);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get input filename param length failed.");
        return DEFAULT_FILENAME;
    }
    const int bufMax = 128;
    if (bufLen > bufMax || bufLen == 0) {
        HiLog::Error(LABEL, "input filename param length is illegal.");
        return DEFAULT_FILENAME;
    }
    char buf[bufLen + 1];
    napi_get_value_string_utf8(env, argv[0], buf, bufLen + 1, &bufLen);
    std::string fileName = buf;
    return fileName;
}

static bool GetFileNameParamThrowErrorVersion(napi_env env, napi_callback_info info, std::string &fileName)
{
    size_t argc = ONE_VALUE_LIMIT;
    napi_value argv[ONE_VALUE_LIMIT] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc != ONE_VALUE_LIMIT) {
        HiLog::Error(LABEL, "invalid number = %{public}d of params.", ONE_VALUE_LIMIT);
        return false;
    }
    if (!MatchValueType(env, argv[ARRAY_INDEX_FIRST], napi_string)) {
        HiLog::Error(LABEL, "Type error, should be string type!");
        return false;
    }
    size_t bufLen = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &bufLen);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get input filename param length failed.");
        return false;
    }
    const int bufMax = 128;
    if (bufLen > bufMax || bufLen == 0) {
        HiLog::Error(LABEL, "input filename param length is illegal.");
        return false;
    }
    char buf[bufLen + 1];
    napi_get_value_string_utf8(env, argv[0], buf, bufLen + 1, &bufLen);
    fileName = buf;
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
    if (!GetFileNameParamThrowErrorVersion(env, info, fileName)) {
        std::string paramErrorMessage = "Invalid parameter, require a string parameter.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    HiLog::Info(LABEL, "filename: %{public}s.", fileName.c_str());
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
    engine->DumpHeapSnapshot(filePath);
    return CreateUndefined(env);
}

napi_value DumpJsHeapData(napi_env env, napi_callback_info info)
{
    std::string fileName;
    if (!GetFileNameParamThrowErrorVersion(env, info, fileName)) {
        std::string paramErrorMessage = "Invalid parameter, require a string parameter.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    HiLog::Error(LABEL, "filename: %{public}s.", fileName.c_str());
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
    engine->DumpHeapSnapshot(filePath);
    return CreateUndefined(env);
}

napi_value GetPss(napi_env env, napi_callback_info info)
{
    napi_value pss;
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    if (dumpUsage) {
        int pid = getpid();
        uint64_t pssInfo = dumpUsage->GetPss(pid);
        napi_create_bigint_uint64(env, pssInfo, &pss);
    } else {
        napi_create_bigint_uint64(env, 0, &pss);
    }
    return pss;
}

napi_value GetSharedDirty(napi_env env, napi_callback_info info)
{
    napi_value sharedDirty;
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    if (dumpUsage) {
        int pid = getpid();
        uint64_t sharedDirtyInfo = dumpUsage->GetSharedDirty(pid);
        napi_create_bigint_uint64(env, sharedDirtyInfo, &sharedDirty);
    } else {
        napi_create_bigint_uint64(env, 0, &sharedDirty);
    }
    return sharedDirty;
}

napi_value GetPrivateDirty(napi_env env, napi_callback_info info)
{
    napi_value privateDirtyValue;
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    if (dumpUsage) {
        pid_t pid = getpid();
        uint64_t privateDirty = dumpUsage->GetPrivateDirty(pid);
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
    if (dumpUsage) {
        pid_t pid = getpid();
        float tmpCpuUsage = dumpUsage->GetCpuUsage(pid);
        double cpuUsage = double(tmpCpuUsage);
        napi_create_double(env, cpuUsage, &cpuUsageValue);
    } else {
        napi_create_double(env, 0, &cpuUsageValue);
    }
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
        HiLog::Error(LABEL, "no this system ability.");
        std::string idErrorMessage = "service id is invalid, system ability is not exist.";
        napi_throw_error(env, std::to_string(ErrorCode::SYSTEM_ABILITY_NOT_FOUND).c_str(), idErrorMessage.c_str());
        return CreateUndefined(env);
    }
    int dumpResult = sa->Dump(fd, args);
    HiLog::Info(LABEL, "Dump result: %{public}d", dumpResult);
    return CreateUndefined(env);
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
        DECLARE_NAPI_FUNCTION("getNativeHeapFreeSize", GetNativeHeapFreeSize)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
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
