/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <inttypes.h>
#include <signal.h>
#include <sys/file.h>
#include <unistd.h>

#include <functional>
#include <map>
#include <sstream>
#include <string>

#include "common_types.pb.h"
#include "hisysevent_plugin_result.pb.h"
#include "trace_file_reader.h"


namespace {
using UsageHandle = std::function<void(void)>;
static std::map<std::string, std::string> params;

int inline parseArgs(int argc, char** argv, UsageHandle usage)
{
    params.clear();

    for (int i = 1; i < argc;) {
        std::string key = argv[i];
        i++;
        if (i >= argc) {
            if (usage) usage();
            break;
        }
        std::string val = argv[i];
        i++;
        params.insert(std::make_pair(key, val));
    }

    return params.size();
}

int inline getStringArg(const char* name, std::string& val, const char* init_val)
{
    val = params[name];
    if (val.empty()) {
        val = init_val;
        return 0;
    }
    return 1;
}
} // namespace

void usage()
{
    printf("usage: xpowertool.exe [-f filepath] \n");
}
int main(int argc, char* argv[])
{
    std::string filePath;
    int ret = parseArgs(argc, argv, usage);
    if (ret == 0) {
        std::cout << "parse parameters error!" << std::endl;
        return 0;
    }
    getStringArg("-f", filePath, "/data/local/tmp/hiprofiler_data.htrace");
    auto reader = std::make_shared<TraceFileReader>();
    if (!reader->Open(filePath)) {
        std::cout << "open file :" << filePath << "failed!" << std::endl;
        return 0;
    }
    long bytes = 0;
    do {
        ProfilerPluginData data{};
        bytes = reader->Read(data);
        std::cout << "name = " << data.name() << ",status=" << data.status() << ",tv_sec=" << data.tv_sec()
                  << ",tv_nsec" << data.tv_nsec() << ",version=" << data.version() << std::endl;
        HisyseventInfo eventInfo;
        if (!eventInfo.ParseFromArray(data.data().data(), data.data().size())) {
            std::cout << "parse profiler plugin data failed!" << std::endl;
            continue;
        }
        for (int i = 0; i < eventInfo.info().size(); i++) {
            std::cout << "context:" << eventInfo.info(i).context() << std::endl;
        }
    } while (bytes > 0);
    return 0;
}
