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
#include "utils/leak_file_reader.hpp"
#include "utils/file_utils.hpp"  // open_ifstream: Windows wide-path 安全
#include <fstream>
#include <stdexcept>
#include <cstring>

namespace mem_analyzer {

namespace {

constexpr int NSAPI_VERSON_LENGTH = 8; // "NSNAP1.0"
constexpr int UINT64_BYTES = 8;
constexpr int BITS_PER_BYTE = 8;
constexpr int LEAK_FILE_HEADER_BYTES = 16;
constexpr uint64_t MAX_LEAK_FILE_BYTES = 4ULL * 1024 * 1024 * 1024;  // 4 GiB

// 从二进制流读取 8 字节小端 uint64（不依赖主机字节序）
bool ReadU64Le(std::ifstream& f, uint64_t& out)
{
    unsigned char b[UINT64_BYTES];
    if (!f.read(reinterpret_cast<char*>(b), UINT64_BYTES)) {
        return false;
    }
    out = static_cast<uint64_t>(b[0]);
    for (int i = 1; i < UINT64_BYTES; ++i) {
        out |= static_cast<uint64_t>(b[i]) << (i * BITS_PER_BYTE);
    }
    return true;
}

} // namespace

std::unordered_set<uint64_t> ReadLeakFile(const std::string& path)
{
    std::unordered_set<uint64_t> addrs;
    // open_ifstream 打不开时抛 std::system_error（不会返回未打开的流），
    // 这里统一转成本函数的错误信息，与下面的 !f 分支保持一致。
    std::ifstream f;
    try {
        f = OpenIfstream(path, std::ios::binary);
    } catch (const std::exception&) {
        throw std::runtime_error("Cannot open leak file: " + path);
    }
    if (!f) {
        throw std::runtime_error("Cannot open leak file: " + path);
    }

    // magic: 8 bytes "NSNAP1.0"
    char magic[NSAPI_VERSON_LENGTH];
    if (!f.read(magic, NSAPI_VERSON_LENGTH)) {
        throw std::runtime_error("Leak file too short for magic: " + path);
    }
    static const char expected[NSAPI_VERSON_LENGTH] = {'N', 'S', 'N', 'A', 'P', '1', '.', '0'};
    if (std::memcmp(magic, expected, NSAPI_VERSON_LENGTH) != 0) {
        throw std::runtime_error("Invalid leak file magic (expected NSNAP1.0): " + path);
    }

    // count: 8 bytes uint64 LE
    uint64_t count = 0;
    if (!ReadU64Le(f, count)) {
        throw std::runtime_error("Leak file too short for count: " + path);
    }
    if (count == 0) {
        return addrs;  // 空文件，无泄漏数据
    }

    // 校验文件长度，防止恶意/损坏文件声明超大 count 触发 OOM（除法防 count*16 溢出）
    f.seekg(0, std::ios::end);
    auto filesize = f.tellg();
    f.seekg(LEAK_FILE_HEADER_BYTES, std::ios::beg);  // 回到数据起始（magic 8 + count 8）
    if (filesize < LEAK_FILE_HEADER_BYTES ||
        count > (static_cast<uint64_t>(filesize) - LEAK_FILE_HEADER_BYTES) / LEAK_FILE_HEADER_BYTES) {
        throw std::runtime_error("Leak file truncated: declared count " +
            std::to_string(count) + " exceeds file capacity (" +
            std::to_string(static_cast<uint64_t>(filesize)) + " bytes): " + path);
    }

    // 防超大文件本身触发 OOM：即使 count 合法，超大文件也会导致 reserve 海量内存
    if (static_cast<uint64_t>(filesize) > MAX_LEAK_FILE_BYTES) {
        throw std::runtime_error("Leak file too large (" +
            std::to_string(static_cast<uint64_t>(filesize)) + " bytes): " + path);
    }

    addrs.reserve(static_cast<size_t>(count));
    for (uint64_t i = 0; i < count; ++i) {
        uint64_t addr = 0;
        uint64_t size = 0;  // 大小暂不使用，仅消费 8 字节
        if (!ReadU64Le(f, addr) || !ReadU64Le(f, size)) {
            throw std::runtime_error("Leak file truncated at entry " + std::to_string(i) +
                                     " (expected " + std::to_string(count) + ")");
        }
        addrs.insert(addr);
    }
    return addrs;
}

} // namespace mem_analyzer
