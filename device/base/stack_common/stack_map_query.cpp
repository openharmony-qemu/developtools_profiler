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
#include "stack_map_query.h"

#include "dfx_map.h"

namespace OHOS {
namespace Developtools {
namespace StackCommon {

using DfxMapSharedVec = std::vector<std::shared_ptr<HiviewDFX::DfxMap>>;

namespace {
constexpr int DIVISOR = 2;

template<typename EndFn, typename BeginFn>
MapAddrLookupResult LookupSortedByEnd(size_t n, uint64_t addr, EndFn endAt, BeginFn beginAt)
{
    MapAddrLookupResult out {};
    if (n == 0) {
        return out;
    }
    if (beginAt(0) > addr) {
        return out;
    }
    const size_t last = n >= 1 ? n - 1 : 0;
    if (endAt(last) <= addr) {
        return out;
    }
    size_t left = 0;
    size_t right = n;
    size_t mid = (right - left) / DIVISOR + left;
    while (left < right) {
        if (addr < endAt(mid)) {
            right = mid;
            mid = (right - left) / DIVISOR + left;
            continue;
        }
        if (addr >= endAt(mid)) {
            left = mid + 1;
            mid = (right - left) / DIVISOR + left;
            continue;
        }
    }
    if (left >= n) {
        return out;
    }
    if (addr >= beginAt(left) && addr < endAt(left)) {
        out.ok = true;
        out.sortedPosition = left;
        out.mapVectorIndex = static_cast<int64_t>(left);
    }
    return out;
}
} // namespace

MapAddrLookupResult LookupMapByAddrIndexed(const DfxMapSharedVec &maps,
                                           const std::vector<int> &sortedIndices, uint64_t addr)
{
    MapAddrLookupResult out {};
    const size_t n = sortedIndices.size();
    if (maps.empty() || n == 0) {
        return out;
    }
    auto endAt = [&maps, &sortedIndices](size_t sortedPos) { return maps[sortedIndices[sortedPos]]->end; };
    auto beginAt = [&maps, &sortedIndices](size_t sortedPos) { return maps[sortedIndices[sortedPos]]->begin; };
    auto inner = LookupSortedByEnd(n, addr, endAt, beginAt);
    if (!inner.ok) {
        return out;
    }
    inner.mapVectorIndex = static_cast<int64_t>(sortedIndices[inner.sortedPosition]);
    return inner;
}

MapAddrLookupResult LookupMapByAddrContiguous(const DfxMapSharedVec &maps,
                                              uint64_t addr)
{
    const size_t n = maps.size();
    if (n == 0) {
        return {};
    }
    auto endAt = [&maps](size_t sortedPos) { return maps[sortedPos]->end; };
    auto beginAt = [&maps](size_t sortedPos) { return maps[sortedPos]->begin; };
    auto inner = LookupSortedByEnd(n, addr, endAt, beginAt);
    if (!inner.ok) {
        return {};
    }
    inner.mapVectorIndex = static_cast<int64_t>(inner.sortedPosition);
    return inner;
}

std::shared_ptr<HiviewDFX::DfxMap> FindMapByFileOffset(const DfxMapSharedVec &maps,
                                                       const std::string &name, uint64_t offset)
{
    for (const auto &map : maps) {
        if (map == nullptr) {
            continue;
        }
        if (name != map->name) {
            continue;
        }
        if (offset >= map->offset && (offset - map->offset) < (map->end - map->begin)) {
            return map;
        }
    }
    return nullptr;
}

void SortMapIndicesByEndAscending(const DfxMapSharedVec &maps,
                                  std::vector<int> &sortedIndices)
{
    const int n = static_cast<int>(sortedIndices.size());
    for (int currPos = 1; currPos < n; ++currPos) {
        int targetPos = currPos - 1;
        while (targetPos >= 0 && maps[sortedIndices[currPos]]->end < maps[sortedIndices[targetPos]]->end) {
            --targetPos;
        }
        if (targetPos < currPos - 1) {
            int target = sortedIndices[currPos];
            for (int k = currPos - 1; k > targetPos; --k) {
                sortedIndices[k + 1] = sortedIndices[k];
            }
            sortedIndices[targetPos + 1] = target;
        }
    }
}

void SortSharedMapsByEndAscending(DfxMapSharedVec &maps)
{
    for (size_t currPos = 1; currPos < maps.size(); ++currPos) {
        int targetPos = static_cast<int>(currPos) - 1;
        auto target = maps[currPos];
        while (targetPos >= 0 && target->end < maps[static_cast<size_t>(targetPos)]->end) {
            maps[static_cast<size_t>(targetPos + 1)] = maps[static_cast<size_t>(targetPos)];
            --targetPos;
        }
        maps[static_cast<size_t>(targetPos + 1)] = target;
    }
}

} // namespace StackCommon
} // namespace Developtools
} // namespace OHOS
