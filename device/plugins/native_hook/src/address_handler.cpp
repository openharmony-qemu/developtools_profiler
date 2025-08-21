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
constexpr uint64_t BITPOOL_SIZE_FIRST = 1000 * 1024;
constexpr uint64_t BITPOOL_SIZE_SECOND = 200 * 1024;
constexpr uint64_t BITPOOL_SIZE_TOTAL_ADDR = 1000 * 1024;
constexpr uint64_t FIRST_HASH = 16;
constexpr uint64_t SECOND_HASH = 13;
constexpr uint64_t DIVIDE_VAL = 64;
constexpr uint64_t SHIFT_VAL = 16;
constexpr uint64_t SPIN_SHIFT_FIRST = 30;
constexpr uint64_t SPIN_SHIFT_SECOND = 27;
constexpr uint64_t SPIN_SHIFT_THIRD = 31;
constexpr uint64_t SPIN_FIRST_SALT = 0x9e3779b97f4a7c15;
constexpr uint64_t SPIN_SECOND_SALT = 0xbf58476d1ce4e5b9;
constexpr uint64_t SPIN_THIRD_SALT = 0x94d049bb133111eb;
constexpr uint32_t FNV_HASH = 16777619u;
constexpr uint32_t MUR_FIRST_SALT = 0x85ebca6b;
constexpr uint32_t MUR_SECOND_SALT = 0xc2b2ae35;
}

AddressHandler::AddressHandler(uint64_t poolSize)
{
    bitPoolSize_ = poolSize;
    addressChecker_ = new Bitpool [poolSize] {{0}};
}

AddressHandler::~AddressHandler()
{
    delete [] addressChecker_;
    addressChecker_ = nullptr;
    successor_ = nullptr;
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
    addrKey *= MUR_FIRST_SALT;
    addrKey ^= addrKey >> SECOND_HASH;
    addrKey *= MUR_SECOND_SALT;
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
    hash *= FNV_HASH;
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
    uint64_t hashval = (addrKey += SPIN_FIRST_SALT);
    hashval = (hashval ^ (hashval >> SPIN_SHIFT_FIRST)) * SPIN_SECOND_SALT;
    hashval = (hashval ^ (hashval >> SPIN_SHIFT_SECOND)) * SPIN_THIRD_SALT;
    return hashval ^ (hashval >> SPIN_SHIFT_THIRD);
}