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

#include <gtest/gtest.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include "writer_adapter.h"
#include "buffer_writer.h"

using namespace testing::ext;

namespace {
class WriterAdapterTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};
void *g_smbAddr = nullptr;
constexpr uint32_t SMB1_SIZE = 10 * 4096;
const std::string SMB1_NAME = "testsmb1";

int InitShareMemory()
{
    int fd = syscall(SYS_memfd_create, SMB1_NAME.c_str(), 0);
    CHECK_TRUE(fd >= 0, -1, "CreateBlock FAIL SYS_memfd_create");

    int check = ftruncate(fd, SMB1_SIZE);
    if (check < 0) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock ftruncate ERR : %s", buf);
        return -1;
    }

    g_smbAddr = mmap(nullptr, SMB1_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_smbAddr == static_cast<void*>(MAP_FAILED)) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock g_smbAddr mmap ERR : %s", buf);
        return -1;
    }
    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr);
    // initialize header infos
    header_->info.readOffset_ = 0;
    header_->info.writeOffset_ = 0;
    header_->info.memorySize_ = SMB1_SIZE - sizeof(ShareMemoryBlock::BlockHeader);
    header_->info.bytesCount_ = 0;
    header_->info.chunkCount_ = 0;
    return fd;
}

/**
 * @tc.name: plugin
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(WriterAdapterTest, Writer, TestSize.Level1)
{
    WriterAdapter writerAdapter;
    EXPECT_EQ(writerAdapter.GetWriter(), nullptr);
    EXPECT_EQ(writerAdapter.GetStruct(), &writerAdapter.writerStruct_);
}

/**
 * @tc.name: plugin
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(WriterAdapterTest, Func, TestSize.Level1)
{
    WriterAdapter writerAdapter;
    EXPECT_EQ(writerAdapter.WriteFunc(nullptr, nullptr, 0), 0);
    EXPECT_FALSE(writerAdapter.FlushFunc(nullptr));
}

HWTEST_F(WriterAdapterTest, StartReportFuncTest, TestSize.Level1)
{
    int smbFd = InitShareMemory();
    auto write = std::make_shared<BufferWriter>("testplugin", "1.01", SMB1_SIZE, smbFd, -1, 0);
    WriterAdapter writerAdapter;
    writerAdapter.SetWriter(write);
    EXPECT_NE(writerAdapter.GetWriter(), nullptr);
    const uint8_t data[] = {0x0A, 0x05, 0x31, 0x32, 0x33, 0x34, 0x35};
    const int len = 7;
    EXPECT_GE(WriterAdapter::WriteFunc(reinterpret_cast<WriterStruct*>(&writerAdapter), data, len), 0);
    writerAdapter.SetWriter(nullptr);
    EXPECT_EQ(WriterAdapter::WriteFunc(reinterpret_cast<WriterStruct*>(&writerAdapter), data, len), 0);
    EXPECT_EQ(WriterAdapter::WriteFunc(nullptr, data, len), 0);
    EXPECT_FALSE(writerAdapter.FlushFunc(reinterpret_cast<WriterStruct*>(&writerAdapter)));
    EXPECT_EQ(WriterAdapter::StartReportFunc(nullptr), nullptr);

    writerAdapter.SetWriter(write);
    EXPECT_TRUE(writerAdapter.FlushFunc(reinterpret_cast<WriterStruct*>(&writerAdapter)));
    auto ctx = WriterAdapter::StartReportFunc(reinterpret_cast<WriterStruct*>(&writerAdapter));
    EXPECT_NE(ctx, nullptr);
    WriterAdapter::FinishReportFunc(reinterpret_cast<WriterStruct*>(&writerAdapter), 32);
    munmap(g_smbAddr, SMB1_SIZE);
    close(smbFd);
}

} // namespace