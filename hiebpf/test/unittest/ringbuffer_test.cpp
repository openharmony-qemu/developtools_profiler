/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#include <fcntl.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ringbuffer.h"

using namespace testing::ext;

namespace {
constexpr int32_t BUFFER_SIZE = 1 * 1024 * 1024;
const std::string READ_FILE_NAME = "/data/local/tmp/hiebpfreadtest.txt";
const std::string WRITE_FILE_NAME = "/data/local/tmp/hiebpfwritetest.txt";
constexpr int FILE_MODE = 0777;
} // namespace

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class RingbufferTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase()
    {
        if (access(READ_FILE_NAME.c_str(), F_OK) == 0) {
            std::string cmd = "rm " + READ_FILE_NAME;
            system(cmd.c_str());
        }
        if (access(WRITE_FILE_NAME.c_str(), F_OK) == 0) {
            std::string cmd = "rm " + WRITE_FILE_NAME;
            system(cmd.c_str());
        }
    }

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: RingbufferByte
 * @tc.desc: Test Ringbuffer DEFAULT
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, RingbufferDefault, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(0, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    ASSERT_EQ(ringBuffer->bufSize_, RingBuffer::BufferSize::DEFAULT_SIZE);
}

/**
 * @tc.name: RingbufferByte
 * @tc.desc: Test Ringbuffer B_ALIGN_SHIFT
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, RingbufferByte, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, BUFFER_SIZE);

    const size_t size = 263; // 100000111
    ringBuffer = std::make_unique<RingBuffer>(size, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, size);
}

/**
 * @tc.name: RingbufferHalfWord
 * @tc.desc: Test Ringbuffer H_ALIGN_SHIFT
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, RingbufferHalfWord, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::H_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, BUFFER_SIZE);

    const size_t size = 263; // 100000111
    const int bit = RingBuffer::MemAlignShift::H_ALIGN_SHIFT;
    const size_t expectSize = (pow(2, bit)) * (size >> bit);  // 2*131(10000011)
    ringBuffer = std::make_unique<RingBuffer>(size, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, expectSize);
}

/**
 * @tc.name: RingbufferWord
 * @tc.desc: Test Ringbuffer W_ALIGN_SHIFT
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, RingbufferWord, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::W_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, BUFFER_SIZE);

    const size_t size = 263; // 100000111
    const int bit = RingBuffer::MemAlignShift::W_ALIGN_SHIFT;
    const size_t expectSize = (pow(2, bit)) * (size >> bit);  // 4*65(10000011)
    ringBuffer = std::make_unique<RingBuffer>(size, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, expectSize);
}

/**
 * @tc.name: RingbufferDoubleWord
 * @tc.desc: Test Ringbuffer D_ALIGN_SHIFT
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, RingbufferDoubleWord, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::D_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, BUFFER_SIZE);

    const size_t size = 263; // 100000111
    const int bit = RingBuffer::MemAlignShift::D_ALIGN_SHIFT;
    const size_t expectSize = (pow(2, bit)) * (size >> bit);  // 8*32(1000001)
    ringBuffer = std::make_unique<RingBuffer>(size, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, expectSize);
}

/**
 * @tc.name: Capacity
 * @tc.desc: Test Ringbuffer Capacity
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, Capacity, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    EXPECT_EQ(ringBuffer->Capacity(), BUFFER_SIZE);
}

/**
 * @tc.name: ReadAndWrite
 * @tc.desc: Test Ringbuffer Read and Write
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, ReadAndWrite, TestSize.Level1)
{
    int readFd = open(READ_FILE_NAME.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    ASSERT_GT(readFd, 0);
    std::string testStr = "this is hiebpf test file";
    for (int i = testStr.size(); i < RingBuffer::BufferSize::DEFAULT_SIZE - 1; i++) {
        testStr += "1";
    }
    int bytes = write(readFd, testStr.data(), testStr.size());
    EXPECT_EQ(bytes, testStr.size());
    close(readFd);

    readFd = open(READ_FILE_NAME.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    ASSERT_GT(readFd, 0);
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(1, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, RingBuffer::BufferSize::DEFAULT_SIZE);

    size_t ret = ringBuffer->Read(-1, 1);
    EXPECT_EQ(ret, -1);
    ret = ringBuffer->Read(readFd, 0);
    EXPECT_EQ(ret, 0);
    const int expectBytes = 20;
    auto len = ringBuffer->Read(readFd, expectBytes);
    EXPECT_EQ(len, expectBytes);
    close(readFd);

    int writeFd = open(WRITE_FILE_NAME.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    ASSERT_GT(writeFd, 0);

    ret = ringBuffer->Write(-1, 1);
    EXPECT_EQ(ret, -1);
    ret = ringBuffer->Write(writeFd, len);
    EXPECT_EQ(ret, len);
    close(writeFd);

    // The data length is greater than the buffer size, need to splitte read.
    readFd = open(READ_FILE_NAME.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    ASSERT_GT(readFd, 0);
    len = ringBuffer->Read(readFd, bytes);
    EXPECT_EQ(len, bytes);
    close(readFd);

    writeFd = open(WRITE_FILE_NAME.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    ASSERT_GT(writeFd, 0);
    ret = ringBuffer->Write(writeFd, len);
    EXPECT_EQ(ret, len);
    close(writeFd);
}

/**
 * @tc.name: PutAndWriteChar
 * @tc.desc: Test Ringbuffer Put and Write(char)
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, PutAndWriteChar, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(1, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, RingBuffer::BufferSize::DEFAULT_SIZE);

    std::string testStr = "this is hiebpf test file";
    size_t ret = ringBuffer->Put(nullptr, 1);
    EXPECT_EQ(ret, -1);
    ret = ringBuffer->Put(testStr.c_str(), 0);
    EXPECT_EQ(ret, 0);
    auto len = ringBuffer->Put(testStr.c_str(), testStr.size());
    EXPECT_EQ(len, testStr.size());
    EXPECT_EQ(ringBuffer->tail_, testStr.size());

    int writeFd = open(WRITE_FILE_NAME.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    ASSERT_GT(writeFd, 0);

    ret = ringBuffer->Write(-1, 1);
    EXPECT_EQ(ret, -1);
    ret = ringBuffer->Write(writeFd, len);
    EXPECT_EQ(ret, len);
    close(writeFd);
}

/**
 * @tc.name: PutAndWriteStr
 * @tc.desc: Test Ringbuffer Put and Write(str)
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, PutAndWriteStr, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(0, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, RingBuffer::BufferSize::DEFAULT_SIZE);

    size_t ret = ringBuffer->Put("");
    EXPECT_EQ(ret, -1);
    ret = ringBuffer->Put("\0");
    EXPECT_EQ(ret, -1);
    std::string testStr = "this is hiebpf test file";
    auto len = ringBuffer->Put(testStr);
    EXPECT_EQ(len, testStr.size());

    int writeFd = open(WRITE_FILE_NAME.c_str(), O_CREAT | O_RDWR, FILE_MODE);
    ASSERT_GT(writeFd, 0);

    ret = ringBuffer->Write(-1, 1);
    EXPECT_EQ(ret, -1);
    ret = ringBuffer->Write(writeFd, len);
    EXPECT_EQ(ret, len);
    close(writeFd);
}

/**
 * @tc.name: PutAndGetStr
 * @tc.desc: Test Ringbuffer Put and Get(str)
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, PutAndGetStr, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(0, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, RingBuffer::BufferSize::DEFAULT_SIZE);

    std::string testStr = "this is hiebpf test file";
    auto len = ringBuffer->Put(testStr);
    EXPECT_EQ(len, testStr.size());

    std::vector<char> buff;
    buff.resize(testStr.size() + 1);
    len = ringBuffer->Get(nullptr, 1);
    EXPECT_EQ(len, 0);
    len = ringBuffer->Get(buff.data(), 0);
    EXPECT_EQ(len, 0);
    len = ringBuffer->Get(buff.data(), RingBuffer::BufferSize::DEFAULT_SIZE + 1);
    EXPECT_EQ(len, 0);
    len = ringBuffer->Get(buff.data(), testStr.size());
    EXPECT_EQ(len, testStr.size());

    // need to splitte data
    for (int i = testStr.size() + 1; i < RingBuffer::BufferSize::DEFAULT_SIZE; i++) {
        testStr += "1";
    }
    len = ringBuffer->Put(testStr);
    EXPECT_EQ(len, testStr.size());
    buff.resize(testStr.size() + 1);
    len = ringBuffer->Get(buff.data(), testStr.size());
    EXPECT_EQ(len, testStr.size());

    // The data length is greater than the buffer size, need to resize.
    testStr += "11111111";
    len = ringBuffer->Put(testStr);
    EXPECT_EQ(len, testStr.size());
    buff.resize(testStr.size() + 1);
    len = ringBuffer->Get(buff.data(), testStr.size());
    EXPECT_EQ(len, testStr.size());
}

/**
 * @tc.name: Allocate
 * @tc.desc: Test Ringbuffer Allocate
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, Allocate, TestSize.Level1)
{
    auto ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, RingBuffer::MemAlignShift::B_ALIGN_SHIFT);
    ASSERT_TRUE(ringBuffer != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, BUFFER_SIZE);
    auto newBuffer = ringBuffer->Allocate(1);
    ASSERT_TRUE(newBuffer != nullptr);

    ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, RingBuffer::MemAlignShift::H_ALIGN_SHIFT);
    ASSERT_TRUE(ringBuffer != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, BUFFER_SIZE);
    newBuffer = ringBuffer->Allocate(1);
    ASSERT_TRUE(newBuffer != nullptr);

    ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, RingBuffer::MemAlignShift::W_ALIGN_SHIFT);
    ASSERT_TRUE(ringBuffer != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, BUFFER_SIZE);
    newBuffer = ringBuffer->Allocate(1);
    ASSERT_TRUE(newBuffer != nullptr);

    ringBuffer = std::make_unique<RingBuffer>(BUFFER_SIZE, RingBuffer::MemAlignShift::D_ALIGN_SHIFT);
    ASSERT_TRUE(ringBuffer != nullptr);
    EXPECT_EQ(ringBuffer->bufSize_, BUFFER_SIZE);
    newBuffer = ringBuffer->Allocate(1);
    ASSERT_TRUE(newBuffer != nullptr);
}

/**
 * @tc.name: Peek
 * @tc.desc: Test Ringbuffer Peek
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, Peek, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(1, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);

    uint32_t expectNum = 0;
    auto ret = ringBuffer->Peek(&expectNum); // ringBuffer no data
    EXPECT_EQ(ret, -1);

    uint32_t num = 4294967295;
    char* ptr = reinterpret_cast<char *>(&num);
    auto len = ringBuffer->Put(ptr, sizeof(ptr));
    EXPECT_EQ(len, sizeof(ptr));

    ret = ringBuffer->Peek(&expectNum);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(num, expectNum);

    // need to splitte data
    for (int i = sizeof(ptr) + 1; i < RingBuffer::BufferSize::DEFAULT_SIZE - 1; i++) {
        len = ringBuffer->Put("2", 1);
        EXPECT_EQ(len, 1);
    }
    len = ringBuffer->Put("3");
    EXPECT_EQ(len, 1);
    const int size = RingBuffer::BufferSize::DEFAULT_SIZE - 1;
    char buff[size + 1] = {0};
    ret = ringBuffer->Get(buff, size);
    EXPECT_STRNE(buff, "");
    EXPECT_EQ(ret, size);
    len = ringBuffer->Put(ptr, sizeof(ptr));
    EXPECT_EQ(len, sizeof(ptr));

    expectNum = 0;
    ret = ringBuffer->Peek(&expectNum);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(num, expectNum);
}

/**
 * @tc.name: Resize
 * @tc.desc: Test Ringbuffer Resize
 * @tc.type: FUNC
 */
HWTEST_F(RingbufferTest, Resize, TestSize.Level1)
{
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::B_ALIGN_SHIFT};
    auto ringBuffer = std::make_unique<RingBuffer>(1, memAlign);
    ASSERT_TRUE(ringBuffer != nullptr);
    ASSERT_TRUE(ringBuffer->buffer_ != nullptr);
    ASSERT_EQ(ringBuffer->bufSize_, RingBuffer::BufferSize::DEFAULT_SIZE);

    auto oldSize = ringBuffer->bufSize_;
    auto ret = ringBuffer->Resize();
    EXPECT_EQ(ret, 0);
    ASSERT_EQ(ringBuffer->bufSize_, oldSize << 1);

    // data splitted
    std::string putStr = "";
    std::string testStr = "222222";
    std::string destStr = "this is hiebpf test file";
    const int size = destStr.length();
    ret = ringBuffer->Put(destStr);
    EXPECT_EQ(ret, size);
    while (putStr.size() < (ringBuffer->bufSize_ - size - 1)) {
        putStr += "1";
    }
    ret = ringBuffer->Put(putStr.c_str());
    EXPECT_EQ(ret, putStr.size());
    char buff[size + 1];
    memset_s(buff, sizeof(buff), 0, sizeof(buff));
    ret = ringBuffer->Get(buff, size);
    EXPECT_EQ(ret, size);
    EXPECT_STREQ(buff, destStr.c_str());
    ret = ringBuffer->Put(testStr);
    EXPECT_EQ(ret, testStr.size());

    oldSize = ringBuffer->bufSize_;
    ret = ringBuffer->Resize();
    EXPECT_EQ(ret, 0);
    ASSERT_EQ(ringBuffer->bufSize_, oldSize << 1);
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
