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
#include "io_stats.h"
#include "securec.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace {
const int NUM_TWO = 2;
const int NUM_ONEHUNDRED = 100;
const double NUM_ZERO_POINTZEROONE = 0.01;
#if __DEBUG__
const char* SYSTIME_PATH = "/data/local/tmp/systimes";
const char* CPU_PATH = "/data/local/tmp/cpustats";
const char* DISKSTATS_PATH = "/data/local/tmp/diskstats";
const int NUM_SEVEN = 7;
#else
const char* SYSTIME_PATH = "/proc/uptime";
const char* CPU_PATH = "/proc/stat";
const char* DISKSTATS_PATH = "/proc/diskstats";
#endif // #if __DEBUG__
const int DEC_BASE = 10;
} // namespace

IoStats::IoStats(DiskioConfig::IoReportType type) : type_(type)
{
    sysTime_ = GetSystime();
}

uint64_t IoStats::GetSystime()
{
    uint64_t systime = 1;
    std::ifstream input(SYSTIME_PATH, std::ios::in);
    CHECK_TRUE(!input.fail(), systime, "%s:open %s failed, errno = %d", __func__, SYSTIME_PATH, errno);
    do {
        if (!input.good()) {
            return systime;
        }
        std::string line;
        getline(input, line);
        line += '\n';

        uint64_t nsec, ncent;
        if (ParseLineFields(line) > 0) {
            nsec = fields_[0];
            ncent = fields_[1];
            systime = nsec * NUM_ONEHUNDRED + ncent;
        }
        fields_.clear();
    } while (0);
    input.close();

    return systime;
}

bool IoStats::GetIoData()
{
    ParseCpuStats();
    if (type_ == DiskioConfig::IO_REPORT) {
        ParseIoStats();
    } else if (type_ == DiskioConfig::IO_REPORT_EX) {
        ParseIoStatsEx();
    }
    return true;
}

bool IoStats::ParseCpuStats()
{
    std::ifstream input(CPU_PATH, std::ios::in);
    CHECK_TRUE(!input.fail(), false, "%s: open %s failed, errno = %d", __func__, CPU_PATH, errno);
    do {
        if (!input.good()) {
            return false;
        }
        std::string line;
        getline(input, line);

        auto pos = line.find("cpu");
        if (pos != std::string::npos) {
            line += '\n';
            GetCpuStats(line);
        }
    } while (!input.eof());
    input.close();

    return true;
}

bool IoStats::GetCpuStats(std::string& line)
{
    std::string name;
    auto cpuData = std::make_shared<ProcStats>();
    CHECK_NOTNULL(cpuData, false, "create ProcStats FAILED!");

    if (ParseLineFields(line, name) > 0) {
        int index = 0;
        cpuData->name_ = name;
        cpuData->user_ = fields_[index];
        index++;
        cpuData->nice_ = fields_[index];
        index++;
        cpuData->system_ = fields_[index];
        index++;
        cpuData->idle_ = fields_[index];
        index++;
        cpuData->iowait_ = fields_[index];
        index++;
        cpuData->steal_ = fields_[index];
        index++;
        cpuData->hardirq_ = fields_[index];
        index++;
        cpuData->softirq_ = fields_[index];
        index++;
        cpuData->guest_ = fields_[index];
        index++;
        cpuData->guestNice_ = fields_[index];
        cpuDatas_.push_back(cpuData);
        fields_.clear();
        return true;
    }

    return false;
}

bool IoStats::ParseIoStats()
{
    std::ifstream input(DISKSTATS_PATH, std::ios::in);
    CHECK_TRUE(!input.fail(), false, "%s:%d open failed, errno = %d", __func__, __LINE__, errno);
    do {
        if (!input.good()) {
            return false;
        }
        std::string line;
        getline(input, line);
        line += '\n';
        GetIoStats(line);
    } while (!input.eof());
    input.close();

    return true;
}

bool IoStats::GetIoStats(std::string& line)
{
    std::string name;
    auto ioInfo = std::make_shared<DiskStats>();
    CHECK_NOTNULL(ioInfo, false, "create DiskStats FAILED!");

    if (ParseLineFields(line, name) > 0) {
        int index = 0;
        ioInfo->major_ = fields_[index];
        index++;
        ioInfo->minor_ = fields_[index];
        index++;
        ioInfo->deviceName_ = name;

        ioInfo->rSucc_ = fields_[index];
        index++;
        ioInfo->rMerged_  = fields_[index];
        index++;
        ioInfo->rSectors_ = fields_[index];
        index++;
        ioInfo->timeOfRead_ = fields_[index];
        index++;

        ioInfo->wSucc_ = fields_[index];
        index++;
        ioInfo->wMerged_  = fields_[index];
        index++;
        ioInfo->wSectors_ = fields_[index];
        index++;
        ioInfo->timeOfWrite_ = fields_[index];
        index++;

        ioInfo->ios_ = fields_[index];
        index++;
        ioInfo->timeOfIo_ = fields_[index];
        index++;
        ioInfo->weighted_ = fields_[index];
        index++;

        ioInfo->dSucc_ = fields_[index];
        index++;
        ioInfo->dMerged_ = fields_[index];
        index++;
        ioInfo->dSectors_ = fields_[index];
        index++;
        ioInfo->timeOfd_ = fields_[index];
        index++;

        ioInfo->flushSucc_ = fields_[index];
        index++;
        ioInfo->timeOfFlush_ = fields_[index];

        ioDatas_.push_back(ioInfo);
        fields_.clear();
        return true;
    }
#if __DEBUG__
    char debugName[128];
    uint64_t rMergesOrIo = 0;
    uint64_t rwIos = 0;
    uint64_t rTicksOrw = 0;
    auto debugIoInfo = std::make_shared<DiskStats>();
    int ret = sscanf_s(line.c_str(), "%u %u %s %lu %lu %lu %" PRIu64 " %lu %lu %lu %u %u %u %u %lu %lu %lu %u %lu %u",
                       &debugIoInfo->major_, &debugIoInfo->minor_, debugName, sizeof(debugName),
                       &debugIoInfo->rSucc_, &rMergesOrIo, &rwIos, &rTicksOrw,
                       &debugIoInfo->wSucc_, &debugIoInfo->wMerged_,
                       &debugIoInfo->wSectors_, &debugIoInfo->timeOfWrite_,
                       &debugIoInfo->ios_, &debugIoInfo->timeOfIo_, &debugIoInfo->weighted_,
                       &debugIoInfo->dSucc_, &debugIoInfo->dMerged_,
                       &debugIoInfo->dSectors_, &debugIoInfo->timeOfd_,
                       &debugIoInfo->flushSucc_, &debugIoInfo->timeOfFlush_);
    if (ret == NUM_SEVEN) {
        debugIoInfo->rSectors_ = rMergesOrIo;
        debugIoInfo->wSucc_ = rwIos;
        debugIoInfo->wSectors_ = rTicksOrw;
    } else {
        debugIoInfo->rMerged_  = rMergesOrIo;
        debugIoInfo->rSectors_ = rwIos;
        debugIoInfo->timeOfRead_ = rTicksOrw;
    }
    debugIoInfo->deviceName_ = std::string(name);
    ioDatas_.push_back(debugIoInfo);
#endif

    return false;
}

bool IoStats::ParseIoStatsEx()
{
    return true;
}

bool IoStats::PutPluginStatsData(StatsData* pluginStats)
{
    PutCpuStatsData(pluginStats);
    if (type_ == DiskioConfig::IO_REPORT) {
        PutIoStatsData(pluginStats);
    } else if (type_ == DiskioConfig::IO_REPORT_EX) {
        ParseIoStatsEx();
    }
    return true;
}

uint32_t IoStats::PutCpuStatsData(StatsData* pluginStats)
{
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_TRUE(!cpuDatas_.empty(), 0, "cpuDatas_ is empty");

    uint32_t count = 0;
    while (cpuDatas_.size() > 0) {
        auto cpuData = cpuDatas_.front();
        auto* cpuInfo = pluginStats->add_cpuinfo();
        CalcCpuStats(cpuData, cpuInfo);
        cpuDatas_.pop_front();
        count++;
    }
    lock.unlock();
    return count;
}

void IoStats::CalcCpuStats(const CpuDatasPtr& cpuData, CpuStats* cpuInfo)
{
    auto totalTime = cpuData->GetTotalTime();
    cpuInfo->set_name(cpuData->name_);
    cpuInfo->set_cpu_user(KeepTowDigits(cpuData->user_, totalTime));
    cpuInfo->set_cpu_nice(KeepTowDigits(cpuData->nice_, totalTime));
    cpuInfo->set_cpu_iowait(KeepTowDigits(cpuData->iowait_, totalTime));
    cpuInfo->set_cpu_steal(KeepTowDigits(cpuData->steal_, totalTime));

    cpuInfo->set_cpu_sys(KeepTowDigits(cpuData->system_ + cpuData->softirq_ + cpuData->hardirq_, totalTime));
    cpuInfo->set_cpu_idle(KeepTowDigits(cpuData->idle_, totalTime));
}

double IoStats::KeepTowDigits(const uint64_t& data, uint64_t div)
{
    double result = 0.00;
    if (data <= 0 || div == 0) {
        return result;
    }
    double ddiv = div;
    if (ddiv != NUM_TWO) {
        ddiv = div * NUM_ZERO_POINTZEROONE;
    }
    result = static_cast<double>(data) / ddiv;
    return result;
}

uint32_t IoStats::PutIoStatsData(StatsData* pluginStats)
{
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_TRUE(!ioDatas_.empty(), 0, "ioDatas_ is empty");

    uint32_t count = 0;
    while (ioDatas_.size() > 0) {
        auto ioData = ioDatas_.front();
        auto* ioInfo = pluginStats->add_statsinfo();
        CalcIoStats(ioData, ioInfo);
        ioDatas_.pop_front();
        count++;
    }
    lock.unlock();
    return count;
}

void IoStats::CalcIoStats(const DiskDatasPtr& ioData, IoStatData* ioInfo)
{
    ioInfo->set_name(ioData->deviceName_);
    // (成功完成读的总次数 + 写 + 丢弃) / sysTime_
    ioInfo->set_ios_per_sec(KeepTowDigits(ioData->rSucc_ + ioData->wSucc_ + ioData->dSucc_, sysTime_));

    // 读扇区的次数 / sysTime_
    ioInfo->set_rd_per_sec(KeepTowDigits(KeepTowDigits(ioData->rSectors_, sysTime_), NUM_TWO));
    ioInfo->set_wr_per_sec(KeepTowDigits(KeepTowDigits(ioData->wSectors_, sysTime_), NUM_TWO));
    ioInfo->set_dc_per_sec(KeepTowDigits(KeepTowDigits(ioData->dSectors_, sysTime_), NUM_TWO));

    // 读扇区的次数
    ioInfo->set_rd_kb(KeepTowDigits(ioData->rSectors_, NUM_TWO));
    ioInfo->set_wr_kb(KeepTowDigits(ioData->wSectors_, NUM_TWO));
    ioInfo->set_dc_kb(KeepTowDigits(ioData->dSectors_, NUM_TWO));
}

bool IoStats::FindFirstNum(char** p)
{
    CHECK_NOTNULL(*p, false, "IoStats:%s", __func__);
    while (**p > '9' || **p < '0') {
        if (**p == '\0' || **p == '\n') {
            return false;
        }
        (*p)++;
    }
    return true;
}

bool IoStats::RemoveSpaces(char** p)
{
    CHECK_NOTNULL(*p, false, "IoStats:%s", __func__);
    if (**p == '\0' || **p == '\n') {
        return false;
    }
    while (**p == ' ') {
        (*p)++;
        if (**p == '\0' || **p == '\n') {
            return false;
        }
    }
    return true;
}

uint32_t IoStats::ParseLineFields(const std::string& line, std::string& name)
{
    uint64_t num;
    uint32_t count = 0;
    char* end = nullptr;
    char* pTmp = const_cast<char*>(line.c_str());

    fields_.clear();
    while (pTmp != nullptr && *pTmp != '\n') {
        CHECK_TRUE(RemoveSpaces(&pTmp), count, "%s: RemoveSpaces failed!", __func__);
        if (*pTmp >= 'a' && *pTmp <= 'z') {
            char field[64];
            int len = 0;
            int ret = sscanf_s(pTmp, "%63s %n", field, sizeof(field), &len);
            if (ret == 1 && *field) {
                name = std::string(field, strlen(field));
                pTmp += len;
            }
        }
        CHECK_TRUE(FindFirstNum(&pTmp), count, "%s: FindFirstNum failed", __func__);
        num = strtoull(pTmp, &end, DEC_BASE);
        CHECK_TRUE(num >= 0, count, "%s:strtoull failed", __func__);
        fields_.push_back(num);
        pTmp = end;
        count++;
    }
    return count;
}

uint32_t IoStats::ParseLineFields(const std::string& line)
{
    uint64_t num;
    uint32_t count = 0;
    char* end = nullptr;
    char* pTmp = const_cast<char*>(line.c_str());

    while (pTmp != nullptr && *pTmp != '\n') {
        CHECK_TRUE(FindFirstNum(&pTmp), count, "%s: FindFirstNum failed", __func__);
        num = static_cast<uint32_t>(strtoull(pTmp, &end, DEC_BASE));
        CHECK_TRUE(num >= 0, count, "%s:strtoull failed", __func__);
        fields_.push_back(num);
        pTmp = end;
        count++;
    }
    return count;
}