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

#ifndef LIBBPF_LOGGER_H
#define LIBBPF_LOGGER_H

#include <memory>
#include <string>

#include <unistd.h>
#include <fcntl.h>

#include "libbpf.h"

class LIBBPFLogger {
public:
    ~LIBBPFLogger()
    {
        if (fd_ >= 0) {
            close(fd_);
        }
    }

    static std::unique_ptr<LIBBPFLogger> MakeUnique(const std::string& logFile, int logLevel);
    int Printf(int logLevel, const char* format, va_list args);

    enum Constants:int {
        MAX_LIBBPF_LOG_LEN = 1024,
    };

private:
    LIBBPFLogger(int logLevel):logLevel_ {logLevel} {}
    int OpenLogFile(const std::string& logFile);
    std::string GetLogFileName() const;

    const int logLevel_ {LIBBPF_DEBUG};
    int fd_ {-1};
};

#endif