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
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace mem_analyzer {

struct ParseAddrResult {
    std::unordered_map<std::string, uint32_t> addr_to_node_id;

    int duplicate_count = 0;
    int addr_file_invalid_count = 0;

    std::vector<std::string> dup_keys;
    std::vector<std::string> dup_original_addrs;
    std::vector<std::vector<uint32_t>> dup_node_ids;
    std::vector<std::string> invalid_original_addrs;
};

ParseAddrResult parse_addr_to_node_id(const std::string& content);

/**
 * Read an addr file from disk and auto-detect its format:
 *  - Protobuf (HandleNodeIdMap) if the first bytes match the expected wire format
 *  - Plain text (addr nodeId per line) as fallback
 * Returns the same ParseAddrResult as parse_addr_to_node_id.
 */
ParseAddrResult parse_addr_file(const std::string& file_path);

} // namespace mem_analyzer
