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
 *
 * Description: KernelSymbolsParser class define
 */
#ifndef KERNEL_SYMBOLS_PARSER_H
#define KERNEL_SYMBOLS_PARSER_H

#include <functional>
#include <string>
#include <vector> // for std::vector
#include "ftrace_namespace.h"

FTRACE_NS_BEGIN

struct KernelSymbol {
    uint64_t addr = 0;
    char type = 0;
    std::string name;
};

class KernelSymbolsParser {
public:
    KernelSymbolsParser();
    ~KernelSymbolsParser();

    bool Parse(const std::string& kallsyms);
    void Accept(const std::function<void(const KernelSymbol&)>& visitor);

private:
    DISALLOW_COPY_AND_MOVE(KernelSymbolsParser);
    static bool IsValidTextSymbol(const KernelSymbol& a);
    static bool CompareSymbolInfo(const KernelSymbol& a, const KernelSymbol& b);

private:
    std::vector<KernelSymbol> kernelSymbols_;
};
FTRACE_NS_END

#endif // KERNEL_SYMBOLS_PARSER_H
