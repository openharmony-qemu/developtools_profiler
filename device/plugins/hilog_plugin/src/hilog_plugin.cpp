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
#include "hilog_plugin.h"
#include "securec.h"
#include <fcntl.h>
#include <cinttypes>
#include <csignal>
#include <sstream>
#include <cstdio>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "common.h"
#include "utf8_validity.h"

namespace {
using namespace OHOS::Developtools::Profiler;
std::atomic<uint64_t> g_id(1);
const int DEC_BASE = 10;
const int TIME_HOUR_WIDTH = 5;
const int TIME_SEC_WIDTH = 14;
const int TIME_NS_WIDTH = 24;
const int FILE_NAME_LEN = 15;
const int TIME_BUFF_LEN = 32;
const int PIPE_SIZE_RATIO = 8;
const int BYTE_BUFFER_SIZE = 1024;
const int BASE_YEAR = 1900;
const int MAX_BUFFER_LEN = 8192;
const std::string DEFAULT_LOG_PATH("/data/local/tmp/");
FileCache g_fileCache(DEFAULT_LOG_PATH);
const std::string BIN_COMMAND("/system/bin/hilog");
} // namespace

HilogPlugin::HilogPlugin() : fp_(nullptr, nullptr) {}

HilogPlugin::~HilogPlugin()
{
    PROFILER_LOG_INFO(LOG_CORE, "%s: ready!", __func__);
    if (running_.load()) {
        running_.store(false);
        if (workThread_.joinable()) {
            workThread_.join();
        }
    }
    std::unique_lock<std::mutex> locker(mutex_);
    if (protoConfig_.need_record()) {
        g_fileCache.Close();
    }
    locker.unlock();
    if (fp_ != nullptr) {
        fp_.reset();
    }
    PROFILER_LOG_INFO(LOG_CORE, "%s: success!", __func__);
}

std::string HilogPlugin::GetCmdArgs(const HilogConfig& protoConfig)
{
    std::stringstream args;
    args << "log_level: " << std::to_string(protoConfig.log_level()) << ", ";
    args << "pid: " << COMMON::GetProcessNameByPid(protoConfig.pid()) << ", ";
    args << "need_record: " << (protoConfig.need_record() ? "true" : "false") << ", ";
    args << "need_clear: " << (protoConfig.need_clear() ? "true" : "false");
    return args.str();
}

int HilogPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, -1, "HilogPlugin: ParseFromArray failed");
    if (protoConfig_.need_clear()) {
        std::vector<std::string> cmdArg;

        cmdArg.emplace_back(BIN_COMMAND); // exe file path
        cmdArg.emplace_back("hilog"); // exe file name
        cmdArg.emplace_back("-r");
        volatile pid_t childPid = -1;
        int pipeFds[2] = {-1, -1};
        FILE* fp = COMMON::CustomPopen(cmdArg, "r", pipeFds, childPid);
        CHECK_NOTNULL(fp, -1, "%s:clear hilog error", __func__);
        COMMON::CustomPclose(fp, pipeFds, childPid);
    }
    InitHilogCmd();
    fp_ = std::unique_ptr<FILE, std::function<int (FILE*)>>(
        COMMON::CustomPopen(fullCmd_, "r", pipeFds_, childPid_, true), [this](FILE* fp) -> int {
            return COMMON::CustomPclose(fp, pipeFds_, childPid_, true);
        });

    if (protoConfig_.need_record()) {
        OpenLogFile();
    }

    CHECK_NOTNULL(resultWriter_, -1, "HilogPlugin: Writer is no set!!");
    CHECK_NOTNULL(resultWriter_->write, -1, "HilogPlugin: Writer.write is no set!!");
    CHECK_NOTNULL(resultWriter_->flush, -1, "HilogPlugin: Writer.flush is no set!!");
    g_id = 1;
    running_.store(true);
    int oldPipeSize = fcntl(fileno(fp_.get()), F_GETPIPE_SZ);
    fcntl(fileno(fp_.get()), F_SETPIPE_SZ, oldPipeSize * PIPE_SIZE_RATIO);
    int pipeSize = fcntl(fileno(fp_.get()), F_GETPIPE_SZ);
    PROFILER_LOG_INFO(LOG_CORE, "{fp = %d, pipeSize=%d, oldPipeSize=%d}", fileno(fp_.get()), pipeSize, oldPipeSize);
    workThread_ = std::thread([this] { this->Run(); });

    int ret = COMMON::PluginWriteToHisysevent("hilog_plugin", "sh", GetCmdArgs(protoConfig_),
                                              COMMON::ErrorType::RET_SUCC, "success");
    PROFILER_LOG_INFO(LOG_CORE, "hisysevent report hilog_plugin result:%d", ret);
    return 0;
}

int HilogPlugin::Stop()
{
    PROFILER_LOG_INFO(LOG_CORE, "HilogPlugin: ready stop thread!");
    running_.store(false);
    COMMON::CustomPUnblock(pipeFds_);
    if (workThread_.joinable()) {
        workThread_.join();
    }
    std::unique_lock<std::mutex> locker(mutex_);
    if (protoConfig_.need_record() && !dataBuffer_.empty()) {
        g_fileCache.Write(dataBuffer_.data(), dataBuffer_.size());
        dataBuffer_.erase(dataBuffer_.begin(), dataBuffer_.end());
    }
    PROFILER_LOG_INFO(LOG_CORE, "HilogPlugin: stop thread success!");
    if (protoConfig_.need_record()) {
        g_fileCache.Close();
    }
    fp_.reset();
    locker.unlock();
    PROFILER_LOG_INFO(LOG_CORE, "HilogPlugin: stop success!");
    return 0;
}

int HilogPlugin::SetWriter(WriterStruct* writer)
{
    resultWriter_ = writer;
    return 0;
}

bool HilogPlugin::OpenLogFile()
{
    char name[FILE_NAME_LEN] = {0};
    GetDateTime(name, sizeof(name));
    CHECK_TRUE(g_fileCache.Open(name), false, "HilogPlugin:%s failed!", __func__);
    return true;
}

std::string HilogPlugin::GetlevelCmd()
{
    std::string levelCmd = "";
    switch (protoConfig_.log_level()) {
        case ERROR:
            levelCmd = "E";
            break;
        case INFO:
            levelCmd = "I";
            break;
        case DEBUG:
            levelCmd = "D";
            break;
        case WARN:
            levelCmd = "W";
            break;
        case FATAL:
            levelCmd = "F";
            break;
        default:
            break;
    }

    return levelCmd;
}

void HilogPlugin::InitHilogCmd()
{
    fullCmd_.emplace_back(BIN_COMMAND); // exe file path
    fullCmd_.emplace_back("hilog"); // exe file name

    if (protoConfig_.pid() > 0) {
        fullCmd_.emplace_back("-P");
        fullCmd_.emplace_back(std::to_string(protoConfig_.pid()));
    }
    if (GetlevelCmd().length() > 0) {
        fullCmd_.emplace_back("-L");
        fullCmd_.emplace_back(GetlevelCmd());
    }

    fullCmd_.emplace_back("--format");
    fullCmd_.emplace_back("nsec");
}

void HilogPlugin::StartLoopFetchData(std::unique_ptr<uint8_t[]>& buffer, std::unique_ptr<HilogInfo>& dataProto,
                                     std::unique_ptr<ProtoEncoder::HilogInfo>& hilogInfo, std::string startTime)
{
    while (running_.load()) {
        if (fgets(reinterpret_cast<char*>(buffer.get()), MAX_BUFFER_LEN - 1, fp_.get()) == nullptr) {
            continue;
        }
        if ((strlen(reinterpret_cast<char*>(buffer.get())) + 1) == (MAX_BUFFER_LEN - 1)) {
            PROFILER_LOG_ERROR(LOG_CORE, "HilogPlugin:data length is greater than the MAX_BUFFER_LEN(%d)",
                               MAX_BUFFER_LEN);
        }
        auto cptr = reinterpret_cast<char*>(buffer.get());
        std::string curTime = cptr;
        curTime = curTime.substr(0, TIME_SEC_WIDTH);
        if (curTime < startTime) {
            continue;
        }
        if (resultWriter_->isProtobufSerialize) {
            ParseLogLineData(cptr, strlen(cptr), dataProto.get());
            if (dataProto->ByteSizeLong() >= BYTE_BUFFER_SIZE) {
                FlushData(dataProto.get());
                dataProto.reset();
                dataProto = std::make_unique<HilogInfo>();
            }
        } else {
            ParseLogLineData(cptr, strlen(cptr), hilogInfo.get());
            if (hilogInfo->Size() >= BYTE_BUFFER_SIZE) {
                FlushDataOptimize(hilogInfo.get());
                hilogInfo.reset();
                hilogInfo = std::make_unique<ProtoEncoder::HilogInfo>(resultWriter_->startReport(resultWriter_));
            }
        }
        if (protoConfig_.need_record() && dataBuffer_.size() >= BYTE_BUFFER_SIZE) {
            std::unique_lock<std::mutex> locker(mutex_);
            g_fileCache.Write(dataBuffer_.data(), dataBuffer_.size());
            dataBuffer_.erase(dataBuffer_.begin(), dataBuffer_.end());
            locker.unlock();
        }
    }
}

void HilogPlugin::Run(void)
{
    PROFILER_LOG_INFO(LOG_CORE, "HilogPlugin::Run start!");
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(MAX_BUFFER_LEN);
    std::unique_ptr<HilogInfo> dataProto = nullptr;
    std::unique_ptr<ProtoEncoder::HilogInfo> hilogInfo = nullptr;
    if (resultWriter_->isProtobufSerialize) {
        dataProto = std::make_unique<HilogInfo>();
    } else {
        hilogInfo = std::make_unique<ProtoEncoder::HilogInfo>(resultWriter_->startReport(resultWriter_));
    }
    time_t startTm = time(nullptr);
    struct tm* pTime = localtime(&startTm);
    char startTime[FILE_NAME_LEN] = {0};
    if (pTime != nullptr) {
        int res = snprintf_s(startTime, FILE_NAME_LEN, FILE_NAME_LEN - 1, "%02d-%02d %02d:%02d:%02d",
            pTime->tm_mon + 1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
        if (res < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "HilogPlugin::Run snprintf_s error");
            return;
        }
    }
    StartLoopFetchData(buffer, dataProto, hilogInfo, std::string(startTime));
    if (resultWriter_->isProtobufSerialize) {
        FlushData(dataProto.get());
        dataProto.reset();
    } else {
        FlushDataOptimize(hilogInfo.get());
        hilogInfo.reset();
    }
    PROFILER_LOG_INFO(LOG_CORE, "HilogPlugin::Run done!");
}

template <typename T> void HilogPlugin::ParseLogLineInfo(const char* data, size_t len, T& hilogLineInfo)
{
    if (data == nullptr || len < TIME_NS_WIDTH) {
        PROFILER_LOG_ERROR(LOG_CORE, "HilogPlugin:%s param invalid", __func__);
        return;
    }

    for (size_t i = 0; i < len && protoConfig_.need_record(); i++) {
        dataBuffer_.push_back(data[i]);
    }

    SetHilogLineDetails(data, hilogLineInfo);
    return;
}

template <typename T> void HilogPlugin::ParseLogLineData(const char* data, size_t len, T hilogInfoProto)
{
    CHECK_NOTNULL(data, NO_RETVAL, "data is nullptr");
    if (*data >= '0' && *data <= '9') {
        auto* info = hilogInfoProto->add_info();
        ParseLogLineInfo(data, len, *info);
        info->set_id(g_id);
        g_id++;
    }
}

bool HilogPlugin::FindTagString(char*& pTmp, char*& end, int& npos)
{
    if (*pTmp >= '0' && *pTmp <= '9') {
        while (*pTmp != '/') {  // 找 '/'
            if (*pTmp == '\0' || *pTmp == '\n') {
                return false;
            }
            pTmp++;
        }
        pTmp++;
        end = pTmp;
    } else if ((*pTmp >= 'a' && *pTmp <= 'z') || (*pTmp >= 'A' && *pTmp <= 'Z')) {
        end = pTmp;
    }
    int index = 1;
    if (end == nullptr) {
        return false;
    }
    while (*pTmp != ':') {  // 结束符 ':'
        if (*pTmp == '\0' || *pTmp == '\n') {
            return false;
        }
        pTmp++;
        index++;
    }
    npos = index;
    return true;
}

template <typename T> bool HilogPlugin::SetHilogLineDetails(const char* data, T& hilogLineInfo)
{
    char* end = nullptr;
    struct timespec ts = {0};
    char* pTmp = const_cast<char*>(data);
    TimeStringToNS(data, &ts);
    auto* detail = hilogLineInfo.mutable_detail();
    detail->set_tv_sec(ts.tv_sec);
    detail->set_tv_nsec(ts.tv_nsec);
    pTmp = pTmp + TIME_SEC_WIDTH;
    CHECK_TRUE(FindFirstSpace(&pTmp), false, "HilogPlugin:FindFirstSpace failed!");
    uint32_t value = static_cast<uint32_t>(strtoul(pTmp, &end, DEC_BASE));
    CHECK_TRUE(value > 0, false, "HilogPlugin:strtoull pid failed!");
    detail->set_pid(value);
    pTmp = end;
    value = static_cast<uint32_t>(strtoul(pTmp, &end, DEC_BASE));
    CHECK_TRUE(value > 0, false, "HilogPlugin:strtoull tid failed!");
    detail->set_tid(value);
    pTmp = end;
    CHECK_TRUE(RemoveSpaces(&pTmp), false, "HilogPlugin:RemoveSpaces failed!");
    detail->set_level(*pTmp);
    pTmp++;
    CHECK_TRUE(RemoveSpaces(&pTmp), false, "HilogPlugin:RemoveSpaces failed!");
    int npos = 1;
    if (!FindTagString(pTmp, end, npos)) {
        return false;
    }
    detail->set_tag(std::string(end, end + npos - 1));
    pTmp++;
    CHECK_TRUE(RemoveSpaces(&pTmp), false, "HilogPlugin: RemoveSpaces failed!");
    size_t dataLen = strlen(pTmp) > 1 ? strlen(pTmp) - 1 : 0;
    if (utf8_range::IsStructurallyValid({pTmp, dataLen})) {
        hilogLineInfo.set_context(pTmp, dataLen);  // - \n
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "HilogPlugin: log context include invalid UTF-8 data");
        hilogLineInfo.set_context("");
    }
    return true;
}

bool HilogPlugin::FindFirstNum(char** p)
{
    CHECK_NOTNULL(*p, false, "HilogPlugin:%s", __func__);
    while (**p > '9' || **p < '0') {
        if (**p == '\0' || **p == '\n') {
            return false;
        }
        (*p)++;
    }
    return true;
}

bool HilogPlugin::RemoveSpaces(char** p)
{
    CHECK_NOTNULL(*p, false, "HilogPlugin:%s", __func__);
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

bool HilogPlugin::FindFirstSpace(char** p)
{
    CHECK_NOTNULL(*p, false, "HilogPlugin:%s", __func__);
    while (**p != ' ') {
        if (**p == '\0' || **p == '\n') {
            return false;
        }
        (*p)++;
    }
    return true;
}

bool HilogPlugin::StringToL(const char* word, long& value)
{
    char* end = nullptr;
    errno = 0;
    value = strtol(word, &end, DEC_BASE);
    if ((errno == ERANGE && (value == LONG_MAX)) || (errno != 0 && value == 0)) {
        return false;
    } else if (end == word && (*word >= '0' && *word <= '9')) {
        return false;
    }

    return true;
}

bool HilogPlugin::TimeStringToNS(const char* data, struct timespec *tsTime)
{
    struct tm tmTime = {0};
    struct tm result;
    time_t timetTime;
    char* end = nullptr;
    char* pTmp = nullptr;
    time_t nSeconds = time(nullptr);
    uint32_t nsec = 0;
    long fixHour = 0;

    if (localtime_r(&nSeconds, &result) == nullptr) {
        const int bufSize = 128;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "HilogPlugin: get localtime failed!, errno(%d:%s)", errno, buf);
        return false;
    }
    tmTime.tm_year = result.tm_year;
    strptime(data, "%m-%d %H:%M:%S", &tmTime);
    pTmp = const_cast<char*>(data) + TIME_HOUR_WIDTH;
    CHECK_TRUE(StringToL(pTmp, fixHour), false, "%s:strtol fixHour failed", __func__);
    if (static_cast<int>(fixHour) != tmTime.tm_hour) { // hours since midnight - [0, 23]
        PROFILER_LOG_INFO(LOG_CORE, "HilogPlugin: hour(%d) <==> fix hour(%ld)!", tmTime.tm_hour, fixHour);
        tmTime.tm_hour = fixHour;
    }
    pTmp = const_cast<char*>(data) + TIME_SEC_WIDTH;
    FindFirstNum(&pTmp);
    nsec = static_cast<uint32_t>(strtoul(pTmp, &end, DEC_BASE));
    CHECK_TRUE(nsec > 0, false, "%s:strtoull nsec failed", __func__);

    timetTime = mktime(&tmTime);
    tsTime->tv_sec = timetTime;
    tsTime->tv_nsec = nsec;

    char buff[TIME_BUFF_LEN] = {0};
    if (snprintf_s(buff, sizeof(buff), sizeof(buff) - 1, "%ld.%09u\n", timetTime, nsec) < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:snprintf_s error", __func__);
    }
    size_t buffSize = strlen(buff);
    for (size_t i = 0; i < buffSize && protoConfig_.need_record(); i++) {
        dataBuffer_.push_back(buff[i]);
    }

    return true;
}

int HilogPlugin::GetDateTime(char* psDateTime, uint32_t size)
{
    CHECK_NOTNULL(psDateTime, -1, "HilogPlugin:%s param invalid", __func__);
    CHECK_TRUE(size > 1, -1, "HilogPlugin:%s param invalid!", __func__);

    time_t nSeconds;
    struct tm* pTM;

    nSeconds = time(nullptr);
    pTM = localtime(&nSeconds);
    if (pTM == nullptr) {
        const int bufSize = 128;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "HilogPlugin: get localtime failed!, errno(%d:%s)", errno, buf);
        return -1;
    }

    if (snprintf_s(psDateTime, size, size - 1, "%04d%02d%02d%02d%02d%02d", pTM->tm_year + BASE_YEAR, pTM->tm_mon + 1,
                   pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec) < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:snprintf_s error", __func__);
    }

    return 0;
}

template <typename T> void HilogPlugin::FlushData(const T hilogLineProto)
{
    protoBuffer_.resize(hilogLineProto->ByteSizeLong());
    hilogLineProto->SerializeToArray(protoBuffer_.data(), protoBuffer_.size());
    resultWriter_->write(resultWriter_, protoBuffer_.data(), protoBuffer_.size());
    resultWriter_->flush(resultWriter_);
}

template <typename T> void HilogPlugin::FlushDataOptimize(const T hilogLineProto)
{
    int messageLen = hilogLineProto->Finish();
    resultWriter_->finishReport(resultWriter_, messageLen);
    resultWriter_->flush(resultWriter_);
}