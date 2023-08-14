/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FILE_CACHE_H_
#define FILE_CACHE_H_

#include <algorithm>
#include <climits>
#include <cstdint>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

#include "logging.h"
#include "securec.h"

class FileCache {
public:
    FileCache(const std::string &path);
    ~FileCache();

    bool Open(const std::string& name);

    long Write(char* bytes, int32_t len);

    long Read(char* buff);

    bool Close();

private:
    std::string path_;
    FILE* fp_;
    std::unordered_map<uint64_t, uint64_t> size_;
};

#endif // !FILE_CACHE_H_