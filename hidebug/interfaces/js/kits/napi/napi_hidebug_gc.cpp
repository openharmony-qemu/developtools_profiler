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


#include "napi_hidebug_gc.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace HiviewDFX {
napi_value GC::GetGcCount(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value gcCount;
    napi_create_bigint_uint64(env, engine->GetGCCount(), &gcCount);
    return gcCount;
}

napi_value GC::GetGcTime(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value gcTime;
    napi_create_bigint_uint64(env, engine->GetGCDuration(), &gcTime);
    return gcTime;
}

napi_value GC::GetGcBytesAllocated(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value gcBytesAllocated;
    napi_create_bigint_uint64(env, engine->GetAccumulatedAllocateSize(), &gcBytesAllocated);
    return gcBytesAllocated;
}

napi_value GC::GetGcBytesFreed(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value gcBytesFreed;
    napi_create_bigint_uint64(env, engine->GetAccumulatedFreeSize(), &gcBytesFreed);
    return gcBytesFreed;
}

napi_value GC::GetFullGcLongTimeCount(napi_env env)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    napi_value fullGcLongTimeCount;
    napi_create_bigint_uint64(env, engine->GetFullGCLongTimeCount(), &fullGcLongTimeCount);
    return fullGcLongTimeCount;
}

std::map<std::string, napi_value (*)(napi_env value)> GC::vmGcMap_ {
    {"ark.gc.gc-count", GC::GetGcCount},
    {"ark.gc.gc-time", GC::GetGcTime},
    {"ark.gc.gc-bytes-allocated", GC::GetGcBytesAllocated},
    {"ark.gc.gc-bytes-freed", GC::GetGcBytesFreed},
    {"ark.gc.fullgc-longtime-count", GC::GetFullGcLongTimeCount},
};
}
}