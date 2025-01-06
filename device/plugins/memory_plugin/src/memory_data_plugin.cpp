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

#include <cmath>
#include <sstream>

#include "memory_plugin_result.pbencoder.h"
#include "securec.h"
#include "smaps_stats.h"

namespace {
using namespace OHOS::HDI::Memorytracker::V1_0;
using namespace OHOS::Developtools::Profiler;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;
using OHOS::HiviewDFX::CollectResult;
using OHOS::HiviewDFX::GraphicType;

const char* CMD_FORMAT = "memory service meminfo --local ";
constexpr size_t READ_BUFFER_SIZE = 1024 * 16;
constexpr int BUF_MAX_LEN = 2048;
constexpr int MAX_ZRAM_DEVICES = 256;
constexpr int ZRAM_KB = 1024;
constexpr size_t DEFAULT_READ_SIZE = 4096;
const std::string FAKE_DATA_PATH = "/data/local/tmp";
constexpr int DATA_START_LINES = 3;
constexpr size_t PAGE_SIZE = 4096;
constexpr size_t KB_TO_BYTES = 1024;
constexpr size_t MB_TO_BYTES = 1024 * 1024;
constexpr int INDENT_CATEGORY_NUM = 2;
constexpr int INDENT_SUB_TYPE_NUM = 4;
const std::string TOTAL_DMA_STR = "Total dma"; // flag for total of DMA memory size
const std::string RS_IMAGE_CACHE_START_STR = "RSImageCache:"; // flag for start RSImageCache data
const std::string RS_IMAGE_CACHE_END_STR = "  pixelmap:"; // flag for end RSImageCache data
const std::string TOTAL_CPU_STR = "Total CPU memory usage"; // flag for total of CPU memory size
const std::string SKIA_GPU_STR = "Skia GPU Caches"; // flag for GPU
const std::string GPU_LIMIT_STR = "gpu limit"; // flag for gpu limit size
const std::string RENDER_SERVICE_NAME = "render_service";
const std::string MGR_SVC_START_STR = "----------------------------------WindowManagerService------------";
const std::string MGR_SVC_END_STR = "Focus window";
const std::string MGR_SVC_INTERVAL_STR = "----------------------------------------------------------";
const std::string MEM_PROFILE_STR = "Channel:";
} // namespace

MemoryDataPlugin::MemoryDataPlugin() : meminfoFd_(-1), vmstatFd_(-1), err_(-1)
{
    InitProto2StrVector();
    SetPath(const_cast<char*>("/proc"));
    buffer_ = std::make_unique<uint8_t[]>(READ_BUFFER_SIZE);
}

MemoryDataPlugin::~MemoryDataPlugin()
{
    PROFILER_LOG_INFO(LOG_CORE, "%s:~MemoryDataPlugin!", __func__);

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
            PROFILER_LOG_ERROR(LOG_CORE, "%s:realpath failed, errno(%d:%s)", __func__, errno, buf);
            return RET_FAIL;
        }
        meminfoFd_ = open(realPath, O_RDONLY | O_CLOEXEC);
        if (meminfoFd_ == -1) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "%s:open failed, fileName, errno(%d:%s)", __func__, errno, buf);
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
            PROFILER_LOG_ERROR(LOG_CORE, "%s:realpath failed, errno(%d:%s)", __func__, errno, buf);
            return RET_FAIL;
        }
        vmstatFd_ = open(realPath, O_RDONLY | O_CLOEXEC);
        if (vmstatFd_ == -1) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to open(/proc/vmstat), errno(%d:%s)", __func__, errno, buf);
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

    PROFILER_LOG_INFO(LOG_CORE, "%s:start success!", __func__);
    return RET_SUCC;
}

template <typename T> void MemoryDataPlugin::WriteMeminfo(T& memoryData)
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
        auto* meminfo = memoryData.add_meminfo();

        meminfo->set_key(static_cast<SysMeminfoType>(counter_id));
        meminfo->set_value(value);
    } while (totalbuffer.NextLine());

    return;
}

template <typename T> void MemoryDataPlugin::WriteZramData(T& memoryData)
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

    memoryData.set_zram(zramSum / ZRAM_KB);
}

template <typename T> void MemoryDataPlugin::WriteVmstat(T& memoryData)
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
        auto* vmeminfo = memoryData.add_vmeminfo();

        vmeminfo->set_key(static_cast<SysVMeminfoType>(counter_id));
        vmeminfo->set_value(value);
    } while (totalbuffer.NextLine());

    return;
}

template <typename T> void MemoryDataPlugin::WriteAppsummary(T& processMemoryInfo, SmapsStats& smapInfo)
{
    auto* memsummary = processMemoryInfo.mutable_memsummary();
    memsummary->set_java_heap(smapInfo.GetProcessJavaHeap());
    memsummary->set_native_heap(smapInfo.GetProcessNativeHeap());
    memsummary->set_code(smapInfo.GetProcessCode());
    memsummary->set_stack(smapInfo.GetProcessStack());
    memsummary->set_graphics(smapInfo.GetProcessGraphics());
    memsummary->set_private_other(smapInfo.GetProcessPrivateOther());
    memsummary->set_system(smapInfo.GetProcessSystem());
}

int MemoryDataPlugin::ParseNumber(std::string line)
{
    return atoi(line.substr(line.find_first_of("01234567890")).c_str());
}

template <typename T> bool MemoryDataPlugin::GetMemInfoByMemoryService(uint32_t pid, T& processMemoryInfo)
{
    std::string fullCmd = CMD_FORMAT + std::to_string(pid);

    std::unique_ptr<uint8_t[]> buffer {new (std::nothrow) uint8_t[BUF_MAX_LEN]};
    std::unique_ptr<FILE, int (*)(FILE*)> fp(popen(fullCmd.c_str(), "r"), pclose);
    CHECK_TRUE(fp, false, "%s:popen error", __func__);

    size_t ret = fread(buffer.get(), 1, BUF_MAX_LEN, fp.get());
    if (ret == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:fread failed", __func__);
    }
    buffer.get()[BUF_MAX_LEN - 1] = '\0';

    return ParseMemInfo(reinterpret_cast<char*>(buffer.get()), processMemoryInfo);
}

template <typename T, typename S> void MemoryDataPlugin::WriteMemoryData(T& memoryDataProto, S smapsInfo)
{
    if (protoConfig_.report_process_tree()) {
        WriteProcesseList(memoryDataProto);
    }

    if (protoConfig_.report_sysmem_mem_info()) {
        WriteMeminfo(memoryDataProto);
        WriteZramData(memoryDataProto);
    }

    if (protoConfig_.report_sysmem_vmem_info()) {
        WriteVmstat(memoryDataProto);
    }

    for (int i = 0; i < protoConfig_.pid().size(); i++) {
        int32_t pid = protoConfig_.pid(i);
        auto* processinfo = memoryDataProto.add_processesinfo();
        if (protoConfig_.report_process_mem_info()) {
            WriteProcinfoByPidfds(*processinfo, pid);
        }

        bool isReportApp = protoConfig_.report_app_mem_info() && !protoConfig_.report_app_mem_by_memory_service();
        bool isReportSmaps = protoConfig_.report_smaps_mem_info();
        if (i == 0 && (isReportApp || isReportSmaps)) {
            SmapsStats smapInfo;
            smapInfo.ParseMaps(pid, *processinfo, smapsInfo, isReportApp, isReportSmaps);
            if (isReportApp) {
                WriteAppsummary(*processinfo, smapInfo);
            }
        }
    }

    if (protoConfig_.report_purgeable_ashmem_info()) {
        WriteAshmemInfo(memoryDataProto);
    }

    if (protoConfig_.report_dma_mem_info()) {
        WriteDmaInfo(memoryDataProto);
    }

    if (protoConfig_.report_gpu_mem_info()) {
        WriteGpuMemInfo(memoryDataProto);
    }

    WriteDumpProcessInfo(memoryDataProto);

    if (protoConfig_.report_gpu_dump_info()) {
        WriteGpuDumpInfo(memoryDataProto);
        WriteManagerServiceInfo(memoryDataProto);
        WriteProfileMemInfo(memoryDataProto);
    }
}

int MemoryDataPlugin::ReportOptimize(RandomWriteCtx* randomWrite)
{
    ProtoEncoder::MemoryData dataProto(randomWrite);
    ProtoEncoder::SmapsInfo* smapsInfo = nullptr;
    WriteMemoryData(dataProto, smapsInfo);

    int msgSize = dataProto.Finish();
    return msgSize;
}

int MemoryDataPlugin::Report(uint8_t* data, uint32_t dataSize)
{
    MemoryData dataProto;
    SmapsInfo* smapsInfo = nullptr;
    WriteMemoryData(dataProto, smapsInfo);

    uint32_t length = dataProto.ByteSizeLong();
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
    PROFILER_LOG_INFO(LOG_CORE, "%s:stop success!", __func__);
    return 0;
}

template <typename T> void MemoryDataPlugin::WriteProcinfoByPidfds(T& processMemoryInfo, int32_t pid)
{
    char* end = nullptr;
    int32_t readSize;

    readSize = ReadFile(pidFds_[pid][FILE_STATUS]);
    if (readSize != RET_FAIL) {
        WriteProcess(processMemoryInfo, reinterpret_cast<char*>(buffer_.get()), readSize, pid);
    }

    if (ReadFile(pidFds_[pid][FILE_OOM]) != RET_FAIL) {
        processMemoryInfo.set_oom_score_adj(static_cast<int64_t>(strtol(reinterpret_cast<char*>(buffer_.get()),
                                                                        &end, DEC_BASE)));
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
        PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to read(%d), errno(%d:%s)", __func__, fd, errno, buf);
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
        PROFILER_LOG_WARN(LOG_CORE, "open file %s FAILED: %s!", path.c_str(), buf);
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
            PROFILER_LOG_ERROR(LOG_CORE, "%s:snprintf_s error", __func__);
        }
        if (realpath(fileName, realPath) == nullptr) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "%s:realpath failed, errno(%d:%s)", __func__, errno, buf);
        }
        int fd = open(realPath, O_RDONLY | O_CLOEXEC);
        if (fd == -1) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, fileName, errno, buf);
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
        PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to opendir(%s), errno(%d:%s)", __func__, dirPath, errno, buf);
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
        PROFILER_LOG_ERROR(LOG_CORE, "%s:realpath failed, errno(%d:%s)", __func__, errno, buf);
        return RET_FAIL;
    }
    fd = open(realPath, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_INFO(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, fileName, errno, buf);
        err_ = errno;
        return RET_FAIL;
    }
    if (buffer_.get() == nullptr) {
        PROFILER_LOG_INFO(LOG_CORE, "%s:empty address, buffer_ is NULL", __func__);
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
        PROFILER_LOG_INFO(LOG_CORE, "%s:failed to read(%s), errno(%d:%s)", __func__, fileName, errno, buf);
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

template <typename T> void MemoryDataPlugin::SetProcessInfo(T& processMemoryInfo, int key, const char* word)
{
    uint64_t value;

    if ((key >= PRO_TGID && key <= PRO_PURGPIN && key != PRO_NAME) && !StringToUll(word, value)) {
        PROFILER_LOG_ERROR(LOG_CORE, "MemoryDataPlugin:%s, strtoull failed, key(%d), word(%s)", __func__, key, word);
        return;
    }

    switch (key) {
        case PRO_TGID:
            processMemoryInfo.set_pid(static_cast<int32_t>(value));
            break;
        case PRO_VMSIZE:
            processMemoryInfo.set_vm_size_kb(value);
            break;
        case PRO_VMRSS:
            processMemoryInfo.set_vm_rss_kb(value);
            break;
        case PRO_RSSANON:
            processMemoryInfo.set_rss_anon_kb(value);
            break;
        case PRO_RSSFILE:
            processMemoryInfo.set_rss_file_kb(value);
            break;
        case PRO_RSSSHMEM:
            processMemoryInfo.set_rss_shmem_kb(value);
            break;
        case PRO_VMSWAP:
            processMemoryInfo.set_vm_swap_kb(value);
            break;
        case PRO_VMLCK:
            processMemoryInfo.set_vm_locked_kb(value);
            break;
        case PRO_VMHWM:
            processMemoryInfo.set_vm_hwm_kb(value);
            break;
        case PRO_PURGSUM:
            processMemoryInfo.set_purg_sum_kb(value);
            break;
        case PRO_PURGPIN:
            processMemoryInfo.set_purg_pin_kb(value);
            break;
        default:
            break;
    }
    return;
}

template <typename T>
void MemoryDataPlugin::WriteProcess(T& processMemoryInfo, const char* pFile, uint32_t fileLen, int32_t pid)
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
            processMemoryInfo.set_name(totalbuffer.CurWord(), totalbuffer.CurWordSize());
        }
        SetProcessInfo(processMemoryInfo, key, totalbuffer.CurWord());
    } while (totalbuffer.NextLine());
    // update process name
    int32_t ret = ReadProcPidFile(pid, "cmdline");
    if (ret > 0) {
        processMemoryInfo.set_name(reinterpret_cast<char*>(buffer_.get()),
                                   strlen(reinterpret_cast<char*>(buffer_.get())));
    }
}

template <typename T> void MemoryDataPlugin::WriteOomInfo(T& processMemoryInfo, int32_t pid)
{
    char* end = nullptr;

    if (ReadProcPidFile(pid, "oom_score_adj") == RET_FAIL) {
        return;
    }
    if (buffer_.get() == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:invalid params, read buffer_ is NULL", __func__);
        return;
    }
    processMemoryInfo.set_oom_score_adj(static_cast<int64_t>(strtol(reinterpret_cast<char*>(buffer_.get()),
                                                                    &end, DEC_BASE)));
}

template <typename T> void MemoryDataPlugin::WriteProcessInfo(T& memoryData, int32_t pid)
{
    int32_t ret = ReadProcPidFile(pid, "status");
    if (ret == RET_FAIL) {
        return;
    }
    if ((buffer_.get() == nullptr) || (ret == 0)) {
        return;
    }
    auto* processinfo = memoryData.add_processesinfo();
    WriteProcess(*processinfo, reinterpret_cast<char*>(buffer_.get()), ret, pid);
    WriteOomInfo(*processinfo, pid);
}

template <typename T> void MemoryDataPlugin::WriteProcesseList(T& memoryData)
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
        WriteProcessInfo(memoryData, seenPids_[i]);
    }
    closedir(procDir);
}

template <typename T> void MemoryDataPlugin::SetAshmemInfo(T& ashmemInfo, int key, const char* word)
{
    int64_t value = 0;
    int64_t size = 0;
    switch (key) {
        case ASHMEM_PROCESS_NAME:
            ashmemInfo.set_name(word);
            break;
        case ASHMEM_PID:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value > 0, NO_RETVAL, "%s:strtoull pid failed", __func__);
            ashmemInfo.set_pid(value);
            break;
        case ASHMEM_FD:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value >= 0, NO_RETVAL, "%s:strtoull fd failed", __func__);
            ashmemInfo.set_fd(value);
            break;
        case ASHMEM_ADJ:
            value = static_cast<int64_t>(strtoull(word, nullptr, DEC_BASE));
            CHECK_TRUE(value >= 0, NO_RETVAL, "%s:strtoull adj failed", __func__);
            ashmemInfo.set_adj(value);
            break;
        case ASHMEM_NAME:
            ashmemInfo.set_ashmem_name(word);
            break;
        case ASHMEM_SIZE:
            size = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(size >= 0, NO_RETVAL, "%s:strtoull size failed", __func__);
            ashmemInfo.set_size(size);
            break;
        case ASHMEM_ID:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value >= 0, NO_RETVAL, "%s:strtoull id failed", __func__);
            ashmemInfo.set_id(value);
            break;
        case ASHMEM_TIME:
            size = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(size >= 0, NO_RETVAL, "%s:strtoull time failed", __func__);
            ashmemInfo.set_time(size);
            break;
        case ASHMEM_REF_COUNT:
            size = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(size >= 0, NO_RETVAL, "%s:strtoull ref_count failed", __func__);
            ashmemInfo.set_ref_count(size);
            break;
        case ASHMEM_PURGED:
            size = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(size >= 0, NO_RETVAL, "%s:strtoull purged failed", __func__);
            ashmemInfo.set_purged(size);
            break;
        default:
            break;
    }
}

template <typename T> void MemoryDataPlugin::WriteAshmemInfo(T& dataProto)
{
    std::string path = protoConfig_.report_fake_data() ? FAKE_DATA_PATH : std::string(testpath_);
    std::string file = path + "/purgeable_ashmem_trigger";
    std::ifstream input(file, std::ios::in);
    if (input.fail()) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:open %s failed, errno = %d", __func__, file.c_str(), errno);
        return;
    }

    int lines = 0;
    do {
        if (!input.good()) {
            return;
        }

        std::string line;
        getline(input, line);
        line += '\n';
        if (++lines <= DATA_START_LINES) {
            // The first three lines are not data.
            continue;
        }

        BufferSplitter totalBuffer(static_cast<const char*>(line.c_str()), line.size() + 1);
        if (!totalBuffer.NextWord(',')) {
            break;
        }
        auto* ashmemInfo = dataProto.add_ashmeminfo();
        for (int i = ASHMEM_PROCESS_NAME; i <= ASHMEM_PURGED; i++) {
            std::string curWord = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
            SetAshmemInfo(*ashmemInfo, i, curWord.c_str());
            char delimiter = (i == ASHMEM_REF_COUNT) ? '\n' : ',';
            if (!totalBuffer.NextWord(delimiter)) {
                break;
            }
        }
    } while (!input.eof());
    input.close();
}

template <typename T> void MemoryDataPlugin::SetDmaInfo(T& dmaInfo, int key, const char* word)
{
    int64_t value = 0;
    int64_t size = 0;
    switch (key) {
        case DMA_NAME:
            dmaInfo.set_name(word);
            break;
        case DMA_PID:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value > 0, NO_RETVAL, "%s:strtoull pid failed", __func__);
            dmaInfo.set_pid(value);
            break;
        case DMA_FD:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value >= 0, NO_RETVAL, "%s:strtoull fd failed", __func__);
            dmaInfo.set_fd(value);
            break;
        case DMA_SIZE:
            size = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(size >= 0, NO_RETVAL, "%s:strtoull size failed", __func__);
            dmaInfo.set_size(size);
            break;
        case DMA_INO:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value >= 0, NO_RETVAL, "%s:strtoull magic failed", __func__);
            dmaInfo.set_ino(value);
            break;
        case DMA_EXP_PID:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value > 0, NO_RETVAL, "%s:strtoull exp_pid failed", __func__);
            dmaInfo.set_exp_pid(value);
            break;
        case DMA_EXP_TASK_COMM:
            dmaInfo.set_exp_task_comm(word);
            break;
        case DMA_BUF_NAME:
            dmaInfo.set_buf_name(word);
            break;
        case DMA_EXP_NAME:
            if (*(word + strlen(word) - 1) == '\r') {
                dmaInfo.set_exp_name(std::string(word, strlen(word) - 1));
            } else {
                dmaInfo.set_exp_name(word);
            }
            break;
        default:
            break;
    }
}

template <typename T> void MemoryDataPlugin::WriteDmaInfo(T& dataProto)
{
    std::string file = std::string(testpath_) + "/process_dmabuf_info";
    std::ifstream input(file, std::ios::in);
    if (input.fail()) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:open %s failed, errno = %d", __func__, file.c_str(), errno);
        return;
    }

    int lines = 0;
    do {
        if (!input.good()) {
            return;
        }

        std::string line;
        getline(input, line);
        line += '\n';
        if (++lines < DATA_START_LINES
            || strncmp(line.c_str(), TOTAL_DMA_STR.c_str(), TOTAL_DMA_STR.size()) == 0) {
            continue; // not data.
        }

        BufferSplitter totalBuffer(static_cast<const char*>(line.c_str()), line.size() + 1);
        if (!totalBuffer.NextWord(' ')) {
            break;
        }
        auto* dmaInfo = dataProto.add_dmainfo();
        for (int i = DMA_NAME; i <= DMA_EXP_NAME; i++) {
            std::string curWord = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
            SetDmaInfo(*dmaInfo, i, curWord.c_str());
            char delimiter = (i == DMA_BUF_NAME) ? '\n' : ' ';
            if (!totalBuffer.NextWord(delimiter)) {
                break;
            }
        }
    } while (!input.eof());
    input.close();
}

template <typename T> void MemoryDataPlugin::SetGpuProcessInfo(T& gpuProcessInfo, int key, const char* word)
{
    int64_t value = 0;
    int64_t size = 0;
    switch (key) {
        case GPU_ADDR:
            gpuProcessInfo.set_addr(word);
            break;
        case GPU_TID:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value > 0, NO_RETVAL, "%s:strtoull tid failed", __func__);
            gpuProcessInfo.set_tid(value);
            break;
        case GPU_PID:
            value = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(value > 0, NO_RETVAL, "%s:strtoull pid failed", __func__);
            gpuProcessInfo.set_pid(value);
            break;
        case GPU_USED_SIZE:
            size = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(size >= 0, NO_RETVAL, "%s:strtoull used_gpu_size failed", __func__);
            gpuProcessInfo.set_used_gpu_size(size * PAGE_SIZE);
            break;
        default:
            break;
    }
}

template <typename T> void MemoryDataPlugin::WriteGpuMemInfo(T& dataProto)
{
    std::string path = protoConfig_.report_fake_data() ? FAKE_DATA_PATH : std::string(testpath_);
    std::string file = path + "/gpu_memory";
    std::ifstream input(file, std::ios::in);
    if (input.fail()) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:open %s failed, errno = %d", __func__, file.c_str(), errno);
        return;
    }

    std::string line;
    getline(input, line);
    line += '\n';

    BufferSplitter totalBuffer(static_cast<const char*>(line.c_str()), line.size() + 1);
    auto* gpuMemoryInfo = dataProto.add_gpumemoryinfo();
    if (totalBuffer.NextWord(' ')) {
        gpuMemoryInfo->set_gpu_name(std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize()));
    }
    if (totalBuffer.NextWord('\n')) {
        gpuMemoryInfo->set_all_gpu_size((strtoull(totalBuffer.CurWord(), nullptr, DEC_BASE) * PAGE_SIZE));
    }

    do {
        if (!input.good()) {
            return;
        }

        getline(input, line);
        line += '\n';

        BufferSplitter buffer(static_cast<const char*>(line.c_str()), line.size() + 1);
        if (!buffer.NextWord(' ')) {
            break;
        }
        auto* gpuProcessInfo = gpuMemoryInfo->add_gpu_process_info();
        for (int i = GPU_ADDR; i <= GPU_USED_SIZE; i++) {
            std::string curWord = std::string(buffer.CurWord(), buffer.CurWordSize());
            SetGpuProcessInfo(*gpuProcessInfo, i, curWord.c_str());
            if (!buffer.NextWord(' ')) {
                break;
            }
        }
    } while (!input.eof());
    input.close();
}

std::string MemoryDataPlugin::RunCommand(const std::string& cmd)
{
    std::string ret = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:popen(%s) error", __func__, cmd.c_str());
        return ret;
    }

    std::array<char, READ_BUFFER_SIZE> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        ret += buffer.data();
    }
    return ret;
}

int MemoryDataPlugin::GetIndentNum(const std::string& line)
{
    int indentNum = 0;
    while (isspace(line[indentNum])) {
        indentNum++;
    }
    return indentNum;
}

uint64_t MemoryDataPlugin::SizeToBytes(const std::string& sizeStr, const std::string& type)
{
    auto size = std::atof(sizeStr.c_str());
    uint64_t byteSize = round(size);
    if (type == "KB") {
        byteSize = round(size * KB_TO_BYTES);
    } else if (type == "MB") {
        byteSize = round(size * MB_TO_BYTES);
    }
    return byteSize;
}

template <typename T> bool MemoryDataPlugin::SetGpuDumpInfo(T& gpuDumpInfo, BufferSplitter& totalBuffer)
{
    int ret = totalBuffer.NextLine();
    CHECK_TRUE(ret, false, "totalBuffer is end!");
    std::string line = std::string(totalBuffer.CurLine());
    int indentNum = GetIndentNum(line);
    while (indentNum > 0) {
        if (indentNum == INDENT_CATEGORY_NUM && totalBuffer.NextWord(':')) {
            auto* gpuDetailInfo = gpuDumpInfo.add_gpu_detail_info();
            gpuDetailInfo->set_module_name(std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize()));
            ret = totalBuffer.NextLine();
            CHECK_TRUE(ret, false, "totalBuffer is end!");
            line = std::string(totalBuffer.CurLine());
            indentNum = GetIndentNum(line);
            while (indentNum == INDENT_SUB_TYPE_NUM && totalBuffer.NextWord(':')) {
                auto* gpuSubInfo = gpuDetailInfo->add_gpu_sub_info();
                gpuSubInfo->set_category_name(std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize()));
                std::string size = "";
                if (totalBuffer.NextWord(' ')) {
                    size = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
                }
                if (totalBuffer.NextWord(' ')) {
                    std::string type = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
                    gpuSubInfo->set_size(SizeToBytes(size, type));
                }
                if (totalBuffer.NextWord(' ')) {
                    gpuSubInfo->set_entry_num(strtoull(totalBuffer.CurWord() + 1, nullptr, DEC_BASE));
                }
                ret = totalBuffer.NextLine();
                CHECK_TRUE(ret, false, "totalBuffer is end!");
                line = std::string(totalBuffer.CurLine());
                indentNum = GetIndentNum(line);
            }
        }
    }

    ret = totalBuffer.NextLine();
    CHECK_TRUE(ret, false, "totalBuffer is end!");
    std::string size = "";
    if (totalBuffer.NextWord('(') && totalBuffer.NextWord(' ')) {
        size = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
    }
    if (totalBuffer.NextWord(' ')) {
        std::string type = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
        gpuDumpInfo.set_gpu_purgeable_size(SizeToBytes(size, type));
    }
    return true;
}

template <typename T> void MemoryDataPlugin::SetRSImageDumpInfo(T& rsDumpInfo, int key, const char* word)
{
    int64_t pid = 0;
    int64_t size = 0;
    size_t dataLen = strlen(word) > 1 ? strlen(word) - 1 : 0;
    switch (key) {
        case RS_SIZE:
            size = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(size >= 0, NO_RETVAL, "%s:strtoull size failed", __func__);
            rsDumpInfo.set_size(size);
            break;
        case RS_TYPE:
            if (*(word + dataLen) == '\t') {
                rsDumpInfo.set_type(std::string(word, dataLen));
            } else {
                rsDumpInfo.set_type(word);
            }
            break;
        case RS_PID:
            pid = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(pid >= 0, NO_RETVAL, "%s:strtoull pid failed", __func__);
            rsDumpInfo.set_pid(pid);
            break;
        case RS_SURFACENAME:
            if (*(word + dataLen) == '\t') {
                rsDumpInfo.set_surface_name(std::string(word, dataLen));
            } else {
                rsDumpInfo.set_surface_name(word);
            }
            break;
        default:
            break;
    }
}

template <typename T> void MemoryDataPlugin::WriteGpuDumpInfo(T& dataProto)
{
    std::string content = "";
    if (!protoConfig_.report_fake_data()) {
        if (strcmp(testpath_, "/proc") == 0) {
            content = RunCommand("hidumper -s 10 '-a dumpMem'");
        } else {
            // for UT
            content = ReadFile(std::string(testpath_) + "/dumpMem.txt");
        }
    } else {
        content = ReadFile(FAKE_DATA_PATH + "/dumpMem.txt");
    }
    CHECK_TRUE(content != "", NO_RETVAL, "hidumper no data!");

    BufferSplitter totalBuffer((const char*)content.c_str(), content.size() + 1);
    do {
        std::string line = totalBuffer.CurLine();
        if (strncmp(line.c_str(), RS_IMAGE_CACHE_START_STR.c_str(), RS_IMAGE_CACHE_START_STR.size()) == 0) {
            totalBuffer.NextLine(); // data starts from the next line
            while (totalBuffer.NextLine()) {
                line = totalBuffer.CurLine();
                if (strncmp(line.c_str(), RS_IMAGE_CACHE_END_STR.c_str(), RS_IMAGE_CACHE_END_STR.size()) == 0) {
                    break;
                }
                auto* rsDumpInfo = dataProto.add_rsdumpinfo();
                for (int i = RS_SIZE; i <= RS_SURFACENAME; i++) {
                    if (!totalBuffer.NextWord(' ')) {
                        break;
                    }
                    std::string curWord = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
                    SetRSImageDumpInfo(*rsDumpInfo, i, curWord.c_str());
                }
            }
        } else if (strncmp(line.c_str(), TOTAL_CPU_STR.c_str(), TOTAL_CPU_STR.size()) == 0) {
            totalBuffer.NextLine(); // data starts from the next line
            if (!totalBuffer.NextWord(' ')) {
                break;
            }
            auto* cpuDumpInfo = dataProto.add_cpudumpinfo();
            cpuDumpInfo->set_total_cpu_memory_size(strtoull(totalBuffer.CurWord(), nullptr, DEC_BASE));
        } else if (strncmp(line.c_str(), SKIA_GPU_STR.c_str(), SKIA_GPU_STR.size()) == 0) {
            auto* gpuDumpInfo = dataProto.add_gpudumpinfo();
            if (totalBuffer.NextWord(':') && (totalBuffer.NextWord(' ') || totalBuffer.NextWord('\n'))) {
                std::string name = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
                gpuDumpInfo->set_window_name(name.substr(0, name.find("\r")));
            }
            if (totalBuffer.NextWord('\n')) {
                gpuDumpInfo->set_id(strtoull(totalBuffer.CurWord(), nullptr, DEC_BASE));
            }
            SetGpuDumpInfo(*gpuDumpInfo, totalBuffer);
        } else if (strncmp(line.c_str(), GPU_LIMIT_STR.c_str(), GPU_LIMIT_STR.size()) == 0) {
            if (totalBuffer.NextWord('=') && totalBuffer.NextWord(' ')) {
                dataProto.set_gpu_limit_size(strtoull(totalBuffer.CurWord(), nullptr, DEC_BASE));
            }
            if (totalBuffer.NextWord('=') && totalBuffer.NextWord(' ')) {
                dataProto.set_gpu_used_size(strtoull(totalBuffer.CurWord(), nullptr, DEC_BASE));
            }
            break;
        }
    } while (totalBuffer.NextLine());
}

bool MemoryDataPlugin::isRenderService(int pid)
{
    if (ReadProcPidFile(pid, "cmdline") > 0) {
        std::string processName(reinterpret_cast<char*>(buffer_.get()), strlen(reinterpret_cast<char*>(buffer_.get())));
        int index = static_cast<int>(processName.size()) - static_cast<int>(RENDER_SERVICE_NAME.size());
        if (index >= 0 && processName.substr(index) == RENDER_SERVICE_NAME) {
            return true;
        }
    }
    return false;
}

bool MemoryDataPlugin::GetRenderServiceGlSize(int32_t pid, GraphicsMemory& graphicsMemory)
{
    // refers hidumper
    // https://gitee.com/openharmony/hiviewdfx_hidumper/blob/master/frameworks/native/src/executor/memory/memory_info.cpp#L203
    bool ret = false;
    sptr<IMemoryTrackerInterface> memtrack = IMemoryTrackerInterface::Get(true);
    if (memtrack == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get IMemoryTrackerInterface failed", __func__);
        return ret;
    }

    std::vector<MemoryRecord> records;
    if (memtrack->GetDevMem(pid, MEMORY_TRACKER_TYPE_GL, records) == HDF_SUCCESS) {
        uint64_t value = 0;
        for (const auto& record : records) {
            if ((static_cast<uint32_t>(record.flags) & FLAG_UNMAPPED) == FLAG_UNMAPPED) {
                value = static_cast<uint64_t>(record.size / KB_TO_BYTES);
                break;
            }
        }
        graphicsMemory.gl = value;
        ret = true;
    }
    return ret;
}

bool MemoryDataPlugin::GetGraphicsMemory(int32_t pid, GraphicsMemory &graphicsMemory, GraphicType graphicType)
{
    std::shared_ptr<GraphicMemoryCollector> collector = GraphicMemoryCollector::Create();
    OHOS::HiviewDFX::CollectResult<int32_t> data;
    data = collector->GetGraphicUsage(pid, graphicType);
    if (data.retCode != UcError::SUCCESS) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:collect progress GL or Graph error, ret:%d.", __func__, data.retCode);
        return false;
    }
    if (graphicType == GraphicType::GL) {
        graphicsMemory.gl = data.data;
    } else if (graphicType == GraphicType::GRAPH) {
        graphicsMemory.graph = data.data;
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:graphic type is not support.", __func__);
        return false;
    }
    return true;
}

template <typename T> void MemoryDataPlugin::WriteDumpProcessInfo(T& dataProto)
{
    for (int i = 0; i < protoConfig_.pid().size(); i++) {
        int32_t pid = protoConfig_.pid(i);
        auto* processesInfo = dataProto.add_processesinfo();
        processesInfo->set_pid(pid);

        // refers hidumper
        // https://gitee.com/openharmony/hiviewdfx_hidumper/blob/master/frameworks/native/src/executor/memory/memory_info.cpp#L260
        GraphicsMemory graphicsMemory;
        if (GetGraphicsMemory(pid, graphicsMemory, GraphicType::GL)) {
            processesInfo->set_gl_pss_kb(graphicsMemory.gl);
        }
        if (GetGraphicsMemory(pid, graphicsMemory, GraphicType::GRAPH)) {
            processesInfo->set_graph_pss_kb(graphicsMemory.graph);
        }
    }
}

template <typename T> void MemoryDataPlugin::SetManagerServiceInfo(T& windowinfo, int key, const char* word)
{
    int64_t pid = 0;
    switch (key) {
        case WINDOW_NAME:
            windowinfo.set_window_name(word);
            break;
        case WINDOW_PID:
            pid = strtol(word, nullptr, DEC_BASE);
            CHECK_TRUE(pid >= 0, NO_RETVAL, "%s:strtoull pid failed", __func__);
            windowinfo.set_pid(pid);
            break;
        default:
            break;
    }
}

template <typename T> void MemoryDataPlugin::WriteManagerServiceInfo(T& dataProto)
{
    std::string content = "";
    if (strcmp(testpath_, "/proc") == 0) {
        content = RunCommand("hidumper -s WindowManagerService -a '-a'");
    } else {
        // for UT
        content = ReadFile(std::string(testpath_) + "/window_manager_service.txt");
    }
    CHECK_TRUE(content != "", NO_RETVAL, "hidumper WindowManagerService no data!");

    BufferSplitter totalBuffer((const char*)content.c_str(), content.size() + 1);
    do {
        std::string line = totalBuffer.CurLine();
        if (strncmp(line.c_str(), MGR_SVC_START_STR.c_str(), MGR_SVC_START_STR.size()) == 0) {
            totalBuffer.NextLine();
            totalBuffer.NextLine(); // data starts from the next line
            while (totalBuffer.NextLine()) {
                line = totalBuffer.CurLine();
                if (strncmp(line.c_str(), MGR_SVC_INTERVAL_STR.c_str(), MGR_SVC_INTERVAL_STR.size()) == 0) {
                    continue;
                }

                if (strncmp(line.c_str(), MGR_SVC_END_STR.c_str(), MGR_SVC_END_STR.size()) == 0) {
                    return;
                }

                auto* windowinfo = dataProto.add_windowinfo();
                for (int i = WINDOW_NAME; i <= WINDOW_PID; i++) {
                    if (!totalBuffer.NextWord(' ')) {
                        break;
                    }
                    std::string curWord = std::string(totalBuffer.CurWord(), totalBuffer.CurWordSize());
                    SetManagerServiceInfo(*windowinfo, i, curWord.c_str());
                }
            }
        }
    } while (totalBuffer.NextLine());
}

template <typename T> void MemoryDataPlugin::WriteProfileMemInfo(T& dataProto)
{
    for (int i = 0; i < protoConfig_.pid().size(); i++) {
        std::string file = "";
        if (!protoConfig_.report_fake_data()) {
            if (strcmp(testpath_, "/proc") == 0) {
                file = "/sys/kernel/debug/mali0/ctx/" + std::to_string(protoConfig_.pid(i)) + "_0/mem_profile";
            } else {
                // for UT
                file = std::string(testpath_) + "/mem_profile.txt";
            }
        } else {
            file = FAKE_DATA_PATH + "/mem_profile.txt";
        }

        std::ifstream input(file, std::ios::in);
        if (input.fail()) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s:open %s failed, errno = %d", __func__, file.c_str(), errno);
            return;
        }

        do {
            if (!input.good()) {
                return;
            }

            std::string line;
            getline(input, line);
            line += '\n';

            if (strncmp(line.c_str(), MEM_PROFILE_STR.c_str(), MEM_PROFILE_STR.size()) == 0) {
                auto* profileMemInfo = dataProto.add_profilememinfo();
                BufferSplitter totalBuffer(static_cast<const char*>(line.c_str()), line.size() + 1);
                if (totalBuffer.NextWord(':') && totalBuffer.NextWord('(')) {
                    size_t dataLen =
                        totalBuffer.CurWordSize() > 1 ? static_cast<size_t>(totalBuffer.CurWordSize() - 1) : 0;
                    profileMemInfo->set_channel(std::string(totalBuffer.CurWord(), dataLen));
                }
                if (totalBuffer.NextWord(':') && totalBuffer.NextWord(')')) {
                    profileMemInfo->set_total_memory_size(strtoull(totalBuffer.CurWord(), nullptr, DEC_BASE));
                }
            }
        } while (!input.eof());
        input.close();
    }
}
