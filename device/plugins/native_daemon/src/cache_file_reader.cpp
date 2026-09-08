/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include "cache_file_reader.h"

#include <cstddef>
#include <cstdio>
#include "logging.h"
#include "securec.h"

using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr uint32_t RECORD_HEAD_DUMP_BACKWARD = 16;
constexpr uint32_t RECORD_HEAD_DUMP_FORWARD = 16;
constexpr uint32_t RECORD_HEAD_DUMP_TEXT_SIZE = (RECORD_HEAD_DUMP_BACKWARD + RECORD_HEAD_DUMP_FORWARD) * 3 + 1;
constexpr uint32_t RECORD_HEAD_PEEK_SIZE = 64;
// each byte prints at most 2 hex digits plus one space
constexpr size_t HEX_TEXT_STEP_SIZE = 3;
}

RecordHeadStatus GetRecordHead(const uint8_t* buffer, uint32_t offset, bool isCrashFile,
                               uint32_t& totalSize)
{
    totalSize = 0;
    if (buffer == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetRecordHead invalid buffer");
        return RECORD_HEAD_INVALID;
    }
    if (memcpy_s(&totalSize, sizeof(uint32_t), buffer + offset, sizeof(uint32_t)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetRecordHead memcpy_s failed");
        return RECORD_HEAD_INVALID;
    }
    if (totalSize == 0) {
        if (isCrashFile) {
            PROFILER_LOG_INFO(LOG_CORE, "GetRecordHead crash file reach end of valid data");
            return RECORD_HEAD_END;
        }
        PROFILER_LOG_ERROR(LOG_CORE, "GetRecordHead invalid totalSize=0");
        return RECORD_HEAD_INVALID;
    }
    if (totalSize > DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetRecordHead invalid totalSize=%u", totalSize);
        return RECORD_HEAD_INVALID;
    }
    return RECORD_HEAD_OK;
}

bool GetRecordHeadWithRetry(const uint8_t* buffer, uint32_t totalAvail, uint32_t& offset,
                            uint32_t& totalSize, bool isCrashFile)
{
    const uint32_t entryOffset = offset;
    uint32_t retryCount = 0;
    while (offset + sizeof(uint32_t) <= totalAvail) {
        totalSize = 0;
        if (memcpy_s(&totalSize, sizeof(uint32_t), buffer + offset, sizeof(uint32_t)) != EOK) {
            break;
        }
        if (totalSize != 0 && totalSize <= DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES) {
            return true;
        }
        if (totalSize > DEFAULT_FILE_CACHE_MEMORY_SIZE * MB_TO_BYTES) {
            break;
        }
        // totalSize == 0: in crash mode skip 4 bytes and retry, bounded by MAX_ZERO_HEAD_RETRY
        retryCount++;
        if (!isCrashFile || retryCount > MAX_ZERO_HEAD_RETRY) {
            break;
        }
        PROFILER_LOG_INFO(LOG_CORE, "GetRecordHeadWithRetry totalSize=0 at offset=%u, retry %u/%u",
            offset, retryCount, MAX_ZERO_HEAD_RETRY);
        offset += sizeof(uint32_t);
    }
    // on failure keep offset at the entry head: callers treat false as end-of-data and rely on the
    // offset==0 oversize/readDone path to stop the read loop; consuming zero bytes here would
    // make the loop memmove the zero region forward and reparse it forever
    offset = entryOffset;
    return false;
}

void DumpInvalidRecordHead(const uint8_t* buffer, uint32_t totalAvail, uint32_t offset, uint64_t readfileSize)
{
    if (buffer == nullptr || offset > totalAvail) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpInvalidRecordHead invalid params, offset=%u totalAvail=%u",
            offset, totalAvail);
        return;
    }
    uint32_t begin = offset > RECORD_HEAD_DUMP_BACKWARD ? offset - RECORD_HEAD_DUMP_BACKWARD : 0;
    uint32_t end = totalAvail - offset > RECORD_HEAD_DUMP_FORWARD ? offset + RECORD_HEAD_DUMP_FORWARD : totalAvail;
    char hexText[RECORD_HEAD_DUMP_TEXT_SIZE] = {0};
    size_t pos = 0;
    for (uint32_t i = begin; i < end && pos + HEX_TEXT_STEP_SIZE < sizeof(hexText); ++i) {
        int written = snprintf_s(hexText + pos, sizeof(hexText) - pos, sizeof(hexText) - pos - 1,
            "%02x ", buffer[i]);
        if (written <= 0) {
            break;
        }
        pos += static_cast<size_t>(written);
    }
    PROFILER_LOG_ERROR(LOG_CORE, "DumpInvalidRecordHead fileOffset=%llu(readSize=%llu bufOffset=%u) hex[%u-%u]=[%s]",
        static_cast<unsigned long long>(DATA_SIZE_HEADER_LEN + readfileSize + offset),
        static_cast<unsigned long long>(readfileSize), offset, begin, end, hexText);
    if (offset + sizeof(uint32_t) + RECORD_HEAD_PEEK_SIZE > totalAvail) {
        return;
    }
    uint8_t peekBuf[RECORD_HEAD_PEEK_SIZE] = {0};
    if (memcpy_s(peekBuf, sizeof(peekBuf), buffer + offset + sizeof(uint32_t),
        sizeof(peekBuf)) != EOK) {
        return;
    }
    uint32_t peekPid = 0;
    uint32_t peekTid = 0;
    uint16_t peekType = 0;
    errno_t retPid = memcpy_s(&peekPid, sizeof(peekPid), peekBuf + offsetof(BaseStackRawData, pid), sizeof(peekPid));
    errno_t retTid = memcpy_s(&peekTid, sizeof(peekTid), peekBuf + offsetof(BaseStackRawData, tid), sizeof(peekTid));
    errno_t retType = memcpy_s(&peekType, sizeof(peekType), peekBuf + offsetof(BaseStackRawData, type),
        sizeof(peekType));
    if (retPid == EOK && retTid == EOK && retType == EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "DumpInvalidRecordHead nextHeadGuess type=%u pid=%u tid=%u",
            static_cast<uint32_t>(peekType), peekPid, peekTid);
    }
}

uint32_t GetCacheFileReadSize(CacheFileReadContext& ctx)
{
    ctx.readDone = false;
    if (ctx.readBuffer == nullptr || ctx.remainSize >= FILE_CACHE_READ_BUFFER_SIZE) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetCacheFileReadSize invalid ctx, remainSize=%u", ctx.remainSize);
        return 0;
    }
    // zero probe is only reliable at stream-aligned positions (record boundary):
    // when remainSize > 0 the fd sits in the middle of a record whose content
    // (raw stack memory) may legally contain >=100 consecutive zero bytes
    if (ctx.isCrashFile && ctx.remainSize == 0 && IsNextCacheDataAllZero(ctx.fd)) {
        ctx.readDone = true;
        return 0;
    }
    uint32_t maxRead = FILE_CACHE_READ_BUFFER_SIZE - ctx.remainSize;
    if (!ctx.isCrashFile) {
        // remainSize bytes are already read into buffer but not counted in readfileSize yet,
        // both must be deducted from bytesToRead to avoid reading beyond declared data end
        uint64_t consumedSize = ctx.readfileSize + ctx.remainSize;
        if (consumedSize >= ctx.bytesToRead) {
            maxRead = 0;
        } else if (consumedSize + maxRead > ctx.bytesToRead) {
            maxRead = static_cast<uint32_t>(ctx.bytesToRead - consumedSize);
        }
    }
    return maxRead;
}

bool HandleOversizeRecord(CacheFileReadContext& ctx)
{
    if (ctx.readBuffer == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleOversizeRecord invalid read buffer");
        return false;
    }
    uint32_t oversizeRecord = 0;
    if (memcpy_s(&oversizeRecord, sizeof(uint32_t), ctx.readBuffer, sizeof(uint32_t)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleOversizeRecord memcpy_s failed");
        return false;
    }
    if (ctx.isCrashFile) {
        PROFILER_LOG_INFO(LOG_CORE, "HandleOversizeRecord crash file reach end of valid data");
        ctx.readDone = true;
        return true;
    }
    PROFILER_LOG_WARN(LOG_CORE, "HandleOversizeRecord record too large for buffer, totalSize=%u, skipping",
        oversizeRecord);
    off_t skipBytes = static_cast<off_t>(oversizeRecord) + static_cast<off_t>(sizeof(uint32_t)) -
                      static_cast<off_t>(ctx.remainSize);
    if (lseek(ctx.fd, skipBytes, SEEK_CUR) < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "HandleOversizeRecord lseek skip failed errno=%d", errno);
        return false;
    }
    ctx.readfileSize += sizeof(uint32_t) + oversizeRecord;
    ctx.remainSize = 0;
    return true;
}
