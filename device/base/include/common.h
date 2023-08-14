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

#include <string>
#include <vector>
#include <sys/types.h>
#include <unistd.h>

namespace COMMON {
bool IsProcessRunning(int& lockFileFd); // add file lock, only one process can run
bool IsProcessExist(const std::string& processName, int& pid); // Check if the process exists and get PID
int StartProcess(const std::string& processBin, std::vector<char*>& argv);
int KillProcess(int pid);
void PrintMallinfoLog(const std::string& mallInfoPrefix);
inline int CustomFdClose(int& fd);
inline int CustomFdFclose(FILE** fp);
FILE* CustomPopen(const std::vector<std::string>& command, const char* type, int fds[],
                  volatile pid_t& childPid, bool needUnblock = false);
int CustomPclose(FILE* fp, int fds[], volatile pid_t& childPid, bool needUnblock = false);
int CustomPUnblock(int fds[]);
int GetServicePort();
void SplitString(const std::string& str, const std::string &sep, std::vector<std::string>& ret);
bool CheckApplicationPermission(int pid, const std::string& processName);
bool VerifyPath(const std::string& filePath, const std::vector<std::string>& validPaths);
bool ReadFile(const std::string& filePath, const std::vector<std::string>& validPaths, std::string& fileContent);
std::string GetErrorMsg();
std::string GetTimeStr();
clockid_t GetClockId(const std::string& clockIdStr);
void AdaptSandboxPath(std::string& filePath, int pid);
} // COMMON
#endif // COMMON_H