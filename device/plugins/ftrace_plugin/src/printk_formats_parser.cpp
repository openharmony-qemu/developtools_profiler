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
#include "printk_formats_parser.h"

#include <sstream>
#include "file_utils.h"
#include "ftrace_fs_ops.h"
#include "logging.h"
#include "string_utils.h"

namespace {
constexpr int HEX_BASE = 16;
}

FTRACE_NS_BEGIN
PrintkFormatsParser& PrintkFormatsParser::GetInstance()
{
    static PrintkFormatsParser instance;
    return instance;
}

PrintkFormatsParser::PrintkFormatsParser()
{
    HILOG_INFO(LOG_CORE, "PrintkFormatsParser create!");
}

PrintkFormatsParser::~PrintkFormatsParser()
{
    HILOG_INFO(LOG_CORE, "PrintkFormatsParser destroy!");
}

std::string PrintkFormatsParser::GetSymbol(uint64_t addr)
{
    auto it = printkFormats_.find(addr);
    if (it != printkFormats_.end()) {
        return it->second;
    }
    return "NULL";
}

bool PrintkFormatsParser::Parse(const std::string& printkFormats)
{
    std::stringstream sin(printkFormats);
    std::string line;
    uint64_t addr = 0;
    char seperator = ':';
    std::string symbol = "";
    while (std::getline(sin, line)) {
        auto pos = line.find(seperator);
        if (pos != std::string::npos) {
            std::string addrStr = StringUtils::Strip(line.substr(0, pos));
            addr = static_cast<uint64_t>(strtoull(addrStr.c_str(), nullptr, HEX_BASE));

            symbol = StringUtils::Strip(line.substr(pos + 1));
            if (symbol.back() == '"') {
                symbol.pop_back();
            }
            if (symbol.front() == '"') {
                symbol = symbol.substr(1);
            }
            printkFormats_[addr] = symbol;
        }
    }
    return printkFormats_.size() > 0;
}
FTRACE_NS_END