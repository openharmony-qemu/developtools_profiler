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
#include "config_mgr/htrace_config_mgr.hpp"
#include "utils/file_utils.hpp"
#include "utils/platform.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cctype>
#include <cstdlib>

using json = nlohmann::json;

namespace mem_analyzer {

namespace {
constexpr size_t TS_JS_PATTERN_LEN = 4;
constexpr size_t ETS_PATTERN_LEN = 5;
constexpr size_t TS_JS_COLON_OFFSET = 3;
constexpr size_t ETS_COLON_OFFSET = 4;
constexpr size_t EXT_CHAR2_OFFSET = 2;
constexpr size_t EXT_CHAR3_OFFSET = 3;
} // namespace

// ===== Singleton =====

HtraceConfigMgr& HtraceConfigMgr::Instance()
{
    static HtraceConfigMgr inst;
    return inst;
}

// ===== Path resolution =====

std::string HtraceConfigMgr::ResolveDefaultPath()
{
    // 固定目录：可执行文件同级的 config/htrace/trace_analyzer_config.json
    const std::string rel = "config/htrace/trace_analyzer_config.json";
    const std::string exeDir = platform::GetExecutableDir();
    if (exeDir.empty()) {
        return "";
    }
    const std::string path = platform::JoinPath(exeDir, rel);
    return platform::FileExists(path) ? path : "";
}

// ===== Default rules =====

std::vector<ClassificationRule> HtraceConfigMgr::DefaultClassificationRules()
{
    return {
        {"system", {"libc++.so", "libc++_shared.so"}, {}, {}, false},
        {"app", {"/system/app/", "/sys_prod/app/", "/data/storage/el1/bundle"}, {}, {}, false},
        {"app", {}, {".hap", ".hsp", ".har"}, {}, true},
        {"app", {}, {}, {".ts", ".js", ".ets"}, true},
    };
}

std::vector<SymbolFilterRule> HtraceConfigMgr::DefaultFilterRules()
{
    return {
        {"global", {"unknown", ""}, {"alloc size("}, {}},
        {"global", {"libc++.so", "libc++_shared.so"}, {"operator new"}, {}},
    };
}

// ===== Load =====

bool HtraceConfigMgr::Load(const std::string& path, std::string& err)
{
    std::ifstream ifs(PathFromUtf8(path), std::ios::binary);
    if (!ifs.is_open()) {
        err = "cannot open file: " + path;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    ifs.close();

    json root;
    try {
        root = json::parse(content);
    } catch (const json::parse_error& e) {
        err = std::string("JSON parse error: ") + e.what();
        return false;
    }

    // Algorithm
    if (root.contains("algorithm") && root["algorithm"].is_object()) {
        auto& a = root["algorithm"];
        if (a.contains("low_threshold") && a["low_threshold"].is_number()) {
            algo_.lowThreshold = a["low_threshold"].get<long long>();
        }
        if (a.contains("appro_rel_ratio") && a["appro_rel_ratio"].is_number()) {
            algo_.approRelRatio = a["appro_rel_ratio"].get<int>();
        }
        if (a.contains("appro_rel_cap") && a["appro_rel_cap"].is_number()) {
            algo_.approRelCap = a["appro_rel_cap"].get<long long>();
        }
        if (a.contains("appro_ratio_threshold") && a["appro_ratio_threshold"].is_number()) {
            algo_.approRatioThreshold = a["appro_ratio_threshold"].get<int>();
        }
        if (a.contains("orphan_rel_ratio") && a["orphan_rel_ratio"].is_number()) {
            algo_.orphanRelRatio = a["orphan_rel_ratio"].get<int>();
        }
        if (a.contains("orphan_rel_cap") && a["orphan_rel_cap"].is_number()) {
            algo_.orphanRelCap = a["orphan_rel_cap"].get<long long>();
        }
    }

    // Classification rules
    if (root.contains("symbol_classification_rules") && root["symbol_classification_rules"].is_array()) {
        classRules_.clear();
        for (const auto& item : root["symbol_classification_rules"]) {
            ClassificationRule rule;
            if (item.contains("type") && item["type"].is_string()) {
                rule.type = item["type"].get<std::string>();
            }
            if (item.contains("lib_contains") && item["lib_contains"].is_array()) {
                for (const auto& v : item["lib_contains"]) {
                    if (v.is_string()) {
                        rule.libContains.push_back(v.get<std::string>());
                    }
                }
            }
            if (item.contains("lib_ends_with") && item["lib_ends_with"].is_array()) {
                for (const auto& v : item["lib_ends_with"]) {
                    if (v.is_string()) {
                        rule.libEndsWith.push_back(v.get<std::string>());
                    }
                }
            }
            if (item.contains("lib_ts_pattern") && item["lib_ts_pattern"].is_array()) {
                for (const auto& v : item["lib_ts_pattern"]) {
                    if (v.is_string()) {
                        rule.libTsPattern.push_back(v.get<std::string>());
                    }
                }
            }
            if (item.contains("require_no_vaddr") && item["require_no_vaddr"].is_boolean()) {
                rule.requireNoVaddr = item["require_no_vaddr"].get<bool>();
            }
            classRules_.push_back(std::move(rule));
        }
    }
    if (classRules_.empty()) {
        classRules_ = DefaultClassificationRules();
    }

    // Filter rules
    if (root.contains("symbol_filter_rules") && root["symbol_filter_rules"].is_array()) {
        filterRules_.clear();
        for (const auto& item : root["symbol_filter_rules"]) {
            SymbolFilterRule rule;
            if (item.contains("scope") && item["scope"].is_string()) {
                rule.scope = item["scope"].get<std::string>();
            }
            if (item.contains("lib") && item["lib"].is_array()) {
                for (const auto& v : item["lib"]) {
                    if (v.is_string()) {
                        rule.lib.push_back(v.get<std::string>());
                    }
                }
            }
            if (item.contains("symbol_contains") && item["symbol_contains"].is_array()) {
                for (const auto& v : item["symbol_contains"]) {
                    if (v.is_string()) {
                        rule.symbolContains.push_back(v.get<std::string>());
                    }
                }
            }
            if (item.contains("symbol_equals") && item["symbol_equals"].is_array()) {
                for (const auto& v : item["symbol_equals"]) {
                    if (v.is_string()) {
                        rule.symbolEquals.push_back(v.get<std::string>());
                    }
                }
            }
            if (item.contains("orphan_recoverable") && item["orphan_recoverable"].is_boolean()) {
                rule.orphanRecoverable = item["orphan_recoverable"].get<bool>();
            }
            filterRules_.push_back(std::move(rule));
        }
    }
    if (filterRules_.empty()) {
        filterRules_ = DefaultFilterRules();
    }

    loaded_ = true;
    return true;
}

bool HtraceConfigMgr::LoadDefault(std::string& err)
{
    std::string path = ResolveDefaultPath();
    if (path.empty()) {
        // Config file not found — use defaults
        algo_ = HtraceAlgorithmConfig();
        classRules_ = DefaultClassificationRules();
        filterRules_ = DefaultFilterRules();
        loaded_ = true;
        return true;
    }
    return Load(path, err);
}

bool HtraceConfigMgr::IsLoaded() const
{
    return loaded_;
}

// ===== Accessors =====

const HtraceAlgorithmConfig& HtraceConfigMgr::Algorithm() const
{
    return algo_;
}

const std::vector<ClassificationRule>& HtraceConfigMgr::ClassificationRules() const
{
    return classRules_;
}

const std::vector<SymbolFilterRule>& HtraceConfigMgr::FilterRules() const
{
    return filterRules_;
}

// ===== Classification =====

bool HtraceConfigMgr::MatchTsPattern(const std::string& lib, const std::string& ext)
{
    // Find last '.' and check for .ts:/.js:/.ets: pattern
    auto dot = lib.rfind('.');
    if (dot == std::string::npos) {
        return false;
    }

    // Validate :digits:digits suffix starting at 'start'
    auto validLineCol = [&lib](size_t start) -> bool {
        if (start + 1 >= lib.size() || !std::isdigit((unsigned char)lib[start + 1])) {
            return false;
        }
        size_t i = start + 1;
        while (i < lib.size() && std::isdigit((unsigned char)lib[i])) {
            i++;
        }
        return i + 1 < lib.size() && lib[i] == ':' && std::isdigit((unsigned char)lib[i + 1]);
    };

    if (ext == ".ts" || ext == ".js") {
        if (dot + TS_JS_PATTERN_LEN <= lib.size()) {
            char a = lib[dot + 1];
            char b = lib[dot + EXT_CHAR2_OFFSET];
            char c = lib[dot + TS_JS_COLON_OFFSET];
            if (a == ext[1] && b == ext[EXT_CHAR2_OFFSET] && c == ':') {
                return validLineCol(dot + TS_JS_COLON_OFFSET);
            }
        }
    } else if (ext == ".ets") {
        if (dot + ETS_PATTERN_LEN <= lib.size()) {
            if (lib[dot + 1] == 'e' && lib[dot + EXT_CHAR2_OFFSET] == 't' &&
                lib[dot + EXT_CHAR3_OFFSET] == 's' && lib[dot + ETS_COLON_OFFSET] == ':') {
                return validLineCol(dot + ETS_COLON_OFFSET);
            }
        }
    }
    return false;
}

bool HtraceConfigMgr::MatchClassificationRule(const ClassificationRule& rule,
    const std::string& lib, bool vaddrNull) const
{
    if (rule.requireNoVaddr && !vaddrNull) {
        return false;
    }

    // lib_contains: OR
    for (const auto& pattern : rule.libContains) {
        if (lib.find(pattern) != std::string::npos) {
            return true;
        }
    }

    // lib_ends_with: OR
    for (const auto& suffix : rule.libEndsWith) {
        if (lib.size() >= suffix.size() &&
            lib.compare(lib.size() - suffix.size(), suffix.size(), suffix) == 0) {
            return true;
        }
    }

    // lib_ts_pattern: OR
    for (const auto& ext : rule.libTsPattern) {
        if (MatchTsPattern(lib, ext)) {
            return true;
        }
    }

    return false;
}

bool HtraceConfigMgr::IsAppSymbol(const std::string& lib, bool vaddrNull) const
{
    for (const auto& rule : classRules_) {
        if (MatchClassificationRule(rule, lib, vaddrNull)) {
            return rule.type == "app";
        }
    }
    return false;
}

bool HtraceConfigMgr::IsTsSymbol(const std::string& lib, bool vaddrNull) const
{
    for (const auto& rule : classRules_) {
        if (rule.type != "app") {
            continue;
        }
        if (rule.requireNoVaddr && !vaddrNull) {
            continue;
        }
        for (const auto& ext : rule.libTsPattern) {
            if (MatchTsPattern(lib, ext)) {
                return true;
            }
        }
        for (const auto& suffix : rule.libEndsWith) {
            if (lib.size() >= suffix.size() &&
                lib.compare(lib.size() - suffix.size(), suffix.size(), suffix) == 0) {
                return true;
            }
        }
    }
    return false;
}

// ===== Filtering =====

bool HtraceConfigMgr::MatchFilterRule(const SymbolFilterRule& rule,
    const std::string& libFile,
    const std::string& symName,
    Scope scope) const
{
    if (rule.scope == "app") {
        if (scope != Scope::APP_ONLY && scope != Scope::APP_AND_SYSTEM && scope != Scope::MIXED) {
            return false;
        }
    } else if (rule.scope == "system") {
        if (scope != Scope::SYSTEM_ONLY && scope != Scope::APP_AND_SYSTEM && scope != Scope::MIXED) {
            return false;
        }
    }

    if (!rule.lib.empty()) {
        bool libMatched = false;
        for (const auto& l : rule.lib) {
            if (libFile == l) {
                libMatched = true;
                break;
            }
        }
        if (!libMatched) {
            return false;
        }
    }

    if (rule.symbolContains.empty() && rule.symbolEquals.empty()) {
        return true;
    }

    for (const auto& substr : rule.symbolContains) {
        if (symName.find(substr) != std::string::npos) {
            return true;
        }
    }
    for (const auto& exact : rule.symbolEquals) {
        if (symName == exact) {
            return true;
        }
    }

    return false;
}

bool HtraceConfigMgr::ShouldFilter(const std::string& libFile, const std::string& symName, Scope scope) const
{
    for (const auto& rule : filterRules_) {
        if (MatchFilterRule(rule, libFile, symName, scope)) {
            return true;
        }
    }
    return false;
}

bool HtraceConfigMgr::IsOrphanRecoverable(const std::string& libFile, const std::string& symName, Scope scope) const
{
    // 仅在符号命中至少一条噪声规则时才可能回收；若同时命中任意一条
    // orphan_recoverable=false 的规则，则孤儿栈兜底时也不回收它。
    bool matched = false;
    for (const auto& rule : filterRules_) {
        if (MatchFilterRule(rule, libFile, symName, scope)) {
            matched = true;
            if (!rule.orphanRecoverable) {
                return false;
            }
        }
    }
    return matched;
}

} // namespace mem_analyzer
