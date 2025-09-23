/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef DEVELOPTOOLS_PROFILER_HIDEBUG_ANI_UTIL_H
#define DEVELOPTOOLS_PROFILER_HIDEBUG_ANI_UTIL_H

#include <string>

#include "ani.h"

namespace OHOS {
namespace HiviewDFX {
constexpr int32_t HIDEBUG_DEFAULT_ERROR_CODE = 11400104;  //SYSTEM_STATUS_ABNORMAL
class AniUtil {
public:
    static void ThrowErrorMessage(ani_env *env, const std::string &msg, int32_t errCode = HIDEBUG_DEFAULT_ERROR_CODE);
    static ani_object CreateUndefined(ani_env *env);
    static ani_status ParseAniString(ani_env *env, ani_string aniStr, std::string &str);
    static ani_status ParseAniEnum(ani_env *env, ani_enum_item enumItem, uint32_t &value);
    static ani_status SetNamedPropertyNumber(ani_env *env, ani_object object, const std::string& name, double value);
    static ani_vm* GetAniVm(ani_env *env);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif //DEVELOPTOOLS_PROFILER_HIDEBUG_ANI_UTIL_H
