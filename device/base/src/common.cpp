/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <array>
#include <cinttypes>
#include <csignal>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#ifdef HOOK_ENABLE
#include <malloc.h>
#endif
#include <sstream>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "application_info.h"
#include "bundle_mgr_proxy.h"
#include "file_ex.h"
#include "iservice_registry.h"
#include "logging.h"
#include "system_ability_definition.h"

using namespace OHOS;
using namespace OHOS::AppExecFwk;
namespace COMMON {
constexpr int EXECVP_ERRNO = 2;
const int SHELL_UID = 2000;
const std::string DEFAULT_PATH = "/data/local/tmp/";
constexpr int PIPE_LEN = 2;
constexpr int READ = 0;
constexpr int WRITE = 1;
const int FILE_PATH_SIZE = 512;
const int BUFFER_SIZE = 1024;
const int INVALID_PID = -1;

bool IsProcessRunning()
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
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "%s:failed to open(%s), errno(%d:%s)", __func__, fileName.c_str(), errno, buf);
        return false;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        // 进程正在运行，加锁失败
        close(fd);
        printf("%s is running, please don't start it again.\n", processName.c_str());
        HILOG_ERROR(LOG_CORE, "%s is running, please don't start it again.", processName.c_str());
        return true;
    }

    std::string pidStr = std::to_string(getpid());
    auto nbytes = write(fd, pidStr.data(), pidStr.size());
    CHECK_TRUE(static_cast<size_t>(nbytes) == pidStr.size(), false, "write pid FAILED!");
    return false;
}

bool IsProcessExist(std::string& processName, int& pid)
{
    DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        HILOG_ERROR(LOG_CORE, "open /proc dir failed");
        return false;
    }
    struct dirent* ptr;
    int pidValue = INVALID_PID;
    while ((ptr = readdir(dir)) != nullptr) {
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            continue;
        }
        if ((!isdigit(*ptr->d_name)) || ptr->d_type != DT_DIR) {
            continue;
        }
        char filePath[FILE_PATH_SIZE] = {0};
        int len = snprintf_s(filePath, FILE_PATH_SIZE, FILE_PATH_SIZE - 1, "/proc/%s/cmdline", ptr->d_name);
        if (len < 0) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of cmdline had be cut off");
            continue;
        }
        FILE* fp = fopen(filePath, "r");
        if (fp == nullptr) {
            HILOG_WARN(LOG_CORE, "open file failed!");
            break;
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
    int pid = fork();
    if (pid == 0) {
        CloseStdio();
        argv.push_back(nullptr); // last item in argv must be NULL
        int retval = execvp(processBin.c_str(), argv.data());
        if (retval == -1 && errno == EXECVP_ERRNO) {
            printf("warning: %s does not exist!\n", processBin.c_str());
            HILOG_WARN(LOG_CORE, "warning: %s does not exist!", processBin.c_str());
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
    kill(pid, SIGKILL);
    if (waitpid(pid, &stat, 0) == -1) {
        if (errno != EINTR) {
            stat = -1;
        }
    }
    return stat;
}

void PrintMallinfoLog(const std::string& mallInfoPrefix)
{
#ifdef HOOK_ENABLE
    struct mallinfo2 mallinfo = mallinfo2();
    std::string mallinfoLog = mallInfoPrefix;
    mallinfoLog += "arena = " + std::to_string(mallinfo.arena) + ", ordblks = " + std::to_string(mallinfo.ordblks);
    mallinfoLog += ", smblks = " + std::to_string(mallinfo.smblks) + ", hblks = " + std::to_string(mallinfo.hblks);
    mallinfoLog += ", hblkhd = " + std::to_string(mallinfo.hblkhd) + ", usmblks = " + std::to_string(mallinfo.usmblks);
    mallinfoLog +=
        ", fsmblks = " + std::to_string(mallinfo.fsmblks) + ", uordblks = " + std::to_string(mallinfo.uordblks);
    mallinfoLog +=
        ", fordblks = " + std::to_string(mallinfo.fordblks) + ", keepcost = " + std::to_string(mallinfo.keepcost);
    HILOG_INFO(LOG_CORE, "%s", mallinfoLog.c_str());
#endif  // HOOK_ENABLE
}

FILE* CustomPopen(int& childPid, const std::string& filePath, std::vector<std::string>& argv, const char* type)
{
    if (type == nullptr) {
        HILOG_ERROR(LOG_CORE, "Common:%s param invalid", __func__);
        return nullptr;
    }
    int fd[PIPE_LEN];
    pipe(fd);
    pid_t pid = fork();
    if (pid == -1) {
        HILOG_ERROR(LOG_CORE, "Common:%s fork failed!", __func__);
        return nullptr;
    }
    // child process
    if (pid == 0) {
        if (!strncmp(type, "r", strlen(type))) {
            close(fd[READ]);
            dup2(fd[WRITE], STDOUT_FILENO);  // Redirect stdout to pipe
            dup2(fd[WRITE], STDERR_FILENO);  // 2: Redirect stderr to pipe
        } else {
            close(fd[WRITE]);
            dup2(fd[READ], 0);  // Redirect stdin to pipe
        }
        setpgid(pid, pid);
        std::vector<char*> vectArgv;
        for (auto& item : argv) {
            vectArgv.push_back(const_cast<char*>(item.c_str()));
        }
        // execv : the last argv must be nullptr.
        vectArgv.push_back(nullptr);
        execv(filePath.c_str(), &vectArgv[0]);
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
    childPid = pid;
    if (!strncmp(type, "r", strlen(type))) {
        return fdopen(fd[READ], "r");
    }
    return fdopen(fd[WRITE], "w");
}

int CustomPclose(FILE* fp, int childPid)
{
    HILOG_INFO(LOG_CORE, "BEGN %s: ready!", __func__);
    CHECK_NOTNULL(fp, -1, "NOTE %s: fp is null", __func__);
    int stat = 0;
    if (fclose(fp) != 0) {
        const int bufSize = 256;
        char buf[bufSize] = {0};
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "Common: %s fclose failed! errno(%d:%s)", __func__, errno, buf);
    }
    if (waitpid(childPid, &stat, 0) == -1) {
        if (errno != EINTR) {
            const int bufSize = 256;
            char buf[bufSize] = {0};
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "Common: %s waitpid failed! errno(%d:%s)", __func__, errno, buf);
            return stat;
        }
    }
    HILOG_INFO(LOG_CORE, "END %s: success!", __func__);
    return stat;
}

int GetServicePort()
{
    const std::string portRangePath = "/proc/sys/net/ipv4/ip_local_port_range";
    std::ifstream file(portRangePath.c_str());
    if (!file.is_open()) {
        HILOG_ERROR(LOG_CORE, "Open file failed! filePath:%s", portRangePath.c_str());
        return -1;
    }
    std::string rangeStr;
    copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(rangeStr));

    int minPort;
    int maxPort;
    std::istringstream istr(rangeStr);
    istr >> minPort >> maxPort;
    const int offset = 3168; // To be compatible with previously used port 50051;
    int port = (minPort + maxPort) / 2 + offset;
    HILOG_DEBUG(LOG_CORE, "Service port is: %d", port);
    return port;
}

void SplitString(const std::string& str, const std::string &sep, std::vector<std::string>& ret)
{
    if (str.empty()) {
        HILOG_ERROR(LOG_CORE, "The string splited is empty!");
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
            HILOG_ERROR(LOG_CORE, "Get process name by pid failed!");
            return false;
        }
        bundleName = bundleName.substr(0, strlen(bundleName.c_str()));
    } else {
        bundleName = processName;
    }
    if (bundleName.empty()) {
        HILOG_ERROR(LOG_CORE, "Pid or process name is illegal!");
        return false;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        HILOG_ERROR(LOG_CORE, "GetSystemAbilityManager failed!");
        return false;
    }
    sptr<IRemoteObject> remoteObject = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR(LOG_CORE, "Get BundleMgr SA failed!");
        return false;
    }
    sptr<BundleMgrProxy> proxy = iface_cast<BundleMgrProxy>(remoteObject);
    int uid = proxy->GetUidByDebugBundleName(bundleName, Constants::ANY_USERID);
    if (uid < 0) {
        HILOG_ERROR(LOG_CORE, "Get %s uid = %d", bundleName.c_str(), uid);
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

bool ReadFile(const std::string &filePath, const std::vector<std::string>& validPaths, std::string& fileContent)
{
    char* realFilePath = realpath(filePath.c_str(), nullptr);
    if (realFilePath == nullptr) {
        HILOG_ERROR(LOG_CORE, "Fail to realPath: %s", filePath.c_str());
        return false;
    }

    std::string realFilePathStr(realFilePath);
    free(realFilePath);
    if (!VerifyPath(realFilePathStr, validPaths)) {
        HILOG_ERROR(LOG_CORE, "Fail to VerifyPath: %s", realFilePathStr.c_str());
        return false;
    }

    std::ifstream fileStream(realFilePathStr, std::ios::in);
    if (!fileStream.is_open()) {
        HILOG_ERROR(LOG_CORE, "Fail to open file %s", realFilePathStr.c_str());
        return false;
    }

    std::istreambuf_iterator<char> firstIt = { fileStream };
    std::string content(firstIt, {});
    fileContent = content;
    return true;
}

std::string GetErrorMsg()
{
    const int bufSize = 256;
    char buffer[bufSize] = { 0 };
    strerror_r(errno, buffer, bufSize);
    std::string errorMsg(buffer);
    return errorMsg;
}
} // namespace COMMON
