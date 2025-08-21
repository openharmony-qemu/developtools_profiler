/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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
#ifndef ADDRESS_HANDLER_H
#define ADDRESS_HANDLER_H
#include "logging.h"

struct Bitpool {
    std::atomic<uint64_t> slot;
};

class AddressHandler {
public:
    AddressHandler(uint64_t poolSize);
    virtual ~AddressHandler();

    void SetSuccessor(std::shared_ptr<AddressHandler> successor)
    {
        successor_ = std::move(successor);
    }

    virtual void AddAllocAddr(uint64_t addr) = 0;
    virtual bool CheckAddr(uint64_t addr) = 0;

protected:
    std::shared_ptr<AddressHandler> successor_ = nullptr;
    Bitpool* addressChecker_ = nullptr;
    uint64_t bitPoolSize_ = 0;
};

class LowAddrHandler : public AddressHandler {
public:
    LowAddrHandler();
    void AddAllocAddr(uint64_t addr) override;
    bool CheckAddr(uint64_t addr) override;
    //MurmurHash
    uint32_t HashFunc(uint32_t addrKey);
};

class MidAddrHandler : public AddressHandler {
public:
    MidAddrHandler();
    void AddAllocAddr(uint64_t addr) override;
    bool CheckAddr(uint64_t addr) override;
    //Fnv1aHash
    uint32_t HashFunc(uint32_t addrKey);
};

class WholeAddrHandler : public AddressHandler {
public:
    WholeAddrHandler();
    void AddAllocAddr(uint64_t addr) override;
    bool CheckAddr(uint64_t addr) override;
    //SplitMix64
    uint64_t HashFunc(uint64_t addrKey);
};
#endif