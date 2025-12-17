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

#include "rawdata_builder.h"
#include "hook_client.h"
#include "get_thread_id.h"
#include "logging.h"
#include <atomic>
#include <cstring>
#include <securec.h>
#include <sys/time.h>

RawDataBuilder::RawDataBuilder()
{
    if (memset_s(&data_, sizeof(data_), 0, sizeof(data_)) != EOK) {
        HILOG_BASE_ERROR(LOG_CORE, "RawDataBuilder: memset_s failed to initialize data_");
    }
}

RawDataBuilder& RawDataBuilder::SetType(uint16_t type)
{
    data_.type = type;
    return *this;
}

RawDataBuilder& RawDataBuilder::SetAddr(void* addr)
{
    data_.addr = addr;
    return *this;
}

RawDataBuilder& RawDataBuilder::SetNewAddr(void* newAddr)
{
    data_.newAddr = newAddr;
    return *this;
}

RawDataBuilder& RawDataBuilder::SetSize(size_t size)
{
    data_.mallocSize = size;
    return *this;
}

RawDataBuilder& RawDataBuilder::SetTagId(uint16_t tagId)
{
    data_.tagId = tagId;
    return *this;
}

RawDataBuilder& RawDataBuilder::SetProcessInfo()
{
    data_.pid = static_cast<uint32_t>(g_hookPid.load());
    data_.tid = static_cast<uint32_t>(GetCurThreadId());
    return *this;
}

RawDataBuilder& RawDataBuilder::SetTimestamp()
{
    clock_gettime(g_clientConfig.clockId, &data_.ts);
    return *this;
}

StackRawData RawDataBuilder::Build() const
{
    return data_;
}
