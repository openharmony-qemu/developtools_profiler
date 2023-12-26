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
 *
 * Description: FtraceDataReader implements
 */
#include "ftrace_data_reader.h"
#include "logging.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <climits>

FTRACE_NS_BEGIN
FtraceDataReader::FtraceDataReader(const std::string& path) : path_(path), readFd_(-1)
{
    char realPath[PATH_MAX + 1] = {0};

    if ((path.length() >= PATH_MAX) || (realpath(path.c_str(), realPath) == nullptr)) {
        HILOG_ERROR(LOG_CORE, "%s:so filename invalid, errno=%d", __func__, errno);
    }
    readFd_ = open(realPath, O_CLOEXEC | O_NONBLOCK);
    CHECK_TRUE(readFd_ >= 0, NO_RETVAL, "open %s failed, %d", realPath, errno);
}

FtraceDataReader::~FtraceDataReader()
{
    CHECK_TRUE(close(readFd_) == 0, NO_RETVAL, "close %s failed, %d", path_.c_str(), errno);
}

long FtraceDataReader::Read(uint8_t data[], uint32_t size)
{
    ssize_t nBytes = TEMP_FAILURE_RETRY(read(readFd_, data, size));
    return nBytes;
}
FTRACE_NS_END
