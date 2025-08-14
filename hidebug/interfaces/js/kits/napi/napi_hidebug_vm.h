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


#ifndef HIVIEWDFX_NAPI_HIDEBUG_GC_H
#define HIVIEWDFX_NAPI_HIDEBUG_GC_H

#include <map>
#include <string>

#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
napi_value GetVMRuntimeStats(napi_env env, napi_callback_info info);
napi_value GetVMRuntimeStat(napi_env env, napi_callback_info info);
}
}
#endif //HIVIEWDFX_NAPI_HIDEBUG_GC_H
