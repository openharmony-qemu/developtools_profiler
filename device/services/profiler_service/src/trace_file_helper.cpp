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
#include "crypt_errno.h"
#include <securec.h>
#include "logging.h"

TraceFileHelper::TraceFileHelper()
    : shaCtx_(CRYPT_EAL_MdNewCtx(CRYPT_MD_SHA256), CRYPT_EAL_MdFreeCtx)
{
    if (shaCtx_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "TraceFileHelper MdNewCtx failed");
        return;
    }
    if (CRYPT_EAL_MdInit(shaCtx_.get()) != CRYPT_SUCCESS) {
        PROFILER_LOG_ERROR(LOG_CORE, "TraceFileHelper MdInit failed, release ctx");
        shaCtx_.reset(); // release via MdFreeCtx deleter; null marks the ctx unusable
    }
}

TraceFileHelper::~TraceFileHelper()
{
}

bool TraceFileHelper::AddSegment(const uint8_t data[], uint32_t size)
{
    CHECK_TRUE(shaCtx_ != nullptr, false, "shaCtx_ is null, digest ctx unavailable");
    if (size > std::numeric_limits<decltype(header_.data_.length)>::max() - header_.data_.length - sizeof(size)) {
        return false;
    }
    header_.data_.segments += 1;
    header_.data_.length += size;
    if (data != nullptr) {
        int retval = CRYPT_EAL_MdUpdate(shaCtx_.get(), data, size);
        CHECK_TRUE(retval == CRYPT_SUCCESS, false, "[%u] SHA256_Update FAILED, s:%u, d:%p!",
            header_.data_.segments, size, data);
    }
    return true;
}

bool TraceFileHelper::Finish()
{
    CHECK_TRUE(shaCtx_ != nullptr, false,
        "shaCtx_ is null, digest ctx unavailable (MdNewCtx/MdInit failed at construction)!");
    // CRYPT_EAL_MdFinal is one-shot (sets state=FINAL, subsequent MdUpdate/MdFinal both return
    // CRYPT_EAL_ERR_STATE); OpenSSL SHA256_Final was repeatable. UpdateSaFileHeader() calls Finish()
    // periodically during data write, so finalize a snapshot copy and keep the working ctx updatable.
    std::unique_ptr<CRYPT_EAL_MdCtx, decltype(&CRYPT_EAL_MdFreeCtx)> snapCtx(
        CRYPT_EAL_MdDupCtx(shaCtx_.get()), CRYPT_EAL_MdFreeCtx);
    CHECK_TRUE(snapCtx != nullptr, false, "MdDupCtx failed, cannot snapshot digest ctx!");
    // *len is in/out: the callee validates *len >= digest size, so it must carry the buffer
    // capacity on entry (SHA256_SIZE), not 0; OpenSSL SHA256_Final has no such parameter
    uint32_t digestLen = TraceFileHeader::SHA256_SIZE;
    int retval = CRYPT_EAL_MdFinal(snapCtx.get(), header_.data_.sha256, &digestLen);
    CHECK_TRUE(retval == CRYPT_SUCCESS, false, "[%u] SHA256_FINAL FAILED, ret=%d!", header_.data_.segments, retval);
    CHECK_TRUE(digestLen == TraceFileHeader::SHA256_SIZE, false, "[%u] SHA256_FINAL digest len %u invalid!",
        header_.data_.segments, digestLen);
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
}
