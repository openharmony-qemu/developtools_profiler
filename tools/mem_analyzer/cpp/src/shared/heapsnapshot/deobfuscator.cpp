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

constexpr size_t CHAR_SEARCH_BUF_SIZE = 256;

// ============================================================================
//  Streaming file helpers
//  deobfuscate_heapsnapshot 只需改写顶层 "strings" 数组。为避免对整份
//  heapsnapshot（含巨大的 nodes/edges 数组）做全量 json::parse + dump，
//  这里用流式扫描定位 strings 数组的字节范围，仅解析该数组，
//  其余字节按原始内容流式拷贝到输出文件。
// ============================================================================

constexpr size_t SCAN_WIN = 64 * 1024;       // 64 KB — marker 扫描窗口
constexpr size_t BRACKET_CHUNK = 1024 * 1024; // 1 MB — 匹配 ']' 的扫描块
constexpr size_t COPY_CHUNK = 4 * 1024 * 1024; // 4 MB — 字节流式拷贝块

/**
 * 在文件中从 start_pos 起流式扫描 marker（如 "\"strings\":"），返回其字节偏移。
 * 窗口按 (marker.size() - 1) 字节重叠，避免跨块边界遗漏。
 */
size_t FindMarkerOffset(std::ifstream& file, const std::string& marker, size_t startPos, size_t fileSize)
{
    std::string buf(SCAN_WIN, '\0');
    size_t pos = startPos;
    file.clear();
    while (pos < fileSize) {
        size_t toRead = std::min(SCAN_WIN, fileSize - pos);
        file.seekg(pos);
        file.read(&buf[0], toRead);
        size_t bytes = static_cast<size_t>(file.gcount());
        if (bytes == 0) {
            break;
        }
        std::string view(buf.data(), bytes);
        size_t idx = view.find(marker);
        if (idx != std::string::npos) {
            return pos + idx;
        }
        if (pos + bytes >= fileSize) {
            break;
        }
        pos += bytes;
        if (pos >= marker.size()) {
            pos -= (marker.size() - 1);
        }
    }
    return std::string::npos;
}

/**
 * 从 start_pos 起读取最多 256 字节，返回首个 ch 的字节偏移。
 */
size_t FindCharAfter(std::ifstream& file, char ch, size_t startPos, size_t fileSize)
{
    std::string buf(std::min<size_t>(CHAR_SEARCH_BUF_SIZE, fileSize - startPos), '\0');
    file.clear();
    file.seekg(startPos);
    file.read(&buf[0], buf.size());
    size_t bytes = static_cast<size_t>(file.gcount());
    for (size_t i = 0; i < bytes; ++i) {
        if (buf[i] == ch) {
            return startPos + i;
        }
    }
    return std::string::npos;
}

/**
 * 从 open_pos 的 '[' 起流式扫描其匹配的 ']'，正确处理字符串字面量与转义。
 * 适用于 strings 这种扁平字符串数组（无嵌套对象/数组）。返回 ']' 的字节偏移。
 */
size_t FindMatchingCloseBracket(std::ifstream& file, size_t openPos, size_t fileSize)
{
    std::string buf(BRACKET_CHUNK, '\0');
    size_t pos = openPos + 1;  // 跳过 '['
    bool inString = false;
    bool escaped = false;
    file.clear();
    while (pos < fileSize) {
        size_t toRead = std::min(BRACKET_CHUNK, fileSize - pos);
        file.seekg(pos);
        file.read(&buf[0], toRead);
        size_t bytes = static_cast<size_t>(file.gcount());
        if (bytes == 0) {
            break;
        }
        for (size_t i = 0; i < bytes; ++i) {
            char c = buf[i];
            if (inString) {
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '"') {
                    inString = false;
                }
            } else if (c == '"') {
                inString = true;
                escaped = false;
            } else if (c == ']') {
                return pos + i;
            }
        }
        pos += bytes;
    }
    return std::string::npos;
}

/**
 * 读取 [start, end) 字节范围为字符串。
 */
std::string ReadByteRange(std::ifstream& file, size_t start, size_t end)
{
    if (end <= start) {
        return {};
    }
    std::string buf(end - start, '\0');
    file.clear();
    file.seekg(start);
    file.read(&buf[0], end - start);
    buf.resize(static_cast<size_t>(file.gcount()));
    return buf;
}

/**
 * 将输入文件 [start, end) 字节流式拷贝到输出流。
 */
bool CopyByteRange(std::ifstream& in, std::ofstream& out, size_t start, size_t end)
{
    if (end <= start) {
        return true;
    }
    std::string buf(COPY_CHUNK, '\0');
    size_t pos = start;
    in.clear();
    while (pos < end) {
        size_t toRead = std::min(COPY_CHUNK, end - pos);
        in.seekg(pos);
        in.read(&buf[0], toRead);
        size_t bytes = static_cast<size_t>(in.gcount());
        if (bytes == 0) {
            return false;
        }
        out.write(buf.data(), bytes);
        if (!out.good()) {
            return false;
        }
        pos += bytes;
    }
    return true;
}

/**
 * 对 JSON 文本做 sanitize：把 U+0000-U+001F 控制字符替换为空格，
 * 使 nlohmann::json 能够解析（参考 heap_snapshot.cpp read_strings_array 思路）。
 * 仅在首次解析失败时作为降级手段使用。
 */
std::string SanitizeControlChars(const std::string& text)
{
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

/**
 * 解析 heapsnapshot 中的 ArkTS 模块路径串，形如
 *   [<bundle>/][@scope/]<module>@<version>/<rel_path>
 * （如 "com.xxx/entry@1.0.0/src/main/ets/web/a.ts" 或
 *    "com.xxx/@douyin/camera_impl@1.0.0/src/main/ets/camera/a.ts"，
 *    bundle 前缀与 @scope 前缀均可省略）。
 * 成功时返回 true 并填充 module / version / rel_path。
 */
bool ParseArktsModulePath(const std::string& pathPart, std::string& module, std::string& version, std::string& relPath)
{
    const size_t atPos = pathPart.rfind('@');
    if (atPos == std::string::npos) {
        return false;
    }
    const size_t versionEnd = pathPart.find('/', atPos + 1);
    if (versionEnd == std::string::npos) {
        return false;
    }
    version = pathPart.substr(atPos + 1, versionEnd - atPos - 1);
    if (version.empty()) {
        return false;
    }

    const std::string modulePart = pathPart.substr(0, atPos);
    const size_t lastSlash = modulePart.rfind('/');
    std::string extractedModule;
    if (lastSlash == std::string::npos) {
        extractedModule = modulePart;
    } else if (lastSlash == 0) {
        // 路径以 '/' 开头（如 "/entry@1.0.0/..."），无 scope 前缀，取 '/' 之后的部分
        extractedModule = modulePart.substr(1);
    } else {
        // 检查上一级目录段是否以 '@' 开头，若是则合并两段作为完整 module 名
        // （作用域包名格式为 @scope/name，需要把 @scope/ 前缀一并保留）。
        const size_t prevSlash = modulePart.rfind('/', lastSlash - 1);
        std::string scopeCandidate;
        if (prevSlash == std::string::npos) {
            scopeCandidate = modulePart.substr(0, lastSlash);
        } else {
            scopeCandidate = modulePart.substr(prevSlash + 1, lastSlash - prevSlash - 1);
        }
        if (!scopeCandidate.empty() && scopeCandidate.front() == '@') {
            if (prevSlash == std::string::npos) {
                extractedModule = modulePart;
            } else {
                extractedModule = modulePart.substr(prevSlash + 1);
            }
        } else {
            extractedModule = modulePart.substr(lastSlash + 1);
        }
    }
    module = extractedModule;
    if (module.empty()) {
        return false;
    }

    relPath = pathPart.substr(versionEnd + 1);
    return !relPath.empty();
}

void SelectNameCacheCandidate(const std::vector<NameCacheCandidate>& candidates,
    const std::string& sourcePath, int origLine,
    const NameCacheCandidate*& hit, const NameCacheCandidate*& fallback)
{
    for (const auto& cand : candidates) {
        if (cand.startLine <= 0 || cand.endLine < cand.startLine) {
            continue;
        }
        if (!SourcePathMatches(sourcePath, cand.filePath)) {
            continue;
        }
        if (origLine >= cand.startLine && origLine <= cand.endLine) {
            hit = &cand;
            break;
        }
        if (fallback == nullptr) {
            fallback = &cand;
        } else if (cand.startLine <= origLine) {
            if (fallback->startLine > origLine || cand.startLine > fallback->startLine) {
                fallback = &cand;
            }
        } else if (fallback->startLine > origLine && cand.startLine < fallback->startLine) {
            fallback = &cand;
        }
    }
}

/**
 * 用官方 nameCache 行号范围修正源码行号（参考官方"异常堆栈解析原理"文档）。
 * nameCache 的 IdentifierCache/MemberMethodCache 条目格式为
 * "原始方法名:起始行:结束行"（源码 1-based 双闭区间）。官方算法：
 * 1. 名称匹配：'#' 代表上下级关系，匹配时保留最后一级名称；
 * 2. 行号过滤：同名候选存在多个时，用行号范围（双闭区间）包含还原行号的条目。
 * 快照 VM 记录的行号是函数首条执行语句所在行（构造函数因字段初始化提升，
 * 记录落在字段声明区），当还原行号不在该方法的行号范围内时，
 * 修正为定义起始行最贴近还原行号的候选。函数名可能是混淆名（开启属性混淆）
 * 或原始名（未开启时），两者都查。未修正返回 false（保持 sourceMap 映射结果）。
 */
bool RefineLineByNameCache(const NameCacheIndex* ncache, const std::string& sourcePath,
    const std::string& suffix, int& origLine)
{
    if (ncache == nullptr || sourcePath.empty()) {
        return false;
    }
    // 从 suffix（形如 "#funcName(line:N)[module]"）提取函数名，
    // 按官方约定保留最后一级名称（如 "#Class#method" 取 "method"）
    const size_t nameBegin = suffix.find('#');
    if (nameBegin == std::string::npos) {
        return false;
    }
    const size_t nameEnd = suffix.find("(line:", nameBegin + 1);
    if (nameEnd == std::string::npos || nameEnd <= nameBegin + 1) {
        return false;
    }
    const std::string fullName = suffix.substr(nameBegin + 1, nameEnd - nameBegin - 1);
    if (fullName.empty()) {
        return false;
    }
    const size_t lastHash = fullName.rfind('#');
    const std::string shortName =
        (lastHash == std::string::npos) ? fullName : fullName.substr(lastHash + 1);

    // 依次尝试：短名(混淆) → 短名(原始) → 完整名(混淆) → 完整名(原始)，
    // 取首个有候选者即停止。函数名可能是混淆名（开启属性混淆）或原始名（未开启时）。
    std::vector<NameCacheCandidate> candidates;
    auto tryLookup = [&candidates, &ncache](const std::string& name, bool obfuscated) {
        if (name.empty() || !candidates.empty()) {
            return;
        }
        if (obfuscated) {
            ncache->LookupObfuscatedName(name, candidates);
        } else {
            ncache->LookupOriginalName(name, candidates);
        }
    };
    tryLookup(shortName, true);
    tryLookup(shortName, false);
    tryLookup(fullName, true);
    tryLookup(fullName, false);
    if (candidates.empty()) {
        return false;
    }

    // 官方算法：行号范围包含还原行号的条目为命中（此时行号本身正确，无需修正）；
    // 范围外（构造函数首语句提升）时选择定义起始行最贴近还原行号的候选修正：
    // 优先取 start_line <= 还原行号中最大者，否则取最小 start_line
    const NameCacheCandidate* hit = nullptr;
    const NameCacheCandidate* fallback = nullptr;
    SelectNameCacheCandidate(candidates, sourcePath, origLine, hit, fallback);
    if (hit != nullptr || fallback == nullptr) {
        return false;
    }
    origLine = fallback->startLine;
    return true;
}

/**
 * 还原 VM 函数标记串 "path#funcName(line:N)[module]" 后缀中的混淆函数名。
 * 快照中该类字符串形如 "entry@1.0.0/src/main/ets/pages/WebPage.ts#g1(line:24)[entry]"，
 * 路径部分经 sourceMap 还原后为 "entry/src/main/ets/pages/WebPage.ets#g1(line:24)[entry]"，
 * 但其中的函数名（如 g1/d1/a1）仍是混淆名。此处按文件上下文过滤 nameCache 候选，
 * 候选去重后仅一种原始名时才替换，保证不会误还原。
 * Filter name cache candidates by source path and deduplicate by original name.
 * Returns the unique original name, or empty if no match or ambiguous.
 */
static std::string ResolveObfNameFromCandidates(
    const std::vector<NameCacheCandidate>& candidates,
    const std::string& ctxPath, const std::string& obfName)
{
    std::string original;
    for (const auto& cand : candidates) {
        if (cand.original.empty()) {
            continue;
        }
        if (!SourcePathMatches(ctxPath, cand.filePath)) {
            continue;
        }
        std::string name = cand.original;
        if (name.front() == '#') {
            name.erase(0, 1);
        }
        if (name.empty() || name == obfName) {
            continue;
        }
        if (original.empty()) {
            original = name;
        } else if (original != name) {
            return {};  // 同一文件下候选原始名不一致，无法唯一还原
        }
    }
    return original;
}

bool RestoreFunctionNameInSuffix(const NameCacheIndex* ncache, std::string& value)
{
    if (ncache == nullptr) {
        return false;
    }
    const size_t line = value.find("(line:");
    if (line == std::string::npos || line == 0) {
        return false;
    }
    // 取 '(' 之前最后一个 '#'，其后的子串即为混淆函数名（如 "#d1(" → "d1"）
    const size_t hash = value.rfind('#', line - 1);
    if (hash == std::string::npos || hash + 1 >= line) {
        return false;
    }
    const size_t nameLen = line - hash - 1;  // '#' 与 '(' 之间的函数名长度
    if (nameLen == 0) {
        return false;
    }
    const std::string obfName = value.substr(hash + 1, nameLen);
    if (obfName.empty()) {
        return false;
    }

    // 反查混淆名候选，用已还原的源码路径做文件上下文过滤，再按原始名去重。
    // 同一混淆名可能同时命中 IdentifierCache（类名，原始名带 '#' 前缀）与
    // MemberMethodCache（构造函数与方法名，原始名不带 '#'，如类与构造函数同名），
    // 比较与替换前统一剥离 '#' 前缀，避免同名不同前缀导致误判无法唯一还原。
    std::vector<NameCacheCandidate> candidates;
    if (!ncache->LookupObfuscatedName(obfName, candidates)) {
        return false;
    }
    const std::string ctxPath = value.substr(0, hash);
    std::string original = ResolveObfNameFromCandidates(candidates, ctxPath, obfName);
    if (original.empty()) {
        return false;
    }
    value.replace(hash + 1, nameLen, original);
    return true;
}

void ResolveSuffixLineMapping(std::string& suffix,
    const SourceMapIndex* smap, const NameCacheIndex* ncache,
    const SourceMapEntry* entry)
{
    // 映射 (line:N)：取 N，若该行在 mappings 中有映射则替换为源码行号
    const std::string linePrefix = "(line:";
    const size_t lp = suffix.find(linePrefix);
    if (lp != std::string::npos) {
        const size_t numBegin = lp + linePrefix.size();
        const size_t numEnd = suffix.find(')', numBegin);
        if (numEnd != std::string::npos) {
            const std::string numStr = suffix.substr(numBegin, numEnd - numBegin);
            if (AllDigits(numStr)) {
                int genLine = 0;
                // 快照损坏时可能为超长数字串，stoi 抛异常则跳过行号映射
                try {
                    genLine = std::stoi(numStr);
                } catch (const std::exception&) {
                    genLine = -1;
                }
                if (genLine > 0) {
                    int origLine = genLine;
                    smap->ResolveOriginalLine(entry->mappings, genLine, origLine);
                    RefineLineByNameCache(ncache, entry->sourcePath, suffix, origLine);
                    suffix.replace(lp, numEnd - lp + 1,
                        "(line:" + std::to_string(origLine) + ")");
                }
            }
        }
    }
}

/**
 * 对单个 strings 数组元素做反混淆还原（混淆路径→源码路径、混淆名→原始名）。
 * 返回还原后的字符串；发生替换时递增 replaced。
 */
std::string DeobfuscateOneString(const std::string& value,
    const SourceMapIndex* smap,
    const NameCacheIndex* ncache,
    size_t& replaced)
{
    std::string result = value;
    std::string original;

    // 1. obfName 混淆路径 → 原始文件路径
    if (ncache && ncache->LookupObfNamePath(result, original)) {
        result = original;
        replaced++;
    }
    // 2. 混淆编译产物路径 key → 源码路径（sources[0]）
    if (smap) {
        const SourceMapEntry* entry = smap->Lookup(result);
        if (entry != nullptr && !entry->sourcePath.empty()) {
            result = entry->sourcePath;
            replaced++;
        } else {
            // 兼容 heapsnapshot 中的 ArkTS 模块路径格式
            // "[<bundle>/]<module>@<version>/<rel_path>[#func(line:N)[module]]"：
            // 按 (module, version, rel_path) 命中后，仅替换路径部分，保留 "#" 后缀，
            // 并尝试将 (line:N) 由编译产物行号映射为源码行号。
            const size_t hashPos = result.find('#');
            const std::string pathPart = result.substr(0, hashPos);
            std::string module;
            std::string version;
            std::string relPath;
            entry = nullptr;
            if (ParseArktsModulePath(pathPart, module, version, relPath)) {
                entry = smap->LookupByModule(module, version, relPath);
            }
            if (entry != nullptr && !entry->sourcePath.empty()) {
                std::string suffix = (hashPos == std::string::npos) ? "" : result.substr(hashPos);
                ResolveSuffixLineMapping(suffix, smap, ncache, entry);
                result = entry->sourcePath + suffix;
                replaced++;
            }
        }
    }
    // 3. 混淆名 → 原始名（仅当候选唯一）
    if (ncache && ncache->LookupUnique(result, original)) {
        result = original;
        replaced++;
    }
    // 4. "path#funcName(line:N)[module]" 后缀中的混淆函数名 → 原始名
    //    （按文件上下文过滤，候选去重后仅一种原始名时替换）
    if (ncache && RestoreFunctionNameInSuffix(ncache, result)) {
        replaced++;
    }

    return result;
}

} // anonymous namespace

// ============================================================================
//  deobfuscate_heapsnapshot: 流式反混淆 heapsnapshot
//  仅解析并改写顶层 "strings" 字符串数组，nodes / edges / meta 等其余字段
//  按原始字节流式拷贝，避免大快照全量 json::parse + dump 导致的内存峰值。
// ============================================================================
/**
 * Parse the strings array from a heapsnapshot file.
 * Steps 3-4: read array text and parse as JSON (with sanitize fallback).
 */
static bool ParseStringsArray(std::ifstream& in, size_t openBracket, size_t closeBracket,
    const std::string& inputPath, json& stringsArr)
{
    // 3. 读取 strings 数组文本 [open_bracket, close_bracket]
    std::string arrText = ReadByteRange(in, openBracket, closeBracket + 1);

    // 4. 解析 strings 数组（首次失败则对控制字符做 sanitize 后重试）
    bool parsed = false;
    try {
        stringsArr = json::parse(arrText);
        parsed = true;
    } catch (const std::exception&) {
        try {
            stringsArr = json::parse(SanitizeControlChars(arrText));
            parsed = true;
        } catch (const std::exception& e) {
            log::Loge("Error: Failed to parse \"strings\" array in ", inputPath, " as JSON: ", e.what());
        }
    }
    if (!parsed || !stringsArr.is_array()) {
        log::Loge("Error: \"strings\" is not an array in heapsnapshot file: ", inputPath);
        return false;
    }
    return true;
}

/**
 * Write deobfuscated output: header + new strings + tail.
 * Step 7: write output file.
 */
static bool WriteDeobfuscatedOutput(std::ifstream& in, const std::string& outputPath,
    size_t openBracket, size_t closeBracket, size_t fileSize,
    const std::string& newArr)
{
    // 7. 写出输出文件：[0, open_bracket) + 新 strings 数组 + (close_bracket, end)
    std::ofstream out(mem_analyzer::PathFromUtf8(outputPath), std::ios::binary);
    if (!out) {
        log::Loge("Error: Failed to open output file for writing: ", outputPath);
        return false;
    }
    if (!CopyByteRange(in, out, 0, openBracket)) {
        log::Loge("Error: Failed to copy snapshot header to output: ", outputPath);
        return false;
    }
    out.write(newArr.data(), newArr.size());
    if (!out.good()) {
        log::Loge("Error: Failed to write deobfuscated strings to output: ", outputPath);
        return false;
    }
    if (!CopyByteRange(in, out, closeBracket + 1, fileSize)) {
        log::Loge("Error: Failed to copy snapshot tail to output: ", outputPath);
        return false;
    }
    if (!out.good()) {
        log::Loge("Error: Failed to finalize output file: ", outputPath);
        return false;
    }
    return true;
}

bool DeobfuscateHeapsnapshot(const std::string& inputPath, const std::string& outputPath,
    const SourceMapIndex* smap, const NameCacheIndex* ncache)
{
    std::error_code ec;
    if (!fs::is_regular_file(mem_analyzer::PathFromUtf8(inputPath), ec)) {
        log::Loge("Error: Heapsnapshot file not found: ", inputPath);
        return false;
    }

    std::ifstream in(mem_analyzer::PathFromUtf8(inputPath), std::ios::binary);
    if (!in) {
        log::Loge("Error: Failed to open heapsnapshot file: ", inputPath);
        return false;
    }
    in.seekg(0, std::ios::end);
    const size_t fileSize = static_cast<size_t>(in.tellg());
    if (fileSize == 0) {
        log::Loge("Error: Heapsnapshot file is empty: ", inputPath);
        return false;
    }

    // 1. 流式定位 "strings": 标记及其后的 '['
    const std::string marker = "\"strings\":";
    size_t markPos = FindMarkerOffset(in, marker, 0, fileSize);
    if (markPos == std::string::npos) {
        log::Loge("Error: No \"strings\" field in heapsnapshot file: ", inputPath);
        return false;
    }
    size_t openBracket = FindCharAfter(in, '[', markPos + marker.size(), fileSize);
    if (openBracket == std::string::npos) {
        log::Loge("Error: \"strings\" is not an array in heapsnapshot file: ", inputPath);
        return false;
    }

    // 2. 流式扫描 strings 数组的匹配 ']'（正确处理字符串字面量与转义）
    size_t closeBracket = FindMatchingCloseBracket(in, openBracket, fileSize);
    if (closeBracket == std::string::npos) {
        log::Loge("Error: Unterminated \"strings\" array in heapsnapshot file: ", inputPath);
        return false;
    }

    json stringsArr;
    if (!ParseStringsArray(in, openBracket, closeBracket, inputPath, stringsArr)) {
        return false;
    }

    // 5. 逐条反混淆 strings 数组元素
    size_t replaced = 0;
    for (auto& s : stringsArr) {
        if (!s.is_string()) {
            continue;
        }
        std::string value = s.get<std::string>();
        std::string restored = DeobfuscateOneString(value, smap, ncache, replaced);
        s = restored;
    }

    // 6. 重新序列化 strings 数组（紧凑格式）
    std::string newArr = stringsArr.dump();

    if (!WriteDeobfuscatedOutput(in, outputPath, openBracket, closeBracket, fileSize, newArr)) {
        return false;
    }

    log::Logi("  Deobfuscated ", replaced, " strings in: ", inputPath);
    return true;
}

// ============================================================================
//  deobfuscate_with_indices: 加载索引 + 反混淆的公共时序
//  供 jsleak_parse()（jsleak 入口）与 deobfuscate_zip_snapshot
//  （tshandle zip 入口）共用，消除重复逻辑。
// ============================================================================
bool DeobfuscateWithIndices(const std::string& inputPath, const std::string& outputPath,
    const std::string& sourcemapDir,
    const std::string& namecacheDir)
{
    // 两个目录都为空 → 无操作，调用方应直接降级为原始输入
    if (sourcemapDir.empty() && namecacheDir.empty()) {
        return false;
    }

    // 校验路径存在（--sourcemap / --namecache 支持目录或单个文件）
    // 用户路径可能含中文，需按 UTF-8 构造 fs::path（Windows 宽路径安全）
    auto pathExists = [](const std::string& path) {
        std::error_code ec;
        return !path.empty() && fs::exists(mem_analyzer::PathFromUtf8(path), ec);
    };
    if (!sourcemapDir.empty() && !pathExists(sourcemapDir)) {
        log::Loge("Error: Source map file or directory does not exist: ", sourcemapDir);
        return false;
    }
    if (!namecacheDir.empty() && !pathExists(namecacheDir)) {
        log::Loge("Error: nameCache file or directory does not exist: ", namecacheDir);
        return false;
    }

    // 加载索引，加载失败仅警告并继续（对应还原降级）
    SourceMapIndex smap;
    NameCacheIndex ncache;
    if (!sourcemapDir.empty()) {
        if (!smap.LoadFromDir(sourcemapDir)) {
            log::Logw("Warning: Source map index unavailable, path deobfuscation will be skipped.");
        }
    }
    if (!namecacheDir.empty()) {
        if (!ncache.LoadFromDir(namecacheDir)) {
            log::Logw("Warning: nameCache index unavailable, name deobfuscation will be skipped.");
        }
    }
    // 两者均空 → 警告后降级为原始输入
    if (smap.Empty() && ncache.Empty()) {
        log::Logw("Warning: No source map / nameCache entries loaded, proceeding with the original snapshot.");
        return false;
    }

    // 创建输出文件的父目录。注意：macOS libc++ 的 fs::create_directories 在
    // 目录已存在时返回 false 且 ec 为空（与 Linux/标准实现返回 true 不同），
    // 因此仅当 ec 非空（创建真失败）时才报错，目录已存在视为成功。
    const std::string parentDir = mem_analyzer::platform::Dirname(outputPath);
    if (!parentDir.empty()) {
        std::error_code ec;
        if (!fs::create_directories(mem_analyzer::PathFromUtf8(parentDir), ec) && ec) {
            log::Loge("Error: Failed to create output directory: ", parentDir, " (", ec.message(), ")");
            return false;
        }
    }

    const SourceMapIndex* smapPtr = sourcemapDir.empty() ? nullptr : &smap;
    const NameCacheIndex* ncachePtr = namecacheDir.empty() ? nullptr : &ncache;
    return DeobfuscateHeapsnapshot(inputPath, outputPath, smapPtr, ncachePtr);
}

} // namespace mem_analyzer
