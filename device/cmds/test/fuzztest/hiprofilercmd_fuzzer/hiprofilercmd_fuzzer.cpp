/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "hiprofilercmd_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <string>


namespace OHOS {
const std::string DEFAULT_HIPROFILER_CMD_PATH("/system/bin/hiprofiler_cmd");
constexpr uint32_t READ_BUFFER_SIZE = 1024;
bool RunCommand(const std::string& cmd, std::string& content)
{
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    std::array<char, READ_BUFFER_SIZE> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        content += buffer.data();
    }
    return true;
}

std::string GenerateRandomString(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return "";
    }
    FuzzedDataProvider provider(data, size);
    return provider.ConsumeRandomLengthString();
}

bool HiprofilerCmdFuzzTest(const uint8_t* data, size_t size)
{
    std::string configPara = GenerateRandomString(data, size);
    std::string outPara = GenerateRandomString(data, size);
    std::string cmd = DEFAULT_HIPROFILER_CMD_PATH + " -c " + configPara + " -o " + outPara + " -s";
    std::string content = "";
    RunCommand(cmd, content);
    cmd = DEFAULT_HIPROFILER_CMD_PATH + " " + configPara + " -c " + " -o " + outPara + " -k -s";
    RunCommand(cmd, content);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::HiprofilerCmdFuzzTest(data, size);
    return 0;
}
