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
#ifndef PAGED_MEM_POOL_H
#define PAGED_MEM_POOL_H

#include <set>
#include <vector>
#include "ftrace_namespace.h"

FTRACE_NS_BEGIN
class PagedMemPool {
public:
    PagedMemPool(size_t pagePerBlock = 1, size_t maxCacheSize = 0);
    ~PagedMemPool();

    void* Allocate();

    bool Recycle(void* block);

    size_t GetBlockSize() const;

private:
    DISALLOW_COPY_AND_MOVE(PagedMemPool);
    bool Free(void* block);
    bool Valid(void* block);

private:
    size_t blockSize_ = 1;
    size_t maxCacheSize_ = 0;
    std::vector<void*> freeList_;
    std::set<void*> blockSet_;
};
FTRACE_NS_END

#endif // PAGED_MEM_POOL_H