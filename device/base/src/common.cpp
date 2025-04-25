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
#include <dirent.h>
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

#include "application_info.h"
#include "bundle_mgr_proxy.h"
#include "file_ex.h"
#include "hisysevent.h"
#include "iservice_registry.h"
#include "logging.h"
#include "system_ability_definition.h"
#include "os_account_info.h"
#include "os_account_manager.h"

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
const std::string DEFAULT_PATH = "/data/local/tmp/";
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
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        close(fd);
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get fd flags failed!", __func__);
        return false;
    }
    flags |= FD_CLOEXEC;
    if (fcntl(fd, F_SETFD, flags) == -1) {
        close(fd);
        PROFILER_LOG_ERROR(LOG_CORE, "%s: set fd_cloexec failed!", __func__);
        return false;
    }
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        // 进程正在运行，加锁失败
        close(fd);
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
        char filePath[FILE_PATH_SIZE] = {0};
        int len = snprintf_s(filePath, FILE_PATH_SIZE, FILE_PATH_SIZE - 1, "/proc/%s/cmdline", ptr->d_name);
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "maybe, the contents of cmdline had be cut off");
            continue;
        }
        FILE* fp = fopen(filePath, "r");
        if (fp == nullptr) {
            continue;
        }
        char buf[BUFFER_SIZE] = {0};
        if (fgets(buf, sizeof(buf) - 1, fp) == nullptr) {
            fclose(fp);
            continue;
        }
        std::string str(buf);
        size_t found = str.rfind("/");
        std::string fullProcess;
        if (found != std::string::npos) {
            fullProcess = str.substr(found + 1);
        } else {
            fullProcess = str;
        }
        if (fullProcess == processName) {
            pidValue = atoi(ptr->d_name);
            fclose(fp);
            break;
        }
        fclose(fp);
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
    int ret = close(fd);
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

FILE* CustomPopen(const std::vector<std::string>& command, const char* type, int fds[],
                  volatile pid_t& childPid, bool needUnblock)
{
    PROFILER_LOG_DEBUG(LOG_CORE, "BEGN %s: ready!", __func__);
    if (command.size() < MIN_CMD_ARGV || command.size() > MAX_CMD_ARGV || type == nullptr || strlen(type) != 1) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: param invalid", __func__);
        return nullptr;
    }

    // only allow "r" or "w"
    if ((type[0] != 'r' && type[0] != 'w')) {
        errno = EINVAL;
        return  nullptr;
    }
    // check whitelist
    if (!CheckWhiteList(command[0])) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: command not in whitelist!", __func__);
        return nullptr;
    }
    // check cmd arg valid
    for (auto& item : command) {
        if (!CheckCmdLineArgValid(item)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: command arg invalid : %s", __func__, item.c_str());
            return nullptr;
        }
    }
    CHECK_TRUE(pipe(fds) == 0, nullptr, "Pipe open failed!");
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
            dup2(fds[WRITE], STDOUT_FILENO); // Redirect stdout to pipe
            CHECK_TRUE(CustomFdClose(fds[WRITE]) == 0, nullptr, "CustomFdClose failed!");
        } else {
            CHECK_TRUE(CustomFdClose(fds[WRITE]) == 0, nullptr, "CustomFdClose failed!");
            dup2(fds[READ], STDIN_FILENO); // Redirect stdin to pipe
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
        return fdopen(fds[READ], "r");
    }

    PROFILER_LOG_DEBUG(LOG_CORE, "END %s fds[WRITE]: success!", __func__);
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

// IF pipe fds is block, before release other threads, you need call CustomPUnblock
int CustomPUnblock(int fds[])
{
    PROFILER_LOG_DEBUG(LOG_CORE, "BEGN %s: ready!", __func__);

    CHECK_TRUE(fds[READ] != -1 && fds[WRITE] != -1, -1, "END fds[READ/WRITE]=-1");

    int stat = fcntl(fds[READ], F_GETFL);
    CHECK_TRUE(stat != -1, -1, "END fcntl(F_GETFL) failed!");

    if (!(stat & O_NONBLOCK)) {
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
    sptr<BundleMgrProxy> proxy = iface_cast<BundleMgrProxy>(remoteObject);
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
    sptr<BundleMgrProxy> proxy = iface_cast<BundleMgrProxy>(remoteObject);
    AppExecFwk::ApplicationInfo appInfo;
    bool ret = proxy->GetApplicationInfo(bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
                                         AppExecFwk::Constants::ANY_USERID, appInfo);
    CHECK_TRUE(ret, false, "%s:%s GetApplicationInfo failed!", __func__, bundleName.c_str());
    bool isEncrypted = (appInfo.applicationReservedFlag &
                        static_cast<uint32_t>(AppExecFwk::ApplicationReservedFlag::ENCRYPTED_APPLICATION)) != 0;
    PROFILER_LOG_INFO(LOG_CORE, "%s: check application encryped.%d : %s", __func__, isEncrypted, bundleName.c_str());
    return isEncrypted;
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
    if (debugMode != "0") {
        PROFILER_LOG_INFO(LOG_CORE, "It is not user mode!");
    }
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

std::string GetTimeStr()
{
    time_t now = time(nullptr);
    struct tm tmTime;
    localtime_r(&now, &tmTime);

    char buffer[32] = {0};
    // 1900: count of years
    (void)sprintf_s(buffer, sizeof(buffer), "%04d%02d%02d_%02d%02d%02d", tmTime.tm_year + 1900, tmTime.tm_mon + 1,
        tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
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
} // namespace COMMON
