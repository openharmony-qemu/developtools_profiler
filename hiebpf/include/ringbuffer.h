/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <mutex>
#include <securec.h>

class RingBuffer {
public:
    enum MemAlignShift:std::size_t {
        B_ALIGN_SHIFT = 0,  // byte alingment
        H_ALIGN_SHIFT = 1,  // half word alignment
        W_ALIGN_SHIFT = 2,  // word alignment
        D_ALIGN_SHIFT = 3,  // double word alignment
    };

    RingBuffer(
        const std::size_t bufSize = DEFAULT_SIZE,
        const enum MemAlignShift shift = B_ALIGN_SHIFT);

    inline ~RingBuffer()
    {
        if (buffer_ != nullptr) {
            delete[] buffer_;
            buffer_ = nullptr;
        }
    }
    inline operator bool()
    {
        std::lock_guard<std::mutex> lk {mtx_};
        return buffer_;
    }
    inline std::size_t GetFreeSize()
    {
        std::lock_guard<std::mutex> lk {mtx_};
        return FreeSize();
    }
    inline std::size_t GetDataSize()
    {
        std::lock_guard<std::mutex> lk {mtx_};
        return DataSize();
    }
    inline std::size_t GetCapacity()
    {
        std::lock_guard<std::mutex> lk {mtx_};
        return Capacity();
    }
    template<typename T>
    int Peek(T *var)
    {
        const std::size_t len {sizeof(T)};
        char *dest = (char*) var;
        if (dest == nullptr) {
            return -1;
        }
        if (len == 0) {
            return -1;
        }
        std::lock_guard<std::mutex> lk {mtx_};
        auto dataSize = DataSize();
        if (dataSize < len) {
            return -1;
        }
        if (head_ + len > bufSize_) {
            // data splitted
            int ret = memcpy_s(dest, len, buffer_ + head_, bufSize_ - head_);
            if (ret != EOK) {
                return -1;
            }
            ret = memcpy_s(dest + bufSize_ - head_, len + head_ - bufSize_, buffer_, len + head_ - bufSize_);
            if (ret != EOK) {
                return -1;
            }
        } else {
            if (memcpy_s(dest, len, buffer_ + head_, len) != EOK) {
                return -1;
            }
        }
        return 0;
    }

    ssize_t Read(const int fd, const std::size_t len);
    ssize_t Write(const int fd, const std::size_t len);
    std::size_t Get(char* dest, const std::size_t len);
    int Put(const char* str, const std::size_t len);
    int Put(const std::string& str);

private:
    inline std::size_t FreeSize() const
    {
        int res {0};
        res = head_ - tail_;
        if (res <= 0) {
            res += bufSize_;
        }
        return static_cast<std::size_t>(res);
    }
    inline std::size_t DataSize(int noLock = 1) const
    {
        int res {0};
        res = tail_ - head_;
        if (res < 0) {
            res += bufSize_;
        }
        return static_cast<std::size_t>(res);
    }
    inline std::size_t Capacity(int noLock = 1) const
    {
        return bufSize_;
    }

    int Resize();
    char* Allocate(std::size_t bufSize);

    enum BufferSize:std::size_t {
        DEFAULT_SIZE = (1 << 8),
    };
    char *buffer_ {nullptr};
    std::size_t bufSize_ {DEFAULT_SIZE};
    const std::size_t alignShift_ {B_ALIGN_SHIFT};
    /* head_ = tail_ mean empty, the buffer can never be full */
    std::size_t head_ {0};   // first readable byte
    std::size_t tail_ {0};          // first writebale byte
    std::mutex mtx_;
};
#endif