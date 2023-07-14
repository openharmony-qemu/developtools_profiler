/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "bytrace_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include "common_types.h"
#include "string_help.h"
#include "trace_streamer_selector.h"

namespace SysTuning {
namespace TraceStreamer {
bool BytraceParserFuzzTest(const uint8_t* data, size_t size)
{
    if (!size) {
        return true;
    }
    TS_LOGI("size:%u", size);
    TraceStreamerSelector stream_ = {};
    stream_.SetDataType(TRACE_FILETYPE_BY_TRACE);
    std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(size);
    if (memcpy_s(buf.get(), size, data, size)) {
        return false;
    }
    stream_.SetCleanMode(true);
    stream_.ParseTraceDataSegment(std::move(buf), size);
    stream_.WaitForParserEnd();
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning

/* Fuzzer entry point */
extern "C" int32_t LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    SysTuning::TraceStreamer::BytraceParserFuzzTest(data, size);
    return 0;
}
