/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include "tshandle/map_parser.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

namespace mem_analyzer {

// ============================================================================
//  Protobuf helpers for HandleNodeIdMap (proto3)
// ============================================================================

static bool ReadVarint(const uint8_t* data, size_t size, size_t& offset, uint64_t& value)
{
    value = 0;
    int shift = 0;
    while (offset < size) {
        if (shift >= 64) {
            return false;
        }
        uint8_t byte = data[offset];
        value |= (static_cast<uint64_t>(byte & 0x7f) << shift);
        offset++;
        if ((byte & 0x80) == 0) {
            return true;
        }
        shift += 7;
    }
    return false;
}

static size_t SkipProtobufField(const uint8_t* data, size_t size, size_t offset, int wireType)
{
    switch (wireType) {
        case 0: {
            while (offset < size && (data[offset] & 0x80) != 0) {
                offset++;
            }
            return offset + 1;
        }
        case 1: return offset + 8;
        case 2: {
            uint64_t len = 0;
            if (!ReadVarint(data, size, offset, len)) {
                return size;
            }
            return offset + static_cast<size_t>(len);
        }
        case 5: return offset + 4;
        default: return size;
    }
}

static bool IsProtobufAddrFile(const uint8_t* data, size_t size)
{
    if (size < 3) {
        return false;
    }
    if (data[0] != 0x0a) {
        return false;
    }
    if (data[1] >= 0x80) {
        return false;
    }
    return data[2] == 0x09 || data[2] == 0x10;
}

static ParseAddrResult ParseProtobufAddrToNodeId(const uint8_t* data, size_t size)
{
    ParseAddrResult result;
    size_t offset = 0;

    while (offset < size) {
        uint64_t tag = 0;
        if (!ReadVarint(data, size, offset, tag)) {
            break;
        }
        int fieldNumber = static_cast<int>(tag >> 3);
        int wireType = static_cast<int>(tag & 0x07);

        if (fieldNumber == 1 && wireType == 2) {
            uint64_t len = 0;
            if (!ReadVarint(data, size, offset, len)) {
                break;
            }
            // 边界校验：len（不可信 varint）超出剩余字节数即为损坏数据，
            // 否则 entry_end = offset + len 会越过缓冲区上界，下游按 entry_end
            // 读取 data[offset] 时造成越界读。
            if (len > size - offset) {
                break;
            }
            size_t entryEnd = offset + static_cast<size_t>(len);

            uint64_t handleAddress = 0;
            uint64_t nodeId = 0;
            bool hasHandle = false;
            bool hasNode = false;

            while (offset < entryEnd) {
                uint64_t fieldTag = 0;
                if (!ReadVarint(data, entryEnd, offset, fieldTag)) {
                    break;
                }
                int fn = static_cast<int>(fieldTag >> 3);
                int wt = static_cast<int>(fieldTag & 0x07);

                if (fn == 1 && wt == 1) {
                    if (offset + 8 > entryEnd) {
                        break;
                    }
                    uint64_t val = 0;
                    std::memcpy(&val, data + offset, 8);
                    handleAddress = val;
                    hasHandle = true;
                    offset += 8;
                } else if (fn == 2 && wt == 0) {
                    if (!ReadVarint(data, entryEnd, offset, nodeId)) {
                        break;
                    }
                    hasNode = true;
                } else {
                    offset = SkipProtobufField(data, entryEnd, offset, wt);
                }
            }
            offset = entryEnd;

            if (hasHandle && hasNode) {
                std::ostringstream oss;
                oss << std::hex << handleAddress;
                std::string key = oss.str();
                uint32_t nid = static_cast<uint32_t>(nodeId);
                auto existing = result.addrToNodeId.find(key);
                if (existing != result.addrToNodeId.end()) {
                    if (existing->second != nid) {
                        result.duplicateCount++;
                        result.dupKeys.push_back(key);
                        result.dupOriginalAddrs.push_back("0x" + key);
                        result.dupNodeIds.push_back({existing->second, nid});
                    }
                } else {
                    result.addrToNodeId[key] = nid;
                }
            }
        } else {
            offset = SkipProtobufField(data, size, offset, wireType);
        }
    }

    return result;
}

// ============================================================================
//  normalize_addr
// ============================================================================

static std::string NormalizeAddr(const std::string& raw)
{
    std::string trimmed = raw;
    size_t start = trimmed.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    trimmed = trimmed.substr(start);
    size_t end = trimmed.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) {
        trimmed = trimmed.substr(0, end + 1);
    }

    if (trimmed.size() >= 2 && trimmed[0] == '0' && (trimmed[1] == 'x' || trimmed[1] == 'X')) {
        trimmed = trimmed.substr(2);
    }

    if (trimmed.empty()) {
        return "";
    }
    for (char c : trimmed) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return "";
        }
    }

    try {
        unsigned long long val = std::stoull(trimmed, nullptr, 16);
        std::ostringstream oss;
        oss << std::hex << val;
        return oss.str();
    } catch (...)
    {
        return "";
    }
}

// ============================================================================
//  parse_addr_to_node_id
// ============================================================================

ParseAddrResult ParseAddrToNodeId(const std::string& content)
{
    ParseAddrResult result;
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            continue;
        }
        size_t end = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(start, end - start + 1);

        if (trimmed.empty()) {
            continue;
        }
        if (trimmed[0] == '#') {
            continue;
        }
        if (trimmed.size() >= 2 && trimmed[0] == '/' && trimmed[1] == '/') {
            continue;
        }

        std::istringstream lineStream(trimmed);
        std::string addrStr, nodeIdStr;
        if (!(lineStream >> addrStr)) {
            continue;
        }
        if (!(lineStream >> nodeIdStr)) {
            continue;
        }

        char* endPtr = nullptr;
        errno = 0;
        long nodeIdLong = std::strtol(nodeIdStr.c_str(), &endPtr, 10);
        if (endPtr == nodeIdStr.c_str() || errno != 0 || nodeIdLong < 0) {
            continue;
        }
        uint32_t nodeId = static_cast<uint32_t>(nodeIdLong);

        std::string key = NormalizeAddr(addrStr);
        if (key.empty()) {
            result.addrFileInvalidCount++;
            result.invalidOriginalAddrs.push_back(addrStr);
            continue;
        }

        auto existing = result.addrToNodeId.find(key);
        if (existing != result.addrToNodeId.end()) {
            if (existing->second != nodeId) {
                result.duplicateCount++;
                result.dupKeys.push_back(key);
                result.dupOriginalAddrs.push_back(addrStr);
                result.dupNodeIds.push_back({existing->second, nodeId});
            }
        } else {
            result.addrToNodeId[key] = nodeId;
        }
    }

    return result;
}

// ============================================================================
//  parse_addr_file — read file and auto-detect text / protobuf format
// ============================================================================

ParseAddrResult ParseAddrFile(const std::string& filePath)
{
    auto file = OpenIfstream(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open addr file: " + filePath);
    }
    size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0);

    constexpr size_t kMaxAddrFileBytes = 4ULL * 1024 * 1024 * 1024;  // 4 GiB，防恶意超大 addr 文件 OOM
    if (file_size > kMaxAddrFileBytes) {
        throw std::runtime_error("addr file too large (" + std::to_string(file_size) +
                                 " bytes): " + filePath);
    }

    std::vector<uint8_t> data(file_size);
    file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(file_size));
    file.close();

    if (IsProtobufAddrFile(data.data(), data.size())) {
        log::Logd("[map] addr file '", filePath, "' -> protobuf (", file_size, " bytes)");
        return ParseProtobufAddrToNodeId(data.data(), data.size());
    }

    log::Logd("[map] addr file '", filePath, "' -> text (", file_size, " bytes)");
    std::string content(data.begin(), data.end());
    return ParseAddrToNodeId(content);
}

} // namespace mem_analyzer
