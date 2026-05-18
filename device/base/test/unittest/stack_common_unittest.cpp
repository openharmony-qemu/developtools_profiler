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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "dfx_map.h"
#include "kernel_map_lookup.h"
#include "stack_map_query.h"

namespace {
using namespace OHOS::Developtools::StackCommon;
using namespace OHOS::HiviewDFX;
using namespace testing::ext;

std::shared_ptr<DfxMap> MakeSharedMap(uint64_t begin, uint64_t end, uint64_t offset = 0,
                                      const std::string &name = "")
{
    auto m = std::make_shared<DfxMap>();
    m->begin = begin;
    m->end = end;
    m->offset = offset;
    m->name = name;
    return m;
}

DfxMap MakeMapVal(uint64_t begin, uint64_t end)
{
    DfxMap m;
    m.begin = begin;
    m.end = end;
    return m;
}

class StackCommonTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: StackCommonTest
 * @tc.desc: LookupMapByAddrContiguous empty and early exits.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, LookupMapByAddrContiguous_EmptyAndBounds, TestSize.Level1)
{
    std::vector<std::shared_ptr<DfxMap>> empty;
    auto r0 = LookupMapByAddrContiguous(empty, 0x1000u);
    EXPECT_FALSE(r0.ok);
    EXPECT_EQ(r0.mapVectorIndex, MapAddrLookupResult::invalidMapIndex);

    std::vector<std::shared_ptr<DfxMap>> maps = {
        MakeSharedMap(0x1000u, 0x2000u),
        MakeSharedMap(0x3000u, 0x4000u),
    };
    auto r1 = LookupMapByAddrContiguous(maps, 0x500u);
    EXPECT_FALSE(r1.ok);

    auto r2 = LookupMapByAddrContiguous(maps, 0x5000u);
    EXPECT_FALSE(r2.ok);

    auto r3 = LookupMapByAddrContiguous(maps, 0x2500u);
    EXPECT_FALSE(r3.ok);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: LookupMapByAddrContiguous hit first and second segment.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, LookupMapByAddrContiguous_HitMaps, TestSize.Level1)
{
    std::vector<std::shared_ptr<DfxMap>> one = { MakeSharedMap(0x1000u, 0x2000u) };
    auto r0 = LookupMapByAddrContiguous(one, 0x1500u);
    ASSERT_TRUE(r0.ok);
    EXPECT_EQ(r0.mapVectorIndex, 0);

    std::vector<std::shared_ptr<DfxMap>> maps = {
        MakeSharedMap(0x1000u, 0x2000u),
        MakeSharedMap(0x3000u, 0x4000u),
    };
    auto r1 = LookupMapByAddrContiguous(maps, 0x1500u);
    ASSERT_TRUE(r1.ok);
    EXPECT_EQ(r1.sortedPosition, 0u);
    EXPECT_EQ(r1.mapVectorIndex, 0);

    auto r2 = LookupMapByAddrContiguous(maps, 0x3500u);
    ASSERT_TRUE(r2.ok);
    EXPECT_EQ(r2.sortedPosition, 1u);
    EXPECT_EQ(r2.mapVectorIndex, 1);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: LookupMapByAddrContiguous begin inclusive and end exclusive boundaries.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, LookupMapByAddrContiguous_BoundaryTransitions, TestSize.Level1)
{
    std::vector<std::shared_ptr<DfxMap>> maps = {
        MakeSharedMap(0x1000u, 0x2000u),
        MakeSharedMap(0x2000u, 0x3000u),
        MakeSharedMap(0x3000u, 0x3800u),
    };

    auto atBegin = LookupMapByAddrContiguous(maps, 0x1000u);
    ASSERT_TRUE(atBegin.ok);
    EXPECT_EQ(atBegin.mapVectorIndex, 0);

    auto atSplit = LookupMapByAddrContiguous(maps, 0x2000u);
    ASSERT_TRUE(atSplit.ok);
    EXPECT_EQ(atSplit.mapVectorIndex, 1);

    auto atSecondEnd = LookupMapByAddrContiguous(maps, 0x3000u);
    ASSERT_TRUE(atSecondEnd.ok);
    EXPECT_EQ(atSecondEnd.mapVectorIndex, 2);

    auto atLastEnd = LookupMapByAddrContiguous(maps, 0x3800u);
    EXPECT_FALSE(atLastEnd.ok);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: LookupMapByAddrIndexed empty maps or indices.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, LookupMapByAddrIndexed_EmptyInputs, TestSize.Level1)
{
    std::vector<std::shared_ptr<DfxMap>> maps = { MakeSharedMap(0x1000u, 0x2000u) };
    std::vector<int> emptyIdx;
    auto r0 = LookupMapByAddrIndexed(maps, emptyIdx, 0x1500u);
    EXPECT_FALSE(r0.ok);

    std::vector<std::shared_ptr<DfxMap>> emptyMaps;
    std::vector<int> idx = { 0 };
    auto r1 = LookupMapByAddrIndexed(emptyMaps, idx, 0x1500u);
    EXPECT_FALSE(r1.ok);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: LookupMapByAddrIndexed permuted indices resolve correct vector index.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, LookupMapByAddrIndexed_PermutedOrder, TestSize.Level1)
{
    auto m0 = MakeSharedMap(0x1000u, 0x5000u);
    auto m1 = MakeSharedMap(0x6000u, 0x7000u);
    std::vector<std::shared_ptr<DfxMap>> maps = { m1, m0 };
    std::vector<int> sortedByEnd = { 1, 0 };
    auto r = LookupMapByAddrIndexed(maps, sortedByEnd, 0x2000u);
    ASSERT_TRUE(r.ok);
    EXPECT_EQ(r.sortedPosition, 0u);
    EXPECT_EQ(r.mapVectorIndex, 1);

    auto rMiss = LookupMapByAddrIndexed(maps, sortedByEnd, 0x5500u);
    EXPECT_FALSE(rMiss.ok);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: LookupMapByAddrIndexed validates lower/upper and split boundaries.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, LookupMapByAddrIndexed_BoundaryTransitions, TestSize.Level1)
{
    auto m0 = MakeSharedMap(0x2000u, 0x3000u);
    auto m1 = MakeSharedMap(0x1000u, 0x2000u);
    auto m2 = MakeSharedMap(0x3000u, 0x4000u);
    std::vector<std::shared_ptr<DfxMap>> maps = { m2, m0, m1 };
    std::vector<int> sortedByEnd = { 2, 1, 0 };

    auto belowFirstBegin = LookupMapByAddrIndexed(maps, sortedByEnd, 0x0FFFu);
    EXPECT_FALSE(belowFirstBegin.ok);

    auto atBegin = LookupMapByAddrIndexed(maps, sortedByEnd, 0x1000u);
    ASSERT_TRUE(atBegin.ok);
    EXPECT_EQ(atBegin.sortedPosition, 0u);
    EXPECT_EQ(atBegin.mapVectorIndex, 2);

    auto atSplit = LookupMapByAddrIndexed(maps, sortedByEnd, 0x2000u);
    ASSERT_TRUE(atSplit.ok);
    EXPECT_EQ(atSplit.sortedPosition, 1u);
    EXPECT_EQ(atSplit.mapVectorIndex, 1);

    auto atLastEnd = LookupMapByAddrIndexed(maps, sortedByEnd, 0x4000u);
    EXPECT_FALSE(atLastEnd.ok);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: FindMapByFileOffset filters nullptr, name, and vma span.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, FindMapByFileOffset_Branches, TestSize.Level1)
{
    std::vector<std::shared_ptr<DfxMap>> maps = {
        nullptr,
        MakeSharedMap(0x10000u, 0x11000u, 0x1000u, "/foo.so"),
        MakeSharedMap(0x20000u, 0x21000u, 0x1000u, "/bar.so"),
    };
    EXPECT_EQ(FindMapByFileOffset(maps, "/none.so", 0x1000u), nullptr);
    EXPECT_EQ(FindMapByFileOffset(maps, "/foo.so", 0x999u), nullptr);
    EXPECT_EQ(FindMapByFileOffset(maps, "/foo.so", 0x500u), nullptr);
    auto hit = FindMapByFileOffset(maps, "/foo.so", 0x1000u);
    ASSERT_NE(hit, nullptr);
    EXPECT_EQ(hit->name, "/foo.so");
    EXPECT_EQ(FindMapByFileOffset(maps, "/foo.so", 0x2000u), nullptr);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: FindMapByFileOffset continues after same-name range miss and matches later map.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, FindMapByFileOffset_LaterMapMatch, TestSize.Level1)
{
    auto first = MakeSharedMap(0x10000u, 0x10800u, 0x1000u, "/same.so");
    auto second = MakeSharedMap(0x20000u, 0x21000u, 0x1800u, "/same.so");
    std::vector<std::shared_ptr<DfxMap>> maps = { first, second };

    auto hit = FindMapByFileOffset(maps, "/same.so", 0x1900u);
    ASSERT_NE(hit, nullptr);
    EXPECT_EQ(hit->begin, second->begin);

    EXPECT_EQ(FindMapByFileOffset(maps, "/same.so", 0x2800u), nullptr);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: SortMapIndicesByEndAscending sorts index array by map end.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, SortMapIndicesByEndAscending, TestSize.Level1)
{
    std::vector<std::shared_ptr<DfxMap>> maps = {
        MakeSharedMap(0u, 0x3000u),
        MakeSharedMap(0u, 0x1000u),
        MakeSharedMap(0u, 0x2000u),
    };
    std::vector<int> zero;
    SortMapIndicesByEndAscending(maps, zero);
    EXPECT_TRUE(zero.empty());

    std::vector<int> indices = { 0, 1, 2 };
    SortMapIndicesByEndAscending(maps, indices);
    ASSERT_EQ(indices.size(), 3u);
    EXPECT_EQ(indices[0], 1);
    EXPECT_EQ(indices[1], 2);
    EXPECT_EQ(indices[2], 0);

    std::vector<int> already = { 1, 2, 0 };
    SortMapIndicesByEndAscending(maps, already);
    EXPECT_EQ(already, indices);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: SortMapIndicesByEndAscending handles multi-shift and equal-end stability.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, SortMapIndicesByEndAscending_MultiShiftStable, TestSize.Level1)
{
    std::vector<std::shared_ptr<DfxMap>> maps = {
        MakeSharedMap(0u, 0x5000u),
        MakeSharedMap(0u, 0x1000u),
        MakeSharedMap(0u, 0x3000u),
        MakeSharedMap(0u, 0x3000u),
        MakeSharedMap(0u, 0x2000u),
    };
    std::vector<int> indices = { 0, 1, 2, 3, 4 };
    SortMapIndicesByEndAscending(maps, indices);
    std::vector<int> expected = { 1, 4, 2, 3, 0 };
    EXPECT_EQ(indices, expected);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: SortSharedMapsByEndAscending reorders map vector.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, SortSharedMapsByEndAscending, TestSize.Level1)
{
    std::vector<std::shared_ptr<DfxMap>> empty;
    SortSharedMapsByEndAscending(empty);
    EXPECT_TRUE(empty.empty());

    auto single = MakeSharedMap(0u, 0x1000u);
    std::vector<std::shared_ptr<DfxMap>> one = { single };
    SortSharedMapsByEndAscending(one);
    ASSERT_EQ(one.size(), 1u);

    auto a = MakeSharedMap(0u, 0x5000u);
    auto b = MakeSharedMap(0u, 0x1000u);
    std::vector<std::shared_ptr<DfxMap>> maps = { a, b };
    SortSharedMapsByEndAscending(maps);
    ASSERT_EQ(maps.size(), 2u);
    EXPECT_EQ(maps[0]->end, 0x1000u);
    EXPECT_EQ(maps[1]->end, 0x5000u);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: SortSharedMapsByEndAscending handles multi-shift and equal-end stability.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, SortSharedMapsByEndAscending_MultiShiftStable, TestSize.Level1)
{
    auto midA = MakeSharedMap(0u, 0x3000u);
    auto midB = MakeSharedMap(0u, 0x3000u);
    std::vector<std::shared_ptr<DfxMap>> maps = {
        MakeSharedMap(0u, 0x5000u),
        MakeSharedMap(0u, 0x1000u),
        midA,
        midB,
        MakeSharedMap(0u, 0x2000u),
    };
    SortSharedMapsByEndAscending(maps);
    ASSERT_EQ(maps.size(), 5u);
    EXPECT_EQ(maps[0]->end, 0x1000u);
    EXPECT_EQ(maps[1]->end, 0x2000u);
    EXPECT_EQ(maps[2], midA);
    EXPECT_EQ(maps[3], midB);
    EXPECT_EQ(maps[4]->end, 0x5000u);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: FindFirstMapContainingIp value and shared overloads.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, FindFirstMapContainingIp, TestSize.Level1)
{
    std::vector<DfxMap> emptyVal;
    EXPECT_EQ(FindFirstMapContainingIp(0x1000u, emptyVal), K_INVALID_KERNEL_MAP_INDEX);

    std::vector<DfxMap> vals = {
        MakeMapVal(0x1000u, 0x2000u),
        MakeMapVal(0x3000u, 0x4000u),
    };
    EXPECT_EQ(FindFirstMapContainingIp(0x500u, vals), K_INVALID_KERNEL_MAP_INDEX);
    EXPECT_EQ(FindFirstMapContainingIp(0x2000u, vals), K_INVALID_KERNEL_MAP_INDEX);
    EXPECT_EQ(FindFirstMapContainingIp(0x1000u, vals), K_INVALID_KERNEL_MAP_INDEX);
    EXPECT_EQ(FindFirstMapContainingIp(0x1500u, vals), 0u);
    EXPECT_EQ(FindFirstMapContainingIp(0x3500u, vals), 1u);

    std::vector<std::shared_ptr<DfxMap>> sharedEmpty;
    EXPECT_EQ(FindFirstMapContainingIp(0x1000u, sharedEmpty), K_INVALID_KERNEL_MAP_INDEX);

    std::vector<std::shared_ptr<DfxMap>> shared = {
        nullptr,
        MakeSharedMap(0x5000u, 0x6000u),
    };
    EXPECT_EQ(FindFirstMapContainingIp(0x5500u, shared), 1u);
    EXPECT_EQ(FindFirstMapContainingIp(0x4500u, shared), K_INVALID_KERNEL_MAP_INDEX);
}

/**
 * @tc.name: StackCommonTest
 * @tc.desc: FindFirstMapContainingIp returns earliest match when maps overlap.
 * @tc.type: FUNC
 */
HWTEST_F(StackCommonTest, FindFirstMapContainingIp_OverlapReturnsFirst, TestSize.Level1)
{
    std::vector<DfxMap> vals = {
        MakeMapVal(0x1000u, 0x3000u),
        MakeMapVal(0x2000u, 0x4000u),
    };
    EXPECT_EQ(FindFirstMapContainingIp(0x2500u, vals), 0u);

    std::vector<std::shared_ptr<DfxMap>> shared = {
        MakeSharedMap(0x1000u, 0x3000u),
        MakeSharedMap(0x2000u, 0x4000u),
    };
    EXPECT_EQ(FindFirstMapContainingIp(0x2500u, shared), 0u);
}

} // namespace
