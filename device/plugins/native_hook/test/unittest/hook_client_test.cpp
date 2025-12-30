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
#include <memory_trace.h>

using namespace testing::ext;

namespace {
constexpr uint32_t SIZE = 1024;
constexpr uint32_t RESIZE = 2048;
constexpr unsigned int WAIT_THREAD_TIME = 3;
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
HWTEST_F(NativeHookTest, NormalMallocHookTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* mallocBlack = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack, nullptr);

    ohos_malloc_hook_free(mallocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    SystemSetParameter(MEM_FILTER.c_str(), "1,3");
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack2 = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack2, nullptr);

    ohos_malloc_hook_free(mallocBlack2);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    SystemSetParameter(MEM_FILTER.c_str(), "0,0");
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack3 = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack3, nullptr);

    ohos_malloc_hook_free(mallocBlack3);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    SystemSetParameter(MEM_FILTER.c_str(), "2048,4096");
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack4 = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack4, nullptr);

    ohos_malloc_hook_free(mallocBlack4);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    SystemSetParameter(MEM_FILTER.c_str(), "0");
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack5 = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack5, nullptr);

    ohos_malloc_hook_free(mallocBlack5);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook realloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookReallocTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* mallocBlack = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack, nullptr);
    void* reallocBlack = ohos_malloc_hook_realloc(mallocBlack, RESIZE);
    EXPECT_NE(reallocBlack, nullptr);

    ohos_malloc_hook_free(reallocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* mallocBlack2 = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack2, nullptr);
    void* reallocBlack2 = ohos_malloc_hook_realloc(mallocBlack2, SIZE);
    EXPECT_NE(reallocBlack2, nullptr);

    ohos_malloc_hook_free(reallocBlack2);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook calloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookCallocTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* callocBlack = ohos_malloc_hook_calloc(SIZE, RESIZE);
    EXPECT_NE(callocBlack, nullptr);

    ohos_malloc_hook_free(callocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());

    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    void* callocBlack2 = ohos_malloc_hook_calloc(0, RESIZE);
    EXPECT_NE(callocBlack2, nullptr);

    ohos_malloc_hook_free(callocBlack2);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook valloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookVallocTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* vallocBlack = ohos_malloc_hook_valloc(SIZE);
    EXPECT_EQ(vallocBlack, nullptr);

    ohos_malloc_hook_free(vallocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook memtrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookMemtraceTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_memtrace(mem, 1, TAG_RES_GPU_VK, true);
    ohos_malloc_hook_memtrace(mem, 1, TAG_RES_GPU_VK, false);
    delete[] mem;
    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest001, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_GPU_VK, mem, 1, TAG_RES_GPU_VK, true);
    ohos_malloc_hook_restrace(RES_GPU_VK, mem, 1, TAG_RES_GPU_VK, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest002, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_FD_OPEN, mem, 1, TAG_RES_FD_OPEN, true);
    ohos_malloc_hook_restrace(RES_FD_OPEN, mem, 1, TAG_RES_FD_OPEN, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest003, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_THREAD_PTHREAD, mem, 1, TAG_RES_THREAD_PTHREAD, true);
    ohos_malloc_hook_restrace(RES_THREAD_PTHREAD, mem, 1, TAG_RES_THREAD_PTHREAD, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest004, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_DMABUF_MASK, mem, 1, TAG_RES_DMABUF_MASK, true);
    ohos_malloc_hook_restrace(RES_DMABUF_MASK, mem, 1, TAG_RES_DMABUF_MASK, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}
/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest005, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_ARKTS_HEAP_MASK, mem, 1, TAG_RES_ARKTS_HEAP_MASK, true);
    ohos_malloc_hook_restrace(RES_ARKTS_HEAP_MASK, mem, 1, TAG_RES_ARKTS_HEAP_MASK, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}
/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest006, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_JS_HEAP_MASK, mem, 1, TAG_RES_JS_HEAP_MASK, true);
    ohos_malloc_hook_restrace(RES_JS_HEAP_MASK, mem, 1, TAG_RES_JS_HEAP_MASK, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}
/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest007, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_KMP_HEAP_MASK, mem, 1, TAG_RES_KMP_HEAP_MASK, true);
    ohos_malloc_hook_restrace(RES_KMP_HEAP_MASK, mem, 1, TAG_RES_KMP_HEAP_MASK, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}
/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest008, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_RN_HEAP_MASK, mem, 1, TAG_RES_RN_HEAP_MASK, true);
    ohos_malloc_hook_restrace(RES_RN_HEAP_MASK, mem, 1, TAG_RES_RN_HEAP_MASK, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook restrace normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookRestraceTest009, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* mem = new char[1];
    ohos_malloc_hook_restrace(RES_ARK_GLOBAL_HANDLE, mem, 1, TAG_RES_ARK_GLOBAL_HANDLE, true);
    ohos_malloc_hook_restrace(RES_ARK_GLOBAL_HANDLE, mem, 1, TAG_RES_ARK_GLOBAL_HANDLE, false);
    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}
/**
 * @tc.name: ResTraceMoveTest
 * @tc.desc: Test memory move.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, ResTraceMoveTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    char* oldMem = new char[SIZE];
    char* newMem = new char[RESIZE];

    ohos_malloc_hook_resTraceMove(RES_ARKTS_HEAP_MASK, oldMem, newMem, RESIZE);

    delete[] oldMem;
    delete[] newMem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}
/**
 * @tc.name: resTraceFreeRegion
 * @tc.desc: Test memory region free tracking hook function.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, ResTraceFreeRegionTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    char* mem = new char[SIZE];

    ohos_malloc_hook_resTraceFreeRegion(RES_ARKTS_HEAP_MASK, mem, SIZE);

    delete[] mem;

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test hook aligned alloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalHookAlignedAllocTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* alignedAllocBlack = ohos_malloc_hook_aligned_alloc(SIZE, RESIZE);
    EXPECT_NE(alignedAllocBlack, nullptr);

    ohos_malloc_hook_free(alignedAllocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test multi hook malloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    void* mallocBlack = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack, nullptr);
    void* reallocBlack = ohos_malloc_hook_realloc(mallocBlack, RESIZE);
    EXPECT_NE(reallocBlack, nullptr);
    void* callocBlack = ohos_malloc_hook_calloc(SIZE, RESIZE);
    EXPECT_NE(callocBlack, nullptr);
    void* vallocBlack = ohos_malloc_hook_valloc(SIZE);
    EXPECT_EQ(vallocBlack, nullptr);
    void* alignedAllocBlack = ohos_malloc_hook_aligned_alloc(SIZE, RESIZE);
    EXPECT_NE(alignedAllocBlack, nullptr);

    ohos_malloc_hook_free(alignedAllocBlack);
    ohos_malloc_hook_free(vallocBlack);
    ohos_malloc_hook_free(callocBlack);
    ohos_malloc_hook_free(reallocBlack);

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test other process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, NormalOtherTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));

    ohos_malloc_hook_finalize();

    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test failure process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, FailureTest, TestSize.Level0)
{
    EXPECT_TRUE(ohos_malloc_hook_initialize(nullptr, nullptr, nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_start(nullptr));
    EXPECT_TRUE(ohos_malloc_hook_on_end());
    sleep(WAIT_THREAD_TIME);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test set and get malloc hook flag.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, MallocHookFlag, TestSize.Level0)
{
    bool flag = true;
    bool flagPrev = ohos_malloc_hook_set_hook_flag(flag);

    EXPECT_EQ(ohos_malloc_hook_get_hook_flag(), flag);
    EXPECT_EQ(ohos_malloc_hook_set_hook_flag(flagPrev), flag);
    EXPECT_EQ(ohos_malloc_hook_get_hook_flag(), flagPrev);

    EXPECT_TRUE(ohos_malloc_hook_set_hook_flag(false));
    EXPECT_FALSE(ohos_malloc_hook_get_hook_flag());
}

/**
 * @tc.name: SetFilterSizeTest
 * @tc.desc: Test HookGuard::SetFilterSize basic functionality
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookTest, SetFilterSizeTest, TestSize.Level0)
{
    g_clientConfig.filterSize = -1;
    EXPECT_FALSE(ohos_set_filter_size(10, nullptr));

    g_clientConfig.filterSize = 5;
    EXPECT_TRUE(ohos_set_filter_size(10, nullptr));
    EXPECT_FALSE(ohos_set_filter_size(3, nullptr));
}
} // namespace