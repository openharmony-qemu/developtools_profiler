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
#include <future>
#include <map>
#include <mutex>
#include <climits>
#include "parameters.h"
#include "sp_csv_util.h"
#include "sdk_data_recv.h"
#include "GpuCounter.h"
#include "lock_frequency.h"
namespace OHOS {
namespace SmartPerf {
enum class ExceptionMsg {
    NO_ERR,
    SESSION_ID_NULL,
    TASK_CONFIG_NULL,
    PACKAGE_NULL,
};

const std::map<ExceptionMsg, std::string> EXCEPTION_MSG_MAP = {
    { ExceptionMsg::NO_ERR, "NoErr" },
    { ExceptionMsg::SESSION_ID_NULL, "SessionIdNull" },
    { ExceptionMsg::TASK_CONFIG_NULL, "TaskConfigNull" },
    { ExceptionMsg::PACKAGE_NULL, "PackageNull" },
};

enum class ErrCode {
    OK,
    FAILED,
};
struct StuckNotification {
    bool isEffective = false;
    int fps = 0;
    long long frameTime = LLONG_MAX;
};
struct TaskInfo {
    std::string sessionId = "";
    std::string packageName = "";
    std::vector<std::string> taskConfig = {};
    long long freq = 0;
    StuckNotification stuckInfo;
};

class SPTask {
public:
    static SPTask &GetInstance()
    {
        static SPTask instance;
        return instance;
    }
    ErrCode InitTask(const std::string &recvStr);
    ErrCode StartTask(std::function<void(std::string data)> msgTask);
    ErrCode StopTask();
    std::string GetCurrentTimeAsString();
    std::map<std::string, std::string> DetectionAndGrab();
    bool CheckTcpParam(std::string str, std::string &errorInfo);
    std::future<std::map<std::string, std::string>> AsyncCollectRam();
    std::future<std::map<std::string, std::string>> AsyncCollectFps();
    std::future<std::map<std::string, std::string>> AsyncCollectCpu();
    void CheckFutureRam(std::future<std::map<std::string, std::string>> &ramResult,
        std::map<std::string, std::string> &dataMap);
    void CheckFutureFps(std::future<std::map<std::string, std::string>> &fpsResult,
        std::map<std::string, std::string> &dataMap);
    void CheckFutureCpu(std::future<std::map<std::string, std::string>> &cpuResult,
        std::map<std::string, std::string> &dataMap);
    void GetItemData(std::map<std::string, std::string> &dataMap);
    void GetGpuRealtimeData(std::map<std::string, std::string> &dataMap);
    void CreatPath(std::string path);
    void ConfigDataThread();
    void StopSdkRecv();
    void StopGpuCounterRecv();
    void InitDataFile();
    void AsyncGetDataMap(std::function<void(std::string data)> msgTask);
    void StopGetInfo();
    ErrCode StartRecord();
    ErrCode StopRecord();
    bool GetRecordState();
    void SaveScreenShot();
    time_t GetRealStartTime() const;
    void SetTcpToken(std::string token);
    std::string GetTcpToken();

private:
    std::thread ThreadGetHiperf(long long timeStamp);
    void GetHiperf(const std::string &traceName);
    std::string SetHiperf(const std::string &traceName);
    bool CheckCounterId();
    void KillHiperfCmd();
    void ConfigureSdkData(std::string itConfig);
    void RunSdkServer(SdkDataRecv &sdkDataRecv);
    void ResetSdkParam();
    int GetCurrentBattary();

private:
    TaskInfo curTaskInfo;
    long long startTime = 0;
    std::thread thread;
    std::vector<SPData> vmap;
    bool isRunning = false;
    bool isInit = false;
    std::mutex asyncDataMtx;
    std::mutex sdkDataMtx;
    const std::string baseOutPath = "/data/local/tmp/smartperf";
    long long startCaptuerTime = 0;
    int requestId = 1;
    bool sdkData = false;
    std::thread sdk;
    std::thread lockFreqThread;
    std::vector<std::string> sdkvec;
    GpuCounter &gpuCounter = GpuCounter::GetInstance();
    LockFrequency &lockFreq = LockFrequency::GetInstance();
    bool recordState = false;
    bool screenshotFlag = false;
    bool recordTrace = false;
    time_t realTimeStart = 0;
    std::string tcpToken = "";
    long long nextTime = 0;
    int battaryStart = 0;
    int battaryEnd = 0;

    std::string strOne = R"(hiprofiler_cmd \
  -c - \
  -o /data/local/tmp/)";
    std::string strTwo = R"(.htrace \
  -t 5 \
  -s \
  -k \
<<CONFIG)";

    std::string strThree = R"(request_id: )";
    std::string strFour = R"( session_config {
    buffers {
    pages: 16384
    })";
    std::string strFive = R"( result_file: "/data/local/tmp/)";
    std::string strSix = R"(.htrace"
    sample_duration: 5000
    })";
    std::string strNine = R"( plugin_configs {
  plugin_name: "ftrace-plugin"
  sample_interval: 1000
  config_data {
    ftrace_events: "sched/sched_switch"
    ftrace_events: "power/suspend_resume"
    ftrace_events: "sched/sched_wakeup"
    ftrace_events: "sched/sched_wakeup_new"
    ftrace_events: "sched/sched_waking"
    ftrace_events: "sched/sched_process_exit"
    ftrace_events: "sched/sched_process_free"
    ftrace_events: "task/task_newtask"
    ftrace_events: "task/task_rename"
    ftrace_events: "power/cpu_frequency"
    ftrace_events: "power/cpu_idle"
    hitrace_categories: "ace"
    hitrace_categories: "app"
    hitrace_categories: "ark"
    hitrace_categories: "graphic"
    hitrace_categories: "ohos"
    hitrace_categories: "bin)";
    std::string strEleven = R"(der"
    hitrace_categories: "irq"
    hitrace_categories: "pagecache"
    hitrace_categories: "zaudio"
    buffer_size_kb: 20480
    flush_interval_ms: 1000
    flush_threshold_kb: 4096
    parse_ksyms: true
    clock: "boot"
    trace_period_ms: 200
    debug_on: false
    hitrace_time: 5
    }
    })";
    std::string strSeven = R"( plugin_configs {
  plugin_name: "hiperf-plugin"
  sample_interval: 5000
  config_data {
    is_root: false
   outfile_name: "/data/local/tmp/)";
    std::string strEight = R"(.data"
   record_args: "-f 1000 -a  --cpu-limit 100 -e hw-cpu-cycles,sched:sched_waking )";
    std::string strTen = R"(--call-stack dwarf --clockid monotonic --offcpu -m 256"
    }
    })";
    std::string conFig = R"(CONFIG)";
};
}
}

#endif