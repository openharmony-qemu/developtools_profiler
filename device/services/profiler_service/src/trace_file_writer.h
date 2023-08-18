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
#ifndef TRANCE_FILE_WRITER_H
#define TRANCE_FILE_WRITER_H

#include <cstdint>
#include <fstream>
#include <google/protobuf/message_lite.h>
#include <mutex>
#include <queue>
#include <string>

#include "logging.h"
#include "nocopyable.h"
#include "trace_file_helper.h"
#include "writer.h"

using google::protobuf::MessageLite;

class TraceFileWriter : public Writer {
public:
    explicit TraceFileWriter(const std::string& path);

    explicit TraceFileWriter(const std::string& path, bool splitFile, uint32_t splitFileMaxSizeMb,
        uint32_t splitFileMaxNum);

    ~TraceFileWriter();

    std::string Path() const;

    bool SetPluginConfig(const void* data, size_t size);

    bool WriteHeader();

    long Write(const MessageLite& message);

    long Write(const void* data, size_t size) override;

    bool Flush() override;

    bool Finish();

    bool IsSplitFile(uint32_t size);

    void SetStopSplitFile(bool isStop);

private:
    void SetTimeStamp();

    void LogDiskUsage();

    void DeleteOldSplitFile();

    bool FlushStream();

private:
    std::string path_ {};
    std::string oldPath_ {};
    std::ofstream stream_ {};
    uint64_t writeBytes_ = 0;
    uint64_t writeCount_ = 0;
    TraceFileHeader header_ {};
    TraceFileHelper helper_ {};
    uint32_t dataSize_ = 0;
    bool isSplitFile_ = false;
    uint32_t splitFileMaxSize_;
    uint32_t splitFileMaxNum_;
    std::queue<std::string> splitFilePaths_;
    std::vector<std::vector<char>> pluginConfigsData_;
    bool isStop_ = false;
    int fileNum_;

    DISALLOW_COPY_AND_MOVE(TraceFileWriter);
};

using TraceFileWriterPtr = STD_PTR(shared, TraceFileWriter);

#endif // !TRANCE_FILER_WRITER_H