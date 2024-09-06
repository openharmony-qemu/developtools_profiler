/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef GPU_COUNTER_H
#define GPU_COUNTER_H
#include "sp_profiler.h"
#include <string>
#include <vector>
#include <cstdio>
#include <list>
#include <cstdlib>
#include <thread>
namespace OHOS {
namespace SmartPerf {
class GpuCounter : public SpProfiler {
public:
    enum GcStatus {
        GC_INIT = 0,
        GC_START,
        GC_STOP,
    };

    enum ExecutePermissions {
        EP_INVALID = -1,
        EP_PERMISSIVE = 0,
        EP_ENFORCING = 1,
    };

public:
    std::map<std::string, std::string> ItemData() override;

    static GpuCounter &GetInstance()
    {
        static GpuCounter instance;
        return instance;
    }

    GcStatus GetStatus() const
    {
        return gcStatus;
    }

    int Init(const std::string &packageName, std::map<std::string, std::string> &retMap);
    void Rest();
    // 0 ok, -1 run failed
    int Start();
    void Check();
    long long GetCounterDuration();

private:
    // 0 succed, -1 dependency file not detected, -2 package name error, -3 non root users, -4 NonHisilicon chips
    int CheckResources(const std::string &packageName, std::string &errorInfo);
    void GetCounterId(std::vector<std::string> &pidList);
    void KillCounter();
    int Capture();
    std::thread ThreadCapture();
    void SetPerm(ExecutePermissions code);
    ExecutePermissions GetPerm();
    bool IsNum(const std::string value);

private:
    bool isSandBoxWrite = false;
    ExecutePermissions originalEP = EP_INVALID;
    GcStatus gcStatus = GC_INIT;
    long long startCaptureTime = 0;
    long long captureDuration = 0;
    std::vector<std::string> fileList;
    std::string sandBoxPath;
    std::string initCheckPath;
    std::map<std::string, std::string> initMap;

private:
    const long long constDefaultCaptureDuration = 5000; // Unit ms
    const std::string constMvFile = "mv -f ";
    const std::string constKillProcess = "kill -9  ";
    const std::string constGetCounterId = "pidof counters_collector";
    const std::string constRmCsv = "rm /data/local/tmp/gpu_counter.csv";
    const std::string constAddPermissionsCounter = "chmod 777 /bin/counters_collector";
    const std::string constCmd =
        "LD_LIBRARY_PATH=/bin/ /bin/counters_collector /bin/config.txt >/dev/null 2>&1";

    const std::string constSandBoxFile = "/files/";
    const std::string constSandBoxPath = "/data/app/el2/100/base/";
    const std::string constOutDestCVSPrefix = "gpu_counter";
    const std::string constOutSourCVSFile = "/data/local/tmp/gpu_counter.csv";

    const std::string constConfigFile = "/bin/config.txt";
    const std::string constExecFile = "/bin/counters_collector";
    const std::string constLibFile = "/bin/libGPU_PCM.so";
    const std::string constV2File = "/bin/counters_collector_v2.txt";
    const std::string constCheckProductInfo = "param get const.product.name";
    const std::string constProductInfo = "60";
    const std::string constWhoami = "whoami";
    const std::string constUserInfo = "root";

private:
    GpuCounter();
    GpuCounter(const GpuCounter &);
    GpuCounter &operator = (const GpuCounter &);
};
}
}
#endif // GPU_COUNTER_H
