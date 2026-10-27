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
#include "config.hpp"

namespace mem_analyzer {

struct HtraceAlgorithmConfig {
    long long low_threshold = 1024;
    int appro_rel_ratio = 10;                     // 相对比例（%）
    long long appro_rel_cap = 10LL * 1024 * 1024; // 相对阈值封顶（10MB）
    int appro_ratio_threshold = 30;
    int orphan_rel_ratio = 1;                           // 孤儿栈兜底相对比例（%）
    long long orphan_rel_cap = 10LL * 1024 * 1024;      // 孤儿栈兜底阈值封顶（10MB）
};

struct ClassificationRule {
    std::string type;
    std::vector<std::string> lib_contains;
    std::vector<std::string> lib_ends_with;
    std::vector<std::string> lib_ts_pattern;
    bool require_no_vaddr = false;
};

struct SymbolFilterRule {
    std::string scope;
    std::vector<std::string> lib;
    std::vector<std::string> symbol_contains;
    std::vector<std::string> symbol_equals;
    bool orphan_recoverable = false;   // 孤儿栈兜底时是否可回收该规则命中的符号
};

class HtraceConfigMgr {
public:
    static HtraceConfigMgr& instance();

    bool load(const std::string& path, std::string& err);
    bool load_default(std::string& err);
    bool is_loaded() const;

    const HtraceAlgorithmConfig& algorithm() const;
    const std::vector<ClassificationRule>& classification_rules() const;
    const std::vector<SymbolFilterRule>& filter_rules() const;

    bool is_app_symbol(const std::string& lib, bool vaddr_null) const;
    bool is_ts_symbol(const std::string& lib, bool vaddr_null) const;
    bool should_filter(const std::string& lib_file,
                       const std::string& sym_name,
                       Scope scope) const;
    bool is_orphan_recoverable(const std::string& lib_file,
                               const std::string& sym_name,
                               Scope scope) const;

private:
    HtraceConfigMgr() = default;

    HtraceAlgorithmConfig algo_;
    std::vector<ClassificationRule> class_rules_;
    std::vector<SymbolFilterRule> filter_rules_;
    bool loaded_ = false;

    static std::string resolve_default_path();

    static std::vector<ClassificationRule> default_classification_rules();
    static std::vector<SymbolFilterRule> default_filter_rules();

    bool match_classification_rule(const ClassificationRule& rule,
                                    const std::string& lib, bool vaddr_null) const;
    bool match_filter_rule(const SymbolFilterRule& rule,
                           const std::string& lib_file, const std::string& sym_name,
                           Scope scope) const;
    static bool match_ts_pattern(const std::string& lib, const std::string& ext);
};

} // namespace mem_analyzer
