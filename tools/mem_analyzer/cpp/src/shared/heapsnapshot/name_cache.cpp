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
#include "shared/heapsnapshot/name_cache.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/string_utils.hpp"
#include "utils/file_utils.hpp"  // path_from_utf8/path_to_utf8: Windows wide-path 安全
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace mem_analyzer {

using json = nlohmann::json;

namespace {

/// 解析 nameCache 缓存条目 key 的 ":start:end" 行号后缀。
/// 形如 "方法#成员#变量" / "#类名:start:end" / "方法名:start:end" / 普通变量名。
/// 成功解析后缀时返回 true，name 为去掉后缀的原始名，start_line/end_line 为行号；
/// 无后缀时返回 false，name 原样保留，行号为 -1/-1。
bool ParseLineSuffix(const std::string& key, std::string& name, int& startLine, int& endLine)
{
    startLine = -1;
    endLine = -1;
    name = key;

    size_t endColon = key.rfind(':');
    if (endColon == std::string::npos || endColon + 1 >= key.size()) {
        return false;
    }
    if (!AllDigits(key.substr(endColon + 1))) {
        return false;
    }

    size_t startColon = key.rfind(':', endColon - 1);
    if (startColon == std::string::npos || startColon + 1 >= endColon) {
        return false;
    }
    if (!AllDigits(key.substr(startColon + 1, endColon - startColon - 1))) {
        return false;
    }

    try {
        startLine = std::stoi(key.substr(startColon + 1, endColon - startColon - 1));
        endLine = std::stoi(key.substr(endColon + 1));
    } catch (const std::exception&) {
        startLine = -1;
        endLine = -1;
        return false;
    }
    name = key.substr(0, startColon);
    return true;
}

/// 将单个缓存对象（IdentifierCache/MemberMethodCache/PropertyCache）的
/// 键值对反查入索引：value（混淆名）→ key（原始名，解析行号后缀）。
/// 同时建立 原始名 → 候选 的正向索引（未开启属性混淆时快照中的方法名即为原始名）。
/// 官方约定：key 中 '#' 代表上下级关系（如 "Class#method#__function"），
/// 匹配时保留最后一级名称，故额外以最后一级名称建立别名索引。
void IndexCacheObject(const json& obj, const std::string& filePath,
    std::unordered_map<std::string, std::vector<NameCacheCandidate>>& byObfuscated,
    std::unordered_map<std::string, std::vector<NameCacheCandidate>>& byOriginal)
{
    if (!obj.is_object()) {
        return;
    }
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (!it.value().is_string()) {
            continue;
        }
        const std::string obf = it.value().get<std::string>();
        if (obf.empty()) {
            continue;
        }

        NameCacheCandidate candidate;
        candidate.filePath = filePath;
        ParseLineSuffix(it.key(), candidate.original, candidate.startLine, candidate.endLine);
        byObfuscated[obf].push_back(candidate);
        if (!candidate.original.empty()) {
            byOriginal[candidate.original].push_back(candidate);
            const size_t hashPos = candidate.original.rfind('#');
            if (hashPos != std::string::npos && hashPos + 1 < candidate.original.size()) {
                byOriginal[candidate.original.substr(hashPos + 1)].push_back(std::move(candidate));
            }
        }
    }
}

} // anonymous namespace

bool NameCacheIndex::LoadFromDir(const std::string& dir)
{
    std::error_code ec;
    std::vector<std::string> files;
    const fs::path dirPath = mem_analyzer::PathFromUtf8(dir);

    // 直接指定 nameCache.json 文件本身
    if (fs::is_regular_file(dirPath, ec)) {
        files.push_back(dir);
    } else {
        ec.clear();
        if (!fs::is_directory(dirPath, ec)) {
            log::Logw("Warning: nameCache file or directory not found: ", dir);
            return false;
        }
        // 递归扫描目录：文件名包含 "nameCache" 的 *.json 全部加载合并（兼容多模块产物）
        for (const auto& entry : fs::recursive_directory_iterator(dirPath, ec)) {
            if (ec) {
                break;
            }
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::string filename = mem_analyzer::PathToUtf8(entry.path().filename());
            if (filename.size() < 8) {
                continue;
            }
            std::string lower = filename;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower.find("namecache") == std::string::npos) {
                continue;
            }
            const std::string ext = mem_analyzer::PathToUtf8(entry.path().extension());
            if (ext != ".json" && ext != ".JSON") {
                continue;
            }
            files.push_back(mem_analyzer::PathToUtf8(entry.path()));
        }
    }

    if (files.empty()) {
        log::Logw("Warning: nameCache.json file not found in: ", dir);
        return false;
    }

    size_t loaded = 0;
    for (const auto& f : files) {
        if (IndexNameCacheFile(f)) {
            ++loaded;
        }
    }
    if (loaded == 0) {
        log::Logw("Warning: Failed to parse any nameCache file in: ", dir);
        return false;
    }

    log::Logi("  Loaded ", byObfuscated.size(), " obfuscated names, ", obfNameToPath.size(),
        " obfuscated paths from ", loaded, " nameCache file(s) in: ", dir);
    return true;
}

bool NameCacheIndex::IndexNameCacheFile(const std::string& filePath)
{
    // 用户路径可能含中文，需按 UTF-8 构造 fs::path（Windows 宽路径安全）
    std::ifstream ifs(mem_analyzer::PathFromUtf8(filePath), std::ios::binary);
    if (!ifs) {
        log::Logw("Warning: Failed to open nameCache file: ", filePath);
        return false;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();

    json root;
    try {
        root = json::parse(ss.str());
    } catch (const std::exception& e) {
        log::Logw("Warning: Failed to parse nameCache file ", filePath, ": ", e.what());
        return false;
    }
    if (!root.is_object()) {
        log::Logw("Warning: nameCache file top level is not an object: ", filePath);
        return false;
    }

    // 顶层键：文件原始路径 → { IdentifierCache, MemberMethodCache, PropertyCache, obfName, ... }
    // compileSdkVersion / entryPackageInfo 等顶层字段不是对象，直接跳过。
    for (auto it = root.begin(); it != root.end(); ++it) {
        const std::string& fileOriginalPath = it.key();
        const json& fileObj = it.value();
        if (!fileObj.is_object()) {
            continue;
        }

        static const char* kCaches[] = {"IdentifierCache", "MemberMethodCache", "PropertyCache"};
        for (const char* cacheName : kCaches) {
            auto cacheIt = fileObj.find(cacheName);
            if (cacheIt != fileObj.end()) {
                IndexCacheObject(cacheIt.value(), fileOriginalPath, byObfuscated, byOriginal);
            }
        }

        // obfName 混淆路径 → 文件原始路径（顶层 key）
        auto obfIt = fileObj.find("obfName");
        if (obfIt != fileObj.end() && obfIt.value().is_string()) {
            const std::string obfPath = obfIt.value().get<std::string>();
            if (!obfPath.empty()) {
                obfNameToPath[obfPath] = fileOriginalPath;
            }
        }
    }

    // 顶层 PropertyCache（全局属性名混淆映射，不隶属于单个文件）
    auto topPropertyIt = root.find("PropertyCache");
    if (topPropertyIt != root.end() && topPropertyIt.value().is_object()) {
        IndexCacheObject(topPropertyIt.value(), "", byObfuscated, byOriginal);
    }
    return true;
}

bool NameCacheIndex::LookupObfuscatedName(const std::string& obf, std::vector<NameCacheCandidate>& out) const
{
    auto it = byObfuscated.find(obf);
    if (it == byObfuscated.end()) {
        return false;
    }
    out = it->second;
    return true;
}

bool NameCacheIndex::LookupOriginalName(const std::string& original, std::vector<NameCacheCandidate>& out) const
{
    auto it = byOriginal.find(original);
    if (it == byOriginal.end()) {
        return false;
    }
    out = it->second;
    return true;
}

bool NameCacheIndex::LookupObfNamePath(const std::string& obfPath, std::string& originalPath) const
{
    auto it = obfNameToPath.find(obfPath);
    if (it == obfNameToPath.end()) {
        return false;
    }
    originalPath = it->second;
    return true;
}

bool NameCacheIndex::LookupUnique(const std::string& obf, std::string& original) const
{
    auto it = byObfuscated.find(obf);
    if (it == byObfuscated.end() || it->second.size() != 1) {
        return false;
    }
    original = it->second.front().original;
    return true;
}

bool NameCacheIndex::empty() const
{
    return byObfuscated.empty() && obfNameToPath.empty();
}

} // namespace mem_analyzer
