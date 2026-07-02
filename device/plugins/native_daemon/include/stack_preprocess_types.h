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
#ifndef STACK_PREPROCESS_TYPES_H
#define STACK_PREPROCESS_TYPES_H

#include <cstring>
#include <string>
#include <vector>
#include <ctime>
#include "native_hook_result.pb.h"

namespace OHOS::Developtools::NativeDaemon {

struct RecordStatistic {
    uint64_t applyCount {0};
    uint64_t releaseCount {0};
    uint64_t applySize {0};
    uint64_t releaseSize {0};
    uint64_t nodeId {0};
    uint32_t tagId {0};
    RecordStatisticsEvent::MemoryType type {RecordStatisticsEvent::MALLOC};
    uint32_t nodeType {0};
    uint32_t callstackId {0};
    uint64_t GetNodeId() const;
    void SetNodeId(uint64_t val);
    uint32_t GetNodeType() const;
    void SetNodeType(uint32_t val);
    uint32_t stackSize {0};
    struct timespec ts {0, 0};
    std::vector<uint64_t> addrs;
};

struct SimplifiedRecordStatistics {
    uint64_t applyCount {0};
    uint64_t releaseCount {0};
    uint64_t applySize {0};
    uint64_t releaseSize {0};
    uint32_t tagId {0};
    RecordStatisticsEvent::MemoryType type {RecordStatisticsEvent::MALLOC};
    uint32_t callstackId {0};
    uint64_t GetNodeId() const;
    void SetNodeId(uint64_t val);
    uint32_t GetNodeType() const;
    void SetNodeType(uint32_t val);
};

struct AllocAddrInfo {
    uint64_t mallocSize;
    size_t stackId;
    struct timespec ts;
};

struct CallStackKey {
    uint64_t stackHash;
    uint64_t nodeId;
    uint32_t stackSize;
    uint32_t nodeType;
    uint32_t tagId;

    bool operator == (const CallStackKey& other) const
    {
        return stackHash == other.stackHash &&
               nodeId == other.nodeId &&
               stackSize == other.stackSize &&
               nodeType == other.nodeType &&
               tagId == other.tagId;
    }
};

struct CallStackKeyHash {
    uint64_t operator()(const CallStackKey& key) const
    {
        return key.stackHash;
    }
};

struct SimplifiedCallStackKey {
    uint64_t stackHash;
    uint32_t stackSize;
    uint16_t tagId;

    bool operator == (const SimplifiedCallStackKey& other) const
    {
        return stackHash == other.stackHash &&
               stackSize == other.stackSize &&
               tagId == other.tagId;
    }
};

struct SimplifiedCallStackKeyHash {
    uint64_t operator()(const SimplifiedCallStackKey& key) const
    {
        return key.stackHash;
    }
};

struct AsyncCallStackKey {
    uint64_t stackHash;
    uint64_t mallocSize;
    uint32_t tagId;

    bool operator==(const AsyncCallStackKey& other) const
    {
        return stackHash == other.stackHash &&
               mallocSize == other.mallocSize &&
               tagId == other.tagId;
    }
};

struct AsyncCallStackKeyHash {
    uint64_t operator()(const AsyncCallStackKey& key) const
    {
        constexpr uint64_t kMul = 0x9e3779b97f4a7c15ULL;
        constexpr uint64_t kShift = 16;
        uint64_t h = key.stackHash;
        h ^= key.mallocSize * kMul;
        h ^= key.tagId * (kMul >> kShift);
        return h;
    }
};

struct SimplifiedAsyncCallStackKey {
    uint64_t stackHash; // syncStackId ^ (asyncStackId << 32)
    uint64_t mallocSize;
    uint16_t tagId;

    bool operator==(const SimplifiedAsyncCallStackKey& other) const
    {
        return stackHash == other.stackHash &&
               mallocSize == other.mallocSize &&
               tagId == other.tagId;
    }
};

struct SimplifiedAsyncCallStackKeyHash {
    uint64_t operator()(const SimplifiedAsyncCallStackKey& key) const
    {
        constexpr uint64_t kMul = 0x9e3779b97f4a7c15ULL;
        constexpr uint64_t kShift = 16; // 16 bits for tagId mixing
        uint64_t h = key.stackHash;
        h ^= key.mallocSize * kMul;
        h ^= key.tagId * (kMul >> kShift);
        return h;
    }
};

struct CallStackHash {
    uint64_t operator()(const std::vector<uint64_t>& stack) const
    {
        constexpr uint64_t seed = 0xc70f6907ULL;
        uint64_t hash = seed;
        for (uint64_t val : stack) {
            val ^= val >> 33; // 33 : hash bit
            val *= 0xff51afd7ed558ccdULL;
            val ^= val >> 33; // 33 : hash bit
            val *= 0xc4ceb9fe1a85ec53ULL;
            val ^= val >> 33; // 33 : hash bit
            hash ^= val;
            hash = (hash << 27) | (hash >> 37); // 27: rotate left; 37: rotate right
            hash = hash * 5 + 0x52dce729; // add a constant
        }
        return hash;
    }
};

struct NewCallStack {
    std::vector<uint64_t> callStack;
    uint32_t nodeType;
    uint64_t nodeId;
    std::string tagName;

    // cached callStack hash to avoid repeated accumulation cost
    mutable uint64_t cachedCallStackHash {0};
    mutable bool cachedCallStackHashValid {false};
    uint64_t GetCallStackHash() const
    {
        if (!cachedCallStackHashValid) {
            cachedCallStackHash = CallStackHash{}(callStack);
            cachedCallStackHashValid = true;
        }
        return cachedCallStackHash;
    }

    bool operator == (const NewCallStack& other) const
    {
        return nodeId == other.nodeId &&
               nodeType == other.nodeType &&
               tagName == other.tagName &&
               callStack == other.callStack;
    }
};

struct NewCallStackHash {
    uint64_t operator()(const NewCallStack& key) const
    {
        return key.GetCallStackHash();
    }
};

} // namespace OHOS::Developtools::NativeDaemon

#endif // STACK_PREPROCESS_TYPES_H
