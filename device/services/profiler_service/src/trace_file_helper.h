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
#ifndef TRACE_FILER_HELPER_H
#define TRACE_FILER_HELPER_H

#include <cstdint>
#include <memory>
#if !is_mingw
#include "openssl/sha.h"
#endif
#include "trace_file_header.h"

class TraceFileHelper {
public:
    TraceFileHelper();
    ~TraceFileHelper();

    bool AddSegment(const uint8_t data[], uint32_t size);

    bool Update(TraceFileHeader& header);

    bool Validate(const TraceFileHeader& header);

private:
    bool Finish();

private:
    TraceFileHeader header_ = {};
#if !is_mingw
    std::shared_ptr<SHA256_CTX> shaCtx_ = nullptr;
#endif
};

#endif  // TRACE_FILER_HELPER_H