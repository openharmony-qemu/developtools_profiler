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
#include "crypt_algid.h"
#include "crypt_eal_md.h"
#endif
#include "trace_file_header.h"

class TraceFileHelper {
public:
    TraceFileHelper();
    ~TraceFileHelper();
    // the user-declared dtor suppresses implicit move members; default them explicitly so
    // `helper_ = {}` (reset to a fresh ctx, trace_file_writer.cpp split path) keeps compiling
    TraceFileHelper(TraceFileHelper&&) = default;
    TraceFileHelper& operator=(TraceFileHelper&&) = default;

    bool AddSegment(const uint8_t data[], uint32_t size);

    bool Update(TraceFileHeader& header);

    bool Validate(const TraceFileHeader& header);

private:
    bool Finish();

private:
    TraceFileHeader header_ = {};
#if !is_mingw
    // no default initializer: unique_ptr(nullptr_t) is disabled for function-pointer deleters
    // (libcxx: !is_pointer<D> requirement); the ctor always pairs NewCtx with MdFreeCtx below
    std::unique_ptr<CRYPT_EAL_MdCtx, decltype(&CRYPT_EAL_MdFreeCtx)> shaCtx_;
#endif
};

#endif  // TRACE_FILER_HELPER_H