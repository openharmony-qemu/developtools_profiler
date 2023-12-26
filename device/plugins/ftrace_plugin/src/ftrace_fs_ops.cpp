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

FtraceFsOps::FtraceFsOps() : ftraceRoot_(GetFtraceRoot()), hmTraceDir_(GetHmTraceDir())
{
    PROFILER_LOG_INFO(LOG_CORE, "FtraceFsOps create!");
}

FtraceFsOps::~FtraceFsOps()
{
    PROFILER_LOG_INFO(LOG_CORE, "FtraceFsOps destroy!");
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

std::string FtraceFsOps::GetHmTraceDir()
{
    if (access((ftraceRoot_ + "/hongmeng").c_str(), F_OK) == 0) {
        return "/hongmeng";
    } else {
        return "";
    }
}

int FtraceFsOps::WriteTraceFile(const std::string& path, const std::string& content)
{
    bool ret = false;
    if (access((ftraceRoot_ + hmTraceDir_ + path).c_str(), W_OK) == 0) {
        if (FileUtils::WriteFile(ftraceRoot_ + hmTraceDir_ + path, content) > 0) {
            ret = true;
        }
    }
    if (access((ftraceRoot_ + path).c_str(), W_OK) == 0) {
        if (FileUtils::WriteFile(ftraceRoot_ + path, content) > 0) {
            ret = true;
        }
    }

    return (int)ret;
}

int FtraceFsOps::WriteTraceFile(const std::string& path, const std::string& content, int flags)
{
    bool ret = false;
    if (access((ftraceRoot_ + hmTraceDir_ + path).c_str(), W_OK) == 0) {
        if (FileUtils::WriteFile(ftraceRoot_ + hmTraceDir_ + path, content, flags) > 0) {
            ret = true;
        }
    }
    if (access((ftraceRoot_ + path).c_str(), W_OK) == 0) {
        if (FileUtils::WriteFile(ftraceRoot_ + path, content, flags) > 0) {
            ret = true;
        }
    }

    return (int)ret;
}

std::string FtraceFsOps::ReadTraceFile(const std::string& path) const
{
    if (access((ftraceRoot_ + hmTraceDir_ + path).c_str(), R_OK) == 0) {
        return FileUtils::ReadFile(ftraceRoot_ + hmTraceDir_ + path);
    } else {
        return FileUtils::ReadFile(ftraceRoot_ + path);
    }
}

std::string FtraceFsOps::GetPrintkFormats() const
{
    return ReadTraceFile("/printk_formats");
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
    std::string path = "/saved_cmdlines_size";
    return WriteTraceFile(path, std::to_string(static_cast<int>(size))) > 0;
}

std::string FtraceFsOps::GetSavedCmdLines() const
{
    return ReadTraceFile("/saved_cmdlines");
}

std::string FtraceFsOps::GetSavedTgids() const
{
    return ReadTraceFile("/saved_tgids");
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
    return ReadTraceFile("/per_cpu/cpu" + std::to_string(cpu) + "/stats");
}

std::string FtraceFsOps::GetRawTracePath(int cpu) const
{
    return ftraceRoot_ + "/per_cpu/cpu" + std::to_string(cpu) + "/trace_pipe_raw";
}

std::string FtraceFsOps::GetHmRawTracePath() const
{
    return ftraceRoot_ + hmTraceDir_ + "/trace_pipe_raw";
}

std::string FtraceFsOps::GetPageHeaderFormat() const
{
    return ReadTraceFile("/events/header_page");
}

std::string FtraceFsOps::GetEventDataFormat(const std::string& type, const std::string& name) const
{
    if (access((ftraceRoot_ + "/events/" + type + "/" + name + "/format").c_str(), R_OK) == 0) {
        return FileUtils::ReadFile(ftraceRoot_ + "/events/" + type + "/" + name + "/format");
    } else {
        return "";
    }
}

std::string FtraceFsOps::HmGetEventDataFormat(const std::string& type, const std::string& name) const
{
    if (access((ftraceRoot_ + "/hongmeng/events/" + type + "/" + name + "/format").c_str(), R_OK) == 0) {
        return FileUtils::ReadFile(ftraceRoot_ + "/hongmeng/events/" + type + "/" + name + "/format");
    } else {
        return "";
    }
}

bool FtraceFsOps::ClearTraceBuffer()
{
    char realPath[PATH_MAX + 1] = {0};

    std::string path;
    if (access((ftraceRoot_ + hmTraceDir_ + "/trace").c_str(), F_OK) == 0) {
        path = ftraceRoot_ + hmTraceDir_ + "/trace";
    } else {
        path = ftraceRoot_ + "/trace";
    }

    CHECK_TRUE((path.length() < PATH_MAX) && (realpath(path.c_str(), realPath) != nullptr), false,
               "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
    int fd = open(realPath, O_TRUNC | O_RDWR);
    CHECK_TRUE(fd >= 0, false, "open %s failed!", realPath);
    return close(fd) == 0;
}

bool FtraceFsOps::SetRecordCmdOption(bool enable)
{
    std::string path = "/options/record-cmd";
    return WriteTraceFile(path, std::to_string(static_cast<int>(enable))) > 0;
}

bool FtraceFsOps::SetRecordTgidOption(bool enable)
{
    std::string path = "/options/record-tgid";
    return WriteTraceFile(path, std::to_string(static_cast<int>(enable))) > 0;
}

bool FtraceFsOps::SetBufferSizeKb(int sizeKb)
{
    std::string path = "/buffer_size_kb";
    return WriteTraceFile(path, std::to_string(sizeKb)) > 0;
}

bool FtraceFsOps::SetTraceClock(const std::string& clock)
{
    std::string path = "/trace_clock";
    return WriteTraceFile(path, clock) > 0;
}

std::string FtraceFsOps::GetTraceClock()
{
    std::string path;
    if (access((ftraceRoot_ + hmTraceDir_ + "/trace_clock").c_str(), F_OK) == 0) {
        path = ftraceRoot_ + hmTraceDir_ + "/trace_clock";
    } else {
        path = ftraceRoot_ + "/trace_clock";
    }

    std::string value = FileUtils::ReadFile(path);
    auto pos = value.find('[');
    CHECK_TRUE(pos != std::string::npos, "", "find [ in %s failed!", path.c_str());
    pos++;

    auto rpos = value.find(']', pos);
    CHECK_TRUE(rpos != std::string::npos, "", "find ] in %s failed!", path.c_str());
    return value.substr(pos, rpos - pos);
}

static void AddPlatformEvents(std::set<std::pair<std::string, std::string>> &eventSet,
    const std::string &eventsPath)
{
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
}

std::vector<std::pair<std::string, std::string>> FtraceFsOps::GetPlatformEvents()
{
    std::set<std::pair<std::string, std::string>> eventSet;
    if (hmTraceDir_ != "") {
        AddPlatformEvents(eventSet, ftraceRoot_ + hmTraceDir_ + "/events");
    }
    AddPlatformEvents(eventSet, ftraceRoot_ + "/events");
    PROFILER_LOG_INFO(LOG_CORE, "get platform event formats done, types: %zu!", eventSet.size());
    return {eventSet.begin(), eventSet.end()};
}

bool FtraceFsOps::AppendSetEvent(const std::string& type, const std::string& name)
{
    std::string path = "/set_event";
    return WriteTraceFile(path, type + ":" + name + "\n", O_WRONLY | O_APPEND) > 0;
}

bool FtraceFsOps::ClearSetEvent()
{
    return WriteTraceFile("/set_event", "\n", O_WRONLY | O_TRUNC) > 0;
}

bool FtraceFsOps::EnableEvent(const std::string& type, const std::string& name)
{
    std::string enablePath = "/events/" + type + "/" + name + "/enable";
    return WriteTraceFile(enablePath, "1") > 0;
}

bool FtraceFsOps::DisableEvent(const std::string& type, const std::string& name)
{
    std::string enablePath = "/events/" + type + "/" + name + "/enable";
    return WriteTraceFile(enablePath, "0") > 0;
}

bool FtraceFsOps::DisableCategories(const std::string& categories)
{
    std::string enablePath = "/events/" + categories + "/enable";
    return WriteTraceFile(enablePath, "0") > 0;
}

bool FtraceFsOps::EnableTracing()
{
    std::string tracingOn = "/tracing_on";
    return WriteTraceFile(tracingOn, "1") > 0;
}

bool FtraceFsOps::DisableTracing()
{
    std::string tracingOn = "/tracing_on";
    return WriteTraceFile(tracingOn, "0") > 0;
}
FTRACE_NS_END
