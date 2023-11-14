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
#include "ftrace_field_processor.h"

#include <cinttypes>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include "printk_formats_processor.h"
#include "securec.h"

namespace {
constexpr uint32_t LOCDATA_LENGTH_SHIFT = 16;
constexpr uint32_t LOCDATA_OFFSET_MASK = 0xffff;
constexpr uint32_t LOCDATA_LENGTH_MASK = 0xffff;

std::string HandleString(const uint8_t* startPos, const uint8_t* endPos, int size)
{
    TS_CHECK_TRUE_RET(endPos - startPos >= static_cast<ptrdiff_t>(size), "");
    std::string curStr;
    const uint8_t* curCursor = startPos;
    const uint8_t* sectionEnd = startPos + size;
    while (*curCursor && curCursor < sectionEnd) {
        curCursor++;
    }
    curStr.assign(startPos, curCursor);
    return curStr;
}
} // namespace

namespace SysTuning {
namespace TraceStreamer {
bool FtraceFieldProcessor::HandleTypeData(const uint8_t* startPos, const uint8_t* endPos, void* out, size_t size)
{
    ptrdiff_t memSize = endPos - startPos;
    TS_CHECK_TRUE_RET(memSize >= static_cast<ptrdiff_t>(size), false);
    TS_CHECK_TRUE(memcpy_s(out, size, startPos, size) == EOK, false, "copy %zu byte to memory region [%p, %p) FAILED!",
                  size, startPos, endPos);
    return true;
}

std::string FtraceFieldProcessor::HandleStrField(const FieldFormat& format, uint8_t data[], size_t size)
{
    TS_CHECK_TRUE_RET((format.offset + format.size) <= size, "");
    std::string curStr;
    uint8_t* startPos = data + format.offset;
    uint8_t* endPos = data + size;
    size_t curStrSize = 0;
    uint64_t curStrPtr = 0;
    uint32_t curLocData = 0;
    uint32_t curDataOffset = 0;
    uint32_t curDataLength = 0;

    switch (format.filedType) {
        case FIELD_TYPE_FIXEDCSTRING:
            curStr = HandleString(startPos, endPos, format.size);
            break;
        case FIELD_TYPE_CSTRING:
            curStrSize = format.size;
            if (curStrSize == 0) {
                curStrSize = endPos - startPos;
                curStr = HandleString(startPos, endPos, curStrSize);
            } else {
                curStr = HandleString(startPos, endPos, curStrSize);
            }
            break;
        case FIELD_TYPE_STRINGPTR:
            curStrSize = std::min(static_cast<size_t>(format.size), sizeof(curStrPtr));
            if (memcpy_s(&curStrPtr, sizeof(curStrPtr), startPos, curStrSize) != EOK) {
                TS_LOGE("parse STRINGPTR at %" PRIx64 " with %zu size FAILED!", curStrPtr, curStrSize);
                return "";
            }
            curStr = PrintkFormatsProcessor::GetInstance().GetSymbol(curStrPtr);
            break;
        case FIELD_TYPE_DATALOC:
            curLocData = 0;
            HandleTypeData(startPos, endPos, &curLocData, sizeof(curLocData));
            curDataOffset = curLocData & LOCDATA_OFFSET_MASK;
            curDataLength = (curLocData >> LOCDATA_LENGTH_SHIFT) & LOCDATA_LENGTH_MASK;
            if (curDataOffset > 0 && data + curDataOffset + curDataLength <= endPos) {
                curStr = HandleString(data + curDataOffset, endPos, curDataLength);
            }
            break;
        default:
            break;
    }
    return curStr;
}
} // namespace TraceStreamer
} // namespace SysTuning
