/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common.h"
#include <fcntl.h>
#include <array>
#include <cinttypes>
#include <csignal>
#include <fstream>
#include <iostream>
#include <parameter.h>
#include <parameters.h>
#include <sstream>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <atomic>
#include <regex>
#include <sys/statvfs.h>
#include <sys/sendfile.h>
#include "application_info.h"
#include "bundle_mgr_proxy.h"
#include "file_ex.h"
#include "hisysevent.h"
#include "iservice_registry.h"
#include "logging.h"
#include "system_ability_definition.h"
#include "os_account_info.h"
#include "os_account_manager.h"

namespace fs = std::filesystem;

using namespace OHOS;
using namespace OHOS::AppExecFwk;
namespace COMMON {
namespace {
const std::map<std::string, clockid_t> clockIdMap = {
    {"realtime",            CLOCK_REALTIME},
    {"mono",                CLOCK_MONOTONIC},
    {"process_cputime_id",  CLOCK_PROCESS_CPUTIME_ID},
    {"thread_cputime_id",   CLOCK_THREAD_CPUTIME_ID},
    {"mono_raw",            CLOCK_MONOTONIC_RAW},
    {"realtime_coarse",     CLOCK_REALTIME_COARSE},
    {"mono_coarse",         CLOCK_MONOTONIC_COARSE},
    {"boot",                CLOCK_BOOTTIME},
    {"realtime_alarm",      CLOCK_REALTIME_ALARM},
    {"boot_alarm",          CLOCK_BOOTTIME_ALARM},
    {"sgi_cycle",           CLOCK_SGI_CYCLE},
    {"tai",                 CLOCK_TAI},
};
constexpr int EXECVP_ERRNO = 2;
const int SHELL_UID = 2000;
const int STATUS_ID_INDEX = 4;
const std::string DEFAULT_PATH = "/data/local/tmp/";
const std::string UID_TAG = "Uid:";
const std::string GID_TAG = "Gid:";
constexpr int READ = 0;
constexpr int WRITE = 1;
const int FILE_PATH_SIZE = 512;
const int BUFFER_SIZE = 1024;
const int INVALID_PID = -1;
constexpr int32_t EC_INVALID_VALUE = -2;
const std::string KEY_HIVIEW_USER_TYPE = "const.logsystem.versiontype";
const uint32_t MAX_CMD_ARGV = 50;
const uint32_t MIN_CMD_ARGV = 2;
const std::set<std::string> CMD_WHITE_LIST = {
    "/system/bin/hiperf",    "/system/bin/hitrace",     "/system/bin/hisysevent",         "/system/bin/hidumper",
    "/system/bin/SP_daemon", "/system/bin/hiprofilerd", "/system/bin/hiprofiler_plugins", "/system/bin/native_daemon",
    "/system/bin/hilog"};
const uintmax_t ROM_LIMIT = static_cast<uintmax_t>(4) * 1024 * 1024 * 1024;  // 4G
const time_t DAY_TO_SECONDS = 24 * 60 * 60;
const mode_t DEFAULT_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;  // -rw-rw----
const std::string HTRACE_PATH_PREFIX = "/data/app/el2/";
const std::string HTRACE_PATH_SUFFIX = "/files/";
const size_t HTRACE_FILES_NUM = 5;
}  // namespace

bool IsProcessRunning(int& lockFileFd)
{
    setgid(SHELL_UID);
    char buffer[PATH_MAX + 1] = {0};
    readlink("/proc/self/exe", buffer, PATH_MAX);
    std::string processName = buffer;
    int pos = static_cast<int>(processName.find_last_of('/'));
    if (pos != 0) {
        processName = processName.substr(pos + 1, processName.size());
    }

    std::string fileName = DEFAULT_PATH + processName + ".pid";
    umask(S_IWOTH);
    int fd = open(fileName.c_str(), O_WRONLY | O_CREAT, static_cast<mode_t>(0664)); // 0664: rw-rw-r--
    if (fd < 0) {
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, fileName.c_str(), errno, buf);
        return false;
    }
    fdsan_exchange_owner_tag(fd, 0, PROFILER_SUBSYSTEM);
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        fdsan_close_with_tag(fd, PROFILER_SUBSYSTEM);
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get fd flags failed!", __func__);
        return false;
    }
    unsigned int flagsValue = static_cast<unsigned int>(flags) | FD_CLOEXEC;
    if (fcntl(fd, F_SETFD, flagsValue) == -1) {
        fdsan_close_with_tag(fd, PROFILER_SUBSYSTEM);
        PROFILER_LOG_ERROR(LOG_CORE, "%s: set fd_cloexec failed!", __func__);
        return false;
    }
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        // 进程正在运行，加锁失败
        fdsan_close_with_tag(fd, PROFILER_SUBSYSTEM);
        printf("%s is running, please don't start it again.\n", processName.c_str());
        PROFILER_LOG_ERROR(LOG_CORE, "%s is running, please don't start it again.", processName.c_str());
        return true;
    }
    std::string pidStr = std::to_string(getpid());
    auto nbytes = write(fd, pidStr.data(), pidStr.size());
    lockFileFd = fd;
    CHECK_TRUE(static_cast<size_t>(nbytes) == pidStr.size(), false, "write pid FAILED!");
    return false;
}

std::pair<bool, std::string> ReadProcessNameFromDir(struct dirent* ptr)
{
    if (ptr == nullptr) {
        return {false, ""};
    }
    char filePath[FILE_PATH_SIZE] = {0};
    int len = snprintf_s(filePath, FILE_PATH_SIZE, FILE_PATH_SIZE - 1, "/proc/%s/cmdline", ptr->d_name);
    if (len < 0 || len >= FILE_PATH_SIZE - 1) {
        PROFILER_LOG_WARN(LOG_CORE, "maybe, the contents of cmdline had been cut off");
        return {false, ""};
    }
    FILE* fp = fopen(filePath, "r");
    if (fp == nullptr) {
        return {false, ""};
    }
    char buf[BUFFER_SIZE] = {0};
    if (fgets(buf, sizeof(buf) - 1, fp) == nullptr) {
        fclose(fp);
        return {false, ""};
    }
    std::string str(buf);
    size_t found = str.rfind("/");
    std::string fullProcess;
    if (found != std::string::npos) {
        fullProcess = str.substr(found + 1);
    } else {
        fullProcess = str;
    }
    if (fclose(fp) != 0) {
        return {false, ""};
    }
    return {true, fullProcess};
}
bool IsProcessExist(const std::string& processName, int& pid)
{
    DIR* dir = opendir("/proc");
    CHECK_NOTNULL(dir, false, "open /proc dir failed");
    struct dirent* ptr;
    int pidValue = INVALID_PID;
    while ((ptr = readdir(dir)) != nullptr) {
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            continue;
        }
        if ((!isdigit(*ptr->d_name)) || ptr->d_type != DT_DIR) {
            continue;
        }
        if (!IsNumeric(std::string(ptr->d_name))) {
            continue;
        }
        auto [retFlag, fullProcess] = ReadProcessNameFromDir(ptr);
        if (!retFlag) {
            continue;
        }
        if (fullProcess == processName) {
            pidValue = atoi(ptr->d_name);
            break;
        }
    }
    closedir(dir);
    if (pidValue != INVALID_PID) {
        pid = pidValue;
    }
    return pidValue != INVALID_PID;
}

static void CloseStdio()
{
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int StartProcess(const std::string& processBin, std::vector<char*>& argv)
{
    if (argv.size() > MAX_CMD_ARGV || argv.size() < 1) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: param invalid", __func__);
        return -1;
    }
    // check whitelist
    if (!CheckWhiteList(processBin)) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: command not in whitelist!", __func__);
        return -1;
    }
    for (auto& item : argv) {
        std::string itemStr(item);
        if (!CheckCmdLineArgValid(itemStr)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: command arg invalid : %s", __func__, itemStr.c_str());
            return -1;
        }
    }
    int pid = fork();
    if (pid == 0) {
        CloseStdio();
        argv.push_back(nullptr); // last item in argv must be NULL
        int retval = execv(processBin.data(), argv.data());
        if (retval == -1 && errno == EXECVP_ERRNO) {
            printf("warning: %s does not exist!\n", processBin.c_str());
            PROFILER_LOG_WARN(LOG_CORE, "warning: %s does not exist!", processBin.c_str());
        }
        _exit(EXIT_FAILURE);
    }

    return pid;
}

int KillProcess(int pid)
{
    if (pid == -1) {
        return -1;
    }
    int stat;
    kill(pid, SIGTERM);
    if (waitpid(pid, &stat, 0) == -1) {
        if (errno != EINTR) {
            stat = -1;
        }
    }
    return stat;
}

bool CheckSubscribeVersion(const std::string& version)
{
    char* end;
    double number = std::strtod(version.c_str(), &end);
    if (end == version.c_str() + version.size()) {
        return number >= 1.0;
    } else {
        PROFILER_LOG_INFO(LOG_CORE, "current version doesn't support event push");
        return false;
    }
}

bool ContainsSpecialChars(const std::string& input)
{
    std::regex specialCharsPattern("[\'\"\\n]");
    return std::regex_search(input, specialCharsPattern);
}

void PrintMallinfoLog(const std::string& mallInfoPrefix, const struct mallinfo2& mi)
{
#ifdef HOOK_ENABLE
    std::string mallinfoLog = mallInfoPrefix;
    mallinfoLog += "arena = " + std::to_string(mi.arena) + ", ordblks = " + std::to_string(mi.ordblks);
    mallinfoLog += ", smblks = " + std::to_string(mi.smblks) + ", hblks = " + std::to_string(mi.hblks);
    mallinfoLog += ", hblkhd = " + std::to_string(mi.hblkhd) + ", usmblks = " + std::to_string(mi.usmblks);
    mallinfoLog +=
        ", fsmblks = " + std::to_string(mi.fsmblks) + ", uordblks = " + std::to_string(mi.uordblks);
    mallinfoLog +=
        ", fordblks = " + std::to_string(mi.fordblks) + ", keepcost = " + std::to_string(mi.keepcost);
    PROFILER_LOG_INFO(LOG_CORE, "%s", mallinfoLog.c_str());
#endif  // HOOK_ENABLE
}

std::string CanonicalizeSpecPath(const char* src)
{
    if (src == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "Error: CanonicalizeSpecPath failed");
        return "";
    } else if (strlen(src) + 1 >= PATH_MAX) {
        PROFILER_LOG_ERROR(LOG_CORE, "Error: CanonicalizeSpecPath %s failed", src);
        return "";
    }
    char resolvedPath[PATH_MAX] = { 0 };
    struct stat st;
    if (stat(src, &st) == 0) {
        if (strstr(src, "/proc/") == src && strstr(src, "/data/storage") != nullptr) { // for sandbox
            if (strncpy_s(resolvedPath, sizeof(resolvedPath), src, strlen(src)) != EOK) {
                PROFILER_LOG_ERROR(LOG_CORE, "Error: strncpy_s %s failed", src);
                return "";
            }
        } else if (realpath(src, resolvedPath) == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "Error: realpath %s failed with errno: %s", src, strerror(errno));
            return "";
        }
    } else {
        std::string fileName(src);
        if (fileName.find("..") == std::string::npos) {
            if (sprintf_s(resolvedPath, PATH_MAX, "%s", src) == -1) {
                PROFILER_LOG_ERROR(LOG_CORE, "Error: sprintf_s %s failed", src);
                return "";
            }
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "Error: find .. %s failed", src);
            return "";
        }
    }
    std::string res(resolvedPath);
    return res;
}

bool GetDeveloperMode()
{
    bool developerMode = OHOS::system::GetBoolParameter("const.security.developermode.state", true);
    if (!developerMode) {
        PROFILER_LOG_INFO(LOG_CORE, "It is not developer mode!");
    }
    return developerMode;
}

inline int CustomFdClose(int& fd)
{
    int ret = fdsan_close_with_tag(fd, PROFILER_SUBSYSTEM);
    if (ret == 0) {
        fd = -1;
    }
    return ret;
}

inline int CustomFdFclose(FILE** fp)
{
    int ret = fclose(*fp);
    if (ret == 0) {
        *fp = nullptr;
    }
    return ret;
}

bool CheckPopenParams(const std::vector<std::string>& command, const char* type)
{
    if (command.size() < MIN_CMD_ARGV || command.size() > MAX_CMD_ARGV || type == nullptr || strlen(type) != 1) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: param invalid", __func__);
        return false;
    }
    // only allow "r" or "w"
    if ((type[0] != 'r' && type[0] != 'w')) {
        errno = EINVAL;
        return false;
    }
    // check whitelist
    if (!CheckWhiteList(command[0])) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: command not in whitelist!", __func__);
        return false;
    }
    // check cmd arg valid
    for (auto& item : command) {
        if (!CheckCmdLineArgValid(item)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: command arg invalid : %s", __func__, item.c_str());
            return false;
        }
    }
    return true;
}

bool IsProcessExistByPid(int& pidval)
{
    char filePath[FILE_PATH_SIZE] = {0};
    int len = snprintf_s(filePath, FILE_PATH_SIZE, FILE_PATH_SIZE - 1, "/proc/%d/cmdline", pidval);
    if (len < 0) {
        PROFILER_LOG_WARN(LOG_CORE, "maybe, the contents of cmdline had be cut off");
        return false;
    }
    FILE* fp = fopen(filePath, "r");
    if (fp == nullptr) {
        PROFILER_LOG_DEBUG(LOG_CORE, "open file failed! %s", filePath);
        return false;
    }
    fclose(fp);
    return true;
}

FILE* CustomPopen(const std::vector<std::string>& command, const char* type, int fds[],
                  volatile pid_t& childPid, bool needUnblock)
{
    if (!CheckPopenParams(command, type)) {
        return nullptr;
    }
    CHECK_TRUE(pipe(fds) == 0, nullptr, "Pipe open failed!");
    fdsan_exchange_owner_tag(fds[READ], 0, PROFILER_SUBSYSTEM);
    fdsan_exchange_owner_tag(fds[WRITE], 0, PROFILER_SUBSYSTEM);
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) {
        // execve : the last argv must be nullptr.
        std::vector<char*> argv(command.size() + 1, nullptr);
        for (size_t i = 0, cmdSize = command.size(); i < cmdSize; i++) {
            argv[i] = const_cast<char*>(command[i].data());
        }
        if (strncmp(type, "r", strlen(type)) == 0) {
            CHECK_TRUE(CustomFdClose(fds[READ]) == 0, nullptr, "CustomFdClose failed!");
            fdsan_exchange_owner_tag(fds[WRITE], PROFILER_SUBSYSTEM, 0);
            dup2(fds[WRITE], STDOUT_FILENO); // Redirect stdout to pipe
            fdsan_exchange_owner_tag(fds[WRITE], 0, PROFILER_SUBSYSTEM);
            CHECK_TRUE(CustomFdClose(fds[WRITE]) == 0, nullptr, "CustomFdClose failed!");
        } else {
            CHECK_TRUE(CustomFdClose(fds[WRITE]) == 0, nullptr, "CustomFdClose failed!");
            fdsan_exchange_owner_tag(fds[READ], PROFILER_SUBSYSTEM, 0);
            dup2(fds[READ], STDIN_FILENO); // Redirect stdin to pipe
            fdsan_exchange_owner_tag(fds[READ], 0, PROFILER_SUBSYSTEM);
            CHECK_TRUE(CustomFdClose(fds[READ]) == 0, nullptr, "CustomFdClose failed!");
        }
        setpgid(pid, pid);
        // exe path = argv[0]; exe name = argv[1]
        if (execv(argv[0], &argv[1]) == -1) {
            PROFILER_LOG_ERROR(LOG_CORE, "execv failed {%s:%s}", __func__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    if (!needUnblock) {
        if (strncmp(type, "r", strlen(type)) == 0) {
            // Close the WRITE end of the pipe since parent's fd is read-only
            CHECK_TRUE(CustomFdClose(fds[WRITE]) == 0, nullptr, "%s %d CustomFdClose failed! errno(%s)\n",
                __func__, __LINE__, strerror(errno));
        } else {
            // Close the READ end of the pipe since parent's fd is write-only
            CHECK_TRUE(CustomFdClose(fds[READ]) == 0, nullptr, "%s %d CustomFdClose failed! errno(%s)\n",
                __func__, __LINE__, strerror(errno));
        }
    }
    // Make sure the parent pipe reads and writes exist;CustomPUnblock will use.
    childPid = pid;
    if (strncmp(type, "r", strlen(type)) == 0) {
        PROFILER_LOG_DEBUG(LOG_CORE, "END %s fds[READ]: success!", __func__);
        fdsan_exchange_owner_tag(fds[READ], PROFILER_SUBSYSTEM, 0);
        return fdopen(fds[READ], "r");
    }
    fdsan_exchange_owner_tag(fds[WRITE], PROFILER_SUBSYSTEM, 0);
    return fdopen(fds[WRITE], "w");
}

bool IsNumeric(const std::string& str)
{
    std::istringstream iss(str);
    int number;
    char trailingCharacter;
    if (!(iss >> number)) {
        return false;
    }
    if (iss >> trailingCharacter) {
        return false;
    }
    return true;
}

bool StoiCheck(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    for (char charac : str) {
        if (!isdigit(charac)) {
            PROFILER_LOG_ERROR(LOG_CORE, "StoiCheck FAILED!");
            return false;
        }
    }

    const std::string intMaxStr = std::to_string(INT_MAX);
    if (str.length() < intMaxStr.length()) {
        return true;
    }
    if (str.length() > intMaxStr.length()) {
        return false;
    }
    return str <= intMaxStr;
}

int CustomPclose(FILE* fp, int fds[], volatile pid_t& childPid, bool needUnblock)
{
    PROFILER_LOG_DEBUG(LOG_CORE, "BEGN %s: ready!", __func__);
    CHECK_NOTNULL(fp, -1, "NOTE %s: fp is null", __func__);

    int stat = 0;

    if (needUnblock) {
        PROFILER_LOG_DEBUG(LOG_CORE, "NOTE Kill Endless Loop Child %d.", childPid);
        kill(childPid, SIGKILL);
    }

    while (waitpid(childPid, &stat, 0) == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: %s.", __func__, strerror(errno));
        if (errno == EINTR) {
            continue;
        }
        break;
    }

    if (needUnblock) {
        if (fileno(fp) == fds[READ]) {
            fds[READ] = -1;
            if (CustomFdClose(fds[WRITE]) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "CustomFdClose failed!");
                CHECK_TRUE(CustomFdFclose(&fp) == 0, -1, "CustomFdFclose failed!");
                return -1;
            }
        } else if (fileno(fp) == fds[WRITE]) {
            fds[WRITE] = -1;
            if (CustomFdClose(fds[READ]) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "CustomFdClose failed!");
                CHECK_TRUE(CustomFdFclose(&fp) == 0, -1, "CustomFdFclose failed!");
                return -1;
            }
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "%s: Can't find fp in fds[READ/WRITE].", __func__);
        }
    }

    CHECK_TRUE(CustomFdFclose(&fp) == 0, -1, "CustomFdFclose failed!");

    PROFILER_LOG_DEBUG(LOG_CORE, "END %s: success!", __func__);
    return stat;
}

bool GetStatusLineId(std::string& line, unsigned long& target)
{
    size_t start = line.find_first_of("\t") + 1;
    size_t end = line.find_first_of("\t", start);
    std::string targetStr = line.substr(start, end - start);
    std::istringstream iss(targetStr);
    unsigned long result = 0;
    iss >> result;
    if (iss.fail() || !iss.eof()) {
        return false;
    }
    target = result;
    return true;
}

bool GetUidGidFromPid(pid_t pid, uid_t& ruid, gid_t& rgid)
{
    std::string line;
    std::string procStatusPath = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream fileStream(procStatusPath);

    if (!fileStream.is_open()) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetUidGidFromPid cannot open proc status file for pid: %d", pid);
        return false;
    }
    unsigned long statusId = 0;
    while (std::getline(fileStream, line)) {
        if (line.substr(0, STATUS_ID_INDEX) == UID_TAG) {
            if (!GetStatusLineId(line, statusId)) {
                PROFILER_LOG_ERROR(LOG_CORE, "GetUidGidFromPid get uid failed");
                return false;
            }
            ruid = static_cast<uid_t>(statusId);
        } else if (line.substr(0, STATUS_ID_INDEX) == GID_TAG) {
            if (!GetStatusLineId(line, statusId)) {
                PROFILER_LOG_ERROR(LOG_CORE, "GetUidGidFromPid get gid failed");
                return false;
            }
            rgid = static_cast<gid_t>(statusId);
        }
        if (ruid != 0 && rgid != 0) {
            break;
        }
    }
    fileStream.close();
    return true;
}

// IF pipe fds is block, before release other threads, you need call CustomPUnblock
int CustomPUnblock(int fds[])
{
    PROFILER_LOG_DEBUG(LOG_CORE, "BEGN %s: ready!", __func__);

    CHECK_TRUE(fds[READ] != -1 && fds[WRITE] != -1, -1, "END fds[READ/WRITE]=-1");

    int stat = fcntl(fds[READ], F_GETFL);
    CHECK_TRUE(stat != -1, -1, "END fcntl(F_GETFL) failed!");

    if (!(static_cast<unsigned int>(stat) & O_NONBLOCK)) {
        PROFILER_LOG_DEBUG(LOG_CORE, "NOTE %s: ready!Unblock r_fd and close all", __func__);
        const char* eof = "\n\0";
        write(fds[WRITE], eof, strlen(eof) + 1);
        fcntl(fds[READ], F_SETFL, O_NONBLOCK);
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "END %s: success!", __func__);
    return 0;
}

int GetServicePort()
{
    const std::string portRangePath = "/proc/sys/net/ipv4/ip_local_port_range";
    std::ifstream file(portRangePath.c_str());
    CHECK_TRUE(file.is_open(), -1, "Open file failed! filePath:%s", portRangePath.c_str());

    std::string rangeStr;
    copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(rangeStr));

    int minPort;
    int maxPort;
    std::istringstream istr(rangeStr);
    istr >> minPort >> maxPort;
    const int offset = 3168; // To be compatible with previously used port 50051;
    int port = (minPort + maxPort) / 2 + offset;
    PROFILER_LOG_DEBUG(LOG_CORE, "Service port is: %d", port);
    return port;
}

void SplitString(const std::string& str, const std::string &sep, std::vector<std::string>& ret)
{
    if (str.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "The string splited is empty!");
        return;
    }
    std::string::size_type beginPos = str.find_first_not_of(sep);
    std::string::size_type findPos = 0;
    while (beginPos != std::string::npos) {
        findPos = str.find(sep, beginPos);
        std::string tmp;
        if (findPos != std::string::npos) {
            tmp = str.substr(beginPos, findPos - beginPos);
            beginPos = findPos + sep.length();
        } else {
            tmp = str.substr(beginPos);
            beginPos = findPos;
        }
        if (!tmp.empty()) {
            ret.push_back(tmp);
            tmp.clear();
        }
    }
}

bool CheckApplicationPermission(int pid, const std::string& processName)
{
    std::string bundleName;
    if (pid > 0) {
        std::string filePath = "/proc/" + std::to_string(pid) + "/cmdline";
        if (!LoadStringFromFile(filePath, bundleName)) {
            PROFILER_LOG_ERROR(LOG_CORE, "Get process name by pid failed!");
            return false;
        }
        bundleName.resize(strlen(bundleName.c_str()));
    } else {
        bundleName = processName;
    }
    auto pos = bundleName.find(":");
    if (pos != std::string::npos) {
        bundleName = bundleName.substr(0, pos);
    }
    CHECK_TRUE(!bundleName.empty(), false, "Pid or process name is illegal!");

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_NOTNULL(sam, false, "GetSystemAbilityManager failed!");
    sptr<IRemoteObject> remoteObject = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_NOTNULL(remoteObject, false, "Get BundleMgr SA failed!");
    sptr<AppExecFwk::IBundleMgr> proxy = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    AppExecFwk::ApplicationInfo appInfo;
    bool ret = proxy->GetApplicationInfo(bundleName, AppExecFwk::GET_APPLICATION_INFO_WITH_DISABLE,
                                         AppExecFwk::Constants::ANY_USERID, appInfo);
    if (!ret) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetApplicationInfo failed!");
        return false;
    }
    return appInfo.appProvisionType == Constants::APP_PROVISION_TYPE_DEBUG;
}

bool CheckApplicationEncryped(int pid, const std::string& processName)
{
    std::string bundleName;
    if (pid > 0) {
        std::string filePath = "/proc/" + std::to_string(pid) + "/cmdline";
        if (!LoadStringFromFile(filePath, bundleName)) {
            PROFILER_LOG_ERROR(LOG_CORE, "Get process name by pid failed!");
            return false;
        }
        bundleName.resize(strlen(bundleName.c_str()));
    } else {
        bundleName = processName;
    }
    auto pos = bundleName.find(":");
    if (pos != std::string::npos) {
        bundleName = bundleName.substr(0, pos);
    }
    CHECK_TRUE(!bundleName.empty(), false, "Pid or process name is illegal!");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_NOTNULL(sam, false, "GetSystemAbilityManager failed!");
    sptr<IRemoteObject> remoteObject = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_NOTNULL(remoteObject, false, "Get BundleMgr SA failed!");
    sptr<AppExecFwk::IBundleMgr> proxy = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    AppExecFwk::ApplicationInfo appInfo;
    bool ret = proxy->GetApplicationInfo(bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
                                         AppExecFwk::Constants::ANY_USERID, appInfo);
    CHECK_TRUE(ret, false, "%s:%s GetApplicationInfo failed!", __func__, bundleName.c_str());
    bool isEncrypted = (appInfo.applicationReservedFlag &
                        static_cast<uint32_t>(AppExecFwk::ApplicationReservedFlag::ENCRYPTED_APPLICATION)) != 0;
    PROFILER_LOG_INFO(LOG_CORE, "%s: check application encryped.%d : %s", __func__, isEncrypted, bundleName.c_str());
    return isEncrypted;
}

bool GetBundleNameByProcessInfo(int pid, const std::string& processName, std::string& bundleName)
{
    if (pid > 0) {
        std::string filePath = "/proc/" + std::to_string(pid) + "/cmdline";
        if (!LoadStringFromFile(filePath, bundleName)) {
            PROFILER_LOG_ERROR(LOG_CORE, "Get process name by pid failed!");
            return false;
        }
        bundleName.resize(strlen(bundleName.c_str()));
    } else {
        bundleName = processName;
    }
    auto pos = bundleName.find(":");
    if (pos != std::string::npos) {
        bundleName = bundleName.substr(0, pos);
    }

    return true;
}

bool CheckIsApplication(int pid, const std::string& processName)
{
    std::string bundleName;
    bool ret = GetBundleNameByProcessInfo(pid, processName, bundleName);
    if (!ret) {
        return false;
    }

    CHECK_TRUE(!bundleName.empty(), false, "Pid or process name is illegal!");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_NOTNULL(sam, false, "GetSystemAbilityManager failed!");
    sptr<IRemoteObject> remoteObject = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_NOTNULL(remoteObject, false, "Get BundleMgr SA failed!");
    sptr<AppExecFwk::IBundleMgr> proxy = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_NOTNULL(proxy, false, "Get proxy failed!");
    AppExecFwk::ApplicationInfo appInfo;
    ret = proxy->GetApplicationInfo(bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
        AppExecFwk::Constants::ANY_USERID, appInfo);
    if (!ret) {
        PROFILER_LOG_ERROR(LOG_CORE, "Check target process is not Application!");
        return false;
    }
    return true;
}

bool VerifyPath(const std::string& filePath, const std::vector<std::string>& validPaths)
{
    if (validPaths.size() == 0) {
        return true;
    }

    for (const std::string& path : validPaths) {
        if (filePath.rfind(path, 0) == 0) {
            return true;
        }
    }
    return false;
}

const char* RealPath(std::string &filePath)
{
    if (filePath.size() > 0 && (filePath.back() == '/' || filePath.back() == '.')) {
        return nullptr;
    }
    if (filePath.find('.') == std::string::npos) {
        return filePath.c_str();
    }
    std::vector<std::string> paths;
    SplitString(filePath, "/", paths);
    std::vector<std::string> validPaths;
    for (std::string& pathName: paths) {
        if (pathName == "..") {
            if (validPaths.size() == 0) {
                return nullptr;
            } else {
                validPaths.pop_back();
            }
        } else {
            validPaths.emplace_back(pathName);
        }
    }
    filePath = "/";
    for (const std::string &pathName : validPaths) {
        filePath += pathName;
        filePath += "/";
    }
    filePath.pop_back();
    return filePath.c_str();
}

bool ReadFile(const std::string &filePath, const std::vector<std::string>& validPaths, std::string& fileContent)
{
    std::string pathName = filePath;
    const char* realFilePath = RealPath(pathName);
    CHECK_NOTNULL(realFilePath, false, "Fail to realPath: %s", filePath.c_str());

    std::string realFilePathStr(realFilePath);
    CHECK_TRUE(VerifyPath(realFilePathStr, validPaths), false, "Fail to VerifyPath: %s", realFilePathStr.c_str());

    std::ifstream fileStream(realFilePathStr, std::ios::in);
    CHECK_TRUE(fileStream.is_open(), false, "Fail to open file %s", realFilePathStr.c_str());

    std::istreambuf_iterator<char> firstIt = { fileStream };
    std::string content(firstIt, {});
    fileContent = content;
    return true;
}

bool IsUserMode()
{
    std::string debugMode = "0";
    debugMode = OHOS::system::GetParameter("const.debuggable", debugMode);
    return debugMode == "0";
}

std::string GetErrorMsg()
{
    const int bufSize = 256;
    char buffer[bufSize] = { 0 };
    strerror_r(errno, buffer, bufSize);
    std::string errorMsg(buffer);
    return errorMsg;
}

std::string GetTimeStr(std::time_t gap)
{
    time_t now = time(nullptr);
    if (now == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetTimeStr: time failed.");
        return "";
    }

    now -= gap;
    struct tm tmTime;
    localtime_r(&now, &tmTime);

    char buffer[32] = {0};
    // 1900: count of years
    int ret = sprintf_s(buffer, sizeof(buffer), "%04d%02d%02d_%02d%02d%02d", tmTime.tm_year + 1900, tmTime.tm_mon + 1,
        tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
    if (ret < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetTimeStr sprintf_s error");
        return "";
    }
    std::string timeStr(buffer);
    return timeStr;
}

// get clockid by str, return CLOCK_REALTIME as default
clockid_t GetClockId(const std::string& clockIdStr)
{
    clockid_t clockId = CLOCK_REALTIME;
    auto iter = clockIdMap.find(clockIdStr);
    if (iter != clockIdMap.end()) {
        clockId = iter->second;
    }
    return clockId;
}

std::string GetClockStr(const int32_t clockId)
{
    std::string ret = "realtime";
    for (const auto& [str, id] : clockIdMap) {
        if (id == clockId) {
            ret = str;
            break;
        }
    }
    return ret;
}

void AdaptSandboxPath(std::string& filePath, int pid)
{
    if (filePath.find("/data/storage") == 0 && access(filePath.c_str(), F_OK) != 0) {
        filePath = "/proc/" + std::to_string(pid) + "/root" + filePath;
    }
}

bool GetCurrentUserId(int32_t& userId)
{
    std::vector<int32_t> activeIds;
    int32_t ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "QueryActiveOsAccountIds failed ret:%d", ret);
        return false;
    }

    if (activeIds.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "QueryActiveOsAccountIds activeIds empty");
        return false;
    }
    userId = activeIds[0];
    PROFILER_LOG_INFO(LOG_CORE, "QueryActiveOsAccountIds userId[0]:%d", userId);
    return true;
}

int32_t GetPackageUid(const std::string& name)
{
    int32_t userId = 0;
    if (!GetCurrentUserId(userId)) {
        PROFILER_LOG_ERROR(LOG_CORE, "Failed to get current user id");
        return EC_INVALID_VALUE;
    }
    auto manager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (manager == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "systemAbilityManager is nullptr");
        return EC_INVALID_VALUE;
    }
    sptr<IRemoteObject> remoteObject = manager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "failed to get service id");
        return EC_INVALID_VALUE;
    }
    sptr<AppExecFwk::IBundleMgr> mgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (mgr == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "mgr is nullptr");
        return EC_INVALID_VALUE;
    }
    int32_t uid = mgr->GetUidByBundleName(name, userId);
    PROFILER_LOG_INFO(LOG_CORE, "pkgname is: %s, uid is : %d", name.c_str(), uid);
    return uid;
}

bool IsBetaVersion()
{
    int ret = static_cast<int>(FindParameter(KEY_HIVIEW_USER_TYPE.c_str()));
    if (ret == -1) {
        PROFILER_LOG_INFO(LOG_CORE, "user type is not exist");
        return true;
    }
    std::string userType = OHOS::system::GetParameter(KEY_HIVIEW_USER_TYPE, "");
    PROFILER_LOG_INFO(LOG_CORE, "user type is:%s", userType.c_str());
    return userType == "beta";
}

std::pair<bool, std::string> CheckNotExistsFilePath(const std::string& filePath)
{
    size_t pos = filePath.rfind('/');
    std::string pathDir = "";
    if (pos != std::string::npos) {
        pathDir = filePath.substr(0, pos);
    } else {
        pathDir = ".";
    }
    std::unique_ptr<char[]> pathDirRealPath = std::make_unique<char[]>(PATH_MAX);
    if ((pathDir.length() >= PATH_MAX - 1) || (realpath(pathDir.c_str(), pathDirRealPath.get()) == nullptr)) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:path is invalid: %s, errno=%d", __func__, pathDir.c_str(), errno);
        return {false, ""};
    }
    if (access(pathDir.c_str(), F_OK) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:Fail to access path: %s", __func__, pathDir.c_str());
        return {false, ""};
    }
    return {true, std::string(pathDirRealPath.get()) + "/" + filePath.substr(pos + 1)};
}

static const std::string DATA_TMP_DIR = "/data/local/tmp";
static const std::string FILE_SUFFIX = ".htrace";

std::pair<bool, std::string> CheckNotExistsFilePathSafe(const std::string& filePath)
{
    auto retFile = CheckFilePathPrefixSafe(filePath);
    if (!retFile.first) {
        return {false, std::string()};
    }
    std::string fullPath = retFile.second;
    if (fullPath.size() < FILE_SUFFIX.size() ||
        fullPath.compare(fullPath.size() - FILE_SUFFIX.size(), FILE_SUFFIX.size(), FILE_SUFFIX) != 0) {
        return {false, std::string()};
    }
    return {true, fullPath};
}

std::pair<bool, std::string> CheckFilePathPrefixSafe(const std::string& filePath)
{
    size_t pos = filePath.rfind('/');
    if (pos == std::string::npos || pos == filePath.size() - 1) {
        return {false, std::string()};
    }
    std::string pathDir = filePath.substr(0, pos);
    char realDir[PATH_MAX] = {0};
    if (realpath(pathDir.c_str(), realDir) == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:realpath fail: %s", __func__, pathDir.c_str());
        return {false, std::string()};
    }
    std::string resolved(realDir);
    if (resolved != DATA_TMP_DIR && resolved.rfind(DATA_TMP_DIR + "/", 0) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:not under %s: %s", __func__, DATA_TMP_DIR.c_str(), resolved.c_str());
        return {false, std::string()};
    }
    std::string fullPath = resolved + "/" + filePath.substr(pos + 1);
    return {true, fullPath};
}

bool CheckWhiteList(const std::string& cmdPath)
{
    if (cmdPath.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "cmd is empty {%s}", __func__);
        return false;
    }

    if (CMD_WHITE_LIST.find(cmdPath) == CMD_WHITE_LIST.end() ||
        cmdPath.find_first_of("|;&${}<>'\\!\n~?*%[]^") != std::string::npos) {
        PROFILER_LOG_ERROR(LOG_CORE, "cmd %s:is illegal {%s}", cmdPath.c_str(), __func__);
        return false;
    }
    return true;
}

bool CheckCmdLineArgValid(const std::string& cmdLine)
{
    if (cmdLine.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "cmdLine is empty {%s}", __func__);
        return false;
    }

    if (cmdLine.find_first_of("|;&${}<>'\\!\n~?*%[]^") != std::string::npos) {
        PROFILER_LOG_ERROR(LOG_CORE, "cmdLine %s is illegal {%s}", cmdLine.c_str(), __func__);
        return false;
    }
    return true;
}

int PluginWriteToHisysevent(const std::string& pluginName, const std::string& caller, const std::string& args,
                            int errorCode, const std::string& errorMessage)
{
    return HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::PROFILER, "HIPROFILER_USAGE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "PLUGIN_NAME", pluginName,
        "CALLER", caller,
        "ARGS", args,
        "ERROR_CODE", errorCode,
        "ERROR_MESSAGE", errorMessage);
}

std::string GetProcessNameByPid(int32_t pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream cmdlineFile(path);
    if (!cmdlineFile) {
        return "";
    }
    if (cmdlineFile.fail()) {
        return "";
    }

    std::string processName;
    std::getline(cmdlineFile, processName, '\0');
    if (processName.empty()) {
        return "";
    }
    static constexpr size_t headSize = 2;
    if (processName.substr(0, headSize) == "./") {
        processName = processName.substr(headSize);
    }
    size_t found = processName.rfind("/");
    std::string procName;
    if (found != std::string::npos) {
        procName = processName.substr(found + 1);
    } else {
        procName = processName;
    }
    return procName;
}

std::string GetBundleNameByPid(int32_t pid)
{
    std::string bundleName;
    std::string filePath = "/proc/" + std::to_string(pid) + "/cmdline";
    LoadStringFromFile(filePath, bundleName);
    bundleName.resize(strlen(bundleName.c_str()));
    return bundleName;
}

std::string GetFullTracePath(pid_t uid, pid_t pid, const std::string& processName, const std::string& filename)
{
    const pid_t uidBase = 200000;
    std::string bundleName = processName;
    if (bundleName.empty()) {
        bundleName = GetBundleNameByPid(pid);
    }
    if (filename.empty() || filename.find("..") != std::string::npos ||
        filename.find("/") != std::string::npos) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:invalid filename: %s", __func__, filename.c_str());
        return "";
    }
    return HTRACE_PATH_PREFIX + std::to_string(uid / uidBase) + "/base/" + bundleName + HTRACE_PATH_SUFFIX + filename;
}

bool GetRomSpaceInfo(const std::string& path, uint64_t& totalBytes, uint64_t& freeBytes)
{
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetRomSpaceInfo::Failed to get rom info,error:%s", strerror(errno));
        return false;
    }
    totalBytes = static_cast<uint64_t>(stat.f_blocks) * static_cast<uint64_t>(stat.f_frsize);
    freeBytes = static_cast<uint64_t>(stat.f_bfree) * static_cast<uint64_t>(stat.f_frsize);
    return true;
}

double GetFreeRomPercent(const std::string path)
{
    uint64_t totalBytes = 0;
    uint64_t freeBytes = 0;
    if (!GetRomSpaceInfo(path, totalBytes, freeBytes) || totalBytes == 0) {
        return 0.0;
    }
    return static_cast<double>(freeBytes) / static_cast<double>(totalBytes);
}

static bool DoSendfileCopyOnce(int srcFd, int destFd, ssize_t &outTotal)
{
    struct stat statBuf;
    if (fstat(srcFd, &statBuf) != 0) {
        outTotal = 0;
        return false;
    }

    off_t offset = 0;
    outTotal = 0;
    while (offset < static_cast<off_t>(statBuf.st_size)) {
        ssize_t copied = sendfile(destFd, srcFd, &offset, static_cast<size_t>(statBuf.st_size - offset));
        if (copied < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (copied == 0) {
            break;
        }
        outTotal += copied;
    }

    if (fsync(destFd) != 0) {
        return false;
    }

    return static_cast<ssize_t>(statBuf.st_size) == outTotal;
}

static bool CopyFdContentWithRetry(int srcFd, int destFd, int maxRetries)
{
    bool ret = false;
    do {
        ssize_t total = 0;
        if (DoSendfileCopyOnce(srcFd, destFd, total)) {
            ret = true;
            break;
        }
        if (ftruncate(destFd, 0) < 0 || lseek(destFd, 0, SEEK_SET) < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "clear destFd fail,error:%s", strerror(errno));
            break;
        }
    } while (maxRetries-- > 0);
    return ret;
}

static bool CopyFileWithRetry(uint32_t srcFd, const std::string& destPath, int maxRetries)
{
    if (srcFd < 0) {
        return false;
    }

    int dest = open(destPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW, DEFAULT_MODE);
    if (dest < 0) {
        return false;
    }
    fdsan_exchange_owner_tag(dest, 0, PROFILER_SUBSYSTEM);

    bool ret = false;
    do {
        ssize_t total = 0;
        if (DoSendfileCopyOnce(srcFd, dest, total)) {
            ret = true;
            break;
        }

        std::ofstream ofs(destPath, std::ios::trunc);
        if (!ofs.is_open()) {
            break;
        }
        ofs.close();
    } while (maxRetries-- > 0);

    fdsan_close_with_tag(dest, PROFILER_SUBSYSTEM);

    return ret;
}

bool CopyFileFast(uint32_t srcFd, const std::string& destPath)
{
    int maxRetries = 3;
    if (!CopyFileWithRetry(srcFd, destPath, maxRetries)) {
        PROFILER_LOG_ERROR(LOG_CORE, "copy file failed, err: %s", strerror(errno));
        return false;
    }

    return true;
}

bool CopyFileFastFromPath(const std::string& srcPath, const std::string& destPath)
{
    int srcFd = open(srcPath.c_str(), O_RDONLY | O_NOFOLLOW);
    if (srcFd < 0) {
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, srcPath.c_str(), errno, buf);
        return false;
    }
    fdsan_exchange_owner_tag(srcFd, 0, PROFILER_SUBSYSTEM);
    bool result = COMMON::CopyFileFast(srcFd, destPath);
    if (!result) {
        PROFILER_LOG_ERROR(LOG_CORE, "CopyFileFast fail");
    }
    fdsan_close_with_tag(srcFd, PROFILER_SUBSYSTEM);
    return result;
}

bool CopyFileToFdFastFromPath(const std::string& srcPath, int destFd)
{
    int srcFd = open(srcPath.c_str(), O_RDONLY | O_NOFOLLOW);
    if (srcFd < 0) {
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, srcPath.c_str(), errno, buf);
        return false;
    }
    fdsan_exchange_owner_tag(srcFd, 0, PROFILER_SUBSYSTEM);
    bool result = CopyFdContentWithRetry(srcFd, destFd, 3);
    if (!result) {
        PROFILER_LOG_ERROR(LOG_CORE, "CopyFileToFdFastFromPath fail");
    }
    fdsan_close_with_tag(srcFd, PROFILER_SUBSYSTEM);
    return result;
}

static bool IsHtraceFileName(const std::string& filename)
{
    const std::string fileSuffix = ".htrace";
    return filename.size() > fileSuffix.size() &&
        filename.substr(filename.size() - fileSuffix.size()) == fileSuffix;
}

static std::pair<std::string, std::string> ParseHtraceFileName(const std::string& filename)
{
    const std::string fileSuffix = ".htrace";
    const size_t fileNamePartsNum = 3;
    const char delim = '-';

    if (filename.size() <= fileSuffix.size()) {
        return { "", "" };
    }

    std::string fileWithNoSuffix = filename.substr(0, filename.size() - fileSuffix.size());
    std::vector<size_t> indexes;

    for (size_t i = 0; i < fileWithNoSuffix.size(); ++i) {
        if (fileWithNoSuffix[i] == delim) {
            indexes.push_back(i);
        }
    }
    if (indexes.size() != fileNamePartsNum) {
        return { "", "" };
    }
    std::string srcType = fileWithNoSuffix.substr(0, indexes[0]);
    std::string timestamp = fileWithNoSuffix.substr(indexes[2] + 1);

    return { srcType, timestamp };
}

static void CollectHtraceFiles(const std::string& dirPath, uintmax_t &romTotal,
    std::multimap<std::string, std::pair<std::string, uintmax_t>> &fileInfos,
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> &typeInfos)
{
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(dirPath, ec)) {
        if (ec) {
            break;
        }
        std::string filename = entry.path().filename().string();
        if (!IsHtraceFileName(filename)) {
            continue;
        }
        uintmax_t rom = fs::file_size(entry.path(), ec);
        if (ec) {
            ec.clear();
            continue;
        }
        romTotal += rom;
        auto fileInfo = ParseHtraceFileName(filename);
        if (fileInfo.first.empty() && fileInfo.second.empty()) {
            continue;
        }
        fileInfos.insert({fileInfo.second, {filename, rom}});
        typeInfos[fileInfo.first].insert({fileInfo.second, filename});
    }
}

static std::unordered_set<std::string> SelectFilesToRemove(uintmax_t romTotal,
    const std::multimap<std::string, std::pair<std::string, uintmax_t>> &fileInfos,
    const std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> &typeInfos)
{
    std::unordered_set<std::string> needRemove;
    std::string lastDay = COMMON::GetTimeStr(DAY_TO_SECONDS);

    // rule1 & rule2
    for (const auto& entry : fileInfos) {
        if (entry.first < lastDay || romTotal > ROM_LIMIT) {
            needRemove.insert(entry.second.first);
            if (romTotal >= entry.second.second) {
                romTotal -= entry.second.second;
            } else {
                romTotal = 0;
            }
        }
    }

    // rule3
    for (const auto& entry : typeInfos) {
        size_t sz = entry.second.size();
        if (sz > HTRACE_FILES_NUM) {
            size_t delCount = sz - HTRACE_FILES_NUM;
            for (auto it = entry.second.begin(); it != entry.second.end() && delCount > 0; ++it, --delCount) {
                needRemove.insert(it->second);
            }
        }
    }

    return needRemove;
}

static bool RemoveFilesInSet(const std::string& dirPath, const std::unordered_set<std::string>& needRemove)
{
    bool allOk = true;
    std::error_code ec;
    for (const auto& filename : needRemove) {
        fs::path fullPath = fs::path(dirPath) / filename;
        ec.clear();
        bool removed = fs::remove(fullPath, ec);
        if (ec || !removed) {
            PROFILER_LOG_ERROR(LOG_CORE, "CleanOldenFiles: remove failed %s, err: %s",
                fullPath.string().c_str(), ec.message().c_str());
            allOk = false;
            continue;
        }
    }
    return allOk;
}

/**
 * [NAPI] file olden: <resource_type>-<process_name>-<pid>-<timestamp>.htrace
 * 1.clean file saved over 24 hours
 * 2.clean oldest files if total rom size of '/data' > 4G
 * 3.clean oldest files if any resource type's count > 5
 */
bool CleanOldenFiles(const std::string& path)
{
    uintmax_t romTotal = 0;
    std::multimap<std::string, std::pair<std::string, uintmax_t>> fileInfos;
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> typeInfos;

    CollectHtraceFiles(path, romTotal, fileInfos, typeInfos);

    auto needRemove = SelectFilesToRemove(romTotal, fileInfos, typeInfos);

    bool ret = RemoveFilesInSet(path, needRemove);
    return ret;
}

bool CreateFile(const std::string& filePath)
{
    if (access(filePath.c_str(), F_OK) == 0) {
        return access(filePath.c_str(), W_OK) == 0;
    }
    int fd = creat(filePath.c_str(), DEFAULT_MODE);
    if (fd == -1) {
        return false;
    }
    fdsan_exchange_owner_tag(fd, 0, PROFILER_SUBSYSTEM);
    fdsan_close_with_tag(fd, PROFILER_SUBSYSTEM);
    return true;
}

bool CleanCmdOldenTraceFiles(const std::string& path)
{
    std::error_code ec;
    std::map<fs::file_time_type, std::string> fileInfos;
    for (const auto& entry : fs::directory_iterator(path, ec)) {
        if (ec) {
            break;
        }
        std::string filename = entry.path().filename().string();
        if (!IsHtraceFileName(filename)) {
            continue;
        }
        auto lastWriteTime = fs::last_write_time(entry.path());
        fileInfos[lastWriteTime] = filename;
    }

    std::vector<fs::file_time_type> delNativeMemoryFiles;
    const std::string targetPath = "/data/local/tmp/";
    const std::string pattern = "native_memory_";
    const std::string suffix = ".htrace";
    const std::size_t patternSize = pattern.size();
    const std::size_t suffixSize = suffix.size();
    for (auto it = fileInfos.begin(); it != fileInfos.end(); ++it) {
        const std::string& fileName = it->second;
        if (fileName.size() > suffixSize &&
            fileName.size() >= patternSize &&
            fileName.compare(0, patternSize, pattern) == 0 &&
            fileName.compare(fileName.size() - suffixSize, suffixSize, suffix) == 0) {
            std::string fullPath = targetPath + fileName;
            if (unlink(fullPath.c_str()) == 0) {
                delNativeMemoryFiles.push_back(it->first);
            }
        }
    }

    for (const auto& key : delNativeMemoryFiles) {
        fileInfos.erase(key);
    }

    bool allOk = true;
    if (fileInfos.size() >= HTRACE_FILES_NUM) {
        size_t delCount = fileInfos.size() - HTRACE_FILES_NUM;
        for (auto it = fileInfos.begin(); it != fileInfos.end() && delCount > 0; ++it, --delCount) {
            std::string fullPath = path + it->second;
            if (remove(fullPath.c_str()) != 0) {
                allOk = false;
            }
        }
    }
    return allOk;
}

bool DeleteFiles(const std::string& dir, std::function<bool(const std::string& filename)> filter)
{
    if (dir.empty() || !filter) {
        PROFILER_LOG_ERROR(LOG_CORE, "invalid param");
        return false;
    }
    std::error_code ec;
    if (!fs::is_directory(dir, ec) || ec) {
        PROFILER_LOG_ERROR(LOG_CORE, "invalid param dir:%s,", dir.c_str());
        return false;
    }
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (ec) {
            PROFILER_LOG_ERROR(LOG_CORE, "iter dir err:%s", ec.message().c_str());
            return false;
        }
        bool isRegularFile = entry.is_regular_file(ec);
        if (ec) {
            PROFILER_LOG_ERROR(LOG_CORE, "get is regular file err:%s", ec.message().c_str());
            return false;
        }
        if (isRegularFile && filter(entry.path().filename().string())) {
            fs::remove(entry.path(), ec);
            if (ec) {
                PROFILER_LOG_ERROR(LOG_CORE, "remove file err:%s", ec.message().c_str());
                return false;
            }
        }
    }
    return true;
}

bool StringMatch(const std::string& str, const std::string& prefix, const std::string& suffix)
{
    if (str.length() < prefix.length() + suffix.length()) {
        return false;
    }
    if (!prefix.empty() && str.compare(0, prefix.length(), prefix) != 0) {
        return false;
    }
    if (!suffix.empty() && str.compare(str.length() - suffix.length(), suffix.length(), suffix) != 0) {
        return false;
    }
    return true;
}
} // namespace COMMON
