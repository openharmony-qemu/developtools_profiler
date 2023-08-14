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
#include "process_data_plugin.h"

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "buffer_splitter.h"
#include "securec.h"

namespace {
constexpr size_t READ_BUFFER_SIZE = 1024 * 16;
constexpr int DEC_BASE = 10;
constexpr int STAT_COUNT = 13;
constexpr int CPU_USER_HZ_L = 100;
constexpr int CPU_USER_HZ_H = 1000;
constexpr int CPU_HZ_H = 10;
const int PERCENT = 100;
} // namespace

ProcessDataPlugin::ProcessDataPlugin()
    : buffer_(new (std::nothrow) uint8_t[READ_BUFFER_SIZE]), err_(-1)
{
    SetPath("/proc/");
}

ProcessDataPlugin::~ProcessDataPlugin()
{
    HILOG_INFO(LOG_CORE, "%s:~ProcessDataPlugin!", __func__);

    buffer_ = nullptr;

    return;
}

int ProcessDataPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    CHECK_NOTNULL(buffer_, RET_FAIL, "%s:buffer_ == null", __func__);

    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, RET_FAIL,
               "%s:parseFromArray failed!", __func__);

    HILOG_INFO(LOG_CORE, "%s:start success!", __func__);
    return RET_SUCC;
}

int ProcessDataPlugin::Report(uint8_t* data, uint32_t dataSize)
{
    ProcessData dataProto;
    uint32_t length;

    if (protoConfig_.report_process_tree()) {
        WriteProcesseList(dataProto);
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

int ProcessDataPlugin::Stop()
{
    pids_.clear();
    cpuTime_.clear();
    bootTime_.clear();

    HILOG_INFO(LOG_CORE, "%s:stop success!", __func__);
    return 0;
}

DIR* ProcessDataPlugin::OpenDestDir(const char* dirPath)
{
    DIR* destDir = nullptr;

    destDir = opendir(dirPath);
    if (destDir == nullptr) {
        HILOG_ERROR(LOG_CORE, "%s:failed to opendir(%s), errno=%d", __func__, dirPath, errno);
    }

    return destDir;
}

int32_t ProcessDataPlugin::GetValidPid(DIR* dirp)
{
    if (!dirp) return 0;
    while (struct dirent* dirEnt = readdir(dirp)) {
        if (dirEnt->d_type != DT_DIR) {
            continue;
        }

        int32_t pid = atoi(dirEnt->d_name);
        if (pid) {
            return pid;
        }
    }
    return 0;
}

int32_t ProcessDataPlugin::ReadProcPidFile(int32_t pid, const char* pFileName)
{
    char fileName[PATH_MAX + 1] = {0};
    char realPath[PATH_MAX + 1] = {0};
    int fd = -1;
    ssize_t bytesRead = 0;

    if (snprintf_s(fileName, sizeof(fileName), sizeof(fileName) - 1, "%s%d/%s", path_.c_str(), pid, pFileName) < 0) {
        HILOG_ERROR(LOG_CORE, "%s:snprintf_s error", __func__);
    }
    if (realpath(fileName, realPath) == nullptr) {
        HILOG_ERROR(LOG_CORE, "%s:realpath failed, errno=%d", __func__, errno);
        return RET_FAIL;
    }
    fd = open(realPath, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        HILOG_INFO(LOG_CORE, "%s:failed to open(%s), errno=%d", __func__, fileName, errno);
        err_ = errno;
        return RET_FAIL;
    }
    if (buffer_.get() == nullptr) {
        HILOG_INFO(LOG_CORE, "%s:empty address, buffer_ is NULL", __func__);
        err_ = RET_NULL_ADDR;
        close(fd);
        return RET_FAIL;
    }
    bytesRead = read(fd, buffer_.get(), READ_BUFFER_SIZE - 1);
    if (bytesRead < 0) {
        close(fd);
        HILOG_INFO(LOG_CORE, "%s:failed to read(%s), errno=%d", __func__, fileName, errno);
        err_ = errno;
        return RET_FAIL;
    }
    buffer_.get()[bytesRead] = '\0';
    close(fd);

    return bytesRead;
}

bool ProcessDataPlugin::BufnCmp(const char* src, int srcLen, const char* key, int keyLen)
{
    if (!src || !key || (srcLen < keyLen)) {
        return false;
    }
    for (int i = 0; i < keyLen; i++) {
        if (*src++ != *key++) {
            return false;
        }
    }
    return true;
}

bool ProcessDataPlugin::addPidBySort(int32_t pid)
{
    auto pidsEnd = pids_.end();
    auto it = std::lower_bound(pids_.begin(), pidsEnd, pid);
    if (it != pidsEnd && *it == pid) {
        return false;
    }
    it = pids_.insert(it, std::move(pid));
    return true;
}

void ProcessDataPlugin::WriteProcess(ProcessInfo* processinfo, const char* pFile, uint32_t fileLen, int32_t pid)
{
    BufferSplitter totalbuffer(const_cast<const char*>(pFile), fileLen + 1);

    do {
        totalbuffer.NextWord(':');
        if (!totalbuffer.CurWord()) {
            return;
        }

        if (BufnCmp(totalbuffer.CurWord(), totalbuffer.CurWordSize(), "Name", strlen("Name"))) {
            totalbuffer.NextWord('\n');
            if (!totalbuffer.CurWord()) {
                return;
            }
            processinfo->set_name(totalbuffer.CurWord(), totalbuffer.CurWordSize());
        } else if (BufnCmp(totalbuffer.CurWord(), totalbuffer.CurWordSize(), "Pid", strlen("Pid"))) {
            totalbuffer.NextWord('\n');
            if (!totalbuffer.CurWord()) {
                return;
            }
            char* end = nullptr;
            int32_t value = static_cast<int32_t>(strtoul(totalbuffer.CurWord(), &end, DEC_BASE));
            if (value < 0) {
                HILOG_ERROR(LOG_CORE, "%s:strtoull value failed", __func__);
            }
            processinfo->set_pid(value);
        } else if (BufnCmp(totalbuffer.CurWord(), totalbuffer.CurWordSize(), "PPid", strlen("PPid"))) {
            totalbuffer.NextWord('\n');
            if (!totalbuffer.CurWord()) {
                return;
            }
            char* end = nullptr;
            int32_t value = static_cast<int32_t>(strtoul(totalbuffer.CurWord(), &end, DEC_BASE));
            if (value < 0) {
                HILOG_ERROR(LOG_CORE, "%s:strtoull value failed", __func__);
            }
            processinfo->set_ppid(value);
        } else if (BufnCmp(totalbuffer.CurWord(), totalbuffer.CurWordSize(), "Uid", strlen("Uid"))) {
            totalbuffer.NextWord('\n');
            if (!totalbuffer.CurWord()) {
                return;
            }
            std::string curWord = std::string(totalbuffer.CurWord(), totalbuffer.CurWordSize());
            curWord = curWord.substr(0, curWord.find(" "));
            char* end = nullptr;
            int32_t value = static_cast<int32_t>(strtoul(curWord.c_str(), &end, DEC_BASE));
            if (value < 0) {
                HILOG_ERROR(LOG_CORE, "%s:strtoull value failed", __func__);
            }
            processinfo->set_uid(value);
            break;
        } else {
            totalbuffer.NextWord('\n');
            if (!totalbuffer.CurWord()) {
                continue;
            }
        }
    } while (totalbuffer.NextLine());
    // update process name
    int32_t ret = ReadProcPidFile(pid, "cmdline");
    if (ret > 0) {
        processinfo->set_name(reinterpret_cast<char*>(buffer_.get()), strlen(reinterpret_cast<char*>(buffer_.get())));
    }
}

void ProcessDataPlugin::WriteProcessInfo(ProcessData& data, int32_t pid)
{
    int32_t ret = ReadProcPidFile(pid, "status");
    if (ret == RET_FAIL) {
        return;
    }
    if ((buffer_.get() == nullptr) || (ret == 0)) {
        return;
    }
    auto* processinfo = data.add_processesinfo();
    WriteProcess(processinfo, (char*)buffer_.get(), ret, pid);
    if (protoConfig_.report_cpu()) {
        auto cpuProto = processinfo->mutable_cpuinfo();
        std::vector<uint64_t> cpuUsageVec;
        std::vector<uint64_t> bootTime;
        WriteCpuUsageData(pid, cpuProto);
        WriteThreadData(pid, cpuProto);
    }
    if (protoConfig_.report_diskio()) {
        auto diskProto = processinfo->mutable_diskinfo();
        WriteDiskioData(pid, diskProto);
    }
    if (protoConfig_.report_pss()) {
        auto pssProto = processinfo->mutable_pssinfo();
        WritePssData(pid, pssProto);
    }
}

bool ProcessDataPlugin::WriteProcesseList(ProcessData& data)
{
    DIR* procDir = nullptr;

    procDir = OpenDestDir(path_.c_str());
    if (procDir == nullptr) {
        return false;
    }

    pids_.clear();
    while (int32_t pid = GetValidPid(procDir)) {
        if (pid <= 0) {
            closedir(procDir);
            HILOG_WARN(LOG_CORE, "%s: get pid[%d] failed", __func__, pid);
            return false;
        }
        addPidBySort(pid);
    }

    for (unsigned int i = 0; i < pids_.size(); i++) {
        WriteProcessInfo(data, pids_[i]);
    }

    closedir(procDir);
    return true;
}

bool ProcessDataPlugin::WriteThreadData(int pid, CpuInfo* protoc)
{
    DIR* procDir = nullptr;
    std::string path = path_ + std::to_string(pid) + "/task";
    procDir = OpenDestDir(path.c_str());
    if (procDir == nullptr) {
        return false;
    }

    uint32_t i = 0;
    while (int32_t tid = GetValidPid(procDir)) {
        if (tid <= 0) {
            closedir(procDir);
            HILOG_WARN(LOG_CORE, "%s: get pid[%d] failed", __func__, tid);
            return false;
        }
        i++;
    }
    protoc->set_thread_sum(i);
    closedir(procDir);
    return true;
}

int64_t ProcessDataPlugin::GetUserHz()
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

bool ProcessDataPlugin::WriteCpuUsageData(int pid, CpuInfo* protoc)
{
    uint64_t prevCpuTime = 0;
    uint64_t cpuTime = 0;
    uint64_t prevBootTime = 0;
    uint64_t bootTime = 0;
    double usage = 0.0;
    ReadCpuUsage(pid, protoc, cpuTime);
    ReadBootTime(pid, protoc, bootTime);
    if (cpuTime_.find(pid) != cpuTime_.end()) {
        prevCpuTime = cpuTime_[pid];
    }
    if (bootTime_.find(pid) != bootTime_.end()) {
        prevBootTime = bootTime_[pid];
    }
    if (bootTime - prevBootTime == 0 || bootTime == 0) {
        protoc->set_cpu_usage(0);
        return false;
    }
    if (prevCpuTime == 0) {
        usage = static_cast<double>(cpuTime) / (bootTime);
    } else {
        usage = static_cast<double>(cpuTime - prevCpuTime) / (bootTime - prevBootTime);
    }

    protoc->set_cpu_usage(usage * PERCENT);
    protoc->set_cpu_time_ms(cpuTime);
    cpuTime_[pid] = cpuTime;
    bootTime_[pid] = bootTime;
    return true;
}

bool ProcessDataPlugin::ReadBootTime(int pid, CpuInfo* protoc, uint64_t& bootTime)
{
    std::string path = path_ + "stat";
    std::ifstream input(path, std::ios::in);
    CHECK_TRUE(!input.fail(), false, "%s open %s failed, errno = %d", __func__, path.c_str(), errno);
    do {
        if (!input.good()) {
            return false;
        }
        std::string line;
        getline(input, line);

        auto pos = line.find("cpu ");
        if (pos != std::string::npos) {
            line += '\n';
            GetBootData(line, protoc, bootTime);
        }
    } while (0);
    input.close();

    return true;
}

uint32_t ProcessDataPlugin::GetBootData(const std::string& line, CpuInfo* protoc, uint64_t& bootTime)
{
    uint64_t num;
    uint32_t count = 0;
    char* end = nullptr;
    char* pTmp = const_cast<char*>(line.c_str());
    constexpr uint32_t cntVec = 8;

    std::vector<uint64_t> bootTimeVec;
    bootTime = 0;
    while (pTmp != nullptr && *pTmp != '\n') {
        CHECK_TRUE(FindFirstNum(&pTmp), count, "%s: FindFirstNum failed", __func__);
        num = strtoull(pTmp, &end, DEC_BASE);
        CHECK_TRUE(num >= 0, count, "%s:strtoull failed", __func__);
        bootTimeVec.push_back(num);
        bootTime += num;
        pTmp = end;
        if (++count >= cntVec) {
            break;
        }
    }
    bootTime = bootTime * (uint64_t)GetUserHz();
    return count;
}

bool ProcessDataPlugin::ReadCpuUsage(int pid, CpuInfo* protoc, uint64_t& cpuTime)
{
    std::string path = path_ + std::to_string(pid) + "/stat";
    std::ifstream input(path, std::ios::in);
    CHECK_TRUE(!input.fail(), false, "%s open %s failed, errno = %d", __func__, path.c_str(), errno);
    do {
        if (!input.good()) {
            return false;
        }
        std::string line;
        getline(input, line);
        line += '\n';
        GetCpuUsageData(line, protoc, cpuTime);
    } while (0);
    input.close();

    return true;
}

uint32_t ProcessDataPlugin::GetCpuUsageData(const std::string& line, CpuInfo* protoc, uint64_t& cpuTime)
{
    uint64_t num;
    uint32_t count = 0;
    char* end = nullptr;
    char* pTmp = const_cast<char*>(line.c_str());
    int i = 0;
    constexpr uint32_t cntVec = 4;

    while (FindFirstSpace(&pTmp)) {
        pTmp++;
        if (++i >= STAT_COUNT) {
            break;
        }
    }
    std::vector<uint64_t> cpuUsageVec;
    cpuTime = 0;
    while (pTmp != nullptr && *pTmp != '\n') {
        CHECK_TRUE(FindFirstNum(&pTmp), count, "%s: FindFirstNum failed", __func__);
        num = strtoull(pTmp, &end, DEC_BASE);
        cpuUsageVec.push_back(num);
        cpuTime += num;
        pTmp = end;
        if (++count >= cntVec) {
            break;
        }
    }
    cpuTime = cpuTime * (uint64_t)GetUserHz();
    return count;
}

bool ProcessDataPlugin::WriteDiskioData(int pid, DiskioInfo* protoc)
{
    std::string path = path_ + std::to_string(pid) + "/io";
    std::ifstream input(path, std::ios::in);
    if (input.fail()) {
        return false;
    }
    do {
        if (!input.good()) {
            return false;
        }
        std::string line;
        getline(input, line);
        line += '\n';
        GetDiskioData(line, protoc);
    } while (!input.eof());
    input.close();

    return true;
}

bool ProcessDataPlugin::GetDiskioData(std::string& line, DiskioInfo* protoc)
{
    char* pTmp = const_cast<char*>(line.c_str());
    CHECK_NOTNULL(pTmp, false, "param invalid!");

    uint64_t num;
    if (!std::strncmp(pTmp, "rchar:", strlen("rchar:"))) {
        CHECK_TRUE(GetValidValue(pTmp, num), false, "%s: get rchar failed", __func__);
        protoc->set_rchar(num);
    } else if (!std::strncmp(pTmp, "wchar:", strlen("wchar:"))) {
        CHECK_TRUE(GetValidValue(pTmp, num), false, "%s: get wchar failed", __func__);
        protoc->set_wchar(num);
    } else if (!std::strncmp(pTmp, "syscr:", strlen("syscr:"))) {
        CHECK_TRUE(GetValidValue(pTmp, num), false, "%s: get syscr failed", __func__);
        protoc->set_syscr(num);
    } else if (!std::strncmp(pTmp, "syscw:", strlen("syscw:"))) {
        CHECK_TRUE(GetValidValue(pTmp, num), false, "%s: get syscw failed", __func__);
        protoc->set_syscw(num);
    } else if (!std::strncmp(pTmp, "read_bytes:", strlen("read_bytes:"))) {
        CHECK_TRUE(GetValidValue(pTmp, num), false, "%s: get read_bytes failed", __func__);
        protoc->set_rbytes(num);
    } else if (!std::strncmp(pTmp, "write_bytes:", strlen("write_bytes:"))) {
        CHECK_TRUE(GetValidValue(pTmp, num), false, "%s: get write_bytes failed", __func__);
        protoc->set_wbytes(num);
    } else if (!std::strncmp(pTmp, "cancelled_write_bytes:", strlen("cancelled_write_bytes:"))) {
        CHECK_TRUE(GetValidValue(pTmp, num), false, "%s: get cancelled_write_bytes failed", __func__);
        protoc->set_cancelled_wbytes(num);
    }

    return true;
}

bool ProcessDataPlugin::FindFirstSpace(char** p)
{
    CHECK_NOTNULL(*p, false, "ProcessDataPlugin:%s", __func__);
    while (**p != ' ') {
        if (**p == '\0' || **p == '\n') {
            return false;
        }
        (*p)++;
    }
    return true;
}

bool ProcessDataPlugin::FindFirstNum(char** p)
{
    CHECK_NOTNULL(*p, false, "ProcessDataPlugin:%s", __func__);
    while (**p > '9' || **p < '0') {
        if (**p == '\0' || **p == '\n') {
            return false;
        }
        (*p)++;
    }
    return true;
}

bool ProcessDataPlugin::GetValidValue(char* p, uint64_t& num)
{
    char* end = nullptr;
    CHECK_TRUE(FindFirstNum(&p), false, "%s: FindFirstNum failed", __func__);
    num = strtoull(p, &end, DEC_BASE);
    CHECK_TRUE(num >= 0, false, "%s:strtoull failed", __func__);
    return true;
}

// read /proc/pid/smaps_rollup
bool ProcessDataPlugin::WritePssData(int pid, PssInfo* protoc)
{
    std::string path = path_ + std::to_string(pid) + "/smaps_rollup";
    std::ifstream input(path, std::ios::in);

    // Not capturing ENOENT (file does not exist) errors, it is common for node smaps_rollup files to be unreadable.
    CHECK_TRUE(!input.fail(), false, "%s open %s failed, errno = %d", __func__, path.c_str(), errno);

    // set ifstream to O_NONBLOCK mode
    CHECK_NOTNULL(input.rdbuf(), false, "%s:input rdbuf is nullptr", __func__);
    input.rdbuf()->pubsetbuf(nullptr, 0);

    do {
        if (!input.good()) {
            return false;
        }
        std::string line;
        getline(input, line);
        line += '\n';
        std::string::size_type pos = 0u;
        if (line.find("Pss:", pos) != std::string::npos) {
            char* pTmp = const_cast<char*>(line.c_str());
            uint64_t num;
            CHECK_TRUE(GetValidValue(pTmp, num), false, "%s: FindFirstNum failed", __func__);
            protoc->set_pss_info(num);
            return true;
        }
    } while (!input.eof());
    input.close();

    return false;
}
