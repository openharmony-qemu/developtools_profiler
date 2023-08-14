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

#include "hiebpf_data_file.h"

#include <vector>

#include "kernel_symbol_info.h"


std::shared_ptr<HiebpfDataFile> HiebpfDataFile::MakeShared(
    const std::string& cmd,
    const std::string& filename,
    const std::size_t pages)
{
    std::shared_ptr<HiebpfDataFile> obj {new(std::nothrow) HiebpfDataFile {cmd, filename, pages}};
    CHECK_NOTNULL(obj, nullptr, "failed to make HiebpfDataFile");
    CHECK_TRUE(obj->OpenFile() == 0, nullptr, "failed to open hiebpf data file");
    CHECK_TRUE(obj->MapFile() == 0, nullptr, "failed to map hiebpf data file into memory");
    return obj;
}

void* HiebpfDataFile::Reserve(const std::size_t size)
{
    std::lock_guard<std::mutex> lk {mtx_};
    if (offset_ + size >= length_) {
        if (RemapFile(size) != 0) {
            return nullptr;
        }
    }

    if (mapAddr_ == nullptr) {
        return nullptr;
    }
    char* buffer = static_cast<char *>(mapAddr_);
    buffer += offset_;
    (void)memset_s(buffer, size, 0, size);
    uint32_t *tracer = reinterpret_cast<uint32_t *>(buffer);
    (*tracer) = BADTRACE;
    uint32_t *len = tracer + 1;
    (*len) = size - sizeof(uint32_t) * 2; // 2: double
    offset_ += size;
    return buffer;
}

void HiebpfDataFile::Discard(void *data)
{
    if (data) {
        int64_t interval = static_cast<int64_t>((__u64)data) - ((__u64)mapAddr_);
        if (0 <= interval and static_cast<uint64_t>(interval) < length_) {
            uint32_t *tracer = static_cast<uint32_t*>(data);
            (*tracer) = BADTRACE;
        }
    }
}

void HiebpfDataFile::WriteKernelSymbol()
{
    std::vector<uint8_t> buf;
    uint32_t bufSize = OHOS::Developtools::Hiebpf::KernelSymbolInfo::GetSymbolData(buf);
    char *tmp = static_cast<char *>(Reserve(bufSize + sizeof(uint32_t) * 2));
    if (tmp == nullptr) {
        return;
    }
    uint32_t *type = reinterpret_cast<uint32_t *>(tmp);
    (*type) = KERNEL_SYM;
    uint32_t *len = type + 1;
    (*len) = bufSize;
    if (memcpy_s(tmp + sizeof(uint32_t) * 2, bufSize, buf.data(), bufSize) != EOK) { // 2: double
        HHLOGE(true, "failed to memcpy");
        return;
    }
}

void HiebpfDataFile::Submit(void *data)
{
    __u64 addr = (__u64) data;
    addr &= ~(pageSize_ - 1);
    __u32 *len = static_cast<__u32 *>(data);
    ++len;
    int ret = msync(reinterpret_cast<void*>(addr), *len, MS_ASYNC);
    HHLOGF(ret == -1, "failed msync data item with %u bytes", *len);
    return;
}

int HiebpfDataFile::MapFile()
{
    CHECK_TRUE(ExtendFile(mapPos_, length_) == 0, -1,
               "failed to extend data file from %u with %u bytes", mapPos_, length_);
    HHLOGI(true, "done extending the data file");
    // map the file
    mapAddr_ = mmap(
        nullptr, length_,
        PROT_WRITE | PROT_READ, MAP_SHARED | MAP_POPULATE,
        fd_, mapPos_);
    CHECK_TRUE(mapAddr_ != MAP_FAILED, -1, "mmap() failed: %s", strerror(errno));
    HHLOGI(true, "done mem mapping hiebpf data file");
    // hiebpf data file header
    CHECK_TRUE(WriteFileHeader() == 0, -1, "failed to write hiebpf data file header");
    HHLOGI(true, "done writing hiebpf data file header");
    return 0;
}

int HiebpfDataFile::RemapFile(const std::size_t size)
{
    if (munmap(mapAddr_, length_) != 0) {
        return -1;
    }
    std::size_t curPos = mapPos_ + offset_;
    std::size_t remapPos = curPos & ~(pageSize_ - 1);
    std::size_t remapOff = curPos - remapPos;

    size_t extendLength = DEFAULT_MMAP_LENGTH;
    while (remapOff + size > extendLength) {
        extendLength += DEFAULT_MMAP_LENGTH;
    }

    CHECK_TRUE(ExtendFile(remapPos, extendLength) == 0, -1,
               "failed to extend file from %u with %u bytes", remapPos, length_);
    HHLOGI(true, "done extending the data file");
    mapAddr_ = mmap(
        nullptr, extendLength,
        PROT_WRITE | PROT_READ, MAP_SHARED | MAP_POPULATE,
        fd_, remapPos);
    CHECK_TRUE(mapAddr_ != MAP_FAILED, -1, "failed to remap data file from %u to %u", mapPos_, remapPos);
    HHLOGI(true, "done remapping data file from %u, to %u", mapPos_, remapPos);
    mapPos_ = remapPos;
    offset_ = remapOff;
    length_ = extendLength;
    return 0;
}