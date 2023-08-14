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
#include "memory_data_plugin.h"

#include <sstream>

#include "buffer_splitter.h"
#include "securec.h"
#include "smaps_stats.h"

namespace {
const char* CMD_FORMAT = "memory service meminfo --local ";
constexpr size_t READ_BUFFER_SIZE = 1024 * 16;
constexpr int BUF_MAX_LEN = 2048;
constexpr int MAX_ZRAM_DEVICES = 256;
constexpr int ZRAM_KB = 1024;
constexpr size_t DEFAULT_READ_SIZE = 4096;
} // namespace

MemoryDataPlugin::MemoryDataPlugin()
    : buffer_(new (std::nothrow) uint8_t[READ_BUFFER_SIZE]), meminfoFd_(-1), vmstatFd_(-1), err_(-1)
{
    InitProto2StrVector();
    SetPath(const_cast<char*>("/proc"));
}

MemoryDataPlugin::~MemoryDataPlugin()
{
    HILOG_INFO(LOG_CORE, "%s:~MemoryDataPlugin!", __func__);

    buffer_ = nullptr;

    if (meminfoFd_ > 0) {
        close(meminfoFd_);
        meminfoFd_ = -1;
    }
    if (vmstatFd_ > 0) {
        close(vmstatFd_);
        vmstatFd_ = -1;
    }
    for (auto it = pidFds_.begin(); it != pidFds_.end(); it++) {
        for (int i = FILE_STATUS; i <= FILE_SMAPS; i++) {
            if (it->second[i] != -1) {
                close(it->second[i]);
            }
        }
    }
    return;
}

void MemoryDataPlugin::InitProto2StrVector()
{
    int maxprotobufid = 0;
    for (unsigned int i = 0; i < sizeof(meminfoMapping) / sizeof(meminfoMapping[0]); i++) {
        maxprotobufid = std::max(meminfoMapping[i].protobufid, maxprotobufid);
    }
    meminfoStrList_.resize(maxprotobufid + 1);

    for (unsigned int i = 0; i < sizeof(meminfoMapping) / sizeof(meminfoMapping[0]); i++) {
        meminfoStrList_[meminfoMapping[i].protobufid] = meminfoMapping[i].procstr;
    }

    maxprotobufid = 0;
    for (unsigned int i = 0; i < sizeof(vmeminfoMapping) / sizeof(vmeminfoMapping[0]); i++) {
        maxprotobufid = std::max(vmeminfoMapping[i].protobufid, maxprotobufid);
    }
    vmstatStrList_.resize(maxprotobufid + 1);

    for (unsigned int i = 0; i < sizeof(vmeminfoMapping) / sizeof(vmeminfoMapping[0]); i++) {
        vmstatStrList_[vmeminfoMapping[i].protobufid] = vmeminfoMapping[i].procstr;
    }

    return;
}

int MemoryDataPlugin::InitMemVmemFd()
{
    if (protoConfig_.report_sysmem_mem_info()) {
        char fileName[PATH_MAX + 1] = {0};
        char realPath[PATH_MAX + 1] = {0};
        CHECK_TRUE(snprintf_s(fileName, sizeof(fileName), sizeof(fileName) - 1, "%s/meminfo", testpath_) >= 0, RET_FAIL,
                   "%s:snprintf_s error", __func__);
        if (realpath(fileName, realPath) == nullptr) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "%s:realpath failed, errno(%d:%s)", __func__, errno, buf);
            return RET_FAIL;
        }
        meminfoFd_ = open(realPath, O_RDONLY | O_CLOEXEC);
        if (meminfoFd_ == -1) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "%s:open failed, fileName, errno(%d:%s)", __func__, errno, buf);
            return RET_FAIL;
        }
    }

    if (protoConfig_.report_sysmem_vmem_info()) {
        char fileName[PATH_MAX + 1] = {0};
        char realPath[PATH_MAX + 1] = {0};
        CHECK_TRUE(snprintf_s(fileName, sizeof(fileName), sizeof(fileName) - 1, "%s/vmstat", testpath_) >= 0, RET_FAIL,
                   "%s:snprintf_s error", __func__);
        if (realpath(fileName, realPath) == nullptr) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "%s:realpath failed, errno(%d:%s)", __func__, errno, buf);
            return RET_FAIL;
        }
        vmstatFd_ = open(realPath, O_RDONLY | O_CLOEXEC);
        if (vmstatFd_ == -1) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "%s:failed to open(/proc/vmstat), errno(%d:%s)", __func__, errno, buf);
            return RET_FAIL;
        }
    }

    return RET_SUCC;
}

int MemoryDataPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    CHECK_NOTNULL(buffer_, RET_FAIL, "%s:buffer_ == null", __func__);

    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, RET_FAIL,
               "%s:parseFromArray failed!", __func__);

    CHECK_TRUE(InitMemVmemFd() == RET_SUCC, RET_FAIL, "InitMemVmemFd fail");

    if (protoConfig_.sys_meminfo_counters().size() > 0) {
        for (int i = 0; i < protoConfig_.sys_meminfo_counters().size(); i++) {
            CHECK_TRUE((size_t)protoConfig_.sys_meminfo_counters(i) < meminfoStrList_.size(), RET_FAIL,
                       "%s:sys meminfo counter index invalid!", __func__);
            if (meminfoStrList_[protoConfig_.sys_meminfo_counters(i)]) {
                meminfoCounters_.emplace(meminfoStrList_[protoConfig_.sys_meminfo_counters(i)],
                                         protoConfig_.sys_meminfo_counters(i));
            }
        }
    }

    if (protoConfig_.sys_vmeminfo_counters().size() > 0) {
        for (int i = 0; i < protoConfig_.sys_vmeminfo_counters().size(); i++) {
            CHECK_TRUE((size_t)protoConfig_.sys_vmeminfo_counters(i) < vmstatStrList_.size(), RET_FAIL,
                       "%s:vmstat counter index invalid!", __func__);
            if (vmstatStrList_[protoConfig_.sys_vmeminfo_counters(i)]) {
                vmstatCounters_.emplace(vmstatStrList_[protoConfig_.sys_vmeminfo_counters(i)],
                                        protoConfig_.sys_vmeminfo_counters(i));
            }
        }
    }

    if (protoConfig_.pid().size() > 0) {
        for (int i = 0; i < protoConfig_.pid().size(); i++) {
            int32_t pid = protoConfig_.pid(i);
            pidFds_.emplace(pid, OpenProcPidFiles(pid));
        }
    }

    HILOG_INFO(LOG_CORE, "%s:start success!", __func__);
    return RET_SUCC;
}

void MemoryDataPlugin::WriteMeminfo(MemoryData& data)
{
    int readsize = ReadFile(meminfoFd_);
    if (readsize == RET_FAIL) {
        return;
    }
    BufferSplitter totalbuffer((const char*)buffer_.get(), readsize);

    do {
        if (!totalbuffer.NextWord(':')) {
            continue;
        }
        const_cast<char *>(totalbuffer.CurWord())[totalbuffer.CurWordSize()] = '\0';
        auto it = meminfoCounters_.find(totalbuffer.CurWord());
        if (it == meminfoCounters_.end()) {
            continue;
        }

        int counter_id = it->second;
        if (!totalbuffer.NextWord(' ')) {
            continue;
        }
        auto value = static_cast<uint64_t>(strtoll(totalbuffer.CurWord(), nullptr, DEC_BASE));
        auto* meminfo = data.add_meminfo();

        meminfo->set_key(static_cast<SysMeminfoType>(counter_id));
        meminfo->set_value(value);
    } while (totalbuffer.NextLine());

    return;
}

void MemoryDataPlugin::WriteZramData(MemoryData& data)
{
    uint64_t zramSum = 0;
    for (int i = 0; i < MAX_ZRAM_DEVICES; i++) {
        std::string path = "/sys/block/zram" + std::to_string(i);
        if (access(path.c_str(), F_OK) == 0) {
            uint64_t zramValue = 0;
            std::string file = path + "/mm_stat";
            auto fptr = std::unique_ptr<FILE, decltype(&fclose)>{fopen(file.c_str(), "rb"), fclose};
            if (fptr != nullptr) {
                int ret = fscanf_s(fptr.get(), "%*" PRIu64 " %*" PRIu64 " %" PRIu64, &zramValue);
                if (ret != 1) {
                    file = path + "/mem_used_total";
                    std::string content = ReadFile(file);
                    char* end = nullptr;
                    uint64_t value = strtoull(content.c_str(), &end, DEC_BASE);
                    zramValue = (value > 0) ? value : 0;
                }
            }

            zramSum += zramValue;
        }
    }

    data.set_zram(zramSum / ZRAM_KB);
}

void MemoryDataPlugin::WriteVmstat(MemoryData& data)
{
    int readsize = ReadFile(vmstatFd_);
    if (readsize == RET_FAIL) {
        return;
    }
    BufferSplitter totalbuffer((const char*)buffer_.get(), readsize);

    do {
        if (!totalbuffer.NextWord(' ')) {
            continue;
        }
        const_cast<char *>(totalbuffer.CurWord())[totalbuffer.CurWordSize()] = '\0';
        auto it = vmstatCounters_.find(totalbuffer.CurWord());
        if (it == vmstatCounters_.end()) {
            continue;
        }

        int counter_id = it->second;
        char* valuestr = const_cast<char *>(totalbuffer.CurWord() + totalbuffer.CurWordSize() + 1);
        valuestr[totalbuffer.CurLineSize() - (valuestr - totalbuffer.CurLine())] = '\0';

        auto value = static_cast<uint64_t>(strtoll(valuestr, nullptr, DEC_BASE));
        auto* vmeminfo = data.add_vmeminfo();

        vmeminfo->set_key(static_cast<SysVMeminfoType>(counter_id));
        vmeminfo->set_value(value);
    } while (totalbuffer.NextLine());

    return;
}

void MemoryDataPlugin::WriteAppsummary(ProcessMemoryInfo* processinfo, SmapsStats& smapInfo)
{
    processinfo->mutable_memsummary()->set_java_heap(smapInfo.GetProcessJavaHeap());
    processinfo->mutable_memsummary()->set_native_heap(smapInfo.GetProcessNativeHeap());
    processinfo->mutable_memsummary()->set_code(smapInfo.GetProcessCode());
    processinfo->mutable_memsummary()->set_stack(smapInfo.GetProcessStack());
    processinfo->mutable_memsummary()->set_graphics(smapInfo.GetProcessGraphics());
    processinfo->mutable_memsummary()->set_private_other(smapInfo.GetProcessPrivateOther());
    processinfo->mutable_memsummary()->set_system(smapInfo.GetProcessSystem());
}

int MemoryDataPlugin::ParseNumber(std::string line)
{
    return atoi(line.substr(line.find_first_of("01234567890")).c_str());
}

bool MemoryDataPlugin::ParseMemInfo(const char* data, ProcessMemoryInfo* memoryInfo)
{
    bool ready = false;
    bool done = false;
    std::istringstream ss(data);
    std::string line;

    while (std::getline(ss, line)) {
        std::string s(line);
        if (s.find("App Summary") != s.npos) {
            ready = true;
            continue;
        }

        if (ready) {
            if (s.find("Java Heap:") != s.npos) {
                memoryInfo->mutable_memsummary()->set_java_heap(ParseNumber(s));
                continue;
            }
            if (s.find("Native Heap:") != s.npos) {
                memoryInfo->mutable_memsummary()->set_native_heap(ParseNumber(s));
                continue;
            }
            if (s.find("Code:") != s.npos) {
                memoryInfo->mutable_memsummary()->set_code(ParseNumber(s));
                continue;
            }
            if (s.find("Stack:") != s.npos) {
                memoryInfo->mutable_memsummary()->set_stack(ParseNumber(s));
                continue;
            }
            if (s.find("Graphics:") != s.npos) {
                memoryInfo->mutable_memsummary()->set_graphics(ParseNumber(s));
                continue;
            }
            if (s.find("Private Other:") != s.npos) {
                memoryInfo->mutable_memsummary()->set_private_other(ParseNumber(s));
                continue;
            }
            if (s.find("System:") != s.npos) {
                memoryInfo->mutable_memsummary()->set_system(ParseNumber(s));
                done = true;
                break;
            }
        }
    }
    return done;
}

bool MemoryDataPlugin::GetMemInfoByMemoryService(uint32_t pid, ProcessMemoryInfo* memoryInfo)
{
    std::string fullCmd = CMD_FORMAT + std::to_string(pid);

    std::unique_ptr<uint8_t[]> buffer {new (std::nothrow) uint8_t[BUF_MAX_LEN]};
    std::unique_ptr<FILE, int (*)(FILE*)> fp(popen(fullCmd.c_str(), "r"), pclose);
    CHECK_TRUE(fp, false, "%s:popen error", __func__);

    size_t ret = fread(buffer.get(), 1, BUF_MAX_LEN, fp.get());
    if (ret == 0) {
        HILOG_ERROR(LOG_CORE, "%s:fread failed", __func__);
    }
    buffer.get()[BUF_MAX_LEN - 1] = '\0';

    return ParseMemInfo(reinterpret_cast<char*>(buffer.get()), memoryInfo);
}

int MemoryDataPlugin::Report(uint8_t* data, uint32_t dataSize)
{
    MemoryData dataProto;
    uint32_t length;

    if (protoConfig_.report_process_tree()) {
        WriteProcesseList(dataProto);
    }

    if (protoConfig_.report_sysmem_mem_info()) {
        WriteMeminfo(dataProto);
        WriteZramData(dataProto);
    }

    if (protoConfig_.report_sysmem_vmem_info()) {
        WriteVmstat(dataProto);
    }

    for (int i = 0; i < protoConfig_.pid().size(); i++) {
        int32_t pid = protoConfig_.pid(i);
        auto* processinfo = dataProto.add_processesinfo();
        if (protoConfig_.report_process_mem_info()) {
            WriteProcinfoByPidfds(processinfo, pid);
        }

        bool isReportApp = protoConfig_.report_app_mem_info() && !protoConfig_.report_app_mem_by_memory_service();
        bool isReportSmaps = protoConfig_.report_smaps_mem_info();
        if (i == 0 && (isReportApp || isReportSmaps)) {
            SmapsStats smapInfo;
            smapInfo.ParseMaps(pid, *processinfo, isReportApp, isReportSmaps);
            if (isReportApp) {
                WriteAppsummary(processinfo, smapInfo);
            }
        }
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

int MemoryDataPlugin::Stop()
{
    if (meminfoFd_ > 0) {
        close(meminfoFd_);
        meminfoFd_ = -1;
    }
    if (vmstatFd_ > 0) {
        close(vmstatFd_);
        vmstatFd_ = -1;
    }
    for (auto it = pidFds_.begin(); it != pidFds_.end(); it++) {
        for (int i = FILE_STATUS; i <= FILE_SMAPS; i++) {
            if (it->second[i] != -1) {
                close(it->second[i]);
                it->second[i] = -1;
            }
        }
    }
    HILOG_INFO(LOG_CORE, "%s:stop success!", __func__);
    return 0;
}

void MemoryDataPlugin::WriteProcinfoByPidfds(ProcessMemoryInfo* processinfo, int32_t pid)
{
    char* end = nullptr;
    int32_t readSize;

    readSize = ReadFile(pidFds_[pid][FILE_STATUS]);
    if (readSize != RET_FAIL) {
        WriteProcess(processinfo, (char*)buffer_.get(), readSize, pid);
    }

    if (ReadFile(pidFds_[pid][FILE_OOM]) != RET_FAIL) {
        processinfo->set_oom_score_adj(static_cast<int64_t>(strtol((char*)buffer_.get(), &end, DEC_BASE)));
    } else {
        processinfo->set_oom_score_adj(0);
    }
    return;
}

int32_t MemoryDataPlugin::ReadFile(int fd)
{
    if ((buffer_.get() == nullptr) || (fd == -1)) {
        return RET_FAIL;
    }
    int readsize = pread(fd, buffer_.get(), READ_BUFFER_SIZE - 1, 0);
    if (readsize <= 0) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "%s:failed to read(%d), errno(%d:%s)", __func__, fd, errno, buf);
        err_ = errno;
        return RET_FAIL;
    }
    return readsize;
}

std::string MemoryDataPlugin::ReadFile(const std::string& path)
{
    char realPath[PATH_MAX] = {0};
    CHECK_TRUE((path.length() < PATH_MAX) && (realpath(path.c_str(), realPath) != nullptr), "",
               "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
    int fd = open(realPath, O_RDONLY);
    if (fd == -1) {
        const int maxSize = 256;
        char buf[maxSize] = { 0 };
        strerror_r(errno, buf, maxSize);
        HILOG_WARN(LOG_CORE, "open file %s FAILED: %s!", path.c_str(), buf);
        return "";
    }

    std::string content;
    size_t count = 0;
    while (true) {
        if (content.size() - count < DEFAULT_READ_SIZE) {
            content.resize(content.size() + DEFAULT_READ_SIZE);
        }
        ssize_t nBytes = read(fd, &content[count], content.size() - count);
        if (nBytes <= 0) {
            break;
        }
        count += static_cast<size_t>(nBytes);
    }
    content.resize(count);
    CHECK_TRUE(close(fd) != -1, content, "close %s failed, %d", path.c_str(), errno);
    return content;
}

std::vector<int> MemoryDataPlugin::OpenProcPidFiles(int32_t pid)
{
    char fileName[PATH_MAX + 1] = {0};
    char realPath[PATH_MAX + 1] = {0};
    int count = sizeof(procfdMapping) / sizeof(procfdMapping[0]);
    std::vector<int> profds;

    for (int i = 0; i < count; i++) {
        if (snprintf_s(fileName, sizeof(fileName), sizeof(fileName) - 1,
            "%s/%d/%s", testpath_, pid, procfdMapping[i].file) < 0) {
            HILOG_ERROR(LOG_CORE, "%s:snprintf_s error", __func__);
        }
        if (realpath(fileName, realPath) == nullptr) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "%s:realpath failed, errno(%d:%s)", __func__, errno, buf);
        }
        int fd = open(realPath, O_RDONLY | O_CLOEXEC);
        if (fd == -1) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, fileName, errno, buf);
        }
        profds.emplace(profds.begin() + i, fd);
    }
    return profds;
}

DIR* MemoryDataPlugin::OpenDestDir(const char* dirPath)
{
    DIR* destDir = nullptr;

    destDir = opendir(dirPath);
    if (destDir == nullptr) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "%s:failed to opendir(%s), errno(%d:%s)", __func__, dirPath, errno, buf);
    }

    return destDir;
}

int32_t MemoryDataPlugin::GetValidPid(DIR* dirp)
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

int32_t MemoryDataPlugin::ReadProcPidFile(int32_t pid, const char* pFileName)
{
    char fileName[PATH_MAX + 1] = {0};
    char realPath[PATH_MAX + 1] = {0};
    int fd = -1;
    ssize_t bytesRead = 0;
    CHECK_TRUE(snprintf_s(fileName, sizeof(fileName), sizeof(fileName) - 1, "%s/%d/%s", testpath_, pid, pFileName) >= 0,
               RET_FAIL, "%s:snprintf_s error", __func__);
    if (realpath(fileName, realPath) == nullptr) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "%s:realpath failed, errno(%d:%s)", __func__, errno, buf);
        return RET_FAIL;
    }
    fd = open(realPath, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_INFO(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, fileName, errno, buf);
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
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_INFO(LOG_CORE, "%s:failed to read(%s), errno(%d:%s)", __func__, fileName, errno, buf);
        err_ = errno;
        return RET_FAIL;
    }
    buffer_.get()[bytesRead] = '\0';
    close(fd);

    return bytesRead;
}

bool MemoryDataPlugin::BufnCmp(const char* src, int srcLen, const char* key, int keyLen)
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

bool MemoryDataPlugin::addPidBySort(int32_t pid)
{
    auto pidsEnd = seenPids_.end();
    auto it = std::lower_bound(seenPids_.begin(), pidsEnd, pid);
    if (it != pidsEnd && *it == pid) {
        return false;
    }
    it = seenPids_.insert(it, std::move(pid));
    return true;
}

int MemoryDataPlugin::GetProcStatusId(const char* src, int srcLen)
{
    int count = sizeof(procStatusMapping) / sizeof(procStatusMapping[0]);
    for (int i = 0; i < count; i++) {
        if (BufnCmp(src, srcLen, procStatusMapping[i].procstr, strlen(procStatusMapping[i].procstr))) {
            return procStatusMapping[i].procid;
        }
    }
    return RET_FAIL;
}

bool MemoryDataPlugin::StringToUll(const char* word, uint64_t& value)
{
    char* end = nullptr;
    errno = 0;
    value = strtoull(word, &end, DEC_BASE);
    if ((errno == ERANGE && (value == ULLONG_MAX)) || (errno != 0 && value == 0)) {
        return false;
    } else if (end == word && (*word >= '0' && *word <= '9')) {
        return false;
    }

    return true;
}

void MemoryDataPlugin::SetProcessInfo(ProcessMemoryInfo* processinfo, int key, const char* word)
{
    uint64_t value;

    if ((key >= PRO_TGID && key <= PRO_VMHWM && key != PRO_NAME) && !StringToUll(word, value)) {
        HILOG_ERROR(LOG_CORE, "MemoryDataPlugin:%s, strtoull failed, key(%d), word(%s)", __func__, key, word);
        return;
    }

    switch (key) {
        case PRO_TGID:
            processinfo->set_pid(static_cast<int32_t>(value));
            break;
        case PRO_VMSIZE:
            processinfo->set_vm_size_kb(value);
            break;
        case PRO_VMRSS:
            processinfo->set_vm_rss_kb(value);
            break;
        case PRO_RSSANON:
            processinfo->set_rss_anon_kb(value);
            break;
        case PRO_RSSFILE:
            processinfo->set_rss_file_kb(value);
            break;
        case PRO_RSSSHMEM:
            processinfo->set_rss_shmem_kb(value);
            break;
        case PRO_VMSWAP:
            processinfo->set_vm_swap_kb(value);
            break;
        case PRO_VMLCK:
            processinfo->set_vm_locked_kb(value);
            break;
        case PRO_VMHWM:
            processinfo->set_vm_hwm_kb(value);
            break;
        default:
            break;
    }
    return;
}

void MemoryDataPlugin::WriteProcess(ProcessMemoryInfo* processinfo, const char* pFile, uint32_t fileLen, int32_t pid)
{
    BufferSplitter totalbuffer(const_cast<const char*>(pFile), fileLen + 1);

    do {
        totalbuffer.NextWord(':');
        if (!totalbuffer.CurWord()) {
            return;
        }

        int key = GetProcStatusId(totalbuffer.CurWord(), totalbuffer.CurWordSize());
        totalbuffer.NextWord('\n');
        if (!totalbuffer.CurWord()) {
            continue;
        }
        if (key == PRO_NAME) {
            processinfo->set_name(totalbuffer.CurWord(), totalbuffer.CurWordSize());
        }
        SetProcessInfo(processinfo, key, totalbuffer.CurWord());
    } while (totalbuffer.NextLine());
    // update process name
    int32_t ret = ReadProcPidFile(pid, "cmdline");
    if (ret > 0) {
        processinfo->set_name(reinterpret_cast<char*>(buffer_.get()), strlen(reinterpret_cast<char*>(buffer_.get())));
    }
}

void MemoryDataPlugin::WriteOomInfo(ProcessMemoryInfo* processinfo, int32_t pid)
{
    char* end = nullptr;

    if (ReadProcPidFile(pid, "oom_score_adj") == RET_FAIL) {
        processinfo->set_oom_score_adj(0);
        return;
    }
    if (buffer_.get() == nullptr) {
        processinfo->set_oom_score_adj(0);
        HILOG_ERROR(LOG_CORE, "%s:invalid params, read buffer_ is NULL", __func__);
        return;
    }
    processinfo->set_oom_score_adj(static_cast<int64_t>(strtol((char*)buffer_.get(), &end, DEC_BASE)));
}

void MemoryDataPlugin::WriteProcessInfo(MemoryData& data, int32_t pid)
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
    WriteOomInfo(processinfo, pid);
}

void MemoryDataPlugin::WriteProcesseList(MemoryData& data)
{
    DIR* procDir = nullptr;

    procDir = OpenDestDir(testpath_);
    if (procDir == nullptr) {
        return;
    }

    seenPids_.clear();
    while (int32_t pid = GetValidPid(procDir)) {
        addPidBySort(pid);
    }

    for (unsigned int i = 0; i < seenPids_.size(); i++) {
        WriteProcessInfo(data, seenPids_[i]);
    }
    closedir(procDir);
}
