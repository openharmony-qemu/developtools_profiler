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

static bool read_varint(const uint8_t* data, size_t size, size_t& offset, uint64_t& value) {
    value = 0;
    int shift = 0;
    while (offset < size) {
        if (shift >= 64) return false;
        uint8_t byte = data[offset];
        value |= (static_cast<uint64_t>(byte & 0x7f) << shift);
        offset++;
        if ((byte & 0x80) == 0) return true;
        shift += 7;
    }
    return false;
}

static size_t skip_protobuf_field(const uint8_t* data, size_t size, size_t offset, int wire_type) {
    switch (wire_type) {
        case 0: {
            while (offset < size && (data[offset] & 0x80) != 0) offset++;
            return offset + 1;
        }
        case 1: return offset + 8;
        case 2: {
            uint64_t len = 0;
            if (!read_varint(data, size, offset, len)) return size;
            return offset + static_cast<size_t>(len);
        }
        case 5: return offset + 4;
        default: return size;
    }
}

static bool is_protobuf_addr_file(const uint8_t* data, size_t size) {
    if (size < 3) return false;
    if (data[0] != 0x0a) return false;
    if (data[1] >= 0x80) return false;
    return data[2] == 0x09 || data[2] == 0x10;
}

static ParseAddrResult parse_protobuf_addr_to_node_id(const uint8_t* data, size_t size) {
    ParseAddrResult result;
    size_t offset = 0;

    while (offset < size) {
        uint64_t tag = 0;
        if (!read_varint(data, size, offset, tag)) break;
        int field_number = static_cast<int>(tag >> 3);
        int wire_type = static_cast<int>(tag & 0x07);

        if (field_number == 1 && wire_type == 2) {
            uint64_t len = 0;
            if (!read_varint(data, size, offset, len)) break;
            // 边界校验：len（不可信 varint）超出剩余字节数即为损坏数据，
            // 否则 entry_end = offset + len 会越过缓冲区上界，下游按 entry_end
            // 读取 data[offset] 时造成越界读。
            if (len > size - offset) break;
            size_t entry_end = offset + static_cast<size_t>(len);

            uint64_t handle_address = 0;
            uint64_t node_id = 0;
            bool has_handle = false;
            bool has_node = false;

            while (offset < entry_end) {
                uint64_t field_tag = 0;
                if (!read_varint(data, entry_end, offset, field_tag)) break;
                int fn = static_cast<int>(field_tag >> 3);
                int wt = static_cast<int>(field_tag & 0x07);

                if (fn == 1 && wt == 1) {
                    if (offset + 8 > entry_end) break;
                    uint64_t val = 0;
                    std::memcpy(&val, data + offset, 8);
                    handle_address = val;
                    has_handle = true;
                    offset += 8;
                } else if (fn == 2 && wt == 0) {
                    if (!read_varint(data, entry_end, offset, node_id)) break;
                    has_node = true;
                } else {
                    offset = skip_protobuf_field(data, entry_end, offset, wt);
                }
            }
            offset = entry_end;

            if (has_handle && has_node) {
                std::ostringstream oss;
                oss << std::hex << handle_address;
                std::string key = oss.str();
                uint32_t nid = static_cast<uint32_t>(node_id);
                auto existing = result.addr_to_node_id.find(key);
                if (existing != result.addr_to_node_id.end()) {
                    if (existing->second != nid) {
                        result.duplicate_count++;
                        result.dup_keys.push_back(key);
                        result.dup_original_addrs.push_back("0x" + key);
                        result.dup_node_ids.push_back({existing->second, nid});
                    }
                } else {
                    result.addr_to_node_id[key] = nid;
                }
            }
        } else {
            offset = skip_protobuf_field(data, size, offset, wire_type);
        }
    }

    return result;
}

// ============================================================================
//  normalize_addr
// ============================================================================

static std::string normalize_addr(const std::string& raw) {
    std::string trimmed = raw;
    size_t start = trimmed.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    trimmed = trimmed.substr(start);
    size_t end = trimmed.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) trimmed = trimmed.substr(0, end + 1);

    if (trimmed.size() >= 2 && trimmed[0] == '0' && (trimmed[1] == 'x' || trimmed[1] == 'X')) {
        trimmed = trimmed.substr(2);
    }

    if (trimmed.empty()) return "";
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
    } catch (...) {
        return "";
    }
}

// ============================================================================
//  parse_addr_to_node_id
// ============================================================================

ParseAddrResult parse_addr_to_node_id(const std::string& content) {
    ParseAddrResult result;
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(start, end - start + 1);

        if (trimmed.empty()) continue;
        if (trimmed[0] == '#') continue;
        if (trimmed.size() >= 2 && trimmed[0] == '/' && trimmed[1] == '/') continue;

        std::istringstream line_stream(trimmed);
        std::string addr_str, node_id_str;
        if (!(line_stream >> addr_str)) continue;
        if (!(line_stream >> node_id_str)) continue;

        char* end_ptr = nullptr;
        errno = 0;
        long node_id_long = std::strtol(node_id_str.c_str(), &end_ptr, 10);
        if (end_ptr == node_id_str.c_str() || errno != 0 || node_id_long < 0) continue;
        uint32_t node_id = static_cast<uint32_t>(node_id_long);

        std::string key = normalize_addr(addr_str);
        if (key.empty()) {
            result.addr_file_invalid_count++;
            result.invalid_original_addrs.push_back(addr_str);
            continue;
        }

        auto existing = result.addr_to_node_id.find(key);
        if (existing != result.addr_to_node_id.end()) {
            if (existing->second != node_id) {
                result.duplicate_count++;
                result.dup_keys.push_back(key);
                result.dup_original_addrs.push_back(addr_str);
                result.dup_node_ids.push_back({existing->second, node_id});
            }
        } else {
            result.addr_to_node_id[key] = node_id;
        }
    }

    return result;
}

// ============================================================================
//  parse_addr_file — read file and auto-detect text / protobuf format
// ============================================================================

ParseAddrResult parse_addr_file(const std::string& file_path) {
    auto file = open_ifstream(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open addr file: " + file_path);
    }
    size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0);

    constexpr size_t kMaxAddrFileBytes = 4ULL * 1024 * 1024 * 1024;  // 4 GiB，防恶意超大 addr 文件 OOM
    if (file_size > kMaxAddrFileBytes) {
        throw std::runtime_error("addr file too large (" + std::to_string(file_size)
                                 + " bytes): " + file_path);
    }

    std::vector<uint8_t> data(file_size);
    file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(file_size));
    file.close();

    if (is_protobuf_addr_file(data.data(), data.size())) {
        log::logd("[map] addr file '", file_path, "' -> protobuf (", file_size, " bytes)");
        return parse_protobuf_addr_to_node_id(data.data(), data.size());
    }

    log::logd("[map] addr file '", file_path, "' -> text (", file_size, " bytes)");
    std::string content(data.begin(), data.end());
    return parse_addr_to_node_id(content);
}

} // namespace mem_analyzer
