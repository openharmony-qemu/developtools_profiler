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
    std::map<std::string, std::string> ItemData() override;

    static GpuCounter &GetInstance()
    {
        static GpuCounter instance;
        return instance;
    }

    // -3 sandbox path invalid, -2 package name error -1 dependency file not detected
    int Init(std::string packageName, std::map<std::string, std::string> &retMap);
    void Rest();
    // 0 ok，-1 run failed
    int Start();
    void Check();
    long long GetCounterDuration();

private:
    void GetCounterId(std::vector<std::string> &pidList);
    void KillCounter();
    void ThreadCapture();
    void Capture();

private:
    bool isStop = false;
    long long startCaptureTime = 0;
    long long captureDuration = 0;
    std::vector<std::string> fileList;
    std::string sandBoxPath;

private:
    const long long constDefaultCaptureDuration = 5000; // Unit ms

    const std::string constMvFile = "mv -f ";
    const std::string constKillProcess = "kill -9  ";
    const std::string constGetCounterId = "pidof counters_collector";
    const std::string constRmCsv = "rm /data/local/tmp/gpu_conter.csv";
    const std::string constAddPermissionsCounter = "chmod 777 /data/local/tmp/counters_collector";
    const std::string constCmd =
        "LD_LIBRARY_PATH=/data/local/tmp/ /data/local/tmp/counters_collector /data/local/tmp/config.txt &";

    const std::string constSandBoxFile = "/files/";
    const std::string constSandBoxPath = "/data/app/el2/100/base/";
    const std::string constOutDestCVSPrefix = "gpu_counter";
    const std::string constOutSourCVSFile = "/data/local/tmp/gpu_counter.csv";

    const std::string constConfigFile = "/data/local/tmp/config.txt";
    const std::string constExecFile = "/data/local/tmp/counters_collector";
    const std::string constLibFile = "/data/local/tmp/libGPU_PCM.so";
    const std::string constV2File = "/data/local/tmp/counters_collector_v2.txt";
    const std::string constCheckProductInfo = "param get const.product.name";
    const std::string constProductInfo = "Mate 60 Pro";

private:
    GpuCounter() {};
    GpuCounter(const GpuCounter &);
    GpuCounter &operator = (const GpuCounter &);
};
}
}
#endif // GPU_COUNTER_H
