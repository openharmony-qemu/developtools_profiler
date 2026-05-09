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

#ifndef STACK_COMMON_STACK_MAP_QUERY_H
#define STACK_COMMON_STACK_MAP_QUERY_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class DfxMap;
}

namespace Developtools {
namespace StackCommon {

using DfxMapSharedVec = std::vector<std::shared_ptr<OHOS::HiviewDFX::DfxMap>>;

struct MapAddrLookupResult {
    static constexpr int64_t invalidMapIndex = -1;
    bool ok = false;
    size_t sortedPosition = 0;
    int64_t mapVectorIndex = invalidMapIndex;
};

MapAddrLookupResult LookupMapByAddrIndexed(const DfxMapSharedVec &maps,
                                           const std::vector<int> &sortedIndices, uint64_t addr);

MapAddrLookupResult LookupMapByAddrContiguous(const DfxMapSharedVec &maps,
                                              uint64_t addr);

std::shared_ptr<OHOS::HiviewDFX::DfxMap> FindMapByFileOffset(const DfxMapSharedVec &maps,
                                                             const std::string &name, uint64_t offset);

void SortMapIndicesByEndAscending(const DfxMapSharedVec &maps,
                                  std::vector<int> &sortedIndices);

void SortSharedMapsByEndAscending(DfxMapSharedVec &maps);

} // namespace StackCommon
} // namespace Developtools
} // namespace OHOS

#endif // STACK_COMMON_STACK_MAP_QUERY_H
