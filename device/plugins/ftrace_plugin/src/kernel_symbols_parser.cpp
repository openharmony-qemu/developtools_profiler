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
 * Description: KernelSymbolsParser class implements
 */
#include "kernel_symbols_parser.h"
#include "file_utils.h"
#include "logging.h"
#include "string_utils.h"

#include <algorithm> // for std::sort
#include <sstream>   // for std::stringstream

namespace {
constexpr int ADDR_VALUE_BASE = 16;
constexpr int MAX_BUFFER_SIZE = 10 * 1000;
} // namespace

FTRACE_NS_BEGIN
KernelSymbolsParser::KernelSymbolsParser()
{
    HILOG_INFO(LOG_CORE, "KernelSymbolsParser create!");
}

KernelSymbolsParser::~KernelSymbolsParser()
{
    HILOG_INFO(LOG_CORE, "KernelSymbolsParser destroy!");
}

void KernelSymbolsParser::Accept(const std::function<void(const KernelSymbol&)>& visitor)
{
    std::for_each(kernelSymbols_.begin(), kernelSymbols_.end(), visitor);
}

bool KernelSymbolsParser::IsValidTextSymbol(const KernelSymbol& a)
{
    if (a.addr == 0 || a.name.empty()) {
        return false;
    }
    if (a.name[0] == '$') {
        return false;
    }
    if (a.type != 't' && a.type != 'T') {
        return false;
    }
    return true;
}

bool KernelSymbolsParser::CompareSymbolInfo(const KernelSymbol& a, const KernelSymbol& b)
{
    if (a.addr != b.addr) {
        return a.addr < b.addr;
    }
    if (a.name != b.name) {
        return a.name < b.name;
    }
    return a.type < b.type;
}

bool KernelSymbolsParser::Parse(const std::string& kallsyms)
{
    CHECK_TRUE(!kallsyms.empty(), false, "kallsyms is empty!");
    std::stringstream sin(kallsyms);
    std::string line;
    KernelSymbol info;
    std::string addrStr;
    std::stringstream ss;
    int count = 0;
    while (std::getline(sin, line)) {
        // one of following format:
        // c0109b3c T arm_elf_read_implies_exec
        // bf004fd8 t media_mem_init       [hi_osal]
        ss.clear();
        ss.str(line);
        if (ss >> addrStr >> info.type >> info.name) {
            info.addr = strtoull(addrStr.c_str(), nullptr, ADDR_VALUE_BASE);
        }
        if (info.addr == 0) {
            continue;
        }
        if (StringUtils::EndsWith(info.name, ".cfi")) {
            info.name = info.name.substr(0, info.name.size() - (sizeof(".cfi") - 1));
        }
        if (IsValidTextSymbol(info)) {
            if (count % MAX_BUFFER_SIZE == 0) {
                kernelSymbols_.resize(kernelSymbols_.size() + MAX_BUFFER_SIZE);
            }
            kernelSymbols_[count] = info;
            count++;
        }
    }

    kernelSymbols_.resize(count);
    std::sort(kernelSymbols_.begin(), kernelSymbols_.end(), CompareSymbolInfo);
    return kernelSymbols_.size();
}
FTRACE_NS_END
