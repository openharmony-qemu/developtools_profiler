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
#include "paged_mem_pool.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sys/mman.h>
#include "logging.h"

namespace {
#ifndef PAGE_SIZE
constexpr size_t PAGE_SIZE = 4096;
#endif
} // namespace

FTRACE_NS_BEGIN
PagedMemPool::PagedMemPool(size_t pagePerBlock, size_t maxCacheSize)
    : blockSize_(pagePerBlock * PAGE_SIZE), maxCacheSize_(maxCacheSize)
{
    HILOG_INFO(LOG_CORE, "PagedMemPool init with %zu ppb, %zu slots!", pagePerBlock, maxCacheSize);
}

PagedMemPool::~PagedMemPool()
{
    auto it = blockSet_.begin();
    while (it != blockSet_.end()) {
        Free(*it);
        it = blockSet_.erase(it);
    }
    HILOG_INFO(LOG_CORE, "PagedMemPool deinit!");
}

size_t PagedMemPool::GetBlockSize() const
{
    return blockSize_;
}

void* PagedMemPool::Allocate()
{
    void* block = nullptr;
    if (freeList_.size() > 0) {
        block = freeList_.back();
        freeList_.pop_back();
        return block;
    }

    block = mmap(nullptr, blockSize_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK_NOTNULL(block, nullptr, "mmap failed, %d", errno);
    blockSet_.insert(block);
    HILOG_INFO(LOG_CORE, "PagedMemPool::Allocate %zuB block done!", blockSize_);
    return block;
}

bool PagedMemPool::Recycle(void* block)
{
    CHECK_TRUE(Valid(block), false, "block not Allocate returned!");

    if (freeList_.size() < maxCacheSize_) {
        freeList_.push_back(block);
        return true;
    }
    CHECK_TRUE(Free(block), false, "Free block failed!");
    blockSet_.erase(block);
    return true;
}

bool PagedMemPool::Free(void* block)
{
    CHECK_TRUE(munmap(block, blockSize_) == 0, false, "munmap failed, %d", errno);
    return true;
}

bool PagedMemPool::Valid(void* block)
{
    return blockSet_.count(block) > 0;
}
FTRACE_NS_END
