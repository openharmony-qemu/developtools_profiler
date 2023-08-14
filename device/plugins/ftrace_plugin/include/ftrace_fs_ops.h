/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#ifndef FTRACE_FS_OPS_H
#define FTRACE_FS_OPS_H

#include <string>
#include <vector>

#include "ftrace_namespace.h"

#ifndef MOCKABLE
#define MOCKABLE virtual
#endif

FTRACE_NS_BEGIN
class FtraceFsOps {
public:
    static FtraceFsOps& GetInstance();

    static std::string GetFtraceRoot();

    MOCKABLE std::string GetKernelSymbols() const;
    MOCKABLE std::string GetPrintkFormats() const;

    MOCKABLE std::string GetProcessComm(int pid);
    MOCKABLE std::string GetThreadComm(int pid, int tid);

    // for ftrace status nodes
    bool SetSavedCmdLinesSize(uint32_t size);
    MOCKABLE std::string GetSavedCmdLines() const;
    MOCKABLE std::string GetSavedTgids() const;
    MOCKABLE std::string GetPerCpuStats(int cpu) const;
    MOCKABLE std::string GetRawTracePath(int cpu) const;
    MOCKABLE std::string GetPageHeaderFormat() const;
    MOCKABLE std::string GetEventDataFormat(const std::string& type, const std::string& name) const;
    MOCKABLE std::vector<std::pair<std::string, std::string>> GetPlatformEvents();

    // for ftrace control nodes
    MOCKABLE bool ClearTraceBuffer();
    MOCKABLE bool SetRecordCmdOption(bool enable);
    MOCKABLE bool SetRecordTgidOption(bool enable);
    MOCKABLE bool SetBufferSizeKb(int sizeKb);
    MOCKABLE bool SetTraceClock(const std::string& clock);
    MOCKABLE std::string GetTraceClock();
    MOCKABLE bool AppendSetEvent(const std::string& type, const std::string& name);
    MOCKABLE bool ClearSetEvent();
    MOCKABLE bool EnableEvent(const std::string& type, const std::string& name);
    MOCKABLE bool DisableEvent(const std::string& type, const std::string& name);
    MOCKABLE bool DisableCategories(const std::string& categories);
    MOCKABLE bool EnableTracing();
    MOCKABLE bool DisableTracing();

    // for UT
    void SetFtraceRoot(std::string path)
    {
        ftraceRoot_ = path;
    }

private:
    FtraceFsOps();
    MOCKABLE ~FtraceFsOps();
    DISALLOW_COPY_AND_MOVE(FtraceFsOps);

private:
    std::string ftraceRoot_;
};
FTRACE_NS_END

#endif // FTRACE_FS_OPS_H