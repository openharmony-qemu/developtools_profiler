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
