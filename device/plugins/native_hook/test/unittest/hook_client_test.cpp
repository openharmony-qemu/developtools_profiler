/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dlfcn.h>
#include <gtest/gtest.h>
#include "hook_client.h"
#include "musl_preinit_common.h"
#include "init_param.h"

using namespace testing::ext;

namespace {
constexpr uint32_t g_size = 1024;
constexpr uint32_t g_resize = 2048;
const std::string MEM_FILTER("persist.hiviewdfx.profiler.mem.filter");

class NativeHookTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: native hook
 * @tc.desc: Test hook malloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalMallocHookTest, TestSize.Level1)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* mallocBlack = ohos_malloc_hook_malloc(g_size);
    EXPECT_NE(mallocBlack, nullptr);

    ohos_malloc_hook_free(mallocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    SystemSetParameter(MEM_FILTER.c_str(), "1,3");
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack2 = ohos_malloc_hook_malloc(g_size);
    EXPECT_NE(mallocBlack2, nullptr);

    ohos_malloc_hook_free(mallocBlack2);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    SystemSetParameter(MEM_FILTER.c_str(), "0,0");
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack3 = ohos_malloc_hook_malloc(g_size);
    EXPECT_NE(mallocBlack3, nullptr);

    ohos_malloc_hook_free(mallocBlack3);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    SystemSetParameter(MEM_FILTER.c_str(), "2048,4096");
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack4 = ohos_malloc_hook_malloc(g_size);
    EXPECT_NE(mallocBlack4, nullptr);

    ohos_malloc_hook_free(mallocBlack4);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    SystemSetParameter(MEM_FILTER.c_str(), "0");
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack5 = ohos_malloc_hook_malloc(g_size);
    EXPECT_NE(mallocBlack5, nullptr);

    ohos_malloc_hook_free(mallocBlack5);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook realloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookReallocTest, TestSize.Level1)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* mallocBlack = ohos_malloc_hook_malloc(g_size);
    EXPECT_NE(mallocBlack, nullptr);
    void* reallocBlack = ohos_malloc_hook_realloc(mallocBlack, g_resize);
    EXPECT_NE(reallocBlack, nullptr);

    ohos_malloc_hook_free(reallocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack2 = ohos_malloc_hook_malloc(g_size);
    EXPECT_NE(mallocBlack2, nullptr);
    void* reallocBlack2 = ohos_malloc_hook_realloc(mallocBlack2, g_size);
    EXPECT_NE(reallocBlack2, nullptr);

    ohos_malloc_hook_free(reallocBlack2);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook calloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookCallocTest, TestSize.Level1)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* callocBlack = ohos_malloc_hook_calloc(g_size, g_resize);
    EXPECT_NE(callocBlack, nullptr);

    ohos_malloc_hook_free(callocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* callocBlack2 = ohos_malloc_hook_calloc(0, g_resize);
    EXPECT_NE(callocBlack2, nullptr);

    ohos_malloc_hook_free(callocBlack2);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook valloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookVallocTest, TestSize.Level1)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* vallocBlack = ohos_malloc_hook_valloc(g_size);
    EXPECT_EQ(vallocBlack, nullptr);

    ohos_malloc_hook_free(vallocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook aligned alloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookAlignedAllocTest, TestSize.Level1)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* alignedAllocBlack = ohos_malloc_hook_aligned_alloc(g_size, g_resize);
    EXPECT_NE(alignedAllocBlack, nullptr);

    ohos_malloc_hook_free(alignedAllocBlack);
    EXPECT_TRUE(ohos_malloc_hook_on_end());
}

/**
 * @tc.name: native hook
 * @tc.desc: Test multi hook malloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalTest, TestSize.Level1)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* mallocBlack = ohos_malloc_hook_malloc(g_size);
    EXPECT_NE(mallocBlack, nullptr);
    void* reallocBlack = ohos_malloc_hook_realloc(mallocBlack, g_resize);
    EXPECT_NE(reallocBlack, nullptr);
    void* callocBlack = ohos_malloc_hook_calloc(g_size, g_resize);
    EXPECT_NE(callocBlack, nullptr);
    void* vallocBlack = ohos_malloc_hook_valloc(g_size);
    EXPECT_EQ(vallocBlack, nullptr);
    void* alignedAllocBlack = ohos_malloc_hook_aligned_alloc(g_size, g_resize);
    EXPECT_NE(alignedAllocBlack, nullptr);

    ohos_malloc_hook_free(alignedAllocBlack);
    ohos_malloc_hook_free(vallocBlack);
    ohos_malloc_hook_free(callocBlack);
    ohos_malloc_hook_free(reallocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
}

/**
 * @tc.name: native hook
 * @tc.desc: Test other process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalOtherTest, TestSize.Level1)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    ohos_malloc_hook_finalize();

    EXPECT_TRUE(ohos_malloc_hook_on_end());
}

/**
 * @tc.name: native hook
 * @tc.desc: Test failure process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, FailureTest, TestSize.Level1)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(nullptr, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_end());
}

/**
 * @tc.name: native hook
 * @tc.desc: Test set and get malloc hook flag.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, MallocHookFlag, TestSize.Level1)
{
    bool flag = true;
    bool flagPrev = ohos_malloc_hook_set_hook_flag(flag);

    EXPECT_EQ(ohos_malloc_hook_get_hook_flag(), flag);
    EXPECT_EQ(ohos_malloc_hook_set_hook_flag(flagPrev), flag);
    EXPECT_EQ(ohos_malloc_hook_get_hook_flag(), flagPrev);

    EXPECT_TRUE(ohos_malloc_hook_set_hook_flag(false));
    EXPECT_FALSE(ohos_malloc_hook_get_hook_flag());
}
} // namespace