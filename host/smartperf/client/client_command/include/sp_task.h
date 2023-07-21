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
#ifndef SP_TASK_H
#define SP_TASK_H
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <map>
#include <mutex>
#include "sp_csv_util.h"
namespace OHOS {
namespace SmartPerf {
enum class ExceptionMsg {
    NO_ERR,
    SESSION_ID_NULL,
    TASK_CONFIG_NULL,
    PACKAGE_NULL,
};

const std::map<ExceptionMsg, std::string> ExceptionMsgMap = {
    {ExceptionMsg::NO_ERR, "NoErr"},
    {ExceptionMsg::SESSION_ID_NULL, "SessionIdNull"},
    {ExceptionMsg::TASK_CONFIG_NULL, "TaskConfigNull"},
    {ExceptionMsg::PACKAGE_NULL, "PackageNull"},
};

enum class ErrCode {
    OK,
    FAILED,
};

struct TaskInfo {
    std::string sessionId;
    std::string packageName;
    std::vector<std::string> taskConfig = {};
    long long freq;
};

class SPTask {
public:
    static SPTask &GetInstance()
    {
        static SPTask instance;
        return instance;
    }
    ErrCode InitTask(std::string recvStr);
    ErrCode StartTask(std::function<void(std::string data)> msgTask);
    void StopTask();
private:
    TaskInfo curTaskInfo;
    long long startTime;
    std::thread thread;
    std::vector<SPData> vmap;
    bool isRunning = false;
    bool isInit = false;
    bool isInitNet = false;
    std::mutex mtx;
    const std::string baseOutPath = "/data/local/tmp/smartperf";
};
}
}


#endif