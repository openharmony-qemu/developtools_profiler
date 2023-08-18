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

#include "share_memory_allocator.h"
#include "logging.h"
#include "share_memory_block.h"

namespace {
const int MIN_SHARE_MEMORY_SIZE = 1024;
}
ShareMemoryAllocator& ShareMemoryAllocator::GetInstance()
{
    static ShareMemoryAllocator shareMemoryAllocator;

    return shareMemoryAllocator;
}

ShareMemoryAllocator::ShareMemoryAllocator()
{
    memoryBlocks.clear();
}
ShareMemoryAllocator::~ShareMemoryAllocator()
{
    memoryBlocks.clear();
}

bool ShareMemoryAllocator::ReleaseMemoryBlockLocal(std::string name)
{
    auto pmb = FindMemoryBlockByName(name);

    CHECK_NOTNULL(pmb, false, "FAIL %s", name.c_str());

    memoryBlocks.erase(name);
    return true;
}

bool ShareMemoryAllocator::ReleaseMemoryBlockRemote(std::string name)
{
    auto pmb = FindMemoryBlockByName(name);

    CHECK_NOTNULL(pmb, false, "FAIL %s", name.c_str());

    memoryBlocks.erase(name);
    return true;
}

ShareMemoryBlockPtr ShareMemoryAllocator::CreateMemoryBlockLocal(std::string name, uint32_t size)
{
    CHECK_TRUE(memoryBlocks.find(name) == memoryBlocks.end(), nullptr, "%s already used", name.c_str());
    CHECK_TRUE(size >= MIN_SHARE_MEMORY_SIZE, nullptr, "%s %d size less than %d", name.c_str(), size,
               MIN_SHARE_MEMORY_SIZE);

    auto block = std::make_shared<ShareMemoryBlock>(name, size);
    if (!block->Valid()) {
        HILOG_INFO(LOG_CORE, "CreateMemoryBlockLocal FAIL");
        return nullptr;
    }
    memoryBlocks[name] = block;
    return block;
}

ShareMemoryBlockPtr ShareMemoryAllocator::CreateMemoryBlockRemote(std::string name, uint32_t size, int fd)
{
    CHECK_TRUE(memoryBlocks.find(name) == memoryBlocks.end(), nullptr, "%s already used", name.c_str());
    CHECK_TRUE(size >= MIN_SHARE_MEMORY_SIZE, nullptr, "%s %d size less than %d", name.c_str(), size,
               MIN_SHARE_MEMORY_SIZE);

    auto block = std::make_shared<ShareMemoryBlock>(name, size, fd);
    if (!block->Valid()) {
        HILOG_INFO(LOG_CORE, "CreateMemoryBlockRemote FAIL");
        return nullptr;
    }
    memoryBlocks[name] = block;
    return block;
}

ShareMemoryBlockPtr ShareMemoryAllocator::FindMemoryBlockByName(std::string name)
{
    auto it = memoryBlocks.find(name);
    CHECK_TRUE(it != memoryBlocks.end(), nullptr, "FAIL");
    return it->second;
}
