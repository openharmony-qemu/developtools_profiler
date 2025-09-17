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

#ifndef CPU_DATA_PLUGIN_H
#define CPU_DATA_PLUGIN_H

#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <map>
#include "cpu_plugin_config.pb.h"
#include "cpu_plugin_result.pb.h"
#include "logging.h"
#include "plugin_module_api.h"

enum ErrorType {
    RET_NULL_ADDR,
    RET_IVALID_PID,
    RET_TGID_VALUE_NULL,
    RET_FAIL = -1,
    RET_SUCC = 0,
};

enum ProcessCpuTimeType {
    PROCESS_UTIME = 0,
    PROCESS_STIME,
    PROCESS_CUTIME,
    PROCESS_CSTIME,
    PROCESS_UNSPECIFIED,
};

enum SystemCpuTimeType {
    SYSTEM_USER = 1,
    SYSTEM_NICE,
    SYSTEM_SYSTEM,
    SYSTEM_IDLE,
    SYSTEM_IOWAIT,
    SYSTEM_IRQ,
    SYSTEM_SOFTIRQ,
    SYSTEM_STEAL,
    SYSTEM_UNSPECIFIED,
};

struct CpuTimeData {
    int64_t userModeUsageTime;
    int64_t systemModeUsageTime;
    int64_t systemUsageTime;
    int64_t systemBootTime;
};

struct CpuLoadData {
    double userLoad;
    double sysLoad;
    double totalLoad;
};

class CpuDataPlugin {
public:
    CpuDataPlugin();
    ~CpuDataPlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Report(uint8_t* configData, uint32_t configSize);
    int ReportOptimize(RandomWriteCtx* randomWrite);
    int Stop();

private:
    int32_t ReadFile(std::string& fileName);

    template <typename T> void SetTimestamp(T& sampleTimeStamp);

    int64_t GetUserHz();
    int64_t GetCpuUsageTime(std::vector<std::string>& cpuUsageVec);

    template <typename T> void WriteProcessCpuUsage(T& cpuUsageInfo, const char* pFile, uint32_t fileLen);

    bool GetSystemCpuTime(std::vector<std::string>& cpuUsageVec, CpuTimeData& cpuTimeData);

    template <typename T> void WriteSystemCpuUsage(T& cpuUsageInfo, CpuLoadData& cpuData,
                                                   const char* pFile, uint32_t fileLen);

    template <typename T, typename I> void WriteCpuUsageInfo(T& cpuData, I cpuUsageInfo);

    bool addTidBySort(int32_t tid);
    DIR* OpenDestDir(std::string& dirPath);
    int32_t GetValidTid(DIR* dirp);
    ThreadState GetThreadState(const char threadState);
    std::string GetCmdArgs(const CpuConfig& protoConfig);

    template <typename T> void WriteThread(T& threadInfo, const char* pFile, uint32_t fileLen, int32_t tid);

    template <typename T> void WriteSingleThreadInfo(T& cpuData, int32_t tid);

    template <typename T> void WriteThreadInfo(T& cpuData);

    int32_t GetCpuFrequency(std::string fileName);
    int GetCpuCoreSize();
    int32_t GetMaxCpuFrequencyIndex();

    template <typename T> void SetCpuFrequency(T& cpuCoreUsageInfo, int32_t coreNum);

    template <typename T> bool WriteProcnum(T& cpuData);

    // for UT
    void SetPath(std::string path)
    {
        path_ = path;
    }
    void SetFreqPath(std::string path);
    template <typename T>
    bool SetSystemCpuUsage(T& cpuUsageInfo, CpuLoadData& cpuLoadData, CpuTimeData& cpuTimeData);
    template <typename T>
    bool SetCpuCoreInfo(const std::string& cpuUsageStr, T& cpuUsageInfo, CpuTimeData& cpuTimeData);

private:
    /* data */
    CpuConfig protoConfig_;
    void* buffer_;
    std::string path_;
    int32_t err_;

    int pid_;
    std::vector<int32_t> tidVec_;
    int64_t prevProcessCpuTime_;
    CpuTimeData prevCpuTimeData_;
    std::map<int32_t, int64_t> prevThreadCpuTimeMap_;
    std::map<int32_t, int64_t> prevCoreSystemCpuTimeMap_;
    std::map<int32_t, int64_t> prevCoreSystemBootTimeMap_;
    std::vector<int32_t> maxFrequencyVec_;
    std::vector<int32_t> minFrequencyVec_;
    int32_t maxFreqIndex_;
    std::string freqPath_;
};

#endif
