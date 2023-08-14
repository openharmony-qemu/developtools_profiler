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

#include "ringbuffer.h"

#include <memory>
#include <sys/uio.h>
#include <securec.h>
#include <cstring>
#include <iostream>
RingBuffer::RingBuffer(const std::size_t bufSize, const enum MemAlignShift shift)
    : bufSize_ {bufSize},
      alignShift_ {shift}
{
    if (bufSize_ <= DEFAULT_SIZE) {
        bufSize_ = DEFAULT_SIZE;
    }
    switch (shift) {
        case B_ALIGN_SHIFT: {
            bufSize_ = (bufSize_ >> B_ALIGN_SHIFT);
            buffer_ = new(std::nothrow) char[bufSize_];
            bufSize_ = (bufSize_ << B_ALIGN_SHIFT);
            break;
        }
        case H_ALIGN_SHIFT: {
            bufSize_ = (bufSize_ >> H_ALIGN_SHIFT);
            uint16_t *temp = new(std::nothrow) uint16_t[bufSize_];
            buffer_ = reinterpret_cast<char *>(temp);
            bufSize_ = (bufSize_ << H_ALIGN_SHIFT);
            break;
        }
        case W_ALIGN_SHIFT: {
            bufSize_ = (bufSize_ >> W_ALIGN_SHIFT);
            uint32_t *temp = new(std::nothrow) uint32_t[bufSize_];
            buffer_ = reinterpret_cast<char *>(temp);
            bufSize_ = (bufSize_ << W_ALIGN_SHIFT);
            break;
        }
        case D_ALIGN_SHIFT: {
            bufSize_ = (bufSize_ >> D_ALIGN_SHIFT);
            uint64_t *temp = new(std::nothrow) uint64_t[bufSize_];
            buffer_ = reinterpret_cast<char *>(temp);
            bufSize_ = (bufSize_ << D_ALIGN_SHIFT);
            break;
        }
    }
}

ssize_t RingBuffer::Read(const int fd, const std::size_t len)
{
    if (fd < 0) {
        return -1;
    }
    if (len == 0) {
        return 0;
    }
    constexpr std::size_t numDests {2};
    struct iovec destBufs[numDests];
    // resize if free space is not big enough
    std::lock_guard<std::mutex> lk {mtx_};
    while (len >= FreeSize()) {
        // the equal sign makes sure the buffer will not be fully filled
        if (Resize() != 0) {
            return -1;
        }
    }
    // now we have enough free space to read in from fd
    destBufs[0].iov_base = buffer_ + tail_;
    if (tail_ + len < bufSize_) {
        // continuous free space
        destBufs[0].iov_len = len;
        destBufs[1].iov_base = buffer_ + tail_ + len;
        destBufs[1].iov_len = 0;
    } else {
        // free space splitted
        destBufs[0].iov_len = bufSize_ - tail_;
        destBufs[1].iov_base = buffer_;
        destBufs[1].iov_len = len + tail_ - bufSize_;
    }
    ssize_t ret = readv(fd, destBufs, numDests);
    if (ret != -1) {
        // update buffer status
        tail_ += static_cast<std::size_t>(ret);
        while (tail_ >= bufSize_) {
            tail_ -= bufSize_;
        }
    }
    return ret;
}

ssize_t RingBuffer::Write(const int fd, std::size_t len)
{
    if (fd < 0) {
        return -1;
    }
    constexpr std::size_t numSrcs {2};
    struct iovec srcBufs[numSrcs];
    std::lock_guard<std::mutex> lk {mtx_};
    std::size_t dataSize = DataSize();
    if (dataSize < len) {
        len = dataSize;
    }
    if (len == 0) {
        return 0;
    }
    // now we are sure there is at least 'len' bytes data in the buffer
    srcBufs[0].iov_base = buffer_ + head_;
    if (head_ + len > bufSize_) {
        // data splitted
        srcBufs[0].iov_len = bufSize_ - head_;
        srcBufs[1].iov_base = buffer_;
        srcBufs[1].iov_len = len + head_- bufSize_;
    } else {
        // continuous data
        srcBufs[0].iov_len = len;
        srcBufs[1].iov_base = buffer_ + head_ + len;
        srcBufs[1].iov_len = 0;
    }
    ssize_t ret = writev(fd, srcBufs, numSrcs);
    if (ret != -1) {
        // update buffer status
        head_ += static_cast<std::size_t>(ret);
        while (head_ >= bufSize_) {
            head_ -= bufSize_;
        }
    }
    return ret;
}

std::size_t RingBuffer::Get(char* dest, const std::size_t len)
{
    if (dest == nullptr) {
        return 0;
    }
    if (len == 0) {
        return 0;
    }
    std::lock_guard<std::mutex> lk {mtx_};
    auto dataSize = DataSize();
    if (len > dataSize) {
        return 0;
    }
    if (head_ + len > bufSize_) {
        // data splitted
        if (memcpy_s(dest, len, buffer_ + head_, bufSize_ - head_) != EOK) {
            return 0;
        }
        if (memcpy_s(dest + bufSize_ - head_, len + head_ - bufSize_, buffer_, len + head_ - bufSize_) != EOK) {
            return 0;
        }
    } else {
        if (memcpy_s(dest, len, buffer_ + head_, len) != EOK) {
            return 0;
        }
    }
    // update buffer status
    head_ += len;
    while (head_ >= bufSize_) {
        head_ -= bufSize_;
    }
    return len;
}

int RingBuffer::Put(const char* str, const std::size_t len)
{
    if (str == nullptr) {
        return -1;
    }
    if (len == 0) {
        return 0;
    }
    // resize if free space is not big enough
    std::lock_guard<std::mutex> lk {mtx_};
    while (len >= FreeSize()) {
        // the equal sign makes sure the buffer will not be fully filled
        if (Resize() != 0) {
            return -1;
        }
    }
    if (tail_ + len < bufSize_) {
        // continuous free space
        if (memcpy_s(buffer_ + tail_, bufSize_ - tail_, str, len) != EOK) {
            return -1;
        }
    } else {
        // splitted free space
        if (memcpy_s(buffer_ + tail_, bufSize_ - tail_, str, bufSize_ - tail_) != EOK) {
            return -1;
        }
        if (memcpy_s(buffer_, bufSize_, str + bufSize_ - tail_, len + tail_ - bufSize_) != EOK) {
            return -1;
        }
    }
    // update buffer status
    tail_ += len;
    while (tail_ >= bufSize_) {
        tail_ -= bufSize_;
    }
    return len;
}

int RingBuffer::Put(const std::string& str)
{
    if (str.empty()) {
        return -1;
    }
    std::size_t len = str.length();
    if (len == 0) {
        return 0;
    }
    // resize if free space is not big enough
    std::lock_guard<std::mutex> lk {mtx_};
    while (len >= FreeSize()) {
        // the equal sign makes sure the buffer will not be fully filled
        if (Resize() != 0) {
            return -1;
        }
    }
    if (tail_ + len < bufSize_) {
        // continuous free space
        if (memcpy_s(buffer_ + tail_, bufSize_ - tail_, str.c_str(), len) != EOK) {
            return -1;
        }
    } else {
        // splitted free space
        if (memcpy_s(buffer_ + tail_, bufSize_ - tail_, str.c_str(), bufSize_ - tail_) != EOK) {
            return -1;
        }
        if (memcpy_s(buffer_, bufSize_, str.c_str() + bufSize_ - tail_, len + tail_ - bufSize_) != EOK) {
            return -1;
        }
    }
    // update buffer status
    tail_ += len;
    while (tail_ >= bufSize_) {
        tail_ -= bufSize_;
    }
    return len;
}

char* RingBuffer::Allocate(std::size_t bufSize)
{
    char *newBuffer {nullptr};
    switch (alignShift_) {
        case B_ALIGN_SHIFT: {
            bufSize = (bufSize >> B_ALIGN_SHIFT);
            newBuffer = new(std::nothrow) char[bufSize];
            break;
        }
        case H_ALIGN_SHIFT: {
            bufSize = (bufSize >> H_ALIGN_SHIFT);
            uint16_t *temp = new(std::nothrow) uint16_t[bufSize];
            newBuffer = reinterpret_cast<char *>(temp);
            break;
        }
        case W_ALIGN_SHIFT: {
            bufSize = (bufSize >> W_ALIGN_SHIFT);
            uint32_t *temp = new(std::nothrow) uint32_t[bufSize];
            newBuffer = reinterpret_cast<char *>(temp);
            break;
        }
        case D_ALIGN_SHIFT: {
            bufSize = (bufSize >> D_ALIGN_SHIFT);
            uint64_t *temp = new(std::nothrow) uint64_t[bufSize];
            newBuffer = reinterpret_cast<char *>(temp);
            break;
        }
    }
    return newBuffer;
}

int RingBuffer::Resize()
{
    std::size_t expandedSize {bufSize_ << 1};
    char* newBuf = Allocate(expandedSize);
    if (newBuf == nullptr) {
        return -1;
    }
    // copy data to the new buffer
    auto dataSize = DataSize();
    if (head_ + dataSize > bufSize_) {
        // data splitted
        if (memcpy_s(newBuf, expandedSize, buffer_ + head_, bufSize_ - head_) != EOK) {
            delete[] newBuf;
            return -1;
        }
        if (memcpy_s(newBuf + bufSize_ - head_,
                     expandedSize - (bufSize_ - head_),
                     buffer_,
                     dataSize - (bufSize_ - head_)) != EOK) {
            delete[] newBuf;
            return -1;
        }
    } else {
        // continuous data
        if (memcpy_s(newBuf, expandedSize, buffer_ + head_, dataSize) != EOK) {
            delete[] newBuf;
            return -1;
        }
    }
    // update buffer status
    delete[] buffer_;
    buffer_ = newBuf;
    bufSize_ = expandedSize;
    head_ = 0;
    tail_ = dataSize;

    return 0;
}