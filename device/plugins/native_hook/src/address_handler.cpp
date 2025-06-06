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
#include "address_handler.h"
namespace {
constexpr int BITPOOL_SIZE_FIRST = 1000 * 1024;
constexpr int BITPOOL_SIZE_SECOND = 200 * 1024;
constexpr int BITPOOL_SIZE_TOTAL_ADDR = 1000 * 1024;
constexpr int FIRST_HASH = 16;
constexpr int SECOND_HASH = 13;
constexpr int DIVIDE_VAL = 64;
constexpr int SHIFT_VAL = 16;
constexpr uint32_t MASK_SMALL = 1ULL << 22;
constexpr int SPIN_SHIFT_FIRST = 30;
constexpr int SPIN_SHIFT_SECOND = 27;
constexpr int SPIN_SHIFT_THIRD = 31;
}

AddressHandler::AddressHandler(int poolSize)
{
    bitPoolSize_ = poolSize;
    addressChecker_ = new Bitpool [poolSize] {{0}};
}

AddressHandler::~AddressHandler()
{
    delete [] addressChecker_;
    addressChecker_ = nullptr;
}

LowAddrHandler::LowAddrHandler() : AddressHandler(BITPOOL_SIZE_FIRST) {};

void LowAddrHandler::AddAllocAddr(uint64_t addr)
{
    if (!addressChecker_) {
        return;
    }
    uint32_t addrKey = static_cast<uint32_t>(addr);
    uint32_t val = HashFunc(addrKey) % (bitPoolSize_ * DIVIDE_VAL);
    addressChecker_[val / DIVIDE_VAL].slot |= (0x1 << (val % DIVIDE_VAL));
    if (successor_ != nullptr) {
        successor_->AddAllocAddr(addr);
    }
}

bool LowAddrHandler::CheckAddr(uint64_t addr)
{
    if (!addressChecker_) {
        return true;
    }
    uint32_t addrKey = static_cast<uint32_t>(addr);
    uint32_t val = HashFunc(addrKey) % (bitPoolSize_ * DIVIDE_VAL);
    if (!(addressChecker_[val / DIVIDE_VAL].slot.load() & (0x1 << (val % DIVIDE_VAL)))) {
        return false;
    }
    if (successor_ != nullptr) {
        return successor_->CheckAddr(addr);
    }
    return true;
}

uint32_t LowAddrHandler::HashFunc(uint32_t addrKey)
{
    addrKey ^= addrKey >> FIRST_HASH;
    addrKey *= 0x85ebca6b;
    addrKey ^= addrKey >> SECOND_HASH;
    addrKey *= 0xc2b2ae35;
    addrKey ^= addrKey >> FIRST_HASH;
    return addrKey;
}

MidAddrHandler::MidAddrHandler() : AddressHandler(BITPOOL_SIZE_SECOND) {};

void MidAddrHandler::AddAllocAddr(uint64_t addr)
{
    if (!addressChecker_) {
        return;
    }
    uint32_t addrKey = static_cast<uint32_t>(addr >> SHIFT_VAL);
    uint32_t val = HashFunc(addrKey) % (bitPoolSize_ * DIVIDE_VAL);
    addressChecker_[val / DIVIDE_VAL].slot |= (0x1 << (val % DIVIDE_VAL));
    if (successor_ != nullptr) {
        successor_->AddAllocAddr(addr);
    }
}

bool MidAddrHandler::CheckAddr(uint64_t addr)
{
    if (!addressChecker_) {
        return true;
    }
    uint32_t addrKey = static_cast<uint32_t>(addr >> SHIFT_VAL);
    uint32_t val = HashFunc(addrKey) % (bitPoolSize_ * DIVIDE_VAL);
    if (!(addressChecker_[val / DIVIDE_VAL].slot.load() & (0x1 << (val % DIVIDE_VAL)))) {
        return false;
    }
    if (successor_ != nullptr) {
        return successor_->CheckAddr(addr);
    }
    return true;
}

uint32_t MidAddrHandler::HashFunc(uint32_t addrKey)
{
    uint32_t hash = 2166136261u;
    //mix in the input
    hash ^= addrKey;
    hash *= 16777619u;
    hash = hash & (MASK_SMALL - 1);
    return hash;
}

WholeAddrHandler::WholeAddrHandler() : AddressHandler(BITPOOL_SIZE_TOTAL_ADDR) {};

void WholeAddrHandler::AddAllocAddr(uint64_t addr)
{
    if (!addressChecker_) {
        return;
    }
    uint32_t val = HashFunc(addr) % (bitPoolSize_ * DIVIDE_VAL);
    addressChecker_[val / DIVIDE_VAL].slot |= (0x1 << (val % DIVIDE_VAL));
    if (successor_ != nullptr) {
        successor_->AddAllocAddr(addr);
    }
}

bool WholeAddrHandler::CheckAddr(uint64_t addr)
{
    if (!addressChecker_) {
        return true;
    }
    uint32_t val = HashFunc(addr) % (bitPoolSize_ * DIVIDE_VAL);
    if (!(addressChecker_[val / DIVIDE_VAL].slot.load() & (0x1 << (val % DIVIDE_VAL)))) {
        return false;
    }
    if (successor_ != nullptr) {
        return successor_->CheckAddr(addr);
    }
    return true;
}

uint64_t WholeAddrHandler::HashFunc(uint64_t addrKey)
{
    uint64_t hashval = (addrKey += 0x9e3779b97f4a7c15);
    hashval = (hashval ^ (hashval >> SPIN_SHIFT_FIRST)) * 0xbf58476d1ce4e5b9;
    hashval = (hashval ^ (hashval >> SPIN_SHIFT_SECOND)) * 0x94d049bb133111eb;
    return hashval ^ (hashval >> SPIN_SHIFT_THIRD);
}