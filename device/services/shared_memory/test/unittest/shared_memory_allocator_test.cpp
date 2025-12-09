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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "client_map.h"
#include "plugin_service.ipc.h"
#include "service_entry.h"
#include "share_memory_allocator.h"
#include "socket_context.h"
#include "unix_socket_client.h"
#include "unix_socket_server.h"

using namespace testing::ext;

namespace {
class PluginServiceTest final : public IPluginServiceServer {
public:
    int fileDescriptor_;
    bool GetCommand(SocketContext& context, ::GetCommandRequest& request, ::GetCommandResponse& response) override
    {
        SendResponseGetCommandResponse(context, response);
        context.SendFileDescriptor(fileDescriptor_);
        return false;
    }
};

class PluginClientTest final : public IPluginServiceClient {
public:
    int fileDescriptor_;
    bool OnGetCommandResponse(SocketContext& context, ::GetCommandResponse& response) override
    {
        fileDescriptor_ = context.ReceiveFileDiscriptor();
        return true;
    }
};

class SharedMemoryAllocatorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

const int MIN_SHARE_MEMORY_SIZE = 1024;
constexpr uint32_t PAGE_SIZE = 4096;

/**
 * @tc.name: Service
 * @tc.desc: Creates a memory block of the specified size.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, CreateMemoryBlockLocal001, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 0) ==
                nullptr); // 创建大小为0的内存块，返回空
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 1) ==
                nullptr); // 创建内存块大小<4096，返回空
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", PAGE_SIZE) != nullptr); // 成功创建
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", PAGE_SIZE) ==
                nullptr); // 创建同名内存块返回空
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
}

/**
 * @tc.name: Service
 * @tc.desc: Creates a memory block of the specified size.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, CreateMemoryBlockLocal002, TestSize.Level1)
{
    auto currentMaxShareMemorySize = UINT32_MAX / PAGE_SIZE * PAGE_SIZE;

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", MIN_SHARE_MEMORY_SIZE) ==
        nullptr); // 创建大小为1k的内存块，返回空
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 8 * MIN_SHARE_MEMORY_SIZE) !=
        nullptr); // 成功创建大小为8k的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().FindMemoryBlockByName("testname") != nullptr);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname",
        PAGE_SIZE + MIN_SHARE_MEMORY_SIZE) == nullptr); // 创建大小为10k的内存块，返回空
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", currentMaxShareMemorySize) !=
        nullptr); // 成功创建最大值的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
}

/**
 * @tc.name: Service
 * @tc.desc: Creates a memory block of the specified name size.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, CreateMemoryBlockLocal003, TestSize.Level1)
{
    
    std::string name(256, 's');
    std::string maxLengthName(255, 's');
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(name,
        PAGE_SIZE) == nullptr); // 创建大小为4k，名称长为256的内存块，返回空
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(maxLengthName,
        PAGE_SIZE) != nullptr); // 成功创建大小为4k，名称长为255的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(maxLengthName));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k，名称长为0的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(""));
}

/**
 * @tc.name: Service
 * @tc.desc: Creates a memory block of the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, CreateMemoryBlockLocal004, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(" ",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(" "));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("test~name",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("test~name"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("tes tname",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("tes tname"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("\n",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("\n"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("01000001",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("01000001"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("0x\01",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k，名称为空的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("0x\0 1",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k，名称为空的内存块
}

/**
 * @tc.name: Service
 * @tc.desc: Creates a memory block of the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, CreateMemoryBlockLocal005, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("\0",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("",
        PAGE_SIZE) == nullptr); // 创建大小为4k的内存块，返回空
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("\0",
        PAGE_SIZE) == nullptr); // 创建大小为4k的内存块，返回空
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("\0"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("",
        PAGE_SIZE) != nullptr); // 成功创建大小为4k的内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(""));
}

/**
 * @tc.name: Service
 * @tc.desc: Find memory block by name.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, FindMemoryBlockByName, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().FindMemoryBlockByName("err") == nullptr); // 查找不存在的内存块返回空
}

/**
 * @tc.name: Service
 * @tc.desc: Shared memory MemoryBlockRemote test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, MemoryBlockRemote, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("err", PAGE_SIZE, 99) ==
                nullptr); // 使用不存在的文件描述符映射内存块返回空
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("err"));

    int fd = syscall(SYS_memfd_create, "testnameremote", 0);
    EXPECT_GE(fd, 0);
    int check = ftruncate(fd, 4096);
    EXPECT_GE(check, 0);

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", 0, fd) ==
                nullptr); // 创建大小为0的内存块，返回空
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", 1, fd) ==
                nullptr); // 创建内存块大小<4096，返回空
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));

    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", PAGE_SIZE, fd) ==
                nullptr);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", PAGE_SIZE, fd) ==
                nullptr); // 创建正确fd的重复内存块
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote")); // 重复释放内存块返回-1
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the size of the memory block with the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, GetDataSize, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", PAGE_SIZE) != nullptr); // 成功创建
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().FindMemoryBlockByName("testname")->GetDataSize() == 0);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
}

/**
 * @tc.name: Service
 * @tc.desc: Free a nonexistent memory block.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, ReleaseMemoryBlockLocal, TestSize.Level1)
{
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("or")); // 释放不存在的内存块返回-1
}

/**
 * @tc.name: Service
 * @tc.desc: Free a nonexistent remote memory block.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, ReleaseMemoryBlockRemote, TestSize.Level1)
{
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("or")); // 释放不存在的内存块返回-1
}
} // namespace
