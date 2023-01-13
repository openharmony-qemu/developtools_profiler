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
#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

namespace COMMON {
bool IsProcessRunning(); // add file lock, only one process can run
bool IsProcessExist(std::string& processName, int& pid); // Check if the process exists and get PID
int StartProcess(const std::string& processBin, std::vector<char*>& argv);
int KillProcess(int pid);
void PrintMallinfoLog(const std::string& mallInfoPrefix);
int GetServicePort();
} // COMMON

#endif // COMMON_H