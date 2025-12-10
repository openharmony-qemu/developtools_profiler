/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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

#ifndef RAWDATA_BUILDER_H
#define RAWDATA_BUILDER_H

#include <cstdint>
#include <cstddef>
#include "hook_common.h"

class RawDataBuilder {
public:
    RawDataBuilder();

    RawDataBuilder& SetType(uint16_t type);
    RawDataBuilder& SetAddr(void* addr);
    RawDataBuilder& SetNewAddr(void* newAddr);
    RawDataBuilder& SetSize(size_t size);
    RawDataBuilder& SetTagId(uint16_t tagId);
    RawDataBuilder& SetProcessInfo();
    RawDataBuilder& SetTimestamp();

    StackRawData Build() const;

private:
    StackRawData data_;
};

#endif  // RAWDATA_BUILDER_H
