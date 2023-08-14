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

#include <ctime>
#include <sstream>
#include <sys/time.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include "bpf_log_reader.h"

static constexpr int FILE_MODE = 0644;

BPFLogReader::~BPFLogReader()
{
    Stop();
    if (worker_.joinable()) {
        worker_.join();
    }
    if (ifd_ >= 0) {
        close(ifd_);
        ifd_ = -1;
    }
    if (ofd_ >= 0) {
        close(ofd_);
        ofd_ = -1;
    }
}

std::unique_ptr<BPFLogReader> BPFLogReader::MakeUnique(const std::string& logFile)
{
    std::unique_ptr<BPFLogReader> logReader {new(std::nothrow) BPFLogReader {}};
    if (logReader == nullptr) {
        return nullptr;
    }
    if (logReader->EnableTracePipe() != 0) {
        return nullptr;
    }
    if (logReader->OpenTracePipe() != 0) {
        return nullptr;
    }
    if (logReader->OpenLogFile(logFile) != 0) {
        return nullptr;
    }
    return logReader;
}

std::string BPFLogReader::GetLogFileName() const
{
    struct timeval timer;
    gettimeofday(&timer, nullptr);
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
    ss << std::to_string(tmPtr->tm_sec) << ".bpf.log";
    return ss.str();
}

int BPFLogReader::OpenLogFile(const std::string& logFile)
{
    if (logFile.compare("stdout") == 0) {
        if (fcntl(STDOUT_FILENO, F_GETFL)) {
            ofd_ = open("/dev/stdout", O_WRONLY);
        } else {
            ofd_ = STDOUT_FILENO;
        }
        if (ofd_ < 0) {
            return -1;
        }
        return 0;
    }
    auto fileName = GetLogFileName();
    if (fileName.length() == 0) {
        return -1;
    }
    ofd_ = open(fileName.c_str(), O_WRONLY | O_CREAT, FILE_MODE);
    if (ofd_ < 0) {
        return -1;
    }
    unlink(logFile.c_str());
    if (link(fileName.c_str(), logFile.c_str()) != 0) {
        return -1;
    }

    return 0;
}

int BPFLogReader::EnableTracePipe() const
{
    int fd = open(confPath_.c_str(), O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    if (ftruncate(fd, 0) != 0) {
    }
    char c {'1'};
    int ret = write(fd, &c, sizeof(c));
    if (ret != sizeof(c)) {
        return -1;
    }
    close(fd);

    return 0;
}

int BPFLogReader::OpenTracePipe()
{
    ifd_ = open(pipePath_.c_str(), O_RDONLY | O_NONBLOCK);
    if (ifd_ < 0) {
        return -1;
    }
    return 0;
}

int BPFLogReader::MoveBPFLog()
{
    constexpr size_t bufSize {1024};
    char buffer[bufSize];
    while (true) {
        struct pollfd fds;
        fds.fd = ifd_;
        fds.events = POLLIN;
        constexpr int timeout {10};
        constexpr nfds_t nfds {1};
        int ret = poll(&fds, nfds, timeout);
        if (stop_ or ret == -1) {
            break;
        }
        if (ret == nfds) {
            ret = read(ifd_, buffer, sizeof(buffer));
            if (ret <= 0) {
                break;
            }
            ret = write(ofd_, buffer, ret);
            if (ret <= 0) {
                break;
            }
        }
    }
    return 0;
}