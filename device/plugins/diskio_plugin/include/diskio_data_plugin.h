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

// pgpgin: Total number of kilobytes the system paged in from disk.
// pgpgout: Total number of kilobytes the system paged out to disk.
// see https://man7.org/linux/man-pages/man5/proc.5.html

#ifndef DISKIO_DATA_PLUGIN_H
#define DISKIO_DATA_PLUGIN_H

#include <fcntl.h>
#include <string>
#include <unistd.h>

#include "diskio_plugin_config.pb.h"
#include "diskio_plugin_result.pb.h"
#include "logging.h"
#include "io_stats.h"

enum ErrorType {
    RET_NULL_ADDR,
    RET_IVALID_PID,
    RET_TGID_VALUE_NULL,
    RET_FAIL = -1,
    RET_SUCC = 0,
};

class DiskioDataPlugin {
public:
    DiskioDataPlugin();
    ~DiskioDataPlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Report(uint8_t* configData, uint32_t configSize);
    int Stop();

private:
    int32_t ReadFile(std::string& fileName);
    void SetTimestamp(CollectTimeStamp& prevTimestamp, CollectTimeStamp& timestamp);
    void SetDiskioData(DiskioData& diskioData, const char* pFile, uint32_t fileLen);
    void WriteDiskioData(DiskioData& diskioData);

    // for UT
    void SetPath(std::string path)
    {
        path_ = path;
    }

private:
    /* data */
    void* buffer_;
    std::string path_;
    int32_t err_;

    int64_t prevRdSectorsKb_;
    int64_t prevWrSectorsKb_;
    CollectTimeStamp prevTimestamp_;
    DiskioConfig protoConfig_;
    std::shared_ptr<IoStats> ioEntry_;
};

#endif
