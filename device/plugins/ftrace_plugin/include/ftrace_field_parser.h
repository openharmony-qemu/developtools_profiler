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
#ifndef FTRACE_PARSE_HELPERS_H
#define FTRACE_PARSE_HELPERS_H
#include <string>
#include <vector>
#include "ftrace_common_type.h"

FTRACE_NS_BEGIN
using VoidPtr = std::unique_ptr<void>::pointer;

class FtraceFieldParser {
public:
    template <typename T> static T ParseIntField(const FieldFormat& format, uint8_t data[], size_t size)
    {
        static_assert(std::is_integral<T>::value, "Integral type T required.");
        T retval = {};
        auto end = data + size;
        auto start = data + format.offset;
        ReadData(start, end, &retval, format.size);
        return retval;
    }

    template <typename T> static std::vector<T> ParseVectorIntField(const std::vector<FieldFormat>& fields,
        size_t id, uint8_t data[], size_t size)
    {
        static_assert(std::is_integral<T>::value, "Integral type T required.");
        if (fields.size() <= id) {
            return {};
        }

        FieldFormat format = fields[id];
        std::vector<T> retvalVec = {};
        size_t retvalSize = sizeof(unsigned long);
        size_t count = format.size / retvalSize;
        for (size_t i = 0; i < count; i++) {
            auto start = data + format.offset + i * retvalSize;
            auto end = start + retvalSize;
            T retval = {};
            ReadData(start, end, &retval, retvalSize);
            retvalVec.push_back(retval);
        }
        return retvalVec;
    }

    template <typename T>
    static T ParseIntField(const std::vector<FieldFormat>& fields, size_t id, uint8_t data[], size_t size)
    {
        static_assert(std::is_integral<T>::value, "Integral type T required.");
        if (fields.size() > id) {
            return ParseIntField<T>(fields[id], data, size);
        }
        return {};
    }

    static std::string ParseStrField(const FieldFormat& format, uint8_t data[], size_t size);

    static std::string ParseStrField(const std::vector<FieldFormat>& fields, size_t id, uint8_t data[], size_t size)
    {
        if (fields.size() > id) {
            return ParseStrField(fields[id], data, size);
        }
        return "";
    }

private:
    static bool ReadData(const uint8_t start[], const uint8_t end[], VoidPtr out, size_t size);
};
FTRACE_NS_END
#endif
