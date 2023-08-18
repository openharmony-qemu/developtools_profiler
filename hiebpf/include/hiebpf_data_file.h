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

#ifndef HIEBPF_DATA_FILE_H
#define HIEBPF_DATA_FILE_H
#include <sys/mman.h>
#include <cerrno>
#include <memory>
#include <string>
#include <mutex>

#include <fcntl.h>
#include <unistd.h>

#include "type_headers.h"
#include "ringbuffer.h"
#include "hhlog.h"


struct DataFileHeader {
    enum HeaderConsts:std::size_t {
        MAGIC_LEN = 8,
        TOTAL_HEAD_SIZE = 1024,
        FIXED_HEAD_SIZE = 24
    };
    char magic[MAGIC_LEN] = {'E', 'B', 'P', 'F', 'F', 'I', 'L', 'E'};
    __u32 headSize_ {TOTAL_HEAD_SIZE};
    __u32 version_ {0};
    __u32 clock_ {1};
    __u32 cmdLen_ {0};
    char cmd_[TOTAL_HEAD_SIZE - FIXED_HEAD_SIZE] {};
};

class HiebpfDataFile {
public:
    ~HiebpfDataFile()
    {
        if (mapAddr_) {
            munmap(mapAddr_, length_);
            mapAddr_ = nullptr;
        }
        if (fd_ != -1) {
            std::size_t curPos = mapPos_ + offset_;
            ftruncate(fd_, curPos);
            close(fd_);
            fd_ = -1;
        }
    }

    static std::shared_ptr<HiebpfDataFile> MakeShared(
        const std::string& cmd,
        const std::string& filename,
        const std::size_t pages = DEFAULT_MMAP_PAGES);
    void* Reserve(const std::size_t size);
    void Discard(void *data);
    void Submit(void *data);
    void WriteKernelSymbol();

    enum Constants:std::size_t {
        DEFAULT_MMAP_PAGES = (1 << 8),
        DEFAULT_MMAP_LENGTH = (1 << 20),
        DEFAULT_PAGE_SIZE = (1 << 12),
    };

private:
    int MapFile();
    int RemapFile(const std::size_t size);
    HiebpfDataFile(
        const std::string& cmd,
        const std::string& filename,
        const std::size_t pages = DEFAULT_MMAP_PAGES)
        : hiebpfCmd_ {cmd},
          filename_ {filename}
    {
        long pageSize = sysconf(_SC_PAGE_SIZE);
        HHLOGI(true, "page size of the current OS: %u bytes", pageSize);
        if (pageSize > 0) {
            pageSize_ = static_cast<size_t>(pageSize);
            length_ = static_cast<size_t>(pageSize * pages);
        }
        HHLOGI(true, "mmap length = %u", length_);
    }

    inline int OpenFile()
    {
        constexpr int FILE_MODE = 0644;
        fd_ = open(filename_.c_str(), O_RDWR | O_CREAT, FILE_MODE);
        return (fd_ < 0) ? -1 : 0;
    }

    inline int ExtendFile(const std::size_t mapPos, const std::size_t mapLength)
    {
        if (ftruncate(fd_, mapPos + mapLength) != 0) {
            HHLOGE(true, "failed to extend data file");
            return -1;
        }
        return 0;
    }

    int WriteFileHeader()
    {
        struct DataFileHeader header {};
        header.cmdLen_ = hiebpfCmd_.length() + 1;
        if (strncpy_s(header.cmd_, header.TOTAL_HEAD_SIZE - header.FIXED_HEAD_SIZE,
                      hiebpfCmd_.c_str(), hiebpfCmd_.length() + 1) != EOK) {
            HHLOGE(true, "failed to copy string");
            return -1;
        }
        if (memcpy_s(mapAddr_, length_, &header, sizeof(header)) != EOK) {
            HHLOGE(true, "failed to memcpy_s");
            return -1;
        }
        offset_ += sizeof(header);
        return 0;
    }

    const std::string hiebpfCmd_ {};
    const std::string filename_ {"/data/local/tmp/hiebpf.data"};
    std::size_t length_ {DEFAULT_MMAP_LENGTH};
    std::size_t pageSize_ {};

    int fd_ {-1};
    void *mapAddr_ {MAP_FAILED};
    std::size_t mapPos_ {0};
    std::size_t offset_ {0};
    std::mutex mtx_ {};
};

#endif