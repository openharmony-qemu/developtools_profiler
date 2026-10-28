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
#include <unordered_set>
#include <vector>

namespace mem_analyzer {

/// 预览特性描述（name 同时作为特性 id，供 enabled() 查询）
struct PreviewFeatureInfo {
    std::string name;
    std::string description;
};

/**
 * 预览特性开关管理器。
 *
 * 所有尝鲜（预览）特性由该管理器按特性粒度管控，默认关闭；用户通过
 * `mem-analyzer preview enable|disable` 修改并持久化到
 * config/preview_features.json，工具下次启动时读取该文件，
 * 各功能模块通过 enabled(<特性 id>) 判断是否使能对应的预览特性。
 */
class PreviewFeaturesMgr {
public:
    static PreviewFeaturesMgr& Instance();

    /// 加载配置文件（文件不存在时视为全部关闭）。失败时填充 err 并返回 false
    bool load(std::string& err);

    /// 指定预览特性是否使能；未登记的 id 恒为 false
    bool Enabled(const std::string& featureId) const;

    /// 使能/关闭全部已登记预览特性并持久化
    bool SetEnabled(bool enabled, std::string& err);

    /// 受管控的特性清单（名称、描述）；新增预览特性时在此登记
    static const std::vector<PreviewFeatureInfo>& Registry();

    /// 配置文件的路径（已在 resolve_config_path 中确定）
    std::string ConfigPath() const { return configPath_; }

private:
    PreviewFeaturesMgr() = default;

    /// 解析配置文件路径：优先已存在的 config/preview_features.json
    /// （当前目录 → 可执行文件父目录 → 可执行文件目录），否则落在可执行文件目录的 config/ 下
    static std::string ResolveConfigPath();

    /// 将当前状态写入配置文件（含目录创建）
    bool Save(std::string& err) const;

    /// 当前已使能的特性 id（仅包含 registry 中登记过的 id）
    std::unordered_set<std::string> enabledFeatures_;
    std::string configPath_;
};

} // namespace mem_analyzer
