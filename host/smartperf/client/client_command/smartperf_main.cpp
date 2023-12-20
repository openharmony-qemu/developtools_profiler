/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <cstdio>
#include <thread>
#include <cstring>
#include "unistd.h"
#include "include/smartperf_command.h"
#include "include/editor_command.h"
#include "include/profiler_fps.h"
#include "include/client_control.h"
int main(int argc, char *argv[])
{
    std::vector<std::string> vec;
    for (int i = 0; i < argc; i++) {
        vec.push_back(argv[i]);
    }
    if (argc > 1 && strcmp(argv[1], "-editor") == 0) {
        OHOS::SmartPerf::EditorCommand(argc, vec);
        return 0;
    } else if (argc > 1 && strcmp(argv[1], "-profilerfps") == 0) {
        OHOS::SmartPerf::ProfilerFPS fps;
        fps.GetFPS(argc, vec);
        return 0;
    } else if (argc > 1 && strcmp(argv[1], "-start") == 0) {
        OHOS::SmartPerf::ClientControl cc;
        cc.StartSPDaemon();
        std::string result;
        for (int i = 2; i < argc; i++) {
            result += argv[i];
            if (i != argc - 1) {
                result += " ";
            }
        }
        cc.SocketStart(result);
    } else if (argc > 1 && strcmp(argv[1], "-stop") == 0) {
        OHOS::SmartPerf::ClientControl cc;
        cc.SocketStop();
    }
    OHOS::SmartPerf::SmartPerfCommand cmd(argc, argv);
    std::cout << cmd.ExecCommand() << std::endl;
    return 0;
}
