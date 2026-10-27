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
#include "jsleak/snapshot_analyzer.hpp"
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace mem_analyzer {

/// 一个目标组件基类的配置：name 为技术名（用于继承链包含匹配，也是 concernType 的值），
/// display_type 为展示类型（可空，空时回退为 name）。
struct BaseClassInfo {
    std::string name;
    std::string display_type;
};

/// 属性标签泄漏分析阶段：沿节点出边查找 isDeleting_ / _isDisposed / isDestroyed
/// 等「已销毁」标签，命中后沿继承链确定关注类型（concernType），聚合为可疑对象列表。
/// 与引用链阶段互不相干，独立持有自己的状态与配置。
class PropertyTagStage : public JsleakStage {
public:
    explicit PropertyTagStage(const std::string& jsleak_list_file);

    const std::string& mode() const override;   // "property-tag"
    const char* preview_id() const override;    // nullptr（不受预览开关管控）
    const char* json_key() const override;      // "propertyTagObjects"
    void run(const HeapSnapshot& heap) override;
    json to_json() const override;

    /// Run the property/hash leak rules for a caller-defined subset of nodes.
    json analyze(const HeapSnapshot& heap,
                 const std::function<bool(uint32_t)>& include_node = {});

private:
    std::string check_destoryed_tag(uint32_t u_ni, const HeapSnapshot& heap);
    BaseClassInfo find_concern_type(uint32_t u_ni, const std::vector<BaseClassInfo>& base_classes, const HeapSnapshot& heap);
    void assemble_property_tag_obj(uint32_t u_ni, const HeapSnapshot& heap,
                                   const std::string& property_name, const BaseClassInfo& base_class,
                                   uint32_t hash_value);
    void generate_property_tag_objs(const HeapSnapshot& heap,
                                    const std::function<bool(uint32_t)>& include_node = {});

    void load_jsleak_list_file(const std::string& file_path);
    void load_property_tag_config();

    std::unordered_map<std::string, PropertyTagObjInfo> property_tag_objs_;
    std::unordered_map<std::string, std::vector<BaseClassInfo>> property_tag_base_classes_;  // 属性标签 → 目标组件基类（包含匹配）
    std::string default_display_type_;       // 未命中任何基类时的展示类型（默认「其他」）
    std::unordered_set<uint32_t> jsleak_hash_set_;
};

} // namespace mem_analyzer
