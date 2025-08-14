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
#ifndef COMMON_H
#define COMMON_H

#include <malloc.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <unistd.h>


namespace COMMON {
bool IsProcessRunning(int& lockFileFd); // add file lock, only one process can run
bool IsProcessExist(const std::string& processName, int& pid); // Check if the process exists and get PID
int StartProcess(const std::string& processBin, std::vector<char*>& argv);
int KillProcess(int pid);
bool CheckSubscribeVersion(const std::string& version);
void PrintMallinfoLog(const std::string& mallInfoPrefix, const struct mallinfo2& mi);
inline int CustomFdClose(int& fd);
inline int CustomFdFclose(FILE** fp);
FILE* CustomPopen(const std::vector<std::string>& command, const char* type, int fds[],
                  volatile pid_t& childPid, bool needUnblock = false);
int CustomPclose(FILE* fp, int fds[], volatile pid_t& childPid, bool needUnblock = false);
int CustomPUnblock(int fds[]);
int GetServicePort();
void SplitString(const std::string& str, const std::string &sep, std::vector<std::string>& ret);
bool CheckApplicationPermission(int pid, const std::string& processName);
bool CheckApplicationEncryped(int pid, const std::string& processName);
bool VerifyPath(const std::string& filePath, const std::vector<std::string>& validPaths);
bool ReadFile(const std::string& filePath, const std::vector<std::string>& validPaths, std::string& fileContent);
std::string GetErrorMsg();
std::string GetTimeStr();
clockid_t GetClockId(const std::string& clockIdStr);
std::string GetClockStr(const int32_t clockId);
void AdaptSandboxPath(std::string& filePath, int pid);
int32_t GetPackageUid(const std::string &name);
bool GetCurrentUserId(int32_t &userId);
bool IsNumeric(const std::string& str);
bool IsUserMode();
bool GetUidGidFromPid(pid_t pid, uid_t& ruid, gid_t& rgid);
bool GetDeveloperMode();
bool ContainsSpecialChars(const std::string& input);
bool IsBetaVersion();
std::pair<bool, std::string> CheckNotExistsFilePath(const std::string& filePath);
bool CheckWhiteList(const std::string& cmdPath);
bool CheckCmdLineArgValid(const std::string& cmdLine);
int PluginWriteToHisysevent (const std::string& pluginName, const std::string& caller, const std::string& args,
                             const int errorCode, const std::string& errorMessage);
std::string GetProcessNameByPid(int32_t pid);

static const std::string STATE_VERSION = "1.0";
enum ErrorType {
    RET_NO_PERMISSION,
    RET_NOT_SUPPORT,
    RET_INVALID_PATH,
    RET_INVALID_PID,
    RET_MSG_EMPTY,
    RET_FAIL = -1,
    RET_SUCC = 0,
};

class SpinLock {
public:
    void Lock()
    {
        while (flag.test_and_set(std::memory_order_acquire)) {}
    }

    void Unlock()
    {
        flag.clear(std::memory_order_release);
    }
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};
} // COMMON
#endif // COMMON_H