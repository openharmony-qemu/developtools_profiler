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

#include "kernel_map_lookup.h"

#include "dfx_map.h"

namespace OHOS {
namespace Developtools {
namespace StackCommon {

size_t FindFirstMapContainingIp(uint64_t ip, const std::vector<HiviewDFX::DfxMap> &maps)
{
    for (size_t i = 0; i < maps.size(); ++i) {
        if (ip > maps[i].begin && ip < maps[i].end) {
            return i;
        }
    }
    return K_INVALID_KERNEL_MAP_INDEX;
}

size_t FindFirstMapContainingIp(uint64_t ip,
                                const std::vector<std::shared_ptr<HiviewDFX::DfxMap>> &maps)
{
    for (size_t i = 0; i < maps.size(); ++i) {
        const auto &m = maps[i];
        if (m != nullptr && ip > m->begin && ip < m->end) {
            return i;
        }
    }
    return K_INVALID_KERNEL_MAP_INDEX;
}

} // namespace StackCommon
} // namespace Developtools
} // namespace OHOS
