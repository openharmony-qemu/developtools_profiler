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

#ifndef SHARE_MEMORY_ALLOCATOR_H
#define SHARE_MEMORY_ALLOCATOR_H

#include <map>
#include "share_memory_block.h"
#include "logging.h"

using ShareMemoryBlockPtr = STD_PTR(shared, ShareMemoryBlock);

class ShareMemoryAllocator {
public:
    EXPORT_API static ShareMemoryAllocator& GetInstance();

    EXPORT_API ShareMemoryBlockPtr CreateMemoryBlockLocal(std::string name, uint32_t size);

    EXPORT_API ShareMemoryBlockPtr CreateMemoryBlockRemote(std::string name, uint32_t size, int fd);

    EXPORT_API bool ReleaseMemoryBlockLocal(std::string name);

    EXPORT_API bool ReleaseMemoryBlockRemote(std::string name);

    EXPORT_API ShareMemoryBlockPtr FindMemoryBlockByName(std::string name);
private:
    ShareMemoryAllocator();
    ~ShareMemoryAllocator();

    std::map<std::string, ShareMemoryBlockPtr> memoryBlocks;
};

#endif