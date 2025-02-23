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

#ifndef NAPI_HIDEBUG_DUMP_H
#define NAPI_HIDEBUG_DUMP_H

#include "napi/native_api.h"

namespace OHOS {
namespace HiviewDFX {
napi_value DumpHeapData(napi_env env, napi_callback_info info);
napi_value DumpJsHeapData(napi_env env, napi_callback_info info);
napi_value DumpJsRawHeapData(napi_env env, napi_callback_info info);
}
}

#endif //NAPI_HIDEBUG_DUMP_H
