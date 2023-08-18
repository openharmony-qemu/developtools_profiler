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
#ifndef HIPERF_PERF_FILE_FORMAT_H
#define HIPERF_PERF_FILE_FORMAT_H

#include <string>
#include "utilities.h"

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
// ref struct
struct SymbolStruct {
    uint64_t vaddr_ = 0;
    uint32_t len_ = 0;
    std::string symbolName_ = EMPTY_STRING;
    SymbolStruct() {};
    SymbolStruct(uint64_t vaddr, uint32_t len, const std::string &symbolName)
        : vaddr_(vaddr), len_(len), symbolName_(symbolName)
    {
    }
};

struct SymbolFileStruct {
    std::string filePath_ = EMPTY_STRING;
    uint32_t symbolType_;
    uint64_t textExecVaddr_;
    uint64_t textExecVaddrFileOffset_;
    std::string buildId_;
    std::vector<SymbolStruct> symbolStructs_;
};
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
#endif