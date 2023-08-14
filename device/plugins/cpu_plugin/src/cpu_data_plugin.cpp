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

#include "cpu_data_plugin.h"

#include <ctime>
#include <vector>

#include "buffer_splitter.h"

namespace {
constexpr size_t READ_BUFFER_SIZE = 1024 * 16;
constexpr int SYSTEM_STAT_COUNT = 9;
constexpr int STAT_COUNT = 17;
constexpr int STAT_START = 13;
constexpr int THREAD_NAME_POS = 1;
constexpr int THREAD_STATE_POS = 2;
constexpr int CPU_USER_HZ_L = 100;
constexpr int CPU_USER_HZ_H = 1000;
constexpr int CPU_HZ_H = 10;
const int PERCENT = 100;

const std::string FREQUENCY_PATH = "/sys/devices/system/cpu";
const std::string FREQUENCY_MIN_PATH = "/cpufreq/cpuinfo_min_freq";
const std::string FREQUENCY_MAX_PATH = "/cpufreq/cpuinfo_max_freq";
const std::string FREQUENCY_CUR_PATH = "/cpufreq/cpuinfo_cur_freq";
} // namespace

CpuDataPlugin::CpuDataPlugin()
{
    buffer_ = nullptr;
    path_ = "/proc/";
    err_ = -1;
    pid_ = -1;
    prevProcessCpuTime_ = 0;
    prevCpuTimeData_ = {};
    maxFreqIndex_ = -1;
    freqPath_ = FREQUENCY_PATH;
}

CpuDataPlugin::~CpuDataPlugin()
{
    HILOG_INFO(LOG_CORE, "%s:~CpuDataPlugin!", __func__);
    if (buffer_ != nullptr) {
        free(buffer_);
        buffer_ = nullptr;
    }

    tidVec_.clear();
    prevThreadCpuTimeMap_.clear();
    prevCoreSystemCpuTimeMap_.clear();
    prevCoreSystemBootTimeMap_.clear();
    maxFrequencyVec_.clear();
    minFrequencyVec_.clear();
}

int CpuDataPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    buffer_ = malloc(READ_BUFFER_SIZE);
    CHECK_NOTNULL(buffer_, RET_FAIL, "%s:malloc buffer_ failed!", __func__);

    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, RET_FAIL,
               "%s:parseFromArray failed!", __func__);

    if (protoConfig_.pid() > 0) {
        pid_ = protoConfig_.pid();
    } else if (protoConfig_.report_process_info()) {
        HILOG_INFO(LOG_CORE, "%s:need report process info", __func__);
    } else {
        HILOG_ERROR(LOG_CORE, "%s:invalid pid", __func__);
        return RET_FAIL;
    }
    HILOG_INFO(LOG_CORE, "%s:start success!", __func__);
    return RET_SUCC;
}

int CpuDataPlugin::Report(uint8_t* data, uint32_t dataSize)
{
    CpuData dataProto;
    uint32_t length;

    WriteCpuUsageInfo(dataProto);

    if (pid_ > 0) {
        WriteThreadInfo(dataProto);
    }
    if (protoConfig_.report_process_info()) {
        WriteProcnum(dataProto);
    }

    length = dataProto.ByteSizeLong();
    if (length > dataSize) {
        return -length;
    }
    if (dataProto.SerializeToArray(data, length) > 0) {
        return length;
    }
    return 0;
}

bool CpuDataPlugin::WriteProcnum(CpuData& data)
{
    DIR* procDir = nullptr;
    procDir = OpenDestDir(path_);
    if (procDir == nullptr) {
        return false;
    }

    uint32_t i = 0;
    while (int32_t tid = GetValidTid(procDir)) {
        if (tid <= 0) {
            closedir(procDir);
            HILOG_WARN(LOG_CORE, "%s: get pid[%d] failed", __func__, tid);
            return false;
        }
        i++;
    }
    data.set_process_num(i);
    closedir(procDir);

    return true;
}

int CpuDataPlugin::Stop()
{
    if (buffer_ != nullptr) {
        free(buffer_);
        buffer_ = nullptr;
    }

    tidVec_.clear();
    prevThreadCpuTimeMap_.clear();
    prevCoreSystemCpuTimeMap_.clear();
    prevCoreSystemBootTimeMap_.clear();
    HILOG_INFO(LOG_CORE, "%s:stop success!", __func__);
    return 0;
}

int32_t CpuDataPlugin::ReadFile(std::string& fileName)
{
    int fd = -1;
    ssize_t bytesRead = 0;
    char filePath[PATH_MAX + 1] = {0};
    char realPath[PATH_MAX + 1] = {0};

    if (snprintf_s(filePath, sizeof(filePath), sizeof(filePath) - 1, "%s", fileName.c_str()) < 0) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "snprintf_s(%s) error, errno(%d:%s)", fileName.c_str(), errno, buf);
        return RET_FAIL;
    }
    if (realpath(filePath, realPath) == nullptr) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "realpath(%s) failed, errno(%d:%s)", fileName.c_str(), errno, buf);
        return RET_FAIL;
    }

    fd = open(realPath, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, realPath, errno, buf);
        err_ = errno;
        return RET_FAIL;
    }
    if (buffer_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "%s:empty address, buffer_ is NULL", __func__);
        err_ = RET_NULL_ADDR;
        close(fd);
        return RET_FAIL;
    }
    bytesRead = read(fd, buffer_, READ_BUFFER_SIZE - 1);
    if (bytesRead <= 0) {
        close(fd);
        HILOG_ERROR(LOG_CORE, "%s:failed to read(%s), errno=%d", __func__, realPath, errno);
        err_ = errno;
        return RET_FAIL;
    }
    close(fd);

    return bytesRead;
}

void CpuDataPlugin::SetTimestamp(SampleTimeStamp& timestamp)
{
    timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    timestamp.set_tv_sec(time.tv_sec);
    timestamp.set_tv_nsec(time.tv_nsec);
}

int64_t CpuDataPlugin::GetUserHz()
{
    int64_t hz = -1;
    int64_t user_hz = sysconf(_SC_CLK_TCK);
    switch (user_hz) {
        case CPU_USER_HZ_L:
            hz = CPU_HZ_H;
            break;
        case CPU_USER_HZ_H:
            hz = 1;
            break;
        default:
            break;
    }
    return hz;
}

int64_t CpuDataPlugin::GetCpuUsageTime(std::vector<std::string>& cpuUsageVec)
{
    int64_t utime = 0;
    int64_t stime = 0;
    int64_t usageTime = 0;
    utime = atoi(cpuUsageVec[PROCESS_UTIME].c_str());
    stime = atoi(cpuUsageVec[PROCESS_STIME].c_str());
    // 进程，线程CPU占用率只计算utime(用户态时间),stime(核心态时间)
    usageTime = (utime + stime) * GetUserHz();

    return usageTime;
}

void CpuDataPlugin::WriteProcessCpuUsage(CpuUsageInfo& cpuUsageInfo, const char* pFile, uint32_t fileLen)
{
    BufferSplitter totalbuffer(const_cast<char*>(pFile), fileLen + 1);
    std::vector<std::string> cpuUsageVec;
    for (int i = 0; i < STAT_COUNT; i++) {
        totalbuffer.NextWord(' ');
        if (!totalbuffer.CurWord()) {
            return;
        }

        if (i < STAT_START) {
            continue;
        } else {
            std::string curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
            cpuUsageVec.push_back(curWord);
        }
    }

    // 获取到的数据不包含utime、stime、cutime、cstime四个数值时返回
    if (cpuUsageVec.size() != PROCESS_UNSPECIFIED) {
        HILOG_ERROR(LOG_CORE, "%s:failed to get process cpu usage, size=%zu", __func__, cpuUsageVec.size());
        return;
    }

    int64_t usageTime = GetCpuUsageTime(cpuUsageVec);
    cpuUsageInfo.set_prev_process_cpu_time_ms(prevProcessCpuTime_);
    cpuUsageInfo.set_process_cpu_time_ms(usageTime);
    prevProcessCpuTime_ = usageTime;
}

int32_t CpuDataPlugin::GetCpuFrequency(std::string fileName)
{
    int32_t frequency = 0;
    int32_t ret = ReadFile(fileName);
    if (ret != RET_FAIL) {
        frequency = atoi((char*)buffer_);
    }
    return frequency;
}

int CpuDataPlugin::GetCpuCoreSize()
{
    int coreSize = 0;
    DIR* procDir = nullptr;
    procDir = OpenDestDir(freqPath_);
    CHECK_NOTNULL(procDir, -1, "procDir is nullptr");

    while (struct dirent* dirEnt = readdir(procDir)) {
        if (dirEnt->d_type != DT_DIR) {
            continue;
        }
        if (strncmp(dirEnt->d_name, "cpu", strlen("cpu")) == 0) {
            coreSize++;
        }
    }
    closedir(procDir);
    return coreSize;
}

int32_t CpuDataPlugin::GetMaxCpuFrequencyIndex()
{
    int coreSize = GetCpuCoreSize();
    int index = -1;
    int32_t maxFreq = -1;
    maxFrequencyVec_.clear();
    minFrequencyVec_.clear();
    for (int i = 0; i < coreSize; i++) {
        std::string fileName = freqPath_ + "/cpu" + std::to_string(i) + FREQUENCY_MAX_PATH;
        int32_t maxFrequency = GetCpuFrequency(fileName);
        maxFrequencyVec_.push_back(maxFrequency);
        fileName = freqPath_ + "/cpu" + std::to_string(i) + FREQUENCY_MIN_PATH;
        int32_t minFrequency = GetCpuFrequency(fileName);
        minFrequencyVec_.push_back(minFrequency);

        if (maxFreq < maxFrequency) {
            maxFreq = maxFrequency;
            index = i;
        }
    }

    // 单核或所有核最大频率相同，默认小核
    if (coreSize == 1 || (coreSize > 1 && index == 0 && maxFreq == maxFrequencyVec_[1])) {
        index = -1;
    }

    return index;
}

void CpuDataPlugin::SetCpuFrequency(CpuCoreUsageInfo& cpuCore, int32_t coreNum)
{
    // 第一次获取最大频率核位置，并保存各核最大最小频率到vector
    if (maxFrequencyVec_.empty() || minFrequencyVec_.empty()) {
        maxFreqIndex_ = GetMaxCpuFrequencyIndex();
    }
    std::string fileName = freqPath_ + "/cpu" + std::to_string(coreNum) + FREQUENCY_CUR_PATH;
    int32_t curFrequency = GetCpuFrequency(fileName);
    int32_t maxFrequency = maxFrequencyVec_[coreNum];
    int32_t minFrequency = minFrequencyVec_[coreNum];

    if (coreNum == maxFreqIndex_) {
        cpuCore.set_is_little_core(false);
    } else {
        cpuCore.set_is_little_core(true);
    }
    CpuCoreFrequency* frequency = cpuCore.mutable_frequency();
    frequency->set_min_frequency_khz(minFrequency);
    frequency->set_max_frequency_khz(maxFrequency);
    frequency->set_cur_frequency_khz(curFrequency);
}

bool CpuDataPlugin::GetSystemCpuTime(std::vector<std::string>& cpuUsageVec, CpuTimeData& cpuTimeData)
{
    // 获取到的数据不包含user, nice, system, idle, iowait, irq, softirq, steal八个数值时返回
    CHECK_TRUE(cpuUsageVec.size() == SYSTEM_UNSPECIFIED, false,
               "%s:failed to get system cpu usage, size=%zu", __func__, cpuUsageVec.size());

    int64_t user = 0;
    int64_t nice = 0;
    int64_t system = 0;
    int64_t idle = 0;
    int64_t iowait = 0;
    int64_t irq = 0;
    int64_t softirq = 0;
    int64_t steal = 0;
    user = atoi(cpuUsageVec[SYSTEM_USER].c_str());
    nice = atoi(cpuUsageVec[SYSTEM_NICE].c_str());
    system = atoi(cpuUsageVec[SYSTEM_SYSTEM].c_str());
    idle = atoi(cpuUsageVec[SYSTEM_IDLE].c_str());
    iowait = atoi(cpuUsageVec[SYSTEM_IOWAIT].c_str());
    irq = atoi(cpuUsageVec[SYSTEM_IRQ].c_str());
    softirq = atoi(cpuUsageVec[SYSTEM_SOFTIRQ].c_str());
    steal = atoi(cpuUsageVec[SYSTEM_STEAL].c_str());

    cpuTimeData.userModeUsageTime = user * GetUserHz();
    cpuTimeData.systemModeUsageTime = system * GetUserHz();
    cpuTimeData.systemUsageTime = (user + nice + system + irq + softirq + steal) * GetUserHz();
    cpuTimeData.systemBootTime = cpuTimeData.systemUsageTime + (idle + iowait) * GetUserHz();
    return true;
}

void CpuDataPlugin::WriteSystemCpuUsage(CpuData& cpuData, const char* pFile, uint32_t fileLen)
{
    auto* cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    BufferSplitter totalbuffer(const_cast<char*>(pFile), fileLen + 1);
    std::vector<std::string> cpuUsageVec;
    size_t cpuLength = strlen("cpu");

    do {
        totalbuffer.NextWord(' ');
        if (!totalbuffer.CurWord() || strncmp(totalbuffer.CurWord(), "cpu", cpuLength) != 0) {
            return;
        }

        for (int i = 0; i < SYSTEM_STAT_COUNT; i++) {
            if (!totalbuffer.CurWord()) {
                return;
            }
            std::string curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
            cpuUsageVec.push_back(curWord);
            totalbuffer.NextWord(' ');
        }

        // 获取数据失败返回
        CpuTimeData cpuTimeData;
        if (!GetSystemCpuTime(cpuUsageVec, cpuTimeData)) {
            return;
        }

        if (strcmp(cpuUsageVec[0].c_str(), "cpu") == 0) {
            cpuUsageInfo->set_prev_system_cpu_time_ms(prevCpuTimeData_.systemUsageTime);
            cpuUsageInfo->set_prev_system_boot_time_ms(prevCpuTimeData_.systemBootTime);
            cpuUsageInfo->set_system_cpu_time_ms(cpuTimeData.systemUsageTime);
            cpuUsageInfo->set_system_boot_time_ms(cpuTimeData.systemBootTime);
            bool isTest = false;
            if (strncmp(path_.c_str(), "/proc/", strlen("/proc/")) != 0) {
                isTest = true; // UT needs report load data for the first time
            }
            if ((protoConfig_.report_process_info() && prevCpuTimeData_.systemBootTime != 0) || isTest) {
                double userLoad = static_cast<double>(cpuTimeData.userModeUsageTime -
                    prevCpuTimeData_.userModeUsageTime) /
                    static_cast<double>(cpuTimeData.systemBootTime -
                    prevCpuTimeData_.systemBootTime) * PERCENT;
                double sysLoad = static_cast<double>(cpuTimeData.systemModeUsageTime -
                    prevCpuTimeData_.systemModeUsageTime) /
                    static_cast<double>(cpuTimeData.systemBootTime -
                    prevCpuTimeData_.systemBootTime) * PERCENT;
                double totalLoad = static_cast<double>(cpuTimeData.systemUsageTime -
                    prevCpuTimeData_.systemUsageTime) /
                    static_cast<double>(cpuTimeData.systemBootTime -
                    prevCpuTimeData_.systemBootTime) * PERCENT;
                cpuData.set_user_load(userLoad);
                cpuData.set_sys_load(sysLoad);
                cpuData.set_total_load(totalLoad);
            }
            prevCpuTimeData_ = cpuTimeData;
            if (pid_ < 0 && protoConfig_.report_process_info()) {
                return;
            }
        } else {
            std::string core = std::string(cpuUsageVec[0].c_str() + cpuLength, cpuUsageVec[0].size() - cpuLength);
            int32_t coreNum = atoi(core.c_str());
            // 第一次获取数据时需要将前一个数据置为0
            if (prevCoreSystemCpuTimeMap_.size() == static_cast<size_t>(coreNum)) {
                prevCoreSystemCpuTimeMap_[coreNum] = 0;
                prevCoreSystemBootTimeMap_[coreNum] = 0;
            }
            CpuCoreUsageInfo* cpuCore = cpuUsageInfo->add_cores();
            cpuCore->set_cpu_core(coreNum);
            cpuCore->set_prev_system_cpu_time_ms(prevCoreSystemCpuTimeMap_[coreNum]);
            cpuCore->set_prev_system_boot_time_ms(prevCoreSystemBootTimeMap_[coreNum]);
            cpuCore->set_system_cpu_time_ms(cpuTimeData.systemUsageTime);
            cpuCore->set_system_boot_time_ms(cpuTimeData.systemBootTime);

            SetCpuFrequency(*cpuCore, coreNum);
            prevCoreSystemCpuTimeMap_[coreNum] = cpuTimeData.systemUsageTime;
            prevCoreSystemBootTimeMap_[coreNum] = cpuTimeData.systemBootTime;
        }

        cpuUsageVec.clear();
    } while (totalbuffer.NextLine());
}

void CpuDataPlugin::WriteCpuUsageInfo(CpuData& data)
{
    // write process info
    if (pid_ > 0) {
        std::string fileName = path_ + std::to_string(pid_) + "/stat";
        int32_t ret = ReadFile(fileName);
        if (ret == RET_FAIL) {
            return;
        }
        if ((buffer_ == nullptr) || (ret == 0)) {
            return;
        }

        auto* cpuUsageInfo = data.mutable_cpu_usage_info();
        WriteProcessCpuUsage(*cpuUsageInfo, (char*)buffer_, ret);
    }

    // write system info
    std::string fileName = path_ + "stat";
    int32_t ret = ReadFile(fileName);
    if (ret == RET_FAIL) {
        return;
    }
    if ((buffer_ == nullptr) || (ret == 0)) {
        return;
    }
    WriteSystemCpuUsage(data, (char*)buffer_, ret);

    auto* timestamp = data.mutable_cpu_usage_info()->mutable_timestamp();
    SetTimestamp(*timestamp);
}

bool CpuDataPlugin::addTidBySort(int32_t tid)
{
    auto tidsEnd = tidVec_.end();
    auto it = std::lower_bound(tidVec_.begin(), tidsEnd, tid);
    CHECK_TRUE(!(it != tidsEnd && *it == tid), false, "addTidBySort failed");
    it = tidVec_.insert(it, std::move(tid));
    return true;
}

DIR* CpuDataPlugin::OpenDestDir(std::string& dirPath)
{
    DIR* destDir = nullptr;

    destDir = opendir(dirPath.c_str());
    CHECK_NOTNULL(destDir, nullptr, "%s:failed to opendir(%s), errno=%d", __func__, dirPath.c_str(), errno);

    return destDir;
}

int32_t CpuDataPlugin::GetValidTid(DIR* dirp)
{
    CHECK_TRUE(dirp, 0, "dirp is nullptr");
    while (struct dirent* dirEnt = readdir(dirp)) {
        if (dirEnt->d_type != DT_DIR) {
            continue;
        }

        int32_t tid = atoi(dirEnt->d_name);
        if (tid) {
            return tid;
        }
    }
    return 0;
}

ThreadState CpuDataPlugin::GetThreadState(const char threadState)
{
    ThreadState state = THREAD_UNSPECIFIED;
    switch (threadState) {
        case 'R':
            state = THREAD_RUNNING;
            break;
        case 'S':
            state = THREAD_SLEEPING;
            break;
        case 'T':
            state = THREAD_STOPPED;
            break;
        case 'D':
            state = THREAD_WAITING;
            break;
        default:
            break;
    }

    return state;
}

void CpuDataPlugin::WriteThread(ThreadInfo& threadInfo, const char* pFile, uint32_t fileLen, int32_t tid)
{
    BufferSplitter totalbuffer(const_cast<char*>(pFile), fileLen + 1);
    std::vector<std::string> cpuUsageVec;
    for (int i = 0; i < STAT_COUNT; i++) {
        if (i == THREAD_NAME_POS) { // 线程名是')'作为结束符
            totalbuffer.NextWord(')');
        } else {
            totalbuffer.NextWord(' ');
        }

        if (!totalbuffer.CurWord()) {
            return;
        }

        if (i == THREAD_NAME_POS) {
            std::string curWord = std::string(totalbuffer.CurWord() + 1, totalbuffer.CurWordSize() - 1);
            threadInfo.set_thread_name(curWord);
        } else if (i == THREAD_STATE_POS) {
            std::string curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
            ThreadState state = GetThreadState(curWord[0]);
            threadInfo.set_thread_state(state);
        } else if (i >= STAT_START) {
            std::string curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
            cpuUsageVec.push_back(curWord);
        }
    }

    // 获取到的数据不包含utime、stime、cutime、cstime四个数值时返回
    if (cpuUsageVec.size() != PROCESS_UNSPECIFIED) {
        HILOG_ERROR(LOG_CORE, "%s:failed to get thread cpu usage, size=%zu", __func__, cpuUsageVec.size());
        return;
    }

    // 第一次获取该线程数据时需要将前一个数据置为0
    if (prevThreadCpuTimeMap_.find(tid) == prevThreadCpuTimeMap_.end()) {
        prevThreadCpuTimeMap_[tid] = 0;
    }

    int64_t usageTime = GetCpuUsageTime(cpuUsageVec);
    threadInfo.set_prev_thread_cpu_time_ms(prevThreadCpuTimeMap_[tid]);
    threadInfo.set_thread_cpu_time_ms(usageTime);
    prevThreadCpuTimeMap_[tid] = usageTime;
    threadInfo.set_tid(tid);

    auto* timestamp = threadInfo.mutable_timestamp();
    SetTimestamp(*timestamp);
}

void CpuDataPlugin::WriteSingleThreadInfo(CpuData& data, int32_t tid)
{
    std::string fileName = path_ + std::to_string(pid_) + "/task/" + std::to_string(tid) + "/stat";
    int32_t ret = ReadFile(fileName);
    if (ret == RET_FAIL) {
        return;
    }
    if ((buffer_ == nullptr) || (ret == 0)) {
        return;
    }
    auto* threadInfo = data.add_thread_info();
    WriteThread(*threadInfo, (char*)buffer_, ret, tid);
}

void CpuDataPlugin::WriteThreadInfo(CpuData& data)
{
    DIR* procDir = nullptr;
    std::string path = path_ + std::to_string(pid_) + "/task";
    procDir = OpenDestDir(path);
    if (procDir == nullptr) {
        return;
    }

    tidVec_.clear();
    while (int32_t tid = GetValidTid(procDir)) {
        addTidBySort(tid);
    }

    for (unsigned int i = 0; i < tidVec_.size(); i++) {
        WriteSingleThreadInfo(data, tidVec_[i]);
    }
    closedir(procDir);
}

// for UT
void CpuDataPlugin::SetFreqPath(std::string path)
{
    freqPath_ = path + FREQUENCY_PATH;
}
