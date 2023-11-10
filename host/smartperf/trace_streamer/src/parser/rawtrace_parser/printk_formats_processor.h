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
#ifndef PRINTK_FORMAT_PROCESSOR_H
#define PRINTK_FORMAT_PROCESSOR_H

#include <string>
#include <unordered_map>

namespace SysTuning {
namespace TraceStreamer {
class PrintkFormatsProcessor {
public:
    static PrintkFormatsProcessor& GetInstance();
    std::string GetSymbol(uint64_t addr);
    bool HandlePrintkSyms(const std::string& printkFormats);
    void Clear();

private:
    PrintkFormatsProcessor();
    ~PrintkFormatsProcessor();
    std::unordered_map<uint64_t, std::string> printkFormatsDict_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // PRINTK_FORMAT_PROCESSOR_H
