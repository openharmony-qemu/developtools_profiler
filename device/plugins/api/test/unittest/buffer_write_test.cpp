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

#include <hwext/gtest-ext.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include "buffer_writer.h"
#include "plugin_service_types.pb.h"

using namespace testing::ext;

namespace {
constexpr uint32_t SMB1_SIZE = 10 * 4096;
constexpr uint32_t SMB2_SIZE = 10 * 4096;
const std::string SMB1_NAME = "testsmb1";
const std::string SMB2_NAME = "testsmb2";
const std::string PLUGIN_NAME = "testplugin";
const std::string PLUGIN_VERSION = "1.01";
void *g_smbAddr1 = nullptr;
void *g_smbAddr2 = nullptr;
int g_smbFd1 = 0;
int g_smbFd2 = 0;

int InitShareMemory1()
{
    int fd = syscall(SYS_memfd_create, SMB1_NAME.c_str(), 0);
    CHECK_TRUE(fd >= 0, -1, "CreateBlock FAIL SYS_memfd_create");

    int check = ftruncate(fd, SMB1_SIZE);
    if (check < 0) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "CreateBlock ftruncate ERR : %s", buf);
        return -1;
    }

    g_smbAddr1 = mmap(nullptr, SMB1_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_smbAddr1 == static_cast<void*>(MAP_FAILED)) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "CreateBlock g_smbAddr1 mmap ERR : %s", buf);
        return -1;
    }

    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr1);

    // initialize header infos
    header_->info.readOffset_ = 0;
    header_->info.writeOffset_ = 0;
    header_->info.memorySize_ = SMB1_SIZE - sizeof(ShareMemoryBlock::BlockHeader);
    header_->info.bytesCount_ = 0;
    header_->info.chunkCount_ = 0;

    return fd;
}

int InitShareMemory2()
{
    int fd = syscall(SYS_memfd_create, SMB2_NAME.c_str(), 0);
    CHECK_TRUE(fd >= 0, -1, "CreateBlock FAIL SYS_memfd_create");

    int check = ftruncate(fd, SMB2_SIZE);
    if (check < 0) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "CreateBlock ftruncate ERR : %s", buf);
        return -1;
    }

    g_smbAddr2 = mmap(nullptr, SMB2_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_smbAddr2 == static_cast<void*>(MAP_FAILED)) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "CreateBlock g_smbAddr2 mmap ERR : %s", buf);
        return -1;
    }

    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr2);

    // initialize header infos
    header_->info.readOffset_ = 0;
    header_->info.writeOffset_ = 0;
    header_->info.memorySize_ = SMB2_SIZE - sizeof(ShareMemoryBlock::BlockHeader);
    header_->info.bytesCount_ = 0;
    header_->info.chunkCount_ = 0;

    return fd;
}

class BufferWriteTest : public ::testing::Test {
protected:
    static void SetUpTestCase()
    {
        g_smbFd1 = InitShareMemory1();
        g_smbFd2 = InitShareMemory2();
    }
    static void TearDownTestCase() {}
};

bool CheckBuffer(uint8_t *buffer, size_t size)
{
    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr1);
    uint8_t *cmpaddr = (uint8_t *)g_smbAddr1 + sizeof(ShareMemoryBlock::BlockHeader) + header_->info.readOffset_;
    uint32_t cmpsize = *(uint32_t*)cmpaddr;
    cmpaddr = cmpaddr + sizeof(uint32_t);
    ProfilerPluginData pluginData;
    pluginData.ParseFromArray(cmpaddr, cmpsize);
    const char* data = pluginData.data().c_str();

    header_->info.readOffset_ = header_->info.writeOffset_;
    if (memcmp(buffer, data, size) == 0) {
        return true;
    }
    return false;
}


bool CheckMessage(uint8_t *buffer, size_t size)
{
    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr2);
    uint8_t *cmpaddr = (uint8_t *)g_smbAddr2 + sizeof(ShareMemoryBlock::BlockHeader) + header_->info.readOffset_;
    cmpaddr = cmpaddr + sizeof(uint32_t);
    header_->info.readOffset_ = header_->info.writeOffset_;

    if (memcmp(buffer, cmpaddr, size) == 0) {
        return true;
    }
    return false;
}

/**
 * @tc.name: plugin
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(BufferWriteTest, WriteTest, TestSize.Level1)
{
    auto write = std::make_shared<BufferWriter>(PLUGIN_NAME, PLUGIN_VERSION, SMB1_SIZE, g_smbFd1, -1, 0);

    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};

    EXPECT_TRUE(write->Write(buffer1, sizeof(buffer1)));
    EXPECT_TRUE(CheckBuffer(buffer1, sizeof(buffer1)));
    EXPECT_TRUE(write->Write(buffer2, sizeof(buffer2)));
    EXPECT_TRUE(CheckBuffer(buffer2, sizeof(buffer2)));
    EXPECT_TRUE(write->Write(buffer3, sizeof(buffer3)));
    EXPECT_TRUE(CheckBuffer(buffer3, sizeof(buffer3)));
    EXPECT_FALSE(write->Write(nullptr, 0));
}

/**
 * @tc.name: plugin
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(BufferWriteTest, WriteMessageTest, TestSize.Level1)
{
    uint8_t data[1024];
    auto write = std::make_shared<BufferWriter>(PLUGIN_NAME, PLUGIN_VERSION, SMB2_SIZE, g_smbFd2, -1, 0);

    ProfilerPluginConfig configData;
    configData.set_name("111");
    configData.set_plugin_sha256("222");
    configData.set_sample_interval(1000);
    size_t size = configData.ByteSizeLong();
    configData.SerializeToArray(data, size);

    EXPECT_TRUE(write->WriteMessage(configData, "111"));
    EXPECT_TRUE(CheckMessage(data, size));

    ProfilerPluginState stateData;
    stateData.set_name("st");
    stateData.set_state(ProfilerPluginState::IN_SESSION);
    size = stateData.ByteSizeLong();
    stateData.SerializeToArray(data, size);

    EXPECT_TRUE(write->WriteMessage(stateData, "111"));
    EXPECT_TRUE(CheckMessage(data, size));


    ProfilerPluginData pluginData;
    pluginData.set_name("test");
    pluginData.set_status(1);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    pluginData.set_clock_id(ProfilerPluginData::CLOCKID_REALTIME);
    pluginData.set_tv_sec(ts.tv_sec);
    pluginData.set_tv_nsec(ts.tv_nsec);
    size = pluginData.ByteSizeLong();
    pluginData.SerializeToArray(data, size);

    EXPECT_TRUE(write->WriteMessage(pluginData, "111"));
    EXPECT_TRUE(CheckMessage(data, size));
    EXPECT_TRUE(write->Clear());
}
} // namespace
