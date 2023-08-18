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

#ifndef SHARE_MEMORY_BLOCK_H
#define SHARE_MEMORY_BLOCK_H

#ifndef NO_PROTOBUF
#include "google/protobuf/message.h"
#endif
#include <cstdint>
#include <functional>
#include <string>
#include "logging.h"

class ShareMemoryBlock {
public:
    ShareMemoryBlock(const std::string& name, uint32_t size);
    ShareMemoryBlock(const std::string& name, uint32_t size, int fd);
    ~ShareMemoryBlock();

    bool PutRaw(const int8_t* data, uint32_t size);
    bool PutRawTimeout(const int8_t* data, uint32_t size);
    bool PutWithPayloadTimeout(const int8_t* header, uint32_t headerSize, const int8_t* payload, uint32_t payloadSize);
    bool PutWithPayloadSync(const int8_t* header, uint32_t headerSize, const int8_t* payload, uint32_t payloadSize);
#ifndef NO_PROTOBUF
    EXPORT_API bool PutMessage(const google::protobuf::Message& pmsg, const std::string& pluginName);
#endif
    using DataHandler = std::function<bool(const int8_t*, uint32_t)>;
    EXPORT_API bool TakeData(const DataHandler& func);

    std::string GetName();
    uint32_t GetSize();
    EXPORT_API int GetfileDescriptor();
    EXPORT_API void ClearShareMemoryBlock();

    bool Valid() const;

    enum ReusePolicy {
        DROP_NONE, // buffer满时，不丢弃老数据，不放入新数据
        DROP_OLD,  // buffer满时，丢弃最老的数据
    };
    void SetReusePolicy(enum ReusePolicy dt)
    {
        reusePloicy_ = dt;
    }

private:
    int8_t* GetFreeMemory(uint32_t size);
    bool UseFreeMemory(int8_t* pmem, uint32_t size);

    uint32_t GetDataSize();
    const int8_t* GetDataPoint();
    bool Next();

    struct BlockHeader {
        struct alignas(sizeof(uintptr_t)) {
            uint32_t writeOffset_;
            uint32_t readOffset_;
            uint32_t memorySize_;
            pthread_mutex_t mutex_;
            uint32_t bytesCount_;
            uint32_t chunkCount_;
        } info;
        int8_t data[0];
    };

    ShareMemoryBlock();
    int8_t* GetCurrentFreeMemory(uint32_t size);

    bool CreateBlock(std::string name, uint32_t size);
    bool CreateBlockWithFd(std::string name, uint32_t size, int fd);
    bool ReleaseBlock();

    int fileDescriptor_;
    void* memoryPoint_;
    uint32_t memorySize_;
    std::string memoryName_;

    BlockHeader* header_;
    ReusePolicy reusePloicy_;
};

#endif
