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
#include "trace_file_helper.h"

#include <climits>
#include <cstring>
#include <openssl/sha.h>
#include <securec.h>
#include "logging.h"

TraceFileHelper::TraceFileHelper() : shaCtx_(std::make_shared<SHA256_CTX>())
{
    SHA256_Init(shaCtx_.get());
}

TraceFileHelper::~TraceFileHelper()
{
}

bool TraceFileHelper::AddSegment(const uint8_t data[], uint32_t size)
{
    if (size > std::numeric_limits<decltype(header_.data_.length)>::max() - header_.data_.length - sizeof(size)) {
        return false;
    }
    int retval = 0;
    header_.data_.segments += 1;
    header_.data_.length += size;
    retval = SHA256_Update(shaCtx_.get(), data, size);
    CHECK_TRUE(retval, false, "[%u] SHA256_Update FAILED, s:%u!", header_.data_.segments, size);
    return true;
}

bool TraceFileHelper::Finish()
{
    int retval = 0;
    retval = SHA256_Final(header_.data_.sha256, shaCtx_.get());
    CHECK_TRUE(retval, false, "[%u] SHA256_Final FAILED!", header_.data_.segments);
    return true;
}

bool TraceFileHelper::Update(TraceFileHeader& header)
{
    CHECK_TRUE(Finish(), false, "Finish FAILED!");
    if (memcpy_s(&header, sizeof(header), &header_, sizeof(header)) != 0) {
        return false;
    }
    return true;
}

bool TraceFileHelper::Validate(const TraceFileHeader& header)
{
    CHECK_TRUE(Finish(), false, "Finish FAILED!");
    return memcmp(&header_, &header, sizeof(header_)) == 0;
    return true;
}
