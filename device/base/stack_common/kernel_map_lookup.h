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

#ifndef STACK_COMMON_KERNEL_MAP_LOOKUP_H
#define STACK_COMMON_KERNEL_MAP_LOOKUP_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "dfx_map.h"

namespace OHOS {
namespace Developtools {
namespace StackCommon {

inline constexpr size_t K_INVALID_KERNEL_MAP_INDEX = static_cast<size_t>(-1);

size_t FindFirstMapContainingIp(uint64_t ip, const std::vector<OHOS::HiviewDFX::DfxMap> &maps);

size_t FindFirstMapContainingIp(uint64_t ip,
                                const std::vector<std::shared_ptr<OHOS::HiviewDFX::DfxMap>> &maps);

} // namespace StackCommon
} // namespace Developtools
} // namespace OHOS

#endif // STACK_COMMON_KERNEL_MAP_LOOKUP_H
