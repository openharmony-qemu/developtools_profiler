/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include <fstream>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <random>
#include <string>

#include "paged_mem_pool.h"

using FTRACE_NS::PagedMemPool;
using namespace testing::ext;

namespace {
constexpr uint32_t TEST_PAGE_SIZE = 4096;
constexpr int RAND_INT_MIN = 1;
constexpr int RAND_INT_MAX = 1024;
class PagedMemPoolTest : public ::testing::Test {
    std::mt19937 gen_;

protected:
    void SetUp() override {}

    void TearDown() override {}

    size_t RandomNumber()
    {
        std::uniform_int_distribution<int> distrib(RAND_INT_MIN, RAND_INT_MAX);
        return distrib(gen_);
    }
};

/*
 * @tc.name: normal PagedMemPool size
 * @tc.desc: test PagedMemPool:: normal with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(PagedMemPoolTest, PagedMemPoolNormal, TestSize.Level1)
{
    size_t blockSize = 8;
    size_t maxCacheSize = 2;
    std::unique_ptr<PagedMemPool> memPool = std::make_unique<PagedMemPool>(blockSize, maxCacheSize);

    ASSERT_NE(memPool, nullptr);

    void* block = memPool->Allocate();
    EXPECT_NE(block, nullptr);

    EXPECT_EQ(memPool->GetBlockSize(), blockSize * TEST_PAGE_SIZE);

    EXPECT_TRUE(memPool->Recycle(block));
}

/*
 * @tc.name: false PagedMemPool size
 * @tc.desc: test PagedMemPool:: normal with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(PagedMemPoolTest, PagedMemPoolfreeListSize, TestSize.Level1)
{
    size_t blockSize = 8;
    size_t maxCacheSize = 2;
    std::unique_ptr<PagedMemPool> memPool = std::make_unique<PagedMemPool>(blockSize, maxCacheSize);

    ASSERT_NE(memPool, nullptr);

    void* block = memPool->Allocate();
    EXPECT_NE(block, nullptr);

    EXPECT_EQ(memPool->GetBlockSize(), blockSize * TEST_PAGE_SIZE);

    EXPECT_TRUE(memPool->Recycle(block));

    block = memPool->Allocate();
    EXPECT_NE(block, nullptr);

    EXPECT_EQ(memPool->GetBlockSize(), blockSize * TEST_PAGE_SIZE);

    EXPECT_TRUE(memPool->Recycle(block));
}

/*
 * @tc.name: rand PagedMemPool size
 * @tc.desc: test PagedMemPool:: normal with rand case.
 * @tc.type: FUNC
 */
HWTEST_F(PagedMemPoolTest, PagedMemPoolNormalRand, TestSize.Level1)
{
    size_t blockSize = RandomNumber();
    size_t maxCacheSize = RandomNumber();
    std::unique_ptr<PagedMemPool> memPool = std::make_unique<PagedMemPool>(blockSize, maxCacheSize);

    ASSERT_NE(memPool, nullptr);

    void* block = memPool->Allocate();
    EXPECT_NE(block, nullptr);

    EXPECT_EQ(memPool->GetBlockSize(), blockSize * TEST_PAGE_SIZE);

    EXPECT_TRUE(memPool->Recycle(block));
}
} // namespace