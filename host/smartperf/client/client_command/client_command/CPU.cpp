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

#include <sstream>
#include <cstdio>
#include <cstring>
#include <climits>
#include "securec.h"
#include "include/sp_utils.h"
#include "include/CPU.h"
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> CPU::ItemData()
{
    std::map<std::string, std::string> result;
    if (mCpuNum < 0) {
        GetCpuNum();
    }
    std::vector<float> workloads = GetCpuLoad();
    std::string cpuLoadsStr;
    std::string cpuFreqStr;
    for (size_t i = 0; i < workloads.size(); i++) {
        cpuLoadsStr = std::to_string(workloads[i]);
        cpuFreqStr = std::to_string(GetCpuFreq(i));
        result["cpu" + std::to_string(i) + "Load"] = cpuLoadsStr;
        result["cpu" + std::to_string(i) + "Frequency"] = cpuFreqStr;
    }
    return result;
}
int CPU::GetCpuNum()
{
    int cpuNum = 0;
    while (true) {
        std::stringstream cpuNodeStr;
        cpuNodeStr << cpuBasePath.c_str() << "/cpu" << cpuNum;
        if (!SPUtils::FileAccess(cpuNodeStr.str())) {
            break;
        }
        ++cpuNum;
    }
    return mCpuNum = cpuNum;
}
int CPU::GetCpuFreq(int cpuId) const
{
    std::string curFreq = "-1";
    SPUtils::LoadFile(CpuScalingCurFreq(cpuId), curFreq);
    return atoi(curFreq.c_str());
}
std::vector<float> CPU::GetCpuLoad()
{
    if (mCpuNum <= 0) {
        std::vector<float> workload;
        return workload;
    }
    std::vector<float> workload;

    static char preBuffer[10][256] = {"\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0"};
    if (!SPUtils::FileAccess(procStat)) {
        return workload;
    }
    char realPath[PATH_MAX] = {0x00};
    if (realpath(procStat.c_str(), realPath) == nullptr) {
        std::cout << "" << std::endl;
    }
    FILE *fp = fopen(realPath, "r");
    if (fp == nullptr) {
        for (int i = 0; i <= mCpuNum; ++i) {
            workload.push_back(-1.0f);
        }
        return workload;
    }
    char buffer[1024];
    buffer[0] = '\0';
    int line = 0;
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        const int zeroPos = 0;
        const int firstPos = 1;
        const int secondPos = 2;
        const int length = 3;
        if (strlen(buffer) >= length && buffer[zeroPos] == 'c' && buffer[firstPos] == 'p' && buffer[secondPos] == 'u' &&
            line != 0) {
            float b = CacWorkload(buffer, preBuffer[line]);
            workload.push_back(b);
            if (snprintf_s(preBuffer[line], sizeof(preBuffer[line]), sizeof(preBuffer[line]), "%s", buffer) < 0) {
                std::cout << "snprintf_s turn fail" << std::endl;
            }
        }
        ++line;

        if (line >= mCpuNum + 1) {
            break;
        }
    }
    if (fclose(fp) == EOF) {
        return workload;
    }

    return workload;
}

float CPU::CacWorkload(const char *buffer, const char *preBuffer) const
{
    const size_t defaultIndex = 4;
    const size_t defaultShift = 10;
    const char defaultStart = '0';
    const char defaultEnd = '9';

    size_t preLen = strlen(preBuffer);
    size_t len = strlen(buffer);
    if (preLen == 0 || len == 0) {
        return -1.0f;
    }
    size_t time[10] = {0};
    size_t preTime[10] = {0};
    size_t cnt = 0;

    for (size_t i = defaultIndex; i < len; ++i) {
        size_t tmp = 0;
        if (buffer[i] < defaultStart || buffer[i] > defaultEnd) {
            continue;
        }
        while (buffer[i] >= defaultStart && buffer[i] <= defaultEnd) {
            tmp = tmp * defaultShift + (buffer[i] - defaultStart);
            i++;
        }
        time[cnt++] = tmp;
    }

    size_t preCnt = 0;
    for (size_t i = defaultIndex; i < preLen; ++i) {
        size_t tmp = 0;
        if (preBuffer[i] < defaultStart || preBuffer[i] > defaultEnd) {
            continue;
        }
        while (preBuffer[i] >= defaultStart && preBuffer[i] <= defaultEnd) {
            tmp = tmp * defaultShift + (preBuffer[i] - defaultStart);
            i++;
        }
        preTime[preCnt++] = tmp;
    }

    size_t user = time[0] + time[1] - preTime[0] - preTime[1];
    size_t sys = time[2] - preTime[2];
    size_t idle = time[3] - preTime[3];
    size_t iowait = time[4] - preTime[4];
    size_t irq = time[5] + time[6] - preTime[5] - preTime[6];
    size_t total = user + sys + idle + iowait + irq;

    double perUser = std::atof(std::to_string(user * 100.0 / total).c_str());
    double perSys = std::atof(std::to_string(sys * 100.0 / total).c_str());
    double periowait = std::atof(std::to_string(iowait * 100.0 / total).c_str());
    double perirq = std::atof(std::to_string(irq * 100.0 / total).c_str());

    double workload = perUser + perSys + periowait + perirq;

    return static_cast<float>(workload);
}
}
}
