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

namespace {
std::atomic<uint64_t> g_id(1);
const int DEC_BASE = 10;
const int TIME_HOUR_WIDTH = 5;
const int TIME_SEC_WIDTH = 14;
const int TIME_NS_WIDTH = 24;
const int FILE_NAME_LEN = 15;
const int TIME_BUFF_LEN = 32;
const int BUF_MAX_LEN = 512;
const int BYTE_BUFFER_SIZE = 1024;
const int BASE_YEAR = 1900;
const std::string DEFAULT_LOG_PATH("/data/local/tmp/");
FileCache g_fileCache(DEFAULT_LOG_PATH);
static pid_t volatile g_child;
const int READ = 0;
const int WRITE = 1;
const int PIPE_LEN = 2;
const std::string BIN_COMMAND("/system/bin/hilog");
} // namespace

HilogPlugin::HilogPlugin() : fp_(nullptr, nullptr) {}

HilogPlugin::~HilogPlugin()
{
    HILOG_INFO(LOG_CORE, "%s: ready!", __func__);
    std::unique_lock<std::mutex> locker(mutex_);
    if (running_) {
        running_ = false;
        if (workThread_.joinable()) {
            workThread_.join();
        }
    }
    locker.unlock();

    if (protoConfig_.need_record()) {
        g_fileCache.Close();
    }
    if (fp_ != nullptr) {
        fp_.reset();
    }
    HILOG_INFO(LOG_CORE, "%s: success!", __func__);
}

int HilogPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, -1, "HilogPlugin: ParseFromArray failed");
    if (protoConfig_.need_clear()) {
        fullCmd_ = ClearHilog();
        CHECK_TRUE(!fullCmd_.empty(), -1, "HilogPlugin: fullCmd_ is empty");
        std::vector<std::string> cmdArg;
        COMMON::SplitString(fullCmd_, " ", cmdArg);
        cmdArg.emplace(cmdArg.begin(), BIN_COMMAND);

        volatile pid_t childPid = -1;
        int pipeFds[2] = {-1, -1};
        FILE* fp = COMMON::CustomPopen(cmdArg, "r", pipeFds, childPid);
        CHECK_NOTNULL(fp, -1, "%s:clear hilog error", __func__);
        COMMON::CustomPclose(fp, pipeFds, childPid);
    }
    CHECK_TRUE(InitHilogCmd(), -1, "HilogPlugin: Init HilogCmd failed");

    fp_ = std::unique_ptr<FILE, int (*)(FILE*)>(CustomPopen(fullCmd_.c_str(), "r"), CustomPclose);
    CHECK_NOTNULL(fp_.get(), -1, "HilogPlugin: open(%s) Failed, errno(%d)", fullCmd_.c_str(), errno);
    if (protoConfig_.need_record()) {
        OpenLogFile();
    }

    CHECK_NOTNULL(resultWriter_, -1, "HilogPlugin: Writer is no set!!");
    CHECK_NOTNULL(resultWriter_->write, -1, "HilogPlugin: Writer.write is no set!!");
    CHECK_NOTNULL(resultWriter_->flush, -1, "HilogPlugin: Writer.flush is no set!!");
    g_id = 1;
    std::unique_lock<std::mutex> locker(mutex_);
    running_ = true;
    locker.unlock();
    workThread_ = std::thread(&HilogPlugin::Run, this);

    return 0;
}

int HilogPlugin::Stop()
{
    HILOG_INFO(LOG_CORE, "HilogPlugin: ready stop thread!");
    std::unique_lock<std::mutex> locker(mutex_);
    running_ = false;
    locker.unlock();
    if (workThread_.joinable()) {
        workThread_.join();
    }
    HILOG_INFO(LOG_CORE, "HilogPlugin: stop thread success!");
    if (protoConfig_.need_record()) {
        g_fileCache.Close();
    }
    fp_.reset();

    HILOG_INFO(LOG_CORE, "HilogPlugin: stop success!");
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

inline std::string HilogPlugin::GetPidCmd()
{
    std::string pidCmd = "";
    if (protoConfig_.pid() > 0) {
        pidCmd = std::to_string(protoConfig_.pid());
    }
    return pidCmd;
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
        default:
            break;
    }

    return levelCmd;
}

bool HilogPlugin::InitHilogCmd()
{
    switch (protoConfig_.device_type()) {
        case HI3516:
            fullCmd_ = "hilog";
            if (GetPidCmd().length() > 0) {
                fullCmd_ += " -P ";
                fullCmd_ += GetPidCmd();
            }
            if (GetlevelCmd().length() > 0) {
                fullCmd_ += " -L ";
                fullCmd_ += GetlevelCmd();
            }
            break;
        case P40:
            fullCmd_ = "hilogcat";
            if (GetPidCmd().length() > 0) {
                fullCmd_ += " --pid=";
                fullCmd_ += GetPidCmd();
            }
            if (GetlevelCmd().length() > 0) {
                fullCmd_ += " *:";
                fullCmd_ += GetlevelCmd();
            }
            break;
        default:
            break;
    }

    if (fullCmd_.size()) {
        fullCmd_ = fullCmd_ + std::string(" --format nsec");
        HILOG_INFO(LOG_CORE, "HilogPlugin: hilog cmd(%s)", fullCmd_.c_str());
        return true;
    }

    return false;
}

std::string HilogPlugin::ClearHilog()
{
    std::string cmd;
    switch (protoConfig_.device_type()) {
        case HI3516:
            cmd = "hilog -r";
            break;
        case P40:
            cmd = "hilogcat -c";
            break;
        default:
            break;
    }

    return cmd;
}

void HilogPlugin::Run(void)
{
    HILOG_INFO(LOG_CORE, "HilogPlugin::Run start!");
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(BUF_MAX_LEN);

    HilogInfo dataProto;

    dataProto.set_clock(0);
    fcntl(fileno(fp_.get()), F_SETFL, O_NONBLOCK);
    while (running_) {
        if (fgets(reinterpret_cast<char*>(buffer.get()), BUF_MAX_LEN - 1, fp_.get()) != nullptr) {
            auto cptr = reinterpret_cast<char*>(buffer.get());
            if (*cptr >= '0' && *cptr <= '9') {
                auto* info = dataProto.add_info();
                ParseLogLineInfo(cptr, strlen(cptr), info);
                info->set_id(g_id);
                g_id++;
            }
        }

        if (dataProto.ByteSizeLong() >= BYTE_BUFFER_SIZE) {
            protoBuffer_.resize(dataProto.ByteSizeLong());
            dataProto.SerializeToArray(protoBuffer_.data(), protoBuffer_.size());
            resultWriter_->write(resultWriter_, protoBuffer_.data(), protoBuffer_.size());
            resultWriter_->flush(resultWriter_);
            dataProto.clear_info();
        }

        if (protoConfig_.need_record() && dataBuffer_.size() >= BYTE_BUFFER_SIZE) {
            g_fileCache.Write(dataBuffer_.data(), dataBuffer_.size());
            dataBuffer_.erase(dataBuffer_.begin(), dataBuffer_.end());
        }
    }
    protoBuffer_.resize(dataProto.ByteSizeLong());
    dataProto.SerializeToArray(protoBuffer_.data(), protoBuffer_.size());
    resultWriter_->write(resultWriter_, protoBuffer_.data(), protoBuffer_.size());
    resultWriter_->flush(resultWriter_);
    dataProto.clear_info();
    if (protoConfig_.need_record() && !dataBuffer_.empty()) {
        g_fileCache.Write(dataBuffer_.data(), dataBuffer_.size());
        dataBuffer_.erase(dataBuffer_.begin(), dataBuffer_.end());
    }
    HILOG_INFO(LOG_CORE, "HilogPlugin::Run done!");
}

void HilogPlugin::ParseLogLineInfo(const char* data, size_t len, HilogLine* info)
{
    if (data == nullptr || len < TIME_NS_WIDTH) {
        HILOG_ERROR(LOG_CORE, "HilogPlugin:%s param invalid", __func__);
        return;
    }

    for (size_t i = 0; i < len && protoConfig_.need_record(); i++) {
        dataBuffer_.push_back(data[i]);
    }

    SetHilogLineDetails(data, info);
    return;
}

bool HilogPlugin::SetHilogLineDetails(const char* data, HilogLine* info)
{
    char* end = nullptr;
    struct timespec ts = {0};
    char* pTmp = const_cast<char*>(data);

    TimeStringToNS(data, &ts);
    info->mutable_detail()->set_tv_sec(ts.tv_sec);
    info->mutable_detail()->set_tv_nsec(ts.tv_nsec);
    pTmp = pTmp + TIME_SEC_WIDTH;
    CHECK_TRUE(FindFirstSpace(&pTmp), false, "HilogPlugin:FindFirstSpace failed!");
    uint32_t value = static_cast<uint32_t>(strtoul(pTmp, &end, DEC_BASE));
    CHECK_TRUE(value > 0, false, "HilogPlugin:strtoull pid failed!");
    info->mutable_detail()->set_pid(value);
    pTmp = end;
    value = static_cast<uint32_t>(strtoul(pTmp, &end, DEC_BASE));
    CHECK_TRUE(value > 0, false, "HilogPlugin:strtoull tid failed!");
    info->mutable_detail()->set_tid(value);
    pTmp = end;
    CHECK_TRUE(RemoveSpaces(&pTmp), false, "HilogPlugin:RemoveSpaces failed!");
    info->mutable_detail()->set_level(*pTmp);
    pTmp++;
    CHECK_TRUE(RemoveSpaces(&pTmp), false, "HilogPlugin:RemoveSpaces failed!");

    if (*pTmp >= '0' && *pTmp <= '9') {
        while (*pTmp != '/') { // 找 '/'
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
    while (end != nullptr && *pTmp != ':') { // 结束符 ':'
        if (*pTmp == '\0' || *pTmp == '\n') {
            return false;
        }
        pTmp++;
        index++;
    }
    info->mutable_detail()->set_tag(std::string(end, end + index - 1));
    pTmp++;
    CHECK_TRUE(RemoveSpaces(&pTmp), false, "HilogPlugin: RemoveSpaces failed!");
    if (google::protobuf::internal::IsStructurallyValidUTF8(pTmp, strlen(pTmp) - 1)) {
        info->set_context(pTmp, strlen(pTmp) - 1);  // - \n
    } else {
        HILOG_ERROR(LOG_CORE, "HilogPlugin: log context include invalid UTF-8 data");
        info->set_context("");
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
        HILOG_ERROR(LOG_CORE, "HilogPlugin: get localtime failed!, errno(%d:%s)", errno, buf);
        return false;
    }
    tmTime.tm_year = result.tm_year;
    strptime(data, "%m-%d %H:%M:%S", &tmTime);
    pTmp = const_cast<char*>(data) + TIME_HOUR_WIDTH;
    CHECK_TRUE(StringToL(pTmp, fixHour), false, "%s:strtol fixHour failed", __func__);
    if (static_cast<int>(fixHour) != tmTime.tm_hour) { // hours since midnight - [0, 23]
        HILOG_INFO(LOG_CORE, "HilogPlugin: hour(%d) <==> fix hour(%ld)!", tmTime.tm_hour, fixHour);
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
        HILOG_ERROR(LOG_CORE, "%s:snprintf_s error", __func__);
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
        HILOG_ERROR(LOG_CORE, "HilogPlugin: get localtime failed!, errno(%d:%s)", errno, buf);
        return -1;
    }

    if (snprintf_s(psDateTime, size, size - 1, "%04d%02d%02d%02d%02d%02d", pTM->tm_year + BASE_YEAR, pTM->tm_mon + 1,
                   pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec) < 0) {
        HILOG_ERROR(LOG_CORE, "%s:snprintf_s error", __func__);
    }

    return 0;
}

FILE* HilogPlugin::CustomPopen(const char* command, const char* type)
{
    CHECK_TRUE(command != nullptr && type != nullptr, nullptr, "HilogPlugin:%s param invalid", __func__);

    int fd[PIPE_LEN];
    pipe(fd);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    // child process
    if (pid == 0) {
        if (!strncmp(type, "r", strlen(type))) {
            close(fd[READ]);
            dup2(fd[WRITE], 1); // Redirect stdout to pipe
            dup2(fd[WRITE], 2); // 2: Redirect stderr to pipe
        } else {
            close(fd[WRITE]);
            dup2(fd[READ], 0); // Redirect stdin to pipe
        }
        setpgid(pid, pid);
        std::vector<std::string> cmdArg;
        COMMON::SplitString(std::string(command), " ", cmdArg);
        std::vector<char*> vectArgv;
        for (auto& item : cmdArg) {
            vectArgv.push_back(const_cast<char*>(item.c_str()));
        }
        // execv : the last argv must be nullptr.
        vectArgv.push_back(nullptr);
        execv(BIN_COMMAND.c_str(), &vectArgv[0]);
        exit(0);
    } else {
        if (!strncmp(type, "r", strlen(type))) {
            // Close the WRITE end of the pipe since parent's fd is read-only
            close(fd[WRITE]);
        } else {
            // Close the READ end of the pipe since parent's fd is write-only
            close(fd[READ]);
        }
    }

    g_child = pid;

    if (!strncmp(type, "r", strlen(type))) {
        return fdopen(fd[READ], "r");
    }

    return fdopen(fd[WRITE], "w");
}

int HilogPlugin::CustomPclose(FILE* fp)
{
    CHECK_NOTNULL(fp, -1, "HilogPlugin:%s fp is null", __func__);
    int stat;

    int ret = fclose(fp);
    CHECK_TRUE(ret == 0, -1, "HilogPlugin:%s fclose failed! errno(%d)", __func__, errno);
    kill(g_child, SIGKILL);
    if (waitpid(g_child, &stat, 0) == -1) {
        if (errno != EINTR) {
            stat = -1;
        }
    }

    return stat;
}
