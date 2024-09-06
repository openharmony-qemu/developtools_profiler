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
#include "include/sp_utils.h"
#include "include/sp_log.h"
#include "include/common.h"
#include "parameters.h"


static std::string GetOptions(const std::vector<std::string> &argv)
{
    std::string str = "";
    std::string strFlag;
    bool isFill = false;
    for (std::size_t i = 0; i < argv.size(); i++) {
        if (!isFill) {
            strFlag = argv[i];
            if (strFlag.find("SP_daemon") != std::string::npos) {
                isFill = true;
            }
        } else {
            str += argv[i];
            if (i + 1 != argv.size()) {
                str += " ";
            }
        }
    }
    return str;
}

static bool GCheckCmdParam(std::vector<std::string> &argv, std::string &errorInfo)
{
    std::string str = GetOptions(argv);
    std::set<std::string> keys; // Includes three parts "SP_daemon" CommandType and CommandHelp
    if (str.empty()) {
        return true;
    }
    // 'help' and 'version' start with "--" and are processed separately
    if (str.find("--help") != std::string::npos || str.find("--version") != std::string::npos) {
        std::vector<std::string> out;
        OHOS::SmartPerf::SPUtils::StrSplit(str, "-", out);
        if (out.size() != 1) {
            errorInfo = "--help and --version cannot be used together with other options";
            return false;
        } else {
            return true;
        }
    }
    keys.insert("editor");
    keys.insert("profilerfps");
    keys.insert("start");
    keys.insert("stop");
    keys.insert("screen");
    keys.insert("clear");
    keys.insert("server");
    keys.insert("sections");
    keys.insert("deviceinfo");
    keys.insert("ohtestfps");
    for (auto a : OHOS::SmartPerf::COMMAND_MAP) {
        keys.insert(a.first.substr(1)); // No prefix required '-'
    }

    /* ************The command line for the following parameters is not implemented****************** */
    auto itr = keys.find("f1");
    if (keys.end() != itr) {
        keys.erase(itr);
    }
    itr = keys.find("f2");
    if (keys.end() != itr) {
        keys.erase(itr);
    }
    itr = keys.find("fl");
    if (keys.end() != itr) {
        keys.erase(itr);
    }
    itr = keys.find("ftl");
    if (keys.end() != itr) {
        keys.erase(itr);
    }
    return OHOS::SmartPerf::SPUtils::VeriyParameter(keys, str, errorInfo);
}

static void SocketStopCommand()
{
    OHOS::SmartPerf::ClientControl cc;
    cc.SocketStop();
}

int main(int argc, char *argv[])
{
    if (!OHOS::system::GetBoolParameter("const.security.developermode.state", true)) {
        std::cout << "Not a development mode state" << std::endl;
        return 0;
    }
    if (argc < 0) {
        std::cout << "Invalid argument count" << std::endl;
        return -1;
    }
    std::string errorInfo;
    std::vector<std::string> vec;

    for (int i = 0; i < argc; i++) {
        vec.push_back(argv[i]);
    }
    if (!GCheckCmdParam(vec, errorInfo)) {
        std::cout << "SP_daemon:" << errorInfo << std::endl <<
             "Usage: SP_daemon [options] [arguments]" << std::endl << std::endl <<
             "Try `SP_daemon --help' for more options." << std::endl;
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "-editor") == 0) {
        OHOS::SmartPerf::EditorCommand(argc, vec);
        return 0;
    } else if (argc > 1 && strcmp(argv[1], "-profilerfps") == 0) {
        OHOS::SmartPerf::ProfilerFPS::GetInstance().GetFPS(vec);
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
        SocketStopCommand();
    } else if (argc > 1 && strcmp(argv[1], "-deviceinfo") == 0) {
        std::cout << OHOS::SmartPerf::SPUtils::GetDeviceInfoMap() << std::endl;
        return 0;
    } else if (argc > 1 && strcmp(argv[1], "-ohtestfps") == 0) {
        OHOS::SmartPerf::ProfilerFPS::GetInstance().GetOhFps(vec);
        return 0;
    }
    OHOS::SmartPerf::SmartPerfCommand cmd(vec);
    std::cout << cmd.ExecCommand() << std::endl;
    return 0;
}
