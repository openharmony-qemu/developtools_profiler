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

#include "share_memory_block.h"

#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "ashmem.h"
#include "logging.h"
#include "securec.h"

namespace {
const int PIECE_HEAD_LEN = 4;
constexpr uint32_t INVALID_LENGTH = (uint32_t)-1;
constexpr uint32_t TIMEOUT_SEC = 1;
const int WAIT_RELEASE_TIMEOUT_US = 10;
static std::atomic<int> g_timeCount = 0;
#ifndef PAGE_SIZE
constexpr uint32_t PAGE_SIZE = 4096;
#endif
}  // namespace

struct PthreadLocker {
    explicit PthreadLocker(pthread_mutex_t& mutex) : mutex_(mutex)
    {
        pthread_mutex_lock(&mutex_);
    }

    ~PthreadLocker()
    {
        pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t& mutex_;
};

ShareMemoryBlock::ShareMemoryBlock()
    : fileDescriptor_(-1),
      memoryPoint_(nullptr),
      memorySize_(0),
      memoryName_(),
      header_(nullptr),
      reusePloicy_(ReusePolicy::DROP_NONE)
{
}

bool ShareMemoryBlock::CreateBlockWithFd(std::string name, uint32_t size, int fd)
{
    CHECK_TRUE(fd >= 0, false, "CreateBlock FAIL SYS_memfd_create");

    auto ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlockWithFd mmap ERR : %s", buf);
        return false;
    }

    fileDescriptor_ = fd;
    memoryPoint_ = ptr;
    memorySize_ = size;

    memoryName_ = name;
    header_ = reinterpret_cast<BlockHeader*>(ptr);

    // Reserve 4 bytes to fill the message length.
    messageWriteOffset_ = PIECE_HEAD_LEN;
    // Functions required to bind the BaseMessage class.
    smbCtx_.block = this;
    smbCtx_.ctx.getMemory = [](RandomWriteCtx* ctx, uint32_t size, uint8_t** memory, uint32_t* offset) -> bool {
        ShareMemoryBlockCtx* smbCtx = reinterpret_cast<ShareMemoryBlockCtx*>(ctx);
        return smbCtx->block->GetMemory(size, memory, offset);
    };
    smbCtx_.ctx.seek = [](RandomWriteCtx* ctx, uint32_t offset) -> bool {
        ShareMemoryBlockCtx* smbCtx = reinterpret_cast<ShareMemoryBlockCtx*>(ctx);
        return smbCtx->block->Seek(offset);
    };
    return true;
}

bool ShareMemoryBlock::CreateBlock(std::string name, uint32_t size)
{
    PROFILER_LOG_INFO(LOG_CORE, "CreateBlock %s %d", name.c_str(), size);
    CHECK_TRUE(size > sizeof(BlockHeader), false, "size %u too less!", size);
    CHECK_TRUE(size % PAGE_SIZE == 0, false, "size %u not times of %d!", size, PAGE_SIZE);

    int fd = OHOS::AshmemCreate(name.c_str(), size);
    CHECK_TRUE(fd >= 0, false, "OHOS::AshmemCreate fail.");

    int check = OHOS::AshmemSetProt(fd, PROT_READ | PROT_WRITE);
    if (check < 0) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "OHOS::AshmemSetProt ERR : %s", buf);
        return false;
    }

    auto ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock mmap ERR : %s", buf);
        return false;
    }

    fileDescriptor_ = fd;
    memoryPoint_ = ptr;
    memorySize_ = size;

    memoryName_ = name;
    header_ = reinterpret_cast<BlockHeader*>(ptr);
    if (header_ == nullptr) {
        return false;
    }
    // initialize header infos
    header_->info.readOffset_ = 0;
    header_->info.writeOffset_ = 0;
    header_->info.memorySize_ = size - sizeof(BlockHeader);
    header_->info.bytesCount_ = 0;
    header_->info.chunkCount_ = 0;

    pthread_mutexattr_t muAttr;
    pthread_mutexattr_init(&muAttr);
    pthread_mutexattr_settype(&muAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&header_->info.mutex_, &muAttr);
    return true;
}

bool ShareMemoryBlock::Valid() const
{
    return header_ != nullptr;
}

ShareMemoryBlock::ShareMemoryBlock(const std::string& name, uint32_t size) : ShareMemoryBlock()
{
    CreateBlock(name, size);
}

ShareMemoryBlock::ShareMemoryBlock(const std::string& name, uint32_t size, int fd) : ShareMemoryBlock()
{
    CreateBlockWithFd(name, size, fd);
}

ShareMemoryBlock::~ShareMemoryBlock()
{
    ReleaseBlock();
}

bool ShareMemoryBlock::ReleaseBlock()
{
    if (memorySize_ > 0) {
        munmap(memoryPoint_, memorySize_);
        memoryPoint_ = nullptr;
        memorySize_ = 0;
    }
    g_timeCount = 0;
    if (fileDescriptor_ >= 0) {
        close(fileDescriptor_);
        fileDescriptor_ = -1;
    }
    return true;
}

int8_t* ShareMemoryBlock::GetCurrentFreeMemory(uint32_t size)
{
    CHECK_NOTNULL(header_, nullptr, "header not ready!");
    uint32_t realSize = size + PIECE_HEAD_LEN + PIECE_HEAD_LEN;

    uint32_t wp = header_->info.writeOffset_.load();
    uint32_t rp = header_->info.readOffset_.load();
    if (wp < rp && rp <= wp + realSize) {
        return nullptr;
    }
    if (wp + realSize > header_->info.memorySize_) {  // 后面部分放不下，从头开始放
        if (rp == 0) {
            return nullptr;
        }
        *((uint32_t*)(&header_->data[wp])) = INVALID_LENGTH;
        wp = 0;
        if (wp + realSize >= rp) {
            return nullptr;
        }
    }
    return &header_->data[wp + PIECE_HEAD_LEN];
}

int8_t* ShareMemoryBlock::GetFreeMemory(uint32_t size)
{
    if (reusePloicy_ == ReusePolicy::DROP_NONE) {
        return GetCurrentFreeMemory(size);
    }
    int8_t* ret = nullptr;
    while (true) {
        ret = GetCurrentFreeMemory(size);
        if (ret != nullptr) {
            break;
        }
        if (!Next()) {
            return nullptr;
        }
    }
    return ret;
}

bool ShareMemoryBlock::UseFreeMemory(int8_t* pmem, uint32_t size)
{
    uint32_t wp = pmem - PIECE_HEAD_LEN - header_->data;
    *((int*)(&header_->data[wp])) = size;

    header_->info.writeOffset_ = wp + PIECE_HEAD_LEN + size;
    return true;
}

bool ShareMemoryBlock::PutRaw(const int8_t* data, uint32_t size)
{
    CHECK_NOTNULL(header_, false, "header not ready!");
    PthreadLocker locker(header_->info.mutex_);
    int8_t* rawMemory = GetFreeMemory(size);
    if (rawMemory == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "PutRaw not enough space [%d]", size);
        return false;
    }
    if (memcpy_s(rawMemory, size, data, size) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s error");
        return false;
    }

    UseFreeMemory(rawMemory, size);
    ++header_->info.bytesCount_;
    ++header_->info.chunkCount_;
    return true;
}

bool ShareMemoryBlock::PutRawTimeout(const int8_t* data, uint32_t size)
{
    CHECK_NOTNULL(header_, false, "header not ready!");

    struct timespec time_out;
    clock_gettime(CLOCK_REALTIME, &time_out);
    time_out.tv_sec += TIMEOUT_SEC;
    if (pthread_mutex_timedlock(&header_->info.mutex_, &time_out) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "PutRawTimeout failed %d", errno);
        return false;
    }

    int8_t* rawMemory = GetFreeMemory(size);
    if (rawMemory == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "PutRaw not enough space [%d]", size);
        pthread_mutex_unlock(&header_->info.mutex_);
        return false;
    }
    if (memcpy_s(rawMemory, size, data, size) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s error");
        pthread_mutex_unlock(&header_->info.mutex_);
        return false;
    }

    UseFreeMemory(rawMemory, size);
    ++header_->info.bytesCount_;
    ++header_->info.chunkCount_;

    pthread_mutex_unlock(&header_->info.mutex_);
    return true;
}

bool ShareMemoryBlock::PutWithPayloadTimeout(const int8_t* header, uint32_t headerSize,
    const int8_t* payload, uint32_t payloadSize)
{
    CHECK_NOTNULL(header_, false, "header not ready!");
    struct timespec time_out;
    clock_gettime(CLOCK_REALTIME, &time_out);
    time_out.tv_sec += TIMEOUT_SEC;
    if (pthread_mutex_timedlock(&header_->info.mutex_, &time_out) != 0) {
        return false;
    }

    int8_t* rawMemory = GetFreeMemory(headerSize + payloadSize);
    if (rawMemory == nullptr) {
        PROFILER_LOG_INFO(LOG_CORE, "%s: shared memory exhausted, discarding data", __FUNCTION__);
        pthread_mutex_unlock(&header_->info.mutex_);
        return false;
    }
    if (memcpy_s(rawMemory, headerSize, header, headerSize) != EOK) {
        pthread_mutex_unlock(&header_->info.mutex_);
        return false;
    }
    if (payloadSize > 0) {
        if (memcpy_s(rawMemory + headerSize, payloadSize, payload, payloadSize) != EOK) {
            pthread_mutex_unlock(&header_->info.mutex_);
            return false;
        }
    }
    UseFreeMemory(rawMemory, headerSize + payloadSize);
    ++header_->info.bytesCount_;
    ++header_->info.chunkCount_;

    pthread_mutex_unlock(&header_->info.mutex_);
    return true;
}

#ifndef NO_PROTOBUF
bool ShareMemoryBlock::PutMessage(const google::protobuf::Message& pmsg, const std::string& pluginName)
{
    size_t size = pmsg.ByteSizeLong();

    CHECK_NOTNULL(header_, false, "header not ready!");
    PthreadLocker locker(header_->info.mutex_);
    int8_t* rawMemory = GetFreeMemory(size);
    if (rawMemory == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: PutMessage not enough space [%zu]", pluginName.c_str(), size);
        return false;
    }

    int ret = pmsg.SerializeToArray(rawMemory, size);
    if (ret <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: SerializeToArray failed with %d, size: %zu", __func__, ret, size);
        return false;
    }
    UseFreeMemory(rawMemory, size);
    ++header_->info.bytesCount_;
    ++header_->info.chunkCount_;
    return true;
}
#endif

bool ShareMemoryBlock::TakeData(const DataHandler& func, bool isProtobufSerialize)
{
    if (!isProtobufSerialize) {
        return TakeDataOptimize(func);
    }

    CHECK_NOTNULL(header_, false, "header not ready!");
    CHECK_TRUE(static_cast<bool>(func), false, "func invalid!");

    auto size = GetDataSize();
    if (size == 0 || size > header_->info.memorySize_) {
        return false;
    }
    auto ptr = GetDataPoint();
    CHECK_TRUE(func(ptr, size), false, "call func FAILED!");
    CHECK_TRUE(Next(), false, "move read pointer FAILED!");
    --header_->info.chunkCount_;
    return true;
}

uint32_t ShareMemoryBlock::GetDataSize()
{
    if (header_->info.readOffset_.load() == header_->info.writeOffset_.load()) {
        return 0;
    }
    uint32_t ret = *((uint32_t*)(&header_->data[header_->info.readOffset_.load()]));
    if (ret == INVALID_LENGTH) {
        ret = *((uint32_t*)(&header_->data[0]));
    }
    return ret;
}

const int8_t* ShareMemoryBlock::GetDataPoint()
{
    if (*((uint32_t*)(&header_->data[header_->info.readOffset_.load()])) == INVALID_LENGTH) {
        return &header_->data[PIECE_HEAD_LEN];
    }
    return &header_->data[header_->info.readOffset_ .load()+ PIECE_HEAD_LEN];
}

bool ShareMemoryBlock::Next()
{
    if (header_->info.readOffset_.load() == header_->info.writeOffset_.load()) {
        return false;
    }
    uint32_t size = *((uint32_t*)(&header_->data[header_->info.readOffset_.load()]));
    if (size == INVALID_LENGTH) {
        size = *((uint32_t*)(&header_->data[0]));
        header_->info.readOffset_ = size + PIECE_HEAD_LEN;
    } else {
        header_->info.readOffset_ += size + PIECE_HEAD_LEN;
    }
    return true;
}

std::string ShareMemoryBlock::GetName()
{
    return memoryName_;
}

uint32_t ShareMemoryBlock::GetSize()
{
    return memorySize_;
}

int ShareMemoryBlock::GetfileDescriptor()
{
    return fileDescriptor_;
}

bool ShareMemoryBlock::PutWithPayloadSync(const int8_t* header, uint32_t headerSize,
    const int8_t* payload, uint32_t payloadSize, const std::function<bool()>& callback)
{
    if (header_ == nullptr) {
        return false;
    }
    pthread_mutex_lock(&header_->info.mutex_);
    int8_t* rawMemory = GetFreeMemory(headerSize + payloadSize);
    if (rawMemory == nullptr) {
        while (true) {
            if (rawMemory == nullptr) {
                if ((callback && callback()) || (g_timeCount > waitTime_)) {
                    HILOG_BASE_ERROR(LOG_CORE, "PutWithPayloadSync exit with g_timeCount %{public}d",
                        g_timeCount.load());
                    pthread_mutex_unlock(&header_->info.mutex_);
                    return false;
                }
                pthread_mutex_unlock(&header_->info.mutex_);
                usleep(WAIT_RELEASE_TIMEOUT_US);
                g_timeCount += WAIT_RELEASE_TIMEOUT_US;
                pthread_mutex_lock(&header_->info.mutex_);
                rawMemory = GetFreeMemory(headerSize + payloadSize);
                continue;
            }
            g_timeCount = 0;
            break;
        }
    }
    if (memcpy_s(rawMemory, headerSize + payloadSize, header, headerSize) != EOK) {
        pthread_mutex_unlock(&header_->info.mutex_);
        return false;
    }
    if (payloadSize > 0) {
        if (memcpy_s(rawMemory + headerSize, payloadSize, payload, payloadSize) != EOK) {
            pthread_mutex_unlock(&header_->info.mutex_);
            return false;
        }
    }
    UseFreeMemory(rawMemory, headerSize + payloadSize);
    ++header_->info.bytesCount_;
    ++header_->info.chunkCount_;
    pthread_mutex_unlock(&header_->info.mutex_);
    return true;
}

void ShareMemoryBlock::UseMemory(int32_t size)
{
    CHECK_TRUE(header_ != nullptr, NO_RETVAL, "header not ready!");
    CHECK_TRUE(size > 0, NO_RETVAL, "size(%d) is invalid", size);

    uint32_t wp = header_->info.writeOffset_.load(std::memory_order_relaxed);
    *((int*)(&header_->data[wp])) = size;
    header_->info.writeOffset_.store(wp + PIECE_HEAD_LEN + size, std::memory_order_release);
}

bool ShareMemoryBlock::GetMemory(uint32_t size, uint8_t** memory, uint32_t* offset)
{
    CHECK_NOTNULL(header_, false, "header not ready!");

    // The actual size is to store data with a size of offset and a size of data and a four byte tail tag.
    uint32_t realSize = messageWriteOffset_ + size + PIECE_HEAD_LEN;
    uint32_t wp = header_->info.writeOffset_.load(std::memory_order_relaxed);
    uint32_t rp = header_->info.readOffset_.load(std::memory_order_acquire);
    if (rp <= wp) {
        if (wp + realSize <= header_->info.memorySize_) {
            // enough tail space to store data.
            *memory = reinterpret_cast<uint8_t *>(&header_->data[wp + messageWriteOffset_]);
            *offset = messageWriteOffset_;
            return true;
        } else if (realSize <= rp) {
            // there is data in the tail, and it is need to copy the data in the tail to the header for saving.
            auto ret = memcpy_s(&header_->data[0], messageWriteOffset_, &header_->data[wp], messageWriteOffset_);
            CHECK_TRUE(ret == EOK, false, "memcpy_s messageWriteOffset_(%d) data failed", messageWriteOffset_);
            // set trailing data end tag.
            *((uint32_t*)(&header_->data[wp])) = INVALID_LENGTH;
            // set writeOffset_ to zero.
            header_->info.writeOffset_.store(0, std::memory_order_release);
            *memory = reinterpret_cast<uint8_t *>(&header_->data[messageWriteOffset_]);
            *offset = messageWriteOffset_;
            return true;
        }
    } else {
        if (wp + realSize <= rp) {
            // rp is after wp and there is enough space to store data.
            *memory = reinterpret_cast<uint8_t *>(&header_->data[wp + messageWriteOffset_]);
            *offset = messageWriteOffset_;
            return true;
        }
    }

    PROFILER_LOG_ERROR(LOG_CORE, "Write not enough space, realSize=%u, rp=%u, wp=%u", realSize, rp, wp);
    return false;
}

bool ShareMemoryBlock::TakeDataOptimize(const DataHandler& func)
{
    CHECK_NOTNULL(header_, false, "header not ready!");
    CHECK_TRUE(static_cast<bool>(func), false, "func invalid!");

    uint32_t wp = header_->info.writeOffset_.load(std::memory_order_acquire);
    uint32_t rp = header_->info.readOffset_.load(std::memory_order_relaxed);
    int8_t* ptr = nullptr;
    uint32_t size = 0;
    if (rp < wp) {
        // |---rp<---data--->wp---|
        size = *((uint32_t*)(&header_->data[rp]));
        ptr = &header_->data[rp + PIECE_HEAD_LEN];
    } else if (wp < rp) {
        // |<---data2--->wp---rp<---data1--->|
        size = *((uint32_t*)(&header_->data[rp]));
        // Size is the end tag of the tail and needs to be retrieved from the header.
        if (size == INVALID_LENGTH) {
            if (wp == 0) {
                // no data to read.
                return false;
            }
            rp = 0;
            size = *((uint32_t*)(&header_->data[rp]));
        }
        ptr = &header_->data[rp + PIECE_HEAD_LEN];
    } else {
        // wp == rp
        return false;
    }
    CHECK_NOTNULL(ptr, false, "ptr is nullptr");

    // Start writing file.
    CHECK_TRUE(func(ptr, size), false, "call func FAILED!");

    header_->info.readOffset_.store(rp + size + PIECE_HEAD_LEN, std::memory_order_release);
    return true;
}

bool ShareMemoryBlock::Seek(uint32_t pos)
{
    messageWriteOffset_ = pos;
    return true;
}

void ShareMemoryBlock::ResetPos()
{
    messageWriteOffset_ = PIECE_HEAD_LEN;
}