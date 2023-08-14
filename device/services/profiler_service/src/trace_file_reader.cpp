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
#include <memory>

#include "trace_file_reader.h"

#include "logging.h"

using CharPtr = std::unique_ptr<char>::pointer;

TraceFileReader::~TraceFileReader()
{
    if (stream_.is_open()) {
        stream_.close();
    }
}

TraceFileHeader TraceFileReader::GetHeader() const
{
    return header_;
}

bool TraceFileReader::ValidateHeader()
{
    return helper_.Validate(header_);
}

bool TraceFileReader::Open(const std::string& path)
{
    stream_.open(path, std::ios_base::in | std::ios_base::binary);
    CHECK_TRUE(stream_.is_open(), false, "open %s failed, %d!", path.c_str(), errno);

    stream_.read(reinterpret_cast<CharPtr>(&header_), sizeof(header_));
    CHECK_TRUE(stream_, false, "read header from %s failed!", path_.c_str());

    path_ = path;
    return true;
}

static size_t GetReadPos(std::ifstream& stream)
{
    return static_cast<size_t>(stream.tellg());
}

long TraceFileReader::Read(MessageLite& message)
{
    CHECK_TRUE(stream_.is_open(), 0, "binary file %s not open or open failed!", path_.c_str());
    CHECK_TRUE(!stream_.eof(), 0, "no more data in file %s stream", path_.c_str());

    uint32_t msgLen = 0;
    size_t offset = GetReadPos(stream_);
    stream_.read(reinterpret_cast<CharPtr>(&msgLen), sizeof(msgLen));
    RETURN_IF(stream_.eof(), 0, "read file end");
    CHECK_TRUE(msgLen > 0, 0, "read in file %s msg length: %d", path_.c_str(), msgLen);
    CHECK_TRUE(stream_, 0, "read msg length from %s (offset %zu) failed, or no more data!", path_.c_str(), offset);
    CHECK_TRUE(helper_.AddSegment(reinterpret_cast<uint8_t*>(&msgLen), sizeof(msgLen)),
        0, "Add payload for message length failed!");

    std::vector<char> msgData(msgLen);
    offset = GetReadPos(stream_);
    stream_.read(msgData.data(), msgData.size());
    RETURN_IF(stream_.eof(), 0, "read file end");
    CHECK_TRUE(stream_, 0, "read msg bytes from %s (offset %zu) failed!", path_.c_str(), offset);
    CHECK_TRUE(helper_.AddSegment(reinterpret_cast<uint8_t*>(msgData.data()), msgData.size()),
        0, "Add payload for message bytes failed!");

    CHECK_TRUE(message.ParseFromArray(msgData.data(), msgData.size()), 0, "ParseFromArray failed!");
    return sizeof(msgLen) + msgData.size();
}

long TraceFileReader::ReadLen()
{
    CHECK_TRUE(stream_.is_open(), 0, "binary file %s not open or open failed!", path_.c_str());
    CHECK_TRUE(!stream_.eof(), 0, "no more data in file %s stream", path_.c_str());

    uint32_t dataLen = 0;
    stream_.read(reinterpret_cast<CharPtr>(&dataLen), sizeof(dataLen));
    RETURN_IF(stream_.eof(), 0, "read file end");
    CHECK_TRUE(dataLen > 0, 0, "read in file %s data length: %d", path_.c_str(), dataLen);
    CHECK_TRUE(stream_, 0, "read data length from file %s (offset %zu) failed!", path_.c_str(), GetReadPos(stream_));
    CHECK_TRUE(helper_.AddSegment(reinterpret_cast<uint8_t*>(&dataLen), sizeof(dataLen)),
        0, "Add payload for data length failed!");
    return dataLen;
}

bool TraceFileReader::ReadData(uint8_t buffer[], uint32_t bufferSize)
{
    CHECK_TRUE(stream_.is_open(), false, "binary file %s not open or open failed!", path_.c_str());
    CHECK_TRUE(!stream_.eof(), false, "no more data in file %s stream", path_.c_str());

    stream_.read(reinterpret_cast<CharPtr>(buffer), bufferSize);
    RETURN_IF(stream_.eof(), false, "read file end. read data bytes from %s failed! (offset %zu)",
              path_.c_str(), GetReadPos(stream_));
    CHECK_TRUE(stream_, false, "read data bytes from %s (offset %zu) failed!", path_.c_str(), GetReadPos(stream_));
    CHECK_TRUE(helper_.AddSegment(buffer, bufferSize), false, "Add payload for data bytes failed!");
    return true;
}
