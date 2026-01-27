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
#include <fstream>
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

size_t GetMemAvailable()
{
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    size_t currentMemAvailable = 0;
    while (std::getline(meminfo, line)) {
        if (line.find("MemAvailable:") != std::string::npos) {
            // Extract memory value
            size_t offset = 2;
            size_t pos = line.find(":");
            size_t memPos = line.find(" ");
            currentMemAvailable = std::stoul(line.substr(pos + offset, memPos - pos - offset));
            break;
        }
    }

    meminfo.close();
    return currentMemAvailable;
}

/**
 * @tc.name: Service
 * @tc.desc: Creates a memory block of the specified size.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, CreateMemoryBlockLocal001, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 0) ==
        nullptr); // Create a memory block of size 0, return null.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 1) ==
        nullptr); // Create a memory block with a size less than 4096, return null.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", PAGE_SIZE) !=
        nullptr); // Successfully created
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", PAGE_SIZE) ==
        nullptr); // Creating a memory block with the same name returns null.
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
    // round down to the nearest integer, Get the maximum allocatable space of the current shared memory block
    auto currentMaxShareMemorySize = static_cast<uint32_t>(GetMemAvailable());
    auto uint32MaxKb = UINT32_MAX / static_cast<uint32_t>(MIN_SHARE_MEMORY_SIZE);
    if (currentMaxShareMemorySize > uint32MaxKb) {
        currentMaxShareMemorySize = uint32MaxKb;
    }
    currentMaxShareMemorySize = currentMaxShareMemorySize * static_cast<uint32_t>(MIN_SHARE_MEMORY_SIZE) /
        PAGE_SIZE * PAGE_SIZE;
    // Only 4 KB can be allocated. If the number is not a multiple of 4 KB, you cannot apply for the number.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", MIN_SHARE_MEMORY_SIZE) ==
        nullptr); // Create a memory block of size 1k, return null.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", 8 * MIN_SHARE_MEMORY_SIZE) !=
        nullptr); // Successfully created a memory block of size 8k.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().FindMemoryBlockByName("testname") != nullptr);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal("testname"));
    // Only 4 KB can be allocated. If the number is not a multiple of 4 KB, you cannot apply for the number.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname",
        PAGE_SIZE + MIN_SHARE_MEMORY_SIZE) == nullptr);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", currentMaxShareMemorySize) !=
        nullptr); // Successfully created the largest memory block.
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
        PAGE_SIZE) == nullptr); // Create a memory block with a size of 4k and a name length of 256, returning null.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(maxLengthName,
        PAGE_SIZE) != nullptr); // Successfully created a memory block with a size of 4k and a name length of 255.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(maxLengthName));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("",
        PAGE_SIZE) == nullptr); // Failed to created a memory block with a size of 4k and a name length of 0.
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
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("err", PAGE_SIZE, 99) ==
        nullptr); // Returning null when mapping a memory block with a non-existent file descriptor.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("err"));

    int fd = syscall(SYS_memfd_create, "testnameremote", 0);
    EXPECT_GE(fd, 0);
    int check = ftruncate(fd, PAGE_SIZE);
    EXPECT_GE(check, 0);

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", 0, fd) ==
        nullptr); // Create a memory block of size 0, return null.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", 1, fd) ==
        nullptr); // Create a memory block with a size less than 4096, return null.
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));

    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", PAGE_SIZE, fd) ==
        nullptr);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote", PAGE_SIZE, fd) ==
        nullptr); // Duplicate memory blocks with the correct fd.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));
    // Repeatedly releasing memory blocks returns -1
    ASSERT_FALSE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));
}

/**
 * @tc.name: Service
 * @tc.desc: Shared memory MemoryBlockRemote test.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, MemoryBlockRemote002, TestSize.Level1)
{
    // round down to the nearest integer, Get the maximum allocatable space of the current shared memory block
    auto currentMaxShareMemorySize = static_cast<uint32_t>(GetMemAvailable());
    auto uint32MaxKb = UINT32_MAX / static_cast<uint32_t>(MIN_SHARE_MEMORY_SIZE);
    if (currentMaxShareMemorySize > uint32MaxKb) {
        currentMaxShareMemorySize = uint32MaxKb;
    }
    currentMaxShareMemorySize = currentMaxShareMemorySize * static_cast<uint32_t>(MIN_SHARE_MEMORY_SIZE) /
        PAGE_SIZE * PAGE_SIZE;

    int fd = syscall(SYS_memfd_create, "testnameremote", 0);
    EXPECT_GE(fd, 0);
    int check = ftruncate(fd, PAGE_SIZE);
    EXPECT_GE(check, 0);

    int fd2 = syscall(SYS_memfd_create, "testnameremote", 0);
    EXPECT_GE(fd2, 0);
    int check2 = ftruncate(fd2, PAGE_SIZE);
    EXPECT_GE(check2, 0);

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote",
        PAGE_SIZE, fd) != nullptr); // Successfully created a memory block of size 4k.
    // Successfully created a memory block with a size of 4k, different names, and the same file descriptor.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("test",
        PAGE_SIZE, fd) != nullptr);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testname1",
        currentMaxShareMemorySize, fd) != nullptr); // Successfully created the largest memory block.
    // Create a memory block of size 4k with the same name but different file descriptors, and return null.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote",
        PAGE_SIZE, fd2) == nullptr);
    
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testnameremote"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("test"));
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testname1"));

    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testname1",
        PAGE_SIZE, fd) == nullptr); // Create a memory block with a size less than 4096, return null.
    // Successfully created a memory block of size 4k with the same name but different FDs.
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testname1",
        PAGE_SIZE, fd2) != nullptr);
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockRemote("testname1"));

    int fd3 = -1;
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockRemote("testnameremote",
        PAGE_SIZE, fd3) == nullptr);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the size of the memory block with the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(SharedMemoryAllocatorTest, GetDataSize, TestSize.Level1)
{
    ASSERT_TRUE(ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("testname", PAGE_SIZE) !=
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
