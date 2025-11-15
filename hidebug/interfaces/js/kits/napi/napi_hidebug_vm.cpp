/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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


#include "napi_hidebug_vm.h"

#include "error_code.h"
#include "hiappevent_util.h"
#include "napi_util.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
napi_value GetGcCount(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value gcCount;
    napi_create_bigint_uint64(env, engine->GetGCCount(), &gcCount);
    return gcCount;
}

napi_value GetGcTime(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value gcTime;
    napi_create_bigint_uint64(env, engine->GetGCDuration(), &gcTime);
    return gcTime;
}

napi_value GetGcBytesAllocated(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value gcBytesAllocated;
    napi_create_bigint_uint64(env, engine->GetAccumulatedAllocateSize(), &gcBytesAllocated);
    return gcBytesAllocated;
}

napi_value GetGcBytesFreed(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value gcBytesFreed;
    napi_create_bigint_uint64(env, engine->GetAccumulatedFreeSize(), &gcBytesFreed);
    return gcBytesFreed;
}

napi_value GetFullGcLongTimeCount(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value fullGcLongTimeCount;
    napi_create_bigint_uint64(env, engine->GetFullGCLongTimeCount(), &fullGcLongTimeCount);
    return fullGcLongTimeCount;
}

inline std::map<std::string, napi_value (*)(napi_env value)> GetVmGcFunctionMap()
{
    return {
        {"ark.gc.gc-count", GetGcCount},
        {"ark.gc.gc-time", GetGcTime},
        {"ark.gc.gc-bytes-allocated", GetGcBytesAllocated},
        {"ark.gc.gc-bytes-freed", GetGcBytesFreed},
        {"ark.gc.fullgc-longtime-count", GetFullGcLongTimeCount},
    };
}
}

napi_value GetVMRuntimeStats(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("getVMRuntimeStats");
    napi_value vmRunTimeStats;
    napi_create_object(env, &vmRunTimeStats);
    auto vmGCFuncMap = GetVmGcFunctionMap();
    for (const auto &[k, v] : vmGCFuncMap) {
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
    auto vmGCFuncMap = GetVmGcFunctionMap();
    auto target = vmGCFuncMap.find(param);
    if (target == vmGCFuncMap.end()) {
        std::string paramErrorMessage = "Invalid parameter, unknown property.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        apiInvokeRecorder.SetErrorCode(ErrorCode::PARAMETER_ERROR);
        return CreateUndefined(env);
    }
    return target->second(env);
}

napi_value GetAppVMObjectUsedSize(napi_env env, napi_callback_info info)
{
    constexpr uint32_t limitSize = 100;
    static MultipleRecordReporter multipleRecordReporter(0, limitSize);
    ApiInvokeRecorder apiInvokeRecorder("getAppVMObjectUsedSize", multipleRecordReporter);
    uint64_t vmObjectUsed{0};
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    if (engine) {
        constexpr int byte2KbShiftBits = 10;
        vmObjectUsed = static_cast<uint64_t>(engine->GetHeapObjectSize()) >> byte2KbShiftBits;
    }
    napi_value ret;
    napi_create_bigint_uint64(env, vmObjectUsed, &ret);
    return ret;
}
}
}