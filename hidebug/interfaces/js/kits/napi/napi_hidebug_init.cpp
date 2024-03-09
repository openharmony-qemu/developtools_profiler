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

#include "napi_hidebug_init.h"

#include <map>
#include <utility>
#include <vector>

#include "hitrace_meter.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int32_t MAIN_THREAD = 1;
constexpr int32_t ALL_THREADS = 2;
const std::string TRACE_FLAG_CLASS_NAME = "TraceFlag";
const std::string TAGS_CLASS_NAME = "tags";

napi_value ClassConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv = nullptr;
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, &thisArg, &data);

    napi_value global = 0;
    napi_get_global(env, &global);

    return thisArg;
}

napi_value CreateInt32(const napi_env env, int32_t num)
{
    napi_value intValue = nullptr;
    if (napi_create_int32(env, num, &intValue) != napi_ok) {
        return nullptr;
    }
    return intValue;
}

napi_value CreateBigintUint64(const napi_env env, uint64_t num)
{
    napi_value intValue = nullptr;
    if (napi_create_bigint_uint64(env, num, &intValue) != napi_ok) {
        return nullptr;
    }
    return intValue;
}

void InitTraceFlagVector(napi_env env, std::vector<std::pair<const char*, napi_value>>& traceFlagVector)
{
    traceFlagVector.emplace_back("MAIN_THREAD", CreateInt32(env, MAIN_THREAD));
    traceFlagVector.emplace_back("ALL_THREADS", CreateInt32(env, ALL_THREADS));
}

void InitTagsVector(napi_env env, std::vector<std::pair<const char*, napi_value>>& tagsVector)
{
    tagsVector.emplace_back("ABILITY_MANAGER", CreateBigintUint64(env, HITRACE_TAG_ABILITY_MANAGER));
    tagsVector.emplace_back("ARKUI", CreateBigintUint64(env, HITRACE_TAG_ACE));
    tagsVector.emplace_back("ARK", CreateBigintUint64(env, HITRACE_TAG_ARK));
    tagsVector.emplace_back("BLUETOOTH", CreateBigintUint64(env, HITRACE_TAG_BLUETOOTH));
    tagsVector.emplace_back("COMMON_LIBRARY", CreateBigintUint64(env, HITRACE_TAG_COMMONLIBRARY));
    tagsVector.emplace_back("DISTRIBUTED_HARDWARE_DEVICE_MANAGER", CreateBigintUint64(env, HITRACE_TAG_DEVICE_MANAGER));
    tagsVector.emplace_back("DISTRIBUTED_AUDIO", CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_AUDIO));
    tagsVector.emplace_back("DISTRIBUTED_CAMERA", CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_CAMERA));
    tagsVector.emplace_back("DISTRIBUTED_DATA", CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTEDDATA));
    tagsVector.emplace_back("DISTRIBUTED_HARDWARE_FRAMEWORK",
        CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_HARDWARE_FWK));
    tagsVector.emplace_back("DISTRIBUTED_INPUT", CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_INPUT));
    tagsVector.emplace_back("DISTRIBUTED_SCREEN", CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_SCREEN));
    tagsVector.emplace_back("DISTRIBUTED_SCHEDULER", CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_SCHEDULE));
    tagsVector.emplace_back("FFRT", CreateBigintUint64(env, HITRACE_TAG_FFRT));
    tagsVector.emplace_back("FILE_MANAGEMENT", CreateBigintUint64(env, HITRACE_TAG_FILEMANAGEMENT));
    tagsVector.emplace_back("GLOBAL_RESOURCE_MANAGER", CreateBigintUint64(env, HITRACE_TAG_GLOBAL_RESMGR));
    tagsVector.emplace_back("GRAPHICS", CreateBigintUint64(env, HITRACE_TAG_GRAPHIC_AGP));
    tagsVector.emplace_back("HDF", CreateBigintUint64(env, HITRACE_TAG_HDF));
    tagsVector.emplace_back("MISC", CreateBigintUint64(env, HITRACE_TAG_MISC));
    tagsVector.emplace_back("MULTIMODAL_INPUT", CreateBigintUint64(env, HITRACE_TAG_MULTIMODALINPUT));
    tagsVector.emplace_back("NET", CreateBigintUint64(env, HITRACE_TAG_NET));
    tagsVector.emplace_back("NOTIFICATION", CreateBigintUint64(env, HITRACE_TAG_NOTIFICATION));
    tagsVector.emplace_back("NWEB", CreateBigintUint64(env, HITRACE_TAG_NWEB));
    tagsVector.emplace_back("OHOS", CreateBigintUint64(env, HITRACE_TAG_OHOS));
    tagsVector.emplace_back("POWER_MANAGER", CreateBigintUint64(env, HITRACE_TAG_POWER));
    tagsVector.emplace_back("RPC", CreateBigintUint64(env, HITRACE_TAG_RPC));
    tagsVector.emplace_back("SAMGR", CreateBigintUint64(env, HITRACE_TAG_SAMGR));
    tagsVector.emplace_back("WINDOW_MANAGER", CreateBigintUint64(env, HITRACE_TAG_WINDOW_MANAGER));
    tagsVector.emplace_back("AUDIO", CreateBigintUint64(env, HITRACE_TAG_ZAUDIO));
    tagsVector.emplace_back("CAMERA", CreateBigintUint64(env, HITRACE_TAG_ZCAMERA));
    tagsVector.emplace_back("IMAGE", CreateBigintUint64(env, HITRACE_TAG_ZIMAGE));
    tagsVector.emplace_back("MEDIA", CreateBigintUint64(env, HITRACE_TAG_ZMEDIA));
}

void InitConstClassByName(napi_env env, napi_value exports, const std::string& name)
{
    std::vector<std::pair<const char*, napi_value>> propertyVector;
    if (name == TRACE_FLAG_CLASS_NAME) {
        InitTraceFlagVector(env, propertyVector);
    } else if (name == TAGS_CLASS_NAME) {
        InitTagsVector(env, propertyVector);
    } else {
        return;
    }

    int i = 0;
    napi_property_descriptor descriptors[propertyVector.size()];
    for (auto& it : propertyVector) {
        descriptors[i++] = DECLARE_NAPI_STATIC_PROPERTY(it.first, it.second);
    }

    napi_value result = nullptr;
    napi_define_class(env, name.c_str(), NAPI_AUTO_LENGTH, ClassConstructor, nullptr,
        sizeof(descriptors) / sizeof(*descriptors), descriptors, &result);
    napi_set_named_property(env, exports, name.c_str(), result);
}
}

napi_value InitNapiClass(napi_env env, napi_value exports)
{
    InitConstClassByName(env, exports, TRACE_FLAG_CLASS_NAME);
    InitConstClassByName(env, exports, TAGS_CLASS_NAME);
    return exports;
}

} // namespace HiviewDFX
} // namespace OHOS