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
    long long lowThreshold = 1024;
    int approRelRatio = 10;                     // 相对比例（%）
    long long approRelCap = 10LL * 1024 * 1024; // 相对阈值封顶（10MB）
    int approRatioThreshold = 30;
    int orphanRelRatio = 1;                           // 孤儿栈兜底相对比例（%）
    long long orphanRelCap = 10LL * 1024 * 1024;      // 孤儿栈兜底阈值封顶（10MB）
};

struct ClassificationRule {
    std::string type;
    std::vector<std::string> libContains;
    std::vector<std::string> libEndsWith;
    std::vector<std::string> libTsPattern;
    bool requireNoVaddr = false;
};

struct SymbolFilterRule {
    std::string scope;
    std::vector<std::string> lib;
    std::vector<std::string> symbolContains;
    std::vector<std::string> symbolEquals;
    bool orphanRecoverable = false;   // 孤儿栈兜底时是否可回收该规则命中的符号
};

class HtraceConfigMgr {
public:
    static HtraceConfigMgr& Instance();

    bool load(const std::string& path, std::string& err);
    bool LoadDefault(std::string& err);
    bool IsLoaded() const;

    const HtraceAlgorithmConfig& Algorithm() const;
    const std::vector<ClassificationRule>& ClassificationRules() const;
    const std::vector<SymbolFilterRule>& FilterRules() const;

    bool IsAppSymbol(const std::string& lib, bool vaddrNull) const;
    bool IsTsSymbol(const std::string& lib, bool vaddrNull) const;
    bool ShouldFilter(const std::string& libFile, const std::string& symName, Scope scope) const;
    bool IsOrphanRecoverable(const std::string& libFile, const std::string& symName, Scope scope) const;

private:
    HtraceConfigMgr() = default;

    HtraceAlgorithmConfig algo_;
    std::vector<ClassificationRule> classRules_;
    std::vector<SymbolFilterRule> filterRules_;
    bool loaded_ = false;

    static std::string ResolveDefaultPath();

    static std::vector<ClassificationRule> DefaultClassificationRules();
    static std::vector<SymbolFilterRule> DefaultFilterRules();

    bool MatchClassificationRule(const ClassificationRule& rule, const std::string& lib, bool vaddrNull) const;
    bool MatchFilterRule(const SymbolFilterRule& rule,
        const std::string& libFile, const std::string& symName,
        Scope scope) const;
    static bool MatchTsPattern(const std::string& lib, const std::string& ext);
};

} // namespace mem_analyzer
