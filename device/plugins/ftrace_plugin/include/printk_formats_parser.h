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
#ifndef PRINTK_FORMAT_PARSER_H
#define PRINTK_FORMAT_PARSER_H

#include <string>
#include <unordered_map> // for std::unordered_map
#include "ftrace_namespace.h"

FTRACE_NS_BEGIN
class PrintkFormatsParser {
public:
    static PrintkFormatsParser& GetInstance();

    std::string GetSymbol(uint64_t addr);

    bool Parse(const std::string& printkFormats);

private:
    PrintkFormatsParser();
    ~PrintkFormatsParser();
    DISALLOW_COPY_AND_MOVE(PrintkFormatsParser);
    std::unordered_map<uint64_t, std::string> printkFormats_ = {};
};
FTRACE_NS_END

#endif // PRINTK_FORMAT_PARSER_H
