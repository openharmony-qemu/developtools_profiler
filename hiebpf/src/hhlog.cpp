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

#include "hhlog.h"

static constexpr int FILE_MODE = 0644;

HHLogger::~HHLogger()
{
    stop_.store(true);
    if (logSaver_.joinable()) {
        logSaver_.join();
    }
    if (fd_ >= 0) {
        close(fd_);
    }
}

int HHLogger::InitLogger(const int logLevel, const std::string& logFile)
{
    // Init logLevel, stop_, timer_ and buf_
    logLevel_ = logLevel;
    if (UpdateTimer() != 0) {
        return -1;
    }
    constexpr enum RingBuffer::MemAlignShift memAlign {RingBuffer::MemAlignShift::W_ALIGN_SHIFT};
    buf_.reset(new RingBuffer {SizeConsts::RING_BUF_SIZE, memAlign});
    if (buf_ == nullptr or (not (*buf_))) {
        return -1;
    }

    // open and link log file
    if (logFile.compare("stdout") == 0) {
        // use stdout as log file
        if (fcntl(STDOUT_FILENO, F_GETFL) != 0) {
            // stdout closed, reopen it
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

int HHLogger::Start(const int logLevel, const std::string& logFile)
{
    if (logLevel == HHLOG_NONE) {
        HHLOGD(true, "hhlog level is NONE!");
        return 0;
    }
#if defined(HH_LOGGER_DEBUG) || defined(HH_LOGGER_INFO) || defined(HH_LOGGER_WARN) ||   \
    defined(HH_LOGGER_ERROR) || defined(HH_LOGGER_FATAL)
    if (InitLogger(logLevel, logFile) != 0) {
        return -1;
    }
#endif
    return 0;
}

std::string HHLogger::GetLogFileName() const
{
    auto timer = timer_.load();
    time_t now = (time_t) timer.tv_sec;
    struct tm* tmPtr {nullptr};
    tmPtr = localtime(&now);
    if (tmPtr == nullptr) {
        return "";
    }
    std::stringstream ss;
    constexpr int yearStart {1900};
    constexpr int monthStart {1};
    ss << "/data/local/tmp/";
    ss << std::to_string(tmPtr->tm_year + yearStart) << ".";
    ss << std::to_string(tmPtr->tm_mon + monthStart) << ".";
    ss << std::to_string(tmPtr->tm_mday) << "_";
    ss << std::to_string(tmPtr->tm_hour) << ".";
    ss << std::to_string(tmPtr->tm_min) << ".";
    ss << std::to_string(tmPtr->tm_sec) << ".hhlog";
    return ss.str();
}

std::string HHLogger::GetFormatTime() const
{
    auto timer = timer_.load();
    time_t now = (time_t) timer.tv_sec;
    struct tm* tmPtr {nullptr};
    tmPtr = localtime(&now);
    if (tmPtr == nullptr) {
        return "";
    }
    std::stringstream ss;
    constexpr int yearStart {1900};
    constexpr int monthStart {1};
    ss << std::to_string(tmPtr->tm_year + yearStart) << "-";
    ss << std::to_string(tmPtr->tm_mon + monthStart) << "-";
    ss << std::to_string(tmPtr->tm_mday) << " ";
    ss << std::to_string(tmPtr->tm_hour) << ":";
    ss << std::to_string(tmPtr->tm_min) << ":";
    ss << std::to_string(tmPtr->tm_sec);
    return ss.str();
}

int HHLogger::UpdateTimer()
{
    struct timeval timer;
    int ret = gettimeofday(&timer, nullptr);
    if (ret != 0) {
        return -1;
    }
    timer_.store(timer);
    return 0;
}

int HHLogger::SaveLog()
{
    while (true) {
        if (UpdateTimer() != 0) {
            return -1;
        }
        auto dataSize = buf_->GetDataSize();
        if (stop_.load() and (dataSize == 0)) {
            return 0;
        }
        if (dataSize) {
            ssize_t ret = buf_->Write(fd_, dataSize);
            if (ret <= 0) {
                stop_.store(true);
                return -1;
            }
        }
    }
    return 0;
}