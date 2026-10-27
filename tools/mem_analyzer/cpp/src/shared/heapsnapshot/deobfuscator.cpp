#include "shared/heapsnapshot/deobfuscator.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/file_utils.hpp"
#include "utils/string_utils.hpp"
#include "utils/platform.hpp"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace mem_analyzer {

using json = nlohmann::ordered_json;

namespace {

// ============================================================================
//  Streaming file helpers
//
//  deobfuscate_heapsnapshot 只需改写顶层 "strings" 数组。为避免对整份
//  heapsnapshot（含巨大的 nodes/edges 数组）做全量 json::parse + dump，
//  这里用流式扫描定位 strings 数组的字节范围，仅解析该数组，
//  其余字节按原始内容流式拷贝到输出文件。
// ============================================================================

constexpr size_t SCAN_WIN = 64 * 1024;       // 64 KB — marker 扫描窗口
constexpr size_t BRACKET_CHUNK = 1024 * 1024; // 1 MB — 匹配 ']' 的扫描块
constexpr size_t COPY_CHUNK = 4 * 1024 * 1024; // 4 MB — 字节流式拷贝块

/// 在文件中从 start_pos 起流式扫描 marker（如 "\"strings\":"），返回其字节偏移。
/// 窗口按 (marker.size() - 1) 字节重叠，避免跨块边界遗漏。
size_t find_marker_offset(std::ifstream& file, const std::string& marker,
                         size_t start_pos, size_t file_size) {
    std::string buf(SCAN_WIN, '\0');
    size_t pos = start_pos;
    file.clear();
    while (pos < file_size) {
        size_t to_read = std::min(SCAN_WIN, file_size - pos);
        file.seekg(pos);
        file.read(&buf[0], to_read);
        size_t bytes = static_cast<size_t>(file.gcount());
        if (bytes == 0) break;
        std::string view(buf.data(), bytes);
        size_t idx = view.find(marker);
        if (idx != std::string::npos) return pos + idx;
        if (pos + bytes >= file_size) break;
        pos += bytes;
        if (pos >= marker.size()) pos -= (marker.size() - 1);
    }
    return std::string::npos;
}

/// 从 start_pos 起读取最多 256 字节，返回首个 ch 的字节偏移。
size_t find_char_after(std::ifstream& file, char ch, size_t start_pos, size_t file_size) {
    std::string buf(std::min<size_t>(256, file_size - start_pos), '\0');
    file.clear();
    file.seekg(start_pos);
    file.read(&buf[0], buf.size());
    size_t bytes = static_cast<size_t>(file.gcount());
    for (size_t i = 0; i < bytes; ++i) {
        if (buf[i] == ch) return start_pos + i;
    }
    return std::string::npos;
}

/// 从 open_pos 的 '[' 起流式扫描其匹配的 ']'，正确处理字符串字面量与转义。
/// 适用于 strings 这种扁平字符串数组（无嵌套对象/数组）。返回 ']' 的字节偏移。
size_t find_matching_close_bracket(std::ifstream& file, size_t open_pos, size_t file_size) {
    std::string buf(BRACKET_CHUNK, '\0');
    size_t pos = open_pos + 1;  // 跳过 '['
    bool in_string = false;
    bool escaped = false;
    file.clear();
    while (pos < file_size) {
        size_t to_read = std::min(BRACKET_CHUNK, file_size - pos);
        file.seekg(pos);
        file.read(&buf[0], to_read);
        size_t bytes = static_cast<size_t>(file.gcount());
        if (bytes == 0) break;
        for (size_t i = 0; i < bytes; ++i) {
            char c = buf[i];
            if (in_string) {
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '"') {
                    in_string = false;
                }
            } else if (c == '"') {
                in_string = true;
                escaped = false;
            } else if (c == ']') {
                return pos + i;
            }
        }
        pos += bytes;
    }
    return std::string::npos;
}

/// 读取 [start, end) 字节范围为字符串。
std::string read_byte_range(std::ifstream& file, size_t start, size_t end) {
    if (end <= start) return {};
    std::string buf(end - start, '\0');
    file.clear();
    file.seekg(start);
    file.read(&buf[0], end - start);
    buf.resize(static_cast<size_t>(file.gcount()));
    return buf;
}

/// 将输入文件 [start, end) 字节流式拷贝到输出流。
bool copy_byte_range(std::ifstream& in, std::ofstream& out, size_t start, size_t end) {
    if (end <= start) return true;
    std::string buf(COPY_CHUNK, '\0');
    size_t pos = start;
    in.clear();
    while (pos < end) {
        size_t to_read = std::min(COPY_CHUNK, end - pos);
        in.seekg(pos);
        in.read(&buf[0], to_read);
        size_t bytes = static_cast<size_t>(in.gcount());
        if (bytes == 0) return false;
        out.write(buf.data(), bytes);
        if (!out.good()) return false;
        pos += bytes;
    }
    return true;
}

/// 对 JSON 文本做 sanitize：把 U+0000-U+001F 控制字符替换为空格，
/// 使 nlohmann::json 能够解析（参考 heap_snapshot.cpp read_strings_array 思路）。
/// 仅在首次解析失败时作为降级手段使用。
std::string sanitize_control_chars(const std::string& text) {
    std::string sanitized;
    sanitized.reserve(text.size());
    for (char c : text) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x20) {
            sanitized += ' ';
        } else {
            sanitized += c;
        }
    }
    return sanitized;
}

/// 解析 heapsnapshot 中的 ArkTS 模块路径串，形如
///   [<bundle>/][@scope/]<module>@<version>/<rel_path>
/// （如 "com.xxx/entry@1.0.0/src/main/ets/web/a.ts" 或
///    "com.xxx/@douyin/camera_impl@1.0.0/src/main/ets/camera/a.ts"，
///    bundle 前缀与 @scope 前缀均可省略）。
/// 成功时返回 true 并填充 module / version / rel_path。
bool parse_arkts_module_path(const std::string& path_part, std::string& module,
                             std::string& version, std::string& rel_path) {
    const size_t at_pos = path_part.rfind('@');
    if (at_pos == std::string::npos) return false;
    const size_t version_end = path_part.find('/', at_pos + 1);
    if (version_end == std::string::npos) return false;
    version = path_part.substr(at_pos + 1, version_end - at_pos - 1);
    if (version.empty()) return false;

    const std::string module_part = path_part.substr(0, at_pos);
    const size_t last_slash = module_part.rfind('/');
    std::string extracted_module;
    if (last_slash == std::string::npos) {
        extracted_module = module_part;
    } else if (last_slash == 0) {
        // 路径以 '/' 开头（如 "/entry@1.0.0/..."），无 scope 前缀，取 '/' 之后的部分
        extracted_module = module_part.substr(1);
    } else {
        // 检查上一级目录段是否以 '@' 开头，若是则合并两段作为完整 module 名
        // （作用域包名格式为 @scope/name，需要把 @scope/ 前缀一并保留）。
        const size_t prev_slash = module_part.rfind('/', last_slash - 1);
        std::string scope_candidate;
        if (prev_slash == std::string::npos) {
            scope_candidate = module_part.substr(0, last_slash);
        } else {
            scope_candidate = module_part.substr(prev_slash + 1, last_slash - prev_slash - 1);
        }
        if (!scope_candidate.empty() && scope_candidate.front() == '@') {
            if (prev_slash == std::string::npos) {
                extracted_module = module_part;
            } else {
                extracted_module = module_part.substr(prev_slash + 1);
            }
        } else {
            extracted_module = module_part.substr(last_slash + 1);
        }
    }
    module = extracted_module;
    if (module.empty()) return false;

    rel_path = path_part.substr(version_end + 1);
    return !rel_path.empty();
}

/// 用官方 nameCache 行号范围修正源码行号（参考官方"异常堆栈解析原理"文档）。
/// nameCache 的 IdentifierCache/MemberMethodCache 条目格式为
/// "原始方法名:起始行:结束行"（源码 1-based 双闭区间）。官方算法：
/// 1. 名称匹配：'#' 代表上下级关系，匹配时保留最后一级名称；
/// 2. 行号过滤：同名候选存在多个时，用行号范围（双闭区间）包含还原行号的条目。
/// 快照 VM 记录的行号是函数首条执行语句所在行（构造函数因字段初始化提升，
/// 记录落在字段声明区），当还原行号不在该方法的行号范围内时，
/// 修正为定义起始行最贴近还原行号的候选。函数名可能是混淆名（开启属性混淆）
/// 或原始名（未开启时），两者都查。未修正返回 false（保持 sourceMap 映射结果）。
bool refine_line_by_name_cache(const NameCacheIndex* ncache, const std::string& source_path,
                               const std::string& suffix, int& orig_line) {
    if (ncache == nullptr || source_path.empty()) return false;
    // 从 suffix（形如 "#funcName(line:N)[module]"）提取函数名，
    // 按官方约定保留最后一级名称（如 "#Class#method" 取 "method"）
    const size_t name_begin = suffix.find('#');
    if (name_begin == std::string::npos) return false;
    const size_t name_end = suffix.find("(line:", name_begin + 1);
    if (name_end == std::string::npos || name_end <= name_begin + 1) return false;
    const std::string full_name = suffix.substr(name_begin + 1, name_end - name_begin - 1);
    if (full_name.empty()) return false;
    const size_t last_hash = full_name.rfind('#');
    const std::string short_name =
        (last_hash == std::string::npos) ? full_name : full_name.substr(last_hash + 1);

    // 依次尝试：短名(混淆) → 短名(原始) → 完整名(混淆) → 完整名(原始)，
    // 取首个有候选者即停止。函数名可能是混淆名（开启属性混淆）或原始名（未开启时）。
    std::vector<NameCacheCandidate> candidates;
    auto try_lookup = [&](const std::string& name, bool obfuscated) {
        if (name.empty() || !candidates.empty()) return;
        if (obfuscated) {
            ncache->lookup_obfuscated_name(name, candidates);
        } else {
            ncache->lookup_original_name(name, candidates);
        }
    };
    try_lookup(short_name, true);
    try_lookup(short_name, false);
    try_lookup(full_name, true);
    try_lookup(full_name, false);
    if (candidates.empty()) return false;

    // 官方算法：行号范围包含还原行号的条目为命中（此时行号本身正确，无需修正）；
    // 范围外（构造函数首语句提升）时选择定义起始行最贴近还原行号的候选修正：
    // 优先取 start_line <= 还原行号中最大者，否则取最小 start_line
    const NameCacheCandidate* hit = nullptr;
    const NameCacheCandidate* fallback = nullptr;
    for (const auto& cand : candidates) {
        if (cand.start_line <= 0 || cand.end_line < cand.start_line) continue;
        if (!source_path_matches(source_path, cand.file_path)) continue;
        if (orig_line >= cand.start_line && orig_line <= cand.end_line) {
            hit = &cand;
            break;
        }
        if (fallback == nullptr) {
            fallback = &cand;
        } else if (cand.start_line <= orig_line) {
            if (fallback->start_line > orig_line || cand.start_line > fallback->start_line) {
                fallback = &cand;
            }
        } else if (fallback->start_line > orig_line && cand.start_line < fallback->start_line) {
            fallback = &cand;
        }
    }
    if (hit != nullptr || fallback == nullptr) return false;
    orig_line = fallback->start_line;
    return true;
}

/// 还原 VM 函数标记串 "path#funcName(line:N)[module]" 后缀中的混淆函数名。
/// 快照中该类字符串形如 "entry@1.0.0/src/main/ets/pages/WebPage.ts#g1(line:24)[entry]"，
/// 路径部分经 sourceMap 还原后为 "entry/src/main/ets/pages/WebPage.ets#g1(line:24)[entry]"，
/// 但其中的函数名（如 g1/d1/a1）仍是混淆名。此处按文件上下文过滤 nameCache 候选，
/// 候选去重后仅一种原始名时才替换，保证不会误还原。
bool restore_function_name_in_suffix(const NameCacheIndex* ncache, std::string& value) {
    if (ncache == nullptr) return false;
    const size_t kLine = value.find("(line:");
    if (kLine == std::string::npos || kLine == 0) return false;
    // 取 '(' 之前最后一个 '#'，其后的子串即为混淆函数名（如 "#d1(" → "d1"）
    const size_t hash = value.rfind('#', kLine - 1);
    if (hash == std::string::npos || hash + 1 >= kLine) return false;
    const size_t name_len = kLine - hash - 1;  // '#' 与 '(' 之间的函数名长度
    if (name_len == 0) return false;
    const std::string obf_name = value.substr(hash + 1, name_len);
    if (obf_name.empty()) return false;

    // 反查混淆名候选，用已还原的源码路径做文件上下文过滤，再按原始名去重。
    // 同一混淆名可能同时命中 IdentifierCache（类名，原始名带 '#' 前缀）与
    // MemberMethodCache（构造函数与方法名，原始名不带 '#'，如类与构造函数同名），
    // 比较与替换前统一剥离 '#' 前缀，避免同名不同前缀导致误判无法唯一还原。
    std::vector<NameCacheCandidate> candidates;
    if (!ncache->lookup_obfuscated_name(obf_name, candidates)) return false;
    const std::string ctx_path = value.substr(0, hash);
    std::string original;
    for (const auto& cand : candidates) {
        if (cand.original.empty()) continue;
        if (!source_path_matches(ctx_path, cand.file_path)) continue;
        std::string name = cand.original;
        if (name.front() == '#') name.erase(0, 1);
        if (name.empty() || name == obf_name) continue;
        if (original.empty()) {
            original = name;
        } else if (original != name) {
            return false;  // 同一文件下候选原始名不一致，无法唯一还原
        }
    }
    if (original.empty()) return false;
    value.replace(hash + 1, name_len, original);
    return true;
}

/// 对单个 strings 数组元素做反混淆还原（混淆路径→源码路径、混淆名→原始名）。
/// 返回还原后的字符串；发生替换时递增 replaced。
std::string deobfuscate_one_string(const std::string& value,
                                   const SourceMapIndex* smap,
                                   const NameCacheIndex* ncache,
                                   size_t& replaced) {
    std::string result = value;
    std::string original;

    // 1. obfName 混淆路径 → 原始文件路径
    if (ncache && ncache->lookup_obf_name_path(result, original)) {
        result = original;
        replaced++;
    }
    // 2. 混淆编译产物路径 key → 源码路径（sources[0]）
    if (smap) {
        const SourceMapEntry* entry = smap->lookup(result);
        if (entry != nullptr && !entry->source_path.empty()) {
            result = entry->source_path;
            replaced++;
        } else {
            // 兼容 heapsnapshot 中的 ArkTS 模块路径格式
            // "[<bundle>/]<module>@<version>/<rel_path>[#func(line:N)[module]]"：
            // 按 (module, version, rel_path) 命中后，仅替换路径部分，保留 "#" 后缀，
            // 并尝试将 (line:N) 由编译产物行号映射为源码行号。
            const size_t hash_pos = result.find('#');
            const std::string path_part = result.substr(0, hash_pos);
            std::string module, version, rel_path;
            entry = nullptr;
            if (parse_arkts_module_path(path_part, module, version, rel_path)) {
                entry = smap->lookup_by_module(module, version, rel_path);
            }
            if (entry != nullptr && !entry->source_path.empty()) {
                std::string suffix = (hash_pos == std::string::npos) ? "" : result.substr(hash_pos);
                // 映射 (line:N)：取 N，若该行在 mappings 中有映射则替换为源码行号
                const std::string kLinePrefix = "(line:";
                const size_t lp = suffix.find(kLinePrefix);
                if (lp != std::string::npos) {
                    const size_t num_begin = lp + kLinePrefix.size();
                    const size_t num_end = suffix.find(')', num_begin);
                    if (num_end != std::string::npos) {
                        const std::string num_str = suffix.substr(num_begin, num_end - num_begin);
                        if (all_digits(num_str)) {
                            int gen_line = 0;
                            // 快照损坏时可能为超长数字串，stoi 抛异常则跳过行号映射
                            try {
                                gen_line = std::stoi(num_str);
                            } catch (const std::exception&) {
                                gen_line = -1;
                            }
                            if (gen_line > 0) {
                                int orig_line = gen_line;
                                smap->resolve_original_line(entry->mappings, gen_line, orig_line);
                                refine_line_by_name_cache(ncache, entry->source_path, suffix, orig_line);
                                suffix.replace(lp, num_end - lp + 1,
                                               "(line:" + std::to_string(orig_line) + ")");
                            }
                        }
                    }
                }
                result = entry->source_path + suffix;
                replaced++;
            }
        }
    }
    // 3. 混淆名 → 原始名（仅当候选唯一）
    if (ncache && ncache->lookup_unique(result, original)) {
        result = original;
        replaced++;
    }
    // 4. "path#funcName(line:N)[module]" 后缀中的混淆函数名 → 原始名
    //    （按文件上下文过滤，候选去重后仅一种原始名时替换）
    if (ncache && restore_function_name_in_suffix(ncache, result)) {
        replaced++;
    }

    return result;
}

} // anonymous namespace

// ============================================================================
//  deobfuscate_heapsnapshot: 流式反混淆 heapsnapshot
//
//  仅解析并改写顶层 "strings" 字符串数组，nodes / edges / meta 等其余字段
//  按原始字节流式拷贝，避免大快照全量 json::parse + dump 导致的内存峰值。
// ============================================================================
bool deobfuscate_heapsnapshot(const std::string& input_path, const std::string& output_path,
                              const SourceMapIndex* smap, const NameCacheIndex* ncache) {
    std::error_code ec;
    if (!fs::is_regular_file(mem_analyzer::path_from_utf8(input_path), ec)) {
        log::loge("Error: Heapsnapshot file not found: ", input_path);
        return false;
    }

    std::ifstream in(mem_analyzer::path_from_utf8(input_path), std::ios::binary);
    if (!in) {
        log::loge("Error: Failed to open heapsnapshot file: ", input_path);
        return false;
    }
    in.seekg(0, std::ios::end);
    const size_t file_size = static_cast<size_t>(in.tellg());
    if (file_size == 0) {
        log::loge("Error: Heapsnapshot file is empty: ", input_path);
        return false;
    }

    // 1. 流式定位 "strings": 标记及其后的 '['
    const std::string kMarker = "\"strings\":";
    size_t mark_pos = find_marker_offset(in, kMarker, 0, file_size);
    if (mark_pos == std::string::npos) {
        log::loge("Error: No \"strings\" field in heapsnapshot file: ", input_path);
        return false;
    }
    size_t open_bracket = find_char_after(in, '[', mark_pos + kMarker.size(), file_size);
    if (open_bracket == std::string::npos) {
        log::loge("Error: \"strings\" is not an array in heapsnapshot file: ", input_path);
        return false;
    }

    // 2. 流式扫描 strings 数组的匹配 ']'（正确处理字符串字面量与转义）
    size_t close_bracket = find_matching_close_bracket(in, open_bracket, file_size);
    if (close_bracket == std::string::npos) {
        log::loge("Error: Unterminated \"strings\" array in heapsnapshot file: ", input_path);
        return false;
    }

    // 3. 读取 strings 数组文本 [open_bracket, close_bracket]
    std::string arr_text = read_byte_range(in, open_bracket, close_bracket + 1);

    // 4. 解析 strings 数组（首次失败则对控制字符做 sanitize 后重试）
    json strings_arr;
    bool parsed = false;
    try {
        strings_arr = json::parse(arr_text);
        parsed = true;
    } catch (const std::exception&) {
        try {
            strings_arr = json::parse(sanitize_control_chars(arr_text));
            parsed = true;
        } catch (const std::exception& e) {
            log::loge("Error: Failed to parse \"strings\" array in ", input_path, " as JSON: ", e.what());
        }
    }
    if (!parsed || !strings_arr.is_array()) {
        log::loge("Error: \"strings\" is not an array in heapsnapshot file: ", input_path);
        return false;
    }

    // 5. 逐条反混淆 strings 数组元素
    size_t replaced = 0;
    for (auto& s : strings_arr) {
        if (!s.is_string()) continue;
        std::string value = s.get<std::string>();
        std::string restored = deobfuscate_one_string(value, smap, ncache, replaced);
        s = restored;
    }

    // 6. 重新序列化 strings 数组（紧凑格式）
    std::string new_arr = strings_arr.dump();

    // 7. 写出输出文件：[0, open_bracket) + 新 strings 数组 + (close_bracket, end)
    std::ofstream out(mem_analyzer::path_from_utf8(output_path), std::ios::binary);
    if (!out) {
        log::loge("Error: Failed to open output file for writing: ", output_path);
        return false;
    }
    if (!copy_byte_range(in, out, 0, open_bracket)) {
        log::loge("Error: Failed to copy snapshot header to output: ", output_path);
        return false;
    }
    out.write(new_arr.data(), new_arr.size());
    if (!out.good()) {
        log::loge("Error: Failed to write deobfuscated strings to output: ", output_path);
        return false;
    }
    if (!copy_byte_range(in, out, close_bracket + 1, file_size)) {
        log::loge("Error: Failed to copy snapshot tail to output: ", output_path);
        return false;
    }
    if (!out.good()) {
        log::loge("Error: Failed to finalize output file: ", output_path);
        return false;
    }

    log::logi("  Deobfuscated ", replaced, " strings in: ", input_path);
    return true;
}

// ============================================================================
//  deobfuscate_with_indices: 加载索引 + 反混淆的公共时序
//
//  供 jsleak_parse()（jsleak 入口）与 deobfuscate_zip_snapshot
//  （tshandle zip 入口）共用，消除重复逻辑。
// ============================================================================
bool deobfuscate_with_indices(const std::string& input_path, const std::string& output_path,
                              const std::string& sourcemap_dir,
                              const std::string& namecache_dir) {
    // 两个目录都为空 → 无操作，调用方应直接降级为原始输入
    if (sourcemap_dir.empty() && namecache_dir.empty()) {
        return false;
    }

    // 校验路径存在（--sourcemap / --namecache 支持目录或单个文件）
    // 用户路径可能含中文，需按 UTF-8 构造 fs::path（Windows 宽路径安全）
    auto path_exists = [](const std::string& path) {
        std::error_code ec;
        return !path.empty() && fs::exists(mem_analyzer::path_from_utf8(path), ec);
    };
    if (!sourcemap_dir.empty() && !path_exists(sourcemap_dir)) {
        log::loge("Error: Source map file or directory does not exist: ", sourcemap_dir);
        return false;
    }
    if (!namecache_dir.empty() && !path_exists(namecache_dir)) {
        log::loge("Error: nameCache file or directory does not exist: ", namecache_dir);
        return false;
    }

    // 加载索引，加载失败仅警告并继续（对应还原降级）
    SourceMapIndex smap;
    NameCacheIndex ncache;
    if (!sourcemap_dir.empty()) {
        if (!smap.load_from_dir(sourcemap_dir)) {
            log::logw("Warning: Source map index unavailable, path deobfuscation will be skipped.");
        }
    }
    if (!namecache_dir.empty()) {
        if (!ncache.load_from_dir(namecache_dir)) {
            log::logw("Warning: nameCache index unavailable, name deobfuscation will be skipped.");
        }
    }
    // 两者均空 → 警告后降级为原始输入
    if (smap.empty() && ncache.empty()) {
        log::logw("Warning: No source map / nameCache entries loaded, proceeding with the original snapshot.");
        return false;
    }

    // 创建输出文件的父目录。注意：macOS libc++ 的 fs::create_directories 在
    // 目录已存在时返回 false 且 ec 为空（与 Linux/标准实现返回 true 不同），
    // 因此仅当 ec 非空（创建真失败）时才报错，目录已存在视为成功。
    const std::string parent_dir = mem_analyzer::platform::dirname(output_path);
    if (!parent_dir.empty()) {
        std::error_code ec;
        if (!fs::create_directories(mem_analyzer::path_from_utf8(parent_dir), ec) && ec) {
            log::loge("Error: Failed to create output directory: ", parent_dir, " (", ec.message(), ")");
            return false;
        }
    }

    const SourceMapIndex* smap_ptr = sourcemap_dir.empty() ? nullptr : &smap;
    const NameCacheIndex* ncache_ptr = namecache_dir.empty() ? nullptr : &ncache;
    return deobfuscate_heapsnapshot(input_path, output_path, smap_ptr, ncache_ptr);
}

} // namespace mem_analyzer
