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
#include "ftrace_fs_ops.h"

#include <fcntl.h>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "file_utils.h"
#include "logging.h"

FTRACE_NS_BEGIN
FtraceFsOps& FtraceFsOps::GetInstance()
{
    static FtraceFsOps instance;
    return instance;
}

FtraceFsOps::FtraceFsOps() : ftraceRoot_(GetFtraceRoot())
{
    HILOG_INFO(LOG_CORE, "FtraceFsOps create!");
}

FtraceFsOps::~FtraceFsOps()
{
    HILOG_INFO(LOG_CORE, "FtraceFsOps destroy!");
}

std::string FtraceFsOps::GetFtraceRoot()
{
    std::vector<std::string> testRootPath = {"/sys/kernel/tracing", "/sys/kernel/debug/tracing"};
    for (auto iter = testRootPath.begin(); iter != testRootPath.end(); ++iter) {
        auto path = *iter + "/events";
        struct stat s;
        lstat(path.c_str(), &s);
        if (S_ISDIR(s.st_mode)) {
            return *iter;
        }
    }
    return "";
}

std::string FtraceFsOps::GetPrintkFormats() const
{
    return FileUtils::ReadFile(ftraceRoot_ + "/printk_formats");
}

std::string GetKptrRestrict()
{
    return FileUtils::ReadFile("/proc/sys/kernel/kptr_restrict");
}

bool SetKptrRestrict(const std::string& value)
{
    return FileUtils::WriteFile("/proc/sys/kernel/kptr_restrict", value) > 0;
}

std::string FtraceFsOps::GetKernelSymbols() const
{
    std::string restrictValue = GetKptrRestrict();
    CHECK_TRUE(restrictValue.size() > 0, "", "read kptr_restrict failed!");

    bool valueChanged = false;
    if (std::stoi(restrictValue) == 0) {
        SetKptrRestrict("1");
        valueChanged = true;
    }

    std::string result = FileUtils::ReadFile("/proc/kallsyms");
    if (valueChanged) {
        SetKptrRestrict(restrictValue);
    }
    return result;
}

bool FtraceFsOps::SetSavedCmdLinesSize(uint32_t size)
{
    std::string path = ftraceRoot_ + "/saved_cmdlines_size";
    return FileUtils::WriteFile(path, std::to_string(static_cast<int>(size))) > 0;
}

std::string FtraceFsOps::GetSavedCmdLines() const
{
    return FileUtils::ReadFile(ftraceRoot_ + "/saved_cmdlines");
}

std::string FtraceFsOps::GetSavedTgids() const
{
    return FileUtils::ReadFile(ftraceRoot_ + "/saved_tgids");
}

std::string FtraceFsOps::GetProcessComm(int pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/comm";
    if (access(path.c_str(), R_OK) != 0) {
        return "";
    }
    return FileUtils::ReadFile(path);
}

std::string FtraceFsOps::GetThreadComm(int pid, int tid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/task/" + std::to_string(tid) + "/comm";
    if (access(path.c_str(), R_OK) != 0) {
        return "";
    }
    return FileUtils::ReadFile(path);
}

std::string FtraceFsOps::GetPerCpuStats(int cpu) const
{
    return FileUtils::ReadFile(ftraceRoot_ + "/per_cpu/cpu" + std::to_string(cpu) + "/stats");
}

std::string FtraceFsOps::GetRawTracePath(int cpu) const
{
    return ftraceRoot_ + "/per_cpu/cpu" + std::to_string(cpu) + "/trace_pipe_raw";
}

std::string FtraceFsOps::GetPageHeaderFormat() const
{
    return FileUtils::ReadFile(ftraceRoot_ + "/events/header_page");
}

std::string FtraceFsOps::GetEventDataFormat(const std::string& type, const std::string& name) const
{
    return FileUtils::ReadFile(ftraceRoot_ + "/events/" + type + "/" + name + "/format");
}

bool FtraceFsOps::ClearTraceBuffer()
{
    char realPath[PATH_MAX + 1] = {0};

    std::string path = ftraceRoot_ + "/trace";
    CHECK_TRUE((path.length() < PATH_MAX) && (realpath(path.c_str(), realPath) != nullptr), false,
               "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
    int fd = open(realPath, O_TRUNC);
    CHECK_TRUE(fd >= 0, false, "open %s failed!", realPath);
    return close(fd) == 0;
}

bool FtraceFsOps::SetRecordCmdOption(bool enable)
{
    std::string path = ftraceRoot_ + "/options/record-cmd";
    return FileUtils::WriteFile(path, std::to_string(static_cast<int>(enable))) > 0;
}

bool FtraceFsOps::SetRecordTgidOption(bool enable)
{
    std::string path = ftraceRoot_ + "/options/record-tgid";
    return FileUtils::WriteFile(path, std::to_string(static_cast<int>(enable))) > 0;
}

bool FtraceFsOps::SetBufferSizeKb(int sizeKb)
{
    std::string path = ftraceRoot_ + "/buffer_size_kb";
    return FileUtils::WriteFile(path, std::to_string(sizeKb)) > 0;
}

bool FtraceFsOps::SetTraceClock(const std::string& clock)
{
    std::string path = ftraceRoot_ + "/trace_clock";
    return FileUtils::WriteFile(path, clock) > 0;
}

std::string FtraceFsOps::GetTraceClock()
{
    std::string path = ftraceRoot_ + "/trace_clock";
    std::string value = FileUtils::ReadFile(path);
    auto pos = value.find('[');
    CHECK_TRUE(pos != std::string::npos, "", "find [ in %s failed!", path.c_str());
    pos++;

    auto rpos = value.find(']', pos);
    CHECK_TRUE(rpos != std::string::npos, "", "find ] in %s failed!", path.c_str());
    return value.substr(pos, rpos - pos);
}

std::vector<std::pair<std::string, std::string>> FtraceFsOps::GetPlatformEvents()
{
    std::set<std::pair<std::string, std::string>> eventSet;
    std::string eventsPath = ftraceRoot_ + "/events";
    for (auto& type : FileUtils::ListDir(eventsPath)) {
        struct stat st = {};
        std::string typePath = eventsPath + "/" + type;
        if (stat(typePath.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
            continue;
        }
        for (auto& name : FileUtils::ListDir(typePath)) {
            struct stat st = {};
            std::string namePath = typePath + "/" + name;
            if (stat(namePath.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
                continue;
            }
            eventSet.insert(std::make_pair(type, name));
        }
    }
    HILOG_INFO(LOG_CORE, "get platform event formats done, types: %zu!", eventSet.size());
    return {eventSet.begin(), eventSet.end()};
}

bool FtraceFsOps::AppendSetEvent(const std::string& type, const std::string& name)
{
    std::string path = ftraceRoot_ + "/set_event";
    return FileUtils::WriteFile(path, type + ":" + name + "\n", O_WRONLY | O_APPEND) > 0;
}

bool FtraceFsOps::ClearSetEvent()
{
    return FileUtils::WriteFile(ftraceRoot_ + "/set_event", "\n", O_WRONLY | O_TRUNC) > 0;
}

bool FtraceFsOps::EnableEvent(const std::string& type, const std::string& name)
{
    std::string enablePath = ftraceRoot_ + "/events/" + type + "/" + name + "/enable";
    return FileUtils::WriteFile(enablePath, "1") > 0;
}

bool FtraceFsOps::DisableEvent(const std::string& type, const std::string& name)
{
    std::string enablePath = ftraceRoot_ + "/events/" + type + "/" + name + "/enable";
    return FileUtils::WriteFile(enablePath, "0") > 0;
}

bool FtraceFsOps::DisableCategories(const std::string& categories)
{
    std::string enablePath = ftraceRoot_ + "/events/" + categories + "/enable";
    return FileUtils::WriteFile(enablePath, "0") > 0;
}

bool FtraceFsOps::EnableTracing()
{
    std::string tracingOn = ftraceRoot_ + "/tracing_on";
    return FileUtils::WriteFile(tracingOn, "1") > 0;
}

bool FtraceFsOps::DisableTracing()
{
    std::string tracingOn = ftraceRoot_ + "/tracing_on";
    return FileUtils::WriteFile(tracingOn, "0") > 0;
}
FTRACE_NS_END
