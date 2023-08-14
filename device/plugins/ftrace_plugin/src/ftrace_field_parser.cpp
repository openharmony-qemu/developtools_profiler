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
#include "ftrace_field_parser.h"
#include "logging.h"
#include "printk_formats_parser.h"
#include "securec.h"

#include <cinttypes>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <unistd.h>

FTRACE_NS_BEGIN
namespace {
constexpr uint32_t DATALOC_OFFSET_MASK = 0xffff;
constexpr uint32_t DATALOC_LENGTH_MASK = 0xffff;
constexpr uint32_t DATALOC_LENGTH_SHIFT = 16;

std::string ReadString(const uint8_t start[], const uint8_t end[], int size)
{
    std::string str;

    if (end - start < static_cast<ptrdiff_t>(size)) {
        return str;
    }

    const uint8_t* cursor = start;
    const uint8_t* sectionEnd = start + size;
    while (*cursor && cursor < sectionEnd) {
        cursor++;
    }

    str.assign(start, cursor);
    return str;
}
} // namespace

bool FtraceFieldParser::ReadData(const uint8_t start[], const uint8_t end[], void* out, size_t size)
{
    ptrdiff_t memSize = end - start;
    if (memSize < static_cast<ptrdiff_t>(size)) {
        return false;
    }

    CHECK_TRUE(memcpy_s(out, size, start, size) == EOK, false, "copy %zu byte to memory FAILED!", size);
    return true;
}

std::string FtraceFieldParser::ParseStrField(const FieldFormat& format, uint8_t data[], size_t size)
{
    std::string retval;
    if ((format.offset + format.size) > size) {
        return retval;
    }

    uint8_t* start = data + format.offset;
    uint8_t* end = data + size;
    size_t strSize = 0;
    uint64_t strPtr = 0;
    uint32_t dataLoc = 0;
    uint32_t dataOffset = 0;
    uint32_t dataLength = 0;

    switch (format.filedType) {
        case FIELD_TYPE_FIXEDCSTRING:
            retval = ReadString(start, end, format.size);
            break;
        case FIELD_TYPE_CSTRING:
            strSize = format.size;
            if (strSize == 0) {
                strSize = end - start;
                retval = ReadString(start, end, strSize);
            } else {
                retval = ReadString(start, end, strSize);
            }
            break;
        case FIELD_TYPE_STRINGPTR:
            strSize = std::min(static_cast<size_t>(format.size), sizeof(strPtr));
            CHECK_TRUE(memcpy_s(&strPtr, sizeof(strPtr), start, strSize) == EOK, "",
                       "parse STRINGPTR at %" PRIx64 " with %zu size FAILED!", strPtr, strSize);
            retval = PrintkFormatsParser::GetInstance().GetSymbol(strPtr);
            break;
        case FIELD_TYPE_DATALOC:
            dataLoc = 0;
            ReadData(start, end, &dataLoc, sizeof(dataLoc));
            dataOffset = dataLoc & DATALOC_OFFSET_MASK;
            dataLength = (dataLoc >> DATALOC_LENGTH_SHIFT) & DATALOC_LENGTH_MASK;
            if (dataOffset > 0 && data + dataOffset + dataLength <= end) {
                retval = ReadString(data + dataOffset, end, dataLength);
            }
            break;
        default:
            break;
    }
    return retval;
}
FTRACE_NS_END
