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

void InitTraceFlagMap(napi_env env, std::map<const char*, napi_value>& traceFlagMap)
{
    traceFlagMap["MAIN_THREAD"] = CreateInt32(env, MAIN_THREAD);
    traceFlagMap["ALL_THREADS"] = CreateInt32(env, ALL_THREADS);
}

void InitTagsMap(napi_env env, std::map<const char*, napi_value>& tagsMap)
{
    tagsMap["ABILITY_MANAGER"] = CreateBigintUint64(env, HITRACE_TAG_ABILITY_MANAGER);
    tagsMap["ARKUI"] = CreateBigintUint64(env, HITRACE_TAG_ACE);
    tagsMap["ARK"] = CreateBigintUint64(env, HITRACE_TAG_ARK);
    tagsMap["BLUETOOTH"] = CreateBigintUint64(env, HITRACE_TAG_BLUETOOTH);
    tagsMap["COMMON_LIBRARY"] = CreateBigintUint64(env, HITRACE_TAG_COMMONLIBRARY);
    tagsMap["DISTRIBUTED_HARDWARE_DEVICE_MANAGER"] = CreateBigintUint64(env, HITRACE_TAG_DEVICE_MANAGER);
    tagsMap["DISTRIBUTED_AUDIO"] = CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_AUDIO);
    tagsMap["DISTRIBUTED_CAMERA"] = CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_CAMERA);
    tagsMap["DISTRIBUTED_DATA"] = CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTEDDATA);
    tagsMap["DISTRIBUTED_HARDWARE_FRAMEWORK"] = CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_HARDWARE_FWK);
    tagsMap["DISTRIBUTED_INPUT"] = CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_INPUT);
    tagsMap["DISTRIBUTED_SCREEN"] = CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_SCREEN);
    tagsMap["DISTRIBUTED_SCHEDULER"] = CreateBigintUint64(env, HITRACE_TAG_DISTRIBUTED_SCHEDULE);
    tagsMap["FFRT"] = CreateBigintUint64(env, HITRACE_TAG_FFRT);
    tagsMap["FILE_MANAGEMENT"] = CreateBigintUint64(env, HITRACE_TAG_FILEMANAGEMENT);
    tagsMap["GLOBAL_RESOURCE_MANAGER"] = CreateBigintUint64(env, HITRACE_TAG_GLOBAL_RESMGR);
    tagsMap["GRAPHICS"] = CreateBigintUint64(env, HITRACE_TAG_GRAPHIC_AGP);
    tagsMap["HDF"] = CreateBigintUint64(env, HITRACE_TAG_HDF);
    tagsMap["MISC"] = CreateBigintUint64(env, HITRACE_TAG_MISC);
    tagsMap["MULTIMODAL_INPUT"] = CreateBigintUint64(env, HITRACE_TAG_MULTIMODALINPUT);
    tagsMap["NET"] = CreateBigintUint64(env, HITRACE_TAG_NET);
    tagsMap["NOTIFICATION"] = CreateBigintUint64(env, HITRACE_TAG_NOTIFICATION);
    tagsMap["NWEB"] = CreateBigintUint64(env, HITRACE_TAG_NWEB);
    tagsMap["OHOS"] = CreateBigintUint64(env, HITRACE_TAG_OHOS);
    tagsMap["POWER_MANAGER"] = CreateBigintUint64(env, HITRACE_TAG_POWER);
    tagsMap["RPC"] = CreateBigintUint64(env, HITRACE_TAG_RPC);
    tagsMap["SAMGR"] = CreateBigintUint64(env, HITRACE_TAG_SAMGR);
    tagsMap["WINDOW_MANAGER"] = CreateBigintUint64(env, HITRACE_TAG_WINDOW_MANAGER);
    tagsMap["AUDIO"] = CreateBigintUint64(env, HITRACE_TAG_ZAUDIO);
    tagsMap["CAMERA"] = CreateBigintUint64(env, HITRACE_TAG_ZCAMERA);
    tagsMap["IMAGE"] = CreateBigintUint64(env, HITRACE_TAG_ZIMAGE);
    tagsMap["MEDIA"] = CreateBigintUint64(env, HITRACE_TAG_ZMEDIA);
}

void InitConstClassByName(napi_env env, napi_value exports, const std::string& name)
{
    std::map<const char*, napi_value> propertyMap;
    if (name == TRACE_FLAG_CLASS_NAME) {
        InitTraceFlagMap(env, propertyMap);
    } else if (name == TAGS_CLASS_NAME) {
        InitTagsMap(env, propertyMap);
    } else {
        return;
    }

    int i = 0;
    napi_property_descriptor descriptors[propertyMap.size()];
    for (const auto it : propertyMap) {
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