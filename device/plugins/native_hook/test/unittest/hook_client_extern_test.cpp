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
#include <sys/prctl.h>
#include "hook_client.h"
#include "hook_socket_client_mock.h"
#include "musl_preinit_common.h"
#include "init_param.h"
#include <memory_trace.h>

using namespace testing::ext;

namespace {
constexpr uint32_t SIZE = 1024;
constexpr uint32_t RESIZE = 2048;
constexpr unsigned int WAIT_THREAD_TIME = 3;

class HookClientExternTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        ohos_malloc_hook_initialize(&__libc_malloc_default_dispatch, nullptr, nullptr);
        ohos_malloc_hook_on_start(nullptr);
    }
    static void TearDownTestCase()
    {
        ohos_malloc_hook_on_end();
        sleep(WAIT_THREAD_TIME);
    }
    void SetUp() {}
    void TearDown() {}

    void StartMock(std::vector<int> &typeVec, std::vector<int> &sizeVec)
    {
        g_hookReady = true;
        g_clientConfig.filterSize = 1;
        std::shared_ptr<MockHookSocketClient> mockClient = std::make_shared<MockHookSocketClient>();
        g_hookClient = mockClient;

        EXPECT_CALL(*mockClient, SendStackWithPayload(::testing::_, ::testing::_, ::testing::_,
            ::testing::_, ::testing::_)).WillRepeatedly([&typeVec, &sizeVec]
            (const void* data, size_t size, const void* payload, size_t payloadSize, int smbIndex) {
            if (data) {
                const BaseStackRawData* data_ptr = static_cast<const BaseStackRawData*>(data);
                typeVec.push_back(data_ptr->type);
                sizeVec.push_back(data_ptr->mallocSize);
            }
            return true;
        });
    }
};

/**
 * @tc.name: native hook
 * @tc.desc: Test hook malloc normal process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookMallocTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);

    void* mallocBlack = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack, nullptr);

    ohos_malloc_hook_free(mallocBlack);

    std::vector<int> expectedTypeVec = {THREAD_NAME_MSG, MALLOC_MSG, FREE_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {0, SIZE, 0};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}


/**
 * @tc.name: native hook
 * @tc.desc: Test HookReallocTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookReallocTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);

    void* mallocBlack = ohos_malloc_hook_malloc(SIZE);
    EXPECT_NE(mallocBlack, nullptr);

    void* reallocBlack = ohos_malloc_hook_realloc(mallocBlack, RESIZE);
    EXPECT_NE(reallocBlack, nullptr);

    ohos_malloc_hook_free(reallocBlack);

    std::vector<int> expectedTypeVec = {MALLOC_MSG, FREE_MSG, MALLOC_MSG, FREE_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {SIZE, 0, RESIZE, 0};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookCallocTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookCallocTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);

    void* callocBlack = ohos_malloc_hook_calloc(SIZE, RESIZE);
    EXPECT_NE(callocBlack, nullptr);

    ohos_malloc_hook_free(callocBlack);

    std::vector<int> expectedTypeVec = {MALLOC_MSG, FREE_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {SIZE * RESIZE, 0};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookVallocTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookVallocTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);

    void* vallocBlack = ohos_malloc_hook_valloc(SIZE);
    EXPECT_EQ(vallocBlack, nullptr);

    ohos_malloc_hook_free(vallocBlack);

    std::vector<int> expectedTypeVec = {FREE_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {0};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookAliginedAllocTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookAliginedAllocTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);

    void* alignedAllocBlack = ohos_malloc_hook_aligned_alloc(SIZE, RESIZE);
    EXPECT_NE(alignedAllocBlack, nullptr);

    ohos_malloc_hook_free(alignedAllocBlack);

    std::vector<int> expectedTypeVec = {MALLOC_MSG, FREE_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {RESIZE, 0};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookMmapTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookMmapTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);
    g_clientConfig.mmapDisable = false;
    size_t length = 100;
    int port = 0;
    int flags = 0;
    int fd = 1;
    off_t offset = 0;
    void* alignedAllocBlack = ohos_malloc_hook_mmap(reinterpret_cast<void*>(0x1000), length, port, flags, fd, offset);
    EXPECT_NE(alignedAllocBlack, nullptr);

    std::vector<int> expectedTypeVec = {MMAP_FILE_TYPE, MEMORY_TAG, MMAP_FILE_PAGE_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {length, 0, length};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookMunmapTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookMunmapTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);

    size_t length = 100;
    int ret = ohos_malloc_hook_munmap(reinterpret_cast<void*>(0x1000), length);
    EXPECT_NE(ret, -1);

    std::vector<int> expectedTypeVec = {MUNMAP_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {length};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookMemtraceTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookMemtraceTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);
    g_clientConfig.traceMask = 0;
    g_clientConfig.memtraceEnable = true;
    g_clientConfig.isSaMode = false;

    size_t size = 100;
    const char* tag = "test";
    bool isUsing = false;
    ohos_malloc_hook_memtrace(reinterpret_cast<void*>(0x1000), size, tag, isUsing);

    std::vector<int> expectedTypeVec = {MEMORY_TAG, MEMORY_UNUSING_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {0, size};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookMemHookMemtraceTestSaModetraceTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookMemtraceTestSaMode, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);
    g_clientConfig.traceMask = 0;
    g_clientConfig.memtraceEnable = true;
    g_clientConfig.isSaMode = true;

    size_t size = 100;
    const char* tag = "test";
    bool isUsing = true;
    ohos_malloc_hook_memtrace(reinterpret_cast<void*>(0x1000), size, tag, isUsing);

    std::vector<int> expectedTypeVec = {};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookMemtraceTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookRestraceTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);
    size_t size = 100;
    const char* tag = "test";
    bool isUsing = false;

    g_clientConfig.memtraceEnable = true;
    g_clientConfig.traceMask = 1;
    unsigned long long mask = 0;
    ohos_malloc_hook_restrace(mask, reinterpret_cast<void*>(0x1000), size, tag, isUsing);
    mask = 1;
    ohos_malloc_hook_restrace(mask, reinterpret_cast<void*>(0x1000), size, tag, isUsing);

    std::vector<int> expectedTypeVec = {MEMORY_UNUSING_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {size};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookResTraceMoveTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookResTraceMoveTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);
    g_clientConfig.memtraceEnable = true;

    unsigned long long mask = 1;
    g_clientConfig.traceMask = 1;

    size_t size = 100;
    ohos_malloc_hook_resTraceMove(mask, reinterpret_cast<void*>(0x1000), reinterpret_cast<void*>(0x1000), size);

    std::vector<int> expectedTypeVec = {MALLOC_ARKTS};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {size};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookResTraceFreeRegionTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookResTraceFreeRegionTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);
    g_clientConfig.memtraceEnable = true;

    unsigned long long mask = 1;
    g_clientConfig.traceMask = 1;
    size_t size = 100;
    ohos_malloc_hook_resTraceFreeRegion(mask, reinterpret_cast<void*>(0x1000), size);

    std::vector<int> expectedTypeVec = {FREE_ARKTS};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {size};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookPrctlTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookPrctlTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);
    g_clientConfig.mmapDisable = false;

    int option = PR_SET_VMA;
    unsigned long arg2 = PR_SET_VMA_ANON_NAME;
    size_t size = 100;
    const char* name = "test_name";
    unsigned long arg5 = reinterpret_cast<unsigned long>(const_cast<char*>(name));
    int ret = ohos_malloc_hook_prctl(option, arg2, 0x1000, size, arg5);
    EXPECT_EQ(ret, -1);

    std::vector<int> expectedTypeVec = {PR_SET_VMA_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {size};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

/**
 * @tc.name: native hook
 * @tc.desc: Test HookSendHookMiscDataTest process.
 * @tc.type: FUNC
 */
HWTEST_F(HookClientExternTest, HookSendHookMiscDataTest, TestSize.Level0)
{
    std::vector<int> typeVec;
    std::vector<int> sizeVec;
    StartMock(typeVec, sizeVec);

    uint64_t id = 0;
    size_t stackSize = 0;
    uint32_t jsStackData = 1;
    bool ret = ohos_malloc_hook_send_hook_misc_data(id, nullptr, stackSize, jsStackData);
    EXPECT_EQ(ret, true);

    ret = ohos_malloc_hook_send_hook_misc_data(id, nullptr, stackSize, 0);
    EXPECT_EQ(ret, false);

    std::vector<int> expectedTypeVec = {JS_STACK_MSG};
    EXPECT_EQ(typeVec, expectedTypeVec);
    std::vector<int> expectedsizeVec = {0};
    EXPECT_EQ(sizeVec, expectedsizeVec);
}

}