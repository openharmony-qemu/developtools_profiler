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

#ifndef HIEBPF_KERNEL_SYMBOL_INFO_H_
#define HIEBPF_KERNEL_SYMBOL_INFO_H_

#include <string>
#include <vector>

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
const std::string KALLSYMS_PATH = "/proc/kallsyms";
constexpr uint32_t DEFAULT_BUFF_SIZE = 4 * 1024 * 1024;

class KernelSymbolInfo {
public:
    KernelSymbolInfo() = delete;
    ~KernelSymbolInfo() = delete;

    static uint32_t GetSymbolData(std::vector<uint8_t> &buf);

private:
    struct SymbolItem {
        uint64_t value_ = 0;
        uint32_t size_ = 0;
        std::string name_ = "";
        char type_ = 0;
    };

    static void GetSymbol(const std::string &line, SymbolItem &symbol);
    static void GetBinary(const std::vector<SymbolItem> &symbolItems,
                             uint64_t vaddrStart,
                             uint64_t vaddrEnd,
                             uint32_t strTabLen,
                             std::vector<uint8_t> &buf);
};
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
#endif // HIEBPF_KERNEL_SYMBOL_INFO_H_