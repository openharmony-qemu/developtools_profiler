/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/file.h>
#include <unistd.h>
#include "common_types.pb.h"
#include "trace_file_reader.h"
#include "trace_file_header.h"
#include "native_hook_result_standard.pb.h"
#include "native_hook_config_standard.pb.h"
#include "google/protobuf/text_format.h"

namespace {
using UsageHandle = std::function<void(void)>;
static std::map<std::string, std::string> params;

int ParseArgs(int argc, char** argv, UsageHandle usage)
{
    params.clear();
    for (int i = 1; i < argc;) {
        std::string key = argv[i];
        i++;
        if (i >= argc) {
            if (usage) {
                usage();
            }
            break;
        }
        std::string val = argv[i];
        i++;
        params.insert(std::make_pair(key, val));
    }
    return params.size();
}

int GetStringArg(const char* name, std::string& val, const char* defaultVal)
{
    val = params[std::string(name)];
    if (val.empty()) {
        val = defaultVal;
    }
    return val.size();
}

void Usage()
{
    printf("usage: hookdecoder [-f filepath] \n");
}

} // namespace


int main(int argc, char* argv[])
{
    std::string filePath;
    int ret = ParseArgs(argc, argv, Usage);
    if (ret == 0) {
        std::cout << "parse parameters error!" << std::endl;
        return 0;
    }
    GetStringArg("-f", filePath, "/data/local/tmp/hiprofiler_data.htrace");
    auto reader = std::make_shared<TraceFileReader>();
    if (!reader->Open(filePath)) {
        std::cout << "open file :" << filePath << "failed!" << std::endl;
        return 0;
    }
    long bytes = 0;
    do {
        ProfilerPluginData data{};
        bytes = reader->Read(data);
        if (data.data().size() <= 0) {
            continue;
        }
        std::cout << "name=" << data.name() << ",status=" << data.status() << ",tv_sec=" << data.tv_sec()
                  << ",tv_nsec=" << data.tv_nsec() << ",version=" << data.version() << std::endl;
        std::string str;
        ForStandard::BatchNativeHookData StandardStackData;
        if (!StandardStackData.ParseFromArray(data.data().data(), data.data().size())) {
            std::cout << "parse profiler plugin data failed!" << std::endl;
            continue;
        }
        google::protobuf::TextFormat::PrintToString(StandardStackData, &str);
        std::cout << str << std::endl;
    } while (bytes > 0);
    return 0;
}