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
 * Description: FtraceDataReader define
 */
#ifndef FTRACE_DATA_READER_H
#define FTRACE_DATA_READER_H

#include <string>
#include "ftrace_namespace.h"

FTRACE_NS_BEGIN
class FtraceDataReader {
public:
    explicit FtraceDataReader(const std::string& path);
    ~FtraceDataReader();

    long Read(uint8_t data[], uint32_t size);

private:
    DISALLOW_COPY_AND_MOVE(FtraceDataReader);
    std::string path_;
    int readFd_;
};
FTRACE_NS_END

#endif