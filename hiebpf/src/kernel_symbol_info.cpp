/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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
#include "kernel_symbol_info.h"

#include <cstring>
#include <fstream>
#include <sstream>

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
void KernelSymbolInfo::GetSymbol(const std::string &line, SymbolItem &symbol)
{
    std::stringstream str(line.c_str());
    str >> std::hex >> symbol.value_;
    str >> symbol.type_;
    // only keep text or weak type
    if (std::strchr("TtWw", symbol.type_)) {
        symbol.type_ = 'T';
    } else {
        return;
    }
    str >> symbol.name_;
}

void KernelSymbolInfo::GetBinary(const std::vector<SymbolItem> &symbolItems,
                                 uint64_t vaddrStart,
                                 uint64_t vaddrEnd,
                                 uint32_t strTabLen,
                                 std::vector<uint8_t> &buf)
{
    // (value + size + nameOffset)
    uint32_t symTabLen = (sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t)) *
                         symbolItems.size();
    // vaddrStart + vaddrEnd + symTbaLen + strTabLen + symTabData + strTabData
    uint32_t dataLen = sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t)
                       + symTabLen + strTabLen;
    buf.resize(dataLen);

    uint8_t *p = buf.data();
    *(reinterpret_cast<uint64_t *>(p)) = vaddrStart;
    p += sizeof(uint64_t);
    *(reinterpret_cast<uint64_t *>(p)) = vaddrEnd;
    p += sizeof(uint64_t);
    *(reinterpret_cast<uint32_t *>(p)) = symTabLen;
    p += sizeof(uint32_t);
    *(reinterpret_cast<uint32_t *>(p)) = strTabLen;
    p += sizeof(uint32_t);

    uint32_t strTabSize = 0;
    uint8_t *pStrTab = p + symTabLen;
    for (auto &symbol : symbolItems) {
        *(reinterpret_cast<uint64_t *>(p)) = symbol.value_;
        p += sizeof(uint64_t);
        *(reinterpret_cast<uint32_t *>(p)) = symbol.size_;
        p += sizeof(uint32_t);
        *(reinterpret_cast<uint32_t *>(p)) = strTabSize;
        p += sizeof(uint32_t);

        std::copy(symbol.name_.c_str(),
                  symbol.name_.c_str() + symbol.name_.size() + 1,
                  pStrTab + strTabSize);
        strTabSize += symbol.name_.size() + 1;
    }
}

uint32_t KernelSymbolInfo::GetSymbolData(std::vector<uint8_t> &buf)
{
    buf.clear();
    buf.reserve(DEFAULT_BUFF_SIZE);

    std::ifstream file(KALLSYMS_PATH);
    std::string line;
    std::getline(file, line);
    SymbolItem prevSymbol;
    GetSymbol(line, prevSymbol);
    uint64_t vaddrStart = prevSymbol.value_;

    std::vector<SymbolItem> symbolItems;
    uint32_t strTabLen = 0;
    SymbolItem currSymbol;
    while (std::getline(file, line)) {
        GetSymbol(line, currSymbol);
        if (prevSymbol.value_ >= currSymbol.value_) {
            continue; // duplicated or wrong
        }
        if (prevSymbol.type_ == 'T') {
            prevSymbol.size_ = currSymbol.value_ - prevSymbol.value_;
            symbolItems.emplace_back(prevSymbol);
            strTabLen += prevSymbol.name_.size() + 1;
        }
        prevSymbol = currSymbol;
    }
    file.close();
    if (prevSymbol.type_ == 'T') {
        symbolItems.emplace_back(prevSymbol);
        strTabLen += prevSymbol.name_.size() + 1;
    }
    if (symbolItems.empty()) {
        return 0;
    }

    uint64_t vaddrEnd = prevSymbol.value_ + prevSymbol.size_;
    GetBinary(symbolItems, vaddrStart, vaddrEnd, strTabLen, buf);
    return buf.size();
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS