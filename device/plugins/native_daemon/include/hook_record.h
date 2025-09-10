/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HOOK_RECORD_H
#define HOOK_RECORD_H
#include <memory>
#include <string>
#include <vector>
#include "hook_common.h"
#include "perf_event_record.h"
#include "native_hook_config.pb.h"
#include "native_hook_result.pb.h"
#include "native_hook_result.pbencoder.h"
#include "nocopyable.h"
#include "utilities.h"
#include "logging.h"

namespace OHOS::Developtools::NativeDaemon {
const std::string MMAP_FILE_PAGE_PREFIX = "FilePage:";
const std::string PR_SET_VMA_PREFIX = "Anonymous:";
using NativeHookProto = std::variant<NativeHookData*, OHOS::Developtools::Profiler::ProtoEncoder::NativeHookData*>;

struct RawStack {
    std::unique_ptr<uint8_t[]> baseStackData = nullptr; // save the shared memory data
    BaseStackRawData* stackContext = nullptr; // points to the foundation type data
    union {
        uint8_t* stackData; //cannot initialize multiple members of union
        const char* jsStackData = nullptr;
    };
    uint8_t* data = nullptr; // fp mode data is ip, dwarf mode data is regs
    uint32_t stackSize = 0;
    uint8_t fpDepth = 0; // fp mode fpDepth is ip depth, dwarf mode is invalid
    bool reportFlag = false;
    bool reduceStackFlag = false;
    void Reset()
    {
        baseStackData = nullptr;
        stackContext = nullptr;
        data = nullptr;
        stackData = nullptr;
        stackSize = 0;
        fpDepth = 0;
        reportFlag = false;
        reduceStackFlag = false;
    }
};

struct SerializeInfo {
    std::vector<CallFrame>* callFrames = nullptr;
    uint32_t stackMapId = 0;
    std::string tagName = "";
    NativeHookConfig* config = nullptr;
};

using RawStackPtr = std::shared_ptr<RawStack>;

class HookRecord {
public:
    HookRecord() = default;
    HookRecord(RawStackPtr rawStack) : rawStack_(rawStack){};
    virtual uint16_t GetType();
    virtual void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) {};
    virtual void Reset();
    virtual bool IsValid();
    virtual uint64_t GetAddr();
    RawStackPtr GetRawStack()
    {
        return rawStack_;
    }
    virtual void SetAddr(uint64_t addr);

    template <typename T>
    void SetEventFrame(T* event, SerializeInfo& hookInfo);

    template <typename T>
    void SetFrameInfo(T& frame, CallFrame& callFrame, NativeHookConfig* config);

    template <typename T>
    void SetSize(T* event);

    virtual ~HookRecord() = default;
    RawStackPtr rawStack_ = nullptr;
};

using HookRecordPtr = STD_PTR(shared, OHOS::Developtools::NativeDaemon::HookRecord);

class FreeRecord : public HookRecord {
public:
    FreeRecord() = default;
    FreeRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
    void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) override;
};

class FreeRecordSimp : public HookRecord {
public:
    FreeRecordSimp() = default;
    FreeRecordSimp(uint64_t freeAddr) : freeAddr_(freeAddr) {};
    uint16_t GetType() override
    {
        return FREE_MSG_SIMP;
    }
    void Reset() override
    {
        freeAddr_ = 0;
    }
    uint64_t GetAddr() override
    {
        return freeAddr_;
    }
    void SetAddr(uint64_t freeAddr) override
    {
        freeAddr_ = freeAddr;
    }
    bool IsValid() override
    {
        return (freeAddr_ != 0);
    }

private:
    uint64_t freeAddr_ = 0;
};

class MallocRecord : public HookRecord {
public:
    MallocRecord() = default;
    MallocRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
    void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) override;
};

class MmapRecord : public HookRecord {
public:
    MmapRecord() = default;
    MmapRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
    void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) override;
};

class MmapFilePageRecord : public HookRecord {
public:
    MmapFilePageRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
    void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) override;
};

class MunmapRecord : public HookRecord {
public:
    MunmapRecord() = default;
    MunmapRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
    void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) override;
};

class PrSetVmaRecord : public HookRecord {
public:
    PrSetVmaRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
    void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) override;
};

class MemoryUsingRecord : public HookRecord {
public:
    MemoryUsingRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
    void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) override;
};

class MemoryUnusingRecord : public HookRecord {
public:
    MemoryUnusingRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
    void SerializeData(NativeHookProto stackData, SerializeInfo& hookInfo) override;
};

class NmdRecord : public HookRecord {
public:
    NmdRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
};

class JsRecord : public HookRecord {
public:
    JsRecord() = default;
    JsRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
};

class TagRecord : public HookRecord {
public:
    TagRecord(RawStackPtr rawStack) : HookRecord(rawStack) {};
};
}
#endif //HOOK_RECORD_H