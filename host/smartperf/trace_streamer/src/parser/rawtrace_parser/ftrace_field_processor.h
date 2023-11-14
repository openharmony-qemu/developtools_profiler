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
#ifndef FTRACE_FIELD_PROCESSOR_H
#define FTRACE_FIELD_PROCESSOR_H
#include <string>
#include <vector>
#include "ftrace_common_type.h"
#include "log.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace OHOS::Profiler::Plugins;
class FtraceFieldProcessor {
public:
    template <typename T>
    static T HandleIntField(const FieldFormat& format, uint8_t data[], size_t size)
    {
        static_assert(std::is_integral<T>::value, "T must be Integral type.");
        T curValue = {};
        auto endPos = data + size;
        auto startPos = data + format.offset;
        HandleTypeData(startPos, endPos, &curValue, format.size);
        return curValue;
    }

    template <typename T>
    static std::vector<T> HandleVectorIntField(const std::vector<FieldFormat>& fields,
                                               size_t id,
                                               uint8_t data[],
                                               size_t size)
    {
        static_assert(std::is_integral<T>::value, "T must be Integral type.");
        TS_CHECK_TRUE_RET(fields.size() > id, {});
        FieldFormat format = fields[id];
        std::vector<T> curValueVec = {};
        size_t curValueSize = sizeof(unsigned long);
        size_t valueNum = format.size / curValueSize;
        for (size_t index = 0; index < valueNum; index++) {
            auto start = data + format.offset + index * curValueSize;
            auto end = start + curValueSize;
            T retval = {};
            HandleTypeData(start, end, &retval, curValueSize);
            curValueVec.push_back(retval);
        }
        return curValueVec;
    }

    template <typename T>
    static T HandleIntField(const std::vector<FieldFormat>& fields, size_t id, uint8_t data[], size_t size)
    {
        static_assert(std::is_integral<T>::value, "T must be Integral type.");
        TS_CHECK_TRUE_RET(fields.size() > id, {});
        return HandleIntField<T>(fields[id], data, size);
    }

    static std::string HandleStrField(const FieldFormat& format, uint8_t data[], size_t size);

    static std::string HandleStrField(const std::vector<FieldFormat>& fields, size_t id, uint8_t data[], size_t size)
    {
        TS_CHECK_TRUE_RET(fields.size() > id, "");
        return HandleStrField(fields[id], data, size);
    }

private:
    static bool HandleTypeData(const uint8_t* startPos, const uint8_t* endPos, void* out, size_t size);
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // FTRACE_FIELD_PROCESSOR_H
