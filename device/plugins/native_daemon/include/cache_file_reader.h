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

#ifndef CACHE_FILE_READER_H
#define CACHE_FILE_READER_H

#include <cstdint>
#include "hook_common.h"

enum RecordHeadStatus {
    RECORD_HEAD_OK,
    RECORD_HEAD_END,
    RECORD_HEAD_INVALID,
};

RecordHeadStatus GetRecordHead(const uint8_t* buffer, uint32_t offset, bool isCrashFile,
                               uint32_t& totalSize);
// crash-mode tolerant read: on totalSize==0, skips 4 bytes and retries (max MAX_ZERO_HEAD_RETRY);
// returns true and sets totalSize when a valid record head is found, false when retries exhausted
bool GetRecordHeadWithRetry(const uint8_t* buffer, uint32_t totalAvail, uint32_t& offset,
                            uint32_t& totalSize, bool isCrashFile);
uint32_t GetCacheFileReadSize(CacheFileReadContext& ctx);
bool HandleOversizeRecord(CacheFileReadContext& ctx);

#endif // CACHE_FILE_READER_H
