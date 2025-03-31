/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#include "native_memory_profiler_sa_client_manager.h"

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sstream>

using namespace OHOS::Developtools::NativeDaemon;
const int ONLY_NMD = 2;
namespace {
static uint32_t TestDumpFile(const std::string postfix = "")
{
    uint32_t fd = static_cast<uint32_t>(open(("/data/local/tmp/test_dump_file" + postfix + ".htrace").c_str(),
                                             O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    return fd;
}

bool IsNumeric(const std::string& str)
{
    std::istringstream iss(str);
    int number;
    char trailingCharacter;
    if (!(iss >> number)) {
        return false;
    }
    if (iss >> trailingCharacter) {
        return false;
    }
    return true;
}
}

int32_t main(int32_t argc, char* argv[])
{
    if (argc > 50) { // 50: max args size
        printf("error too many args.\n");
        return 0;
    }
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    bool start = false;
    bool stop = false;
    bool error = false;
    bool dumpData = false;
    for (int32_t i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--help" || arg == "-h") {
            printf("    --start                             -s : start, default: false\n");
            printf("    --stop                              -k : stop, default: false\n");
            printf("    --pid                               -p : pid\n");
            printf("    --filePath                          -f : filePath, default: ");
            printf("/data/local/tmp/hiprofiler_data.htrace\n");
            printf("    --duration                          -d : duration, default: 20s\n");
            printf("    --filterSize                        -fs : filterSize, default: 0\n");
            printf("    --shareMemorySize                   -sms : shareMemorySize, default: 16384\n");
            printf("    --processName                       -pn : processName\n");
            printf("    --maxStackDepth                     -msd : maxStackDepth, default: 30\n");
            printf("    --mallocDisable                     -mad : mallocDisable, default: false\n");
            printf("    --mmapDisable                       -mmd : mmapDisable, default: false\n");
            printf("    --freeStackData                     -fsd : freeStackData, default: false\n");
            printf("    --munmapStackData                   -musd : munmapStackData, default: false\n");
            printf("    --mallocFreeMatchingInterval        -mfmi : mallocFreeMatchingInterval\n");
            printf("    --mallocFreeMatchingCnt             -mfmc : mallocFreeMatchingCnt\n");
            printf("    --disable_stringCompressed          -sc : disable_stringCompressed, ");
            printf("default: stringCompressed\n");
            printf("    --dwarf                             -df : dwarf unwind, default: fp\n");
            printf("    --disable_blocked                   -b : disable_blocked, default: blocked\n");
            printf("    --disable_recordAccurately          -ra : disable_recordAccurately, ");
            printf("default: recordAccurately\n");
            printf("    --startupMode                       -sm : startupMode, default: false\n");
            printf("    --memtraceEnable                    -me : memtraceEnable, default: false\n");
            printf("    --offlineSymbolization              -os : offlineSymbolization, default: false\n");
            printf("    --callframeCompress                 -cc : callframeCompress, default: false\n");
            printf("    --statisticsInterval                -si : statisticsInterval\n");
            printf("    --clockId                           -c : clockId\n");
            printf("    --dumpData                          -dd : dump data\n");
            printf("    --sampleInterval                    -spi : sampleInterval, default: 0\n");
            printf("    --jsStackReport                     -jr : jsStackReport, default: 0\n");
            printf("    --maxJsStackDepth                   -mjsd : maxJsStackDepth, default: 0\n");
            printf("    --filterNapiName                    -fnapi : filterNapiName \n");
            printf("    --hookstandalone                    -hsa : hookstandalone \n");
            printf("    --save_file                         -sf : save_file \n");
            printf("    ----fileName                        -fn : file_name \n");
            return 0;
        }

        if ((arg == "--start") || (arg == "-s")) {
            start = true;
        } else if ((arg == "--stop") || (arg == "-k")) {
            stop = true;
        } else if ((arg == "--pid") || (arg == "-p")) {
            config->pid_ = i + 1 < argc && IsNumeric(argv[i + 1]) ? std::stoi(argv[++i]) : 0;
        } else if ((arg == "--filePath") || (arg == "-f")) {
            config->filePath_ = i + 1 < argc ? std::string(argv[++i]) : "";
        } else if ((arg == "--duration") || (arg == "-d")) {
            config->duration_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<uint32_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--filterSize") || (arg == "-fs")) {
            config->filterSize_ = i + 1 < argc && IsNumeric(argv[i + 1]) ? std::stoi(argv[++i]) : 0;
        } else if ((arg == "--shareMemorySize") || (arg == "-sms")) {
            config->shareMemorySize_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<uint32_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--processName") || (arg == "-pn")) {
            config->processName_ = i + 1 < argc ? std::string(argv[++i]) : "";
        } else if ((arg == "--maxStackDepth") || (arg == "-msd")) {
            config->maxStackDepth_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<uint8_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--mallocDisable") || (arg == "-mad")) {
            config->mallocDisable_ = true;
        } else if ((arg == "--mmapDisable") || (arg == "-mmd")) {
            config->mmapDisable_ = true;
        } else if ((arg == "--freeStackData") || (arg == "-fsd")) {
            config->freeStackData_ = true;
        } else if ((arg == "--munmapStackData") || (arg == "-musd")) {
            config->munmapStackData_ = true;
        } else if ((arg == "--mallocFreeMatchingInterval") || (arg == "-mfmi")) {
            config->mallocFreeMatchingInterval_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<uint32_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--mallocFreeMatchingCnt") || (arg == "-mfmc")) {
            config->mallocFreeMatchingCnt_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<uint32_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--disable_stringCompressed") || (arg == "-sc")) {
            config->stringCompressed_ = false;
        } else if ((arg == "--dwarf") || (arg == "-df")) {
            config->fpUnwind_ = false;
        } else if ((arg == "--disable_blocked") || (arg == "-b")) {
            config->blocked_ = false;
        } else if ((arg == "--disable_recordAccurately") || (arg == "-ra")) {
            config->recordAccurately_ = false;
        } else if ((arg == "--startupMode") || (arg == "-sm")) {
            config->startupMode_ = true;
        } else if ((arg == "--memtraceEnable") || (arg == "-me")) {
            config->memtraceEnable_ = true;
        } else if ((arg == "--onlineSymbolization") || (arg == "-os")) {
            config->offlineSymbolization_ = false;
        } else if ((arg == "--callframeCompress") || (arg == "-cc")) {
            config->callframeCompress_ = true;
        } else if ((arg == "--statisticsInterval") || (arg == "-si")) {
            config->statisticsInterval_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<uint32_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--clockId") || (arg == "-c")) {
            config->clockId_ = i + 1 < argc && IsNumeric(argv[i + 1]) ? std::stoi(argv[++i]) : 0;
        } else if ((arg == "--dumpData") || (arg == "-dd")) {
            dumpData = true;
        } else if ((arg == "--sampleInterval ") || (arg == "-spi")) {
            config->sampleInterval_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<uint32_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--responseLibraryMode") || (arg == "-r")) {
            config->responseLibraryMode_ = true;
        } else if ((arg == "--printNmd") || (arg == "-nmd")) {
            config->printNmd_ = true;
        } else if ((arg == "--jsStackReport") || (arg == "-jr")) {
            config->jsStackReport_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<int32_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--maxJsStackDepth") || (arg == "-mjsd")) {
            config->maxJsStackDepth_ =
                i + 1 < argc && IsNumeric(argv[i + 1]) ? static_cast<uint8_t>(std::stoi(argv[++i])) : 0;
        } else if ((arg == "--filterNapiName") || (arg == "-fnapi")) {
            config->filterNapiName_ = i + 1 < argc ? std::string(argv[i + 1]) : "";
        } else if ((arg == "--hookstandalone") || (arg == "-hsa")) {
            config->hookstandalone_ = true;
        } else if ((arg == "--save_file") || (arg == "-sf")) {
            config->saveFile_ = true;
        } else if ((arg == "--fileName") || (arg == "-fn")) {
            config->fileName_ = i + 1 < argc ? std::string(argv[++i]) : "";
        } else {
            printf("error arg: %s\n", arg.c_str());
            error = true;
            break;
        }
    }

    if (error) {
        return 0;
    }

    if (start) {
        std::cout << "start....." << std::endl;
        if (config->printNmd_) {
            uint32_t fdFirst = TestDumpFile(std::to_string(0));
            NativeMemoryProfilerSaClientManager::GetMallocStats(fdFirst, config->pid_, ONLY_NMD, true);
            close(fdFirst);
        } else if (dumpData) {
            uint32_t fd = TestDumpFile();
            NativeMemoryProfilerSaClientManager::DumpData(fd, config);
            close(fd);
        } else {
            NativeMemoryProfilerSaClientManager::Start(config);
        }
    } else if (stop) {
        std::cout << "stop....." << std::endl;
        if (config->pid_ > 0) {
            NativeMemoryProfilerSaClientManager::Stop(config->pid_);
        } else {
            NativeMemoryProfilerSaClientManager::Stop(config->processName_);
        }
    } else {
        printf("The start or stop parameter is not configured.\n");
    }
    return 0;
}