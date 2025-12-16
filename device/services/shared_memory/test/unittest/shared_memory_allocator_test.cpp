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

/**
 * @tc.name: Service
 * @tc.desc: Creates a memory block of the specified size.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, CreateMemoryBlockLocal, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 0) ==
                nullptr); // Create a memory block of size 0, return null.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 1) ==
                nullptr); // Create a memory block with a size less than 4096, return null.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 4096) !=
        nullptr); // Successfully created
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 4096) ==
        nullptr); // Creating a memory block with the same name returns null.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
}

/**
 * @tc.name: Service
 * @tc.desc: Find memory block by name.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, FindMemoryBlockByName, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().FindMemoryBlockByName("err") ==
        nullptr); // Return null for a memory block that does not exist.
}

/**
 * @tc.name: Service
 * @tc.desc: Shared memory MemoryBlockRemote test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, MemoryBlockRemote, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("err", 4096, 99) ==
        nullptr); // Returning null when mapping a memory block with a non-existent file descriptor.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("err"));

    int fd = syscall(SYS_memfd_create, "testnameremote", 0);
    EXPECT_GE(fd, 0);
    int check = ftruncate(fd, 4096);
    EXPECT_GE(check, 0);

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", 0, fd) ==
                nullptr); // Create a memory block of size 0, return null.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", 1, fd) ==
                nullptr); // Create a memory block with a size less than 4096, return null.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));

    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", 4096, fd) ==
                nullptr);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", 4096, fd) ==
                nullptr); // Duplicate memory blocks with the correct fd.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));
    // Repeatedly releasing memory blocks returns -1
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the size of the memory block with the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, GetDataSize, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 4096) !=
        nullptr); // Successfully created
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
    // Return -1 for releasing a non-existent memory block
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("or"));
}

/**
 * @tc.name: Service
 * @tc.desc: Free a nonexistent remote memory block.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, ReleaseMemoryBlockRemote, TestSize.Level1)
{
    // Return -1 for releasing a non-existent memory block
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("or"));
}
} // namespace
