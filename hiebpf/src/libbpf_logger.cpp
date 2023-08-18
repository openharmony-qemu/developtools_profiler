/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#include <cstdio>
#include <ctime>
#include <sstream>

#include <sys/time.h>


#include "libbpf_logger.h"
#include "hhlog.h"

static constexpr int FILE_MODE = 0644;

std::unique_ptr<LIBBPFLogger> LIBBPFLogger::MakeUnique(const std::string& logFile, int logLevel)
{
    std::unique_ptr<LIBBPFLogger> logger {new(std::nothrow) LIBBPFLogger {logLevel}};
    if (logger == nullptr) {
        return nullptr;
    }
#if defined(BPF_LOGGER_DEBUG) || defined(BPF_LOGGER_INFO) || defined(BPF_LOGGER_WARN) ||   \
    defined(BPF_LOGGER_ERROR) || defined(BPF_LOGGER_FATAL)
    if (logger->OpenLogFile(logFile) != 0) {
        return nullptr;
    }
#endif
    return logger;
}

int LIBBPFLogger::Printf(int logLevel, const char* format, va_list args)
{
    HHLOGI(true, "current libbpf log level = %d, target level = %d", logLevel, logLevel_);
    if (logLevel > logLevel_) {
        return 0;
    }
#if defined(BPF_LOGGER_DEBUG) || defined(BPF_LOGGER_INFO) || defined(BPF_LOGGER_WARN) ||   \
    defined(BPF_LOGGER_ERROR) || defined(BPF_LOGGER_FATAL)
    return vdprintf(fd_, format, args);
#else
    return 0;
#endif
}

int LIBBPFLogger::OpenLogFile(const std::string& logFile)
{
    if (logFile.compare("stdout") == 0) {
        if (fcntl(STDOUT_FILENO, F_GETFL)) {
            fd_ = open("/dev/stdout", O_WRONLY);
        } else {
            fd_ = STDOUT_FILENO;
        }
        if (fd_ < 0) {
            return -1;
        }
        return 0;
    }
    auto fileName = GetLogFileName();
    if (fileName.length() == 0) {
        return -1;
    }
    fileName = "/data/local/tmp/" + fileName;
    fd_ = open(fileName.c_str(), O_WRONLY | O_CREAT, FILE_MODE);
    if (fd_ < 0) {
        return -1;
    }
    unlink(logFile.c_str());
    if (link(fileName.c_str(), logFile.c_str()) != 0) {
        return -1;
    }

    return 0;
}

std::string LIBBPFLogger::GetLogFileName() const
{
    struct timeval timer;
    gettimeofday(&timer, nullptr);
    time_t now = (time_t) timer.tv_sec;
    struct tm* tmPtr {nullptr};
    tmPtr = localtime(&now);
    CHECK_NOTNULL(tmPtr, "", "GetLogFileName fail");
    std::stringstream ss;
    constexpr int yearStart {1900};
    constexpr int monthStart {1};
    ss << std::to_string(tmPtr->tm_year + yearStart) << ".";
    ss << std::to_string(tmPtr->tm_mon + monthStart) << ".";
    ss << std::to_string(tmPtr->tm_mday) << "_";
    ss << std::to_string(tmPtr->tm_hour) << ".";
    ss << std::to_string(tmPtr->tm_min) << ".";
    ss << std::to_string(tmPtr->tm_sec) << ".libbpf.log";
    return ss.str();
}