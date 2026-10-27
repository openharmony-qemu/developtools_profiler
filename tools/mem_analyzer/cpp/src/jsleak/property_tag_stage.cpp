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
#include "jsleak/property_tag_stage.hpp"
#include "utils/file_utils.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

namespace mem_analyzer {

PropertyTagStage::PropertyTagStage(const std::string& jsleak_list_file) {
    load_property_tag_config();
    if (!jsleak_list_file.empty()) {
        load_jsleak_list_file(jsleak_list_file);
    }
}

const std::string& PropertyTagStage::mode() const {
    static const std::string kMode = "property-tag";
    return kMode;
}

const char* PropertyTagStage::preview_id() const {
    return nullptr;
}

const char* PropertyTagStage::json_key() const {
    return "propertyTagObjects";
}

void PropertyTagStage::run(const HeapSnapshot& heap) {
    generate_property_tag_objs(heap);
    log::logok("  Analysis completed. Found ", property_tag_objs_.size(),
               " suspicious object types.");
}

json PropertyTagStage::analyze(
        const HeapSnapshot& heap,
        const std::function<bool(uint32_t)>& include_node) {
    property_tag_objs_.clear();
    generate_property_tag_objs(heap, include_node);
    return to_json();
}

json PropertyTagStage::to_json() const {
    nlohmann::ordered_json output = nlohmann::ordered_json::object();

    for (const auto& [name, obj_info] : property_tag_objs_) {
        nlohmann::ordered_json obj_json;
        obj_json["totalRetainedSize"] = obj_info.total_retained_size;
        obj_json["totalNodeCount"] = obj_info.total_node_count;
        // 为空时也始终输出，保持输出结构稳定
        obj_json["propertyName"] = obj_info.property_name;
        obj_json["displayType"] = obj_info.concern_display_type.empty()
                                      ? default_display_type_
                                      : obj_info.concern_display_type;
        obj_json["hashValues"] = obj_info.hash_values;

        obj_json["referenceChains"] = ref_chains_to_json(obj_info.reference_chains);
        output[name] = std::move(obj_json);
    }

    return output;
}

std::string PropertyTagStage::check_destoryed_tag(uint32_t u_ni, const HeapSnapshot& heap) {
    std::string found_edge_name;

    heap.for_each_edge(u_ni, [&](const HeapSnapshot::EdgeInfo& edge, size_t) {
        std::string edge_name;
        if (edge.name_or_index < static_cast<uint32_t>(heap.strings.size())) {
            edge_name = heap.strings[edge.name_or_index];
        }
        if (property_tag_base_classes_.find(edge_name) != property_tag_base_classes_.end()) {
            std::string target_name = heap.get_node_name(edge.to_node);
            if (target_name.find("true") != std::string::npos) {
                found_edge_name = edge_name;
            }
        }
    });

    return found_edge_name;
}

BaseClassInfo PropertyTagStage::find_concern_type(uint32_t u_ni, const std::vector<BaseClassInfo>& base_classes, const HeapSnapshot& heap) {
    if (base_classes.empty()) return {};

    std::unordered_set<uint32_t> visited;
    visited.reserve(16);
    uint32_t cur = u_ni;
    bool looking_for_hclass = true;

    while (cur < static_cast<uint32_t>(heap.node_count) && visited.insert(cur).second) {
        // 当前节点名做包含匹配，命中即返回（沿继承链从近到远，首个命中优先）
        std::string name = heap.get_node_name(cur);
        for (const auto& base : base_classes) {
            if (!base.name.empty() && name.find(base.name) != std::string::npos) {
                return base;
            }
        }

        // 沿原型链推进：node → hclass → Proto/__proto__ → 父原型 → hclass → ...
        bool advanced = false;
        heap.for_each_edge(cur, [&](const HeapSnapshot::EdgeInfo& edge, size_t) {
            if (advanced) return;
            if (edge.name_or_index >= static_cast<uint32_t>(heap.strings.size())) return;
            const std::string& edge_name = heap.strings[edge.name_or_index];
            if (looking_for_hclass) {
                if (edge_name == "hclass") {
                    cur = edge.to_node;
                    advanced = true;
                    looking_for_hclass = false;
                }
            } else {
                if (edge_name == "Proto" || edge_name == "__proto__") {
                    cur = edge.to_node;
                    advanced = true;
                    looking_for_hclass = true;
                }
            }
        });
        if (!advanced) break;
    }
    return {};
}

void PropertyTagStage::assemble_property_tag_obj(uint32_t u_ni, const HeapSnapshot& heap,
        const std::string& property_name, const BaseClassInfo& base_class, uint32_t hash_value) {
    std::string ref_chain = heap.format_ref_chain(u_ni);
    uint64_t retained_size = static_cast<uint64_t>(heap.get_retained_size(u_ni));

    auto& obj_info = property_tag_objs_[heap.get_extended_node_name(u_ni)];
    obj_info.total_retained_size += retained_size;
    obj_info.total_node_count++;
    if (!property_name.empty() && obj_info.property_name.empty()) {
        obj_info.property_name = property_name;
    }
    // concern_type 记录技术名；display_type 记录展示类型（未配置则回退 name；未命中基类则用 default_display_type_）
    if (!base_class.name.empty() && obj_info.concern_type.empty()) {
        obj_info.concern_type = base_class.name;
        obj_info.concern_display_type = base_class.display_type.empty() ? base_class.name : base_class.display_type;
    } else if (obj_info.concern_display_type.empty()) {
        obj_info.concern_display_type = default_display_type_;
    }
    if (hash_value != 0) {
        obj_info.hash_values.push_back(hash_value);
    }
    auto& rc = obj_info.reference_chains[ref_chain];
    if (rc.frames.empty()) {
        rc.frames = heap.format_ref_chain_frames(u_ni);
    }
    rc.node_ids.push_back(heap.get_node_id(u_ni));
    rc.total_retained_size += retained_size;
}

void PropertyTagStage::generate_property_tag_objs(
        const HeapSnapshot& heap,
        const std::function<bool(uint32_t)>& include_node) {
    for (int ni = 0; ni < heap.node_count; ni++) {
        uint32_t u_ni = static_cast<uint32_t>(ni);
        if (include_node && !include_node(u_ni)) continue;

        // 1) 先过滤属性名：命中配置的属性标签才继续（避免对每个节点都查继承链）
        std::string property_name = check_destoryed_tag(u_ni, heap);

        // 2) 命中属性标签后，再沿继承链查首个命中的目标组件基类（关注类型）
        BaseClassInfo base_class;
        if (!property_name.empty()) {
            auto it = property_tag_base_classes_.find(property_name);
            if (it != property_tag_base_classes_.end() && !it->second.empty()) {
                base_class = find_concern_type(u_ni, it->second, heap);
            }
        }

        // 3) Check if hash value matches.
        uint32_t hash_value = 0;
        if (!jsleak_hash_set_.empty()) {
            hash_value = heap.get_node_internal_hash(u_ni);
            if (hash_value != 0 && jsleak_hash_set_.find(hash_value) == jsleak_hash_set_.end()) {
                hash_value = 0;
            }
        }

        if (property_name.empty() && hash_value == 0) {
            continue;
        }

        assemble_property_tag_obj(u_ni, heap, property_name, base_class, hash_value);
    }
}

void PropertyTagStage::load_jsleak_list_file(const std::string& file_path) {
    // open_ifstream 打不开时会抛 std::system_error，不会返回未打开的流。
    // 必须在这里捕获：-l 是可选参数，路径不可读不应终止整轮分析
    // （此前 Windows 上该异常会一路逃到 JsleakModule::run，直接丢整份报告）。
    std::ifstream file;
    try {
        file = open_ifstream(file_path);
    } catch (const std::exception&) {
        log::logw("Warning: Cannot open jsleaklist file: ", file_path);
        return;
    }
    if (!file.is_open()) {
        log::logw("Warning: Cannot open jsleaklist file: ", file_path);
        return;
    }
    try {
        json j = json::parse(file);
        if (j.contains("leakObjList") && j["leakObjList"].is_array()) {
            for (const auto& obj : j["leakObjList"]) {
                if (obj.contains("hash")) {
                    jsleak_hash_set_.insert(obj["hash"].get<uint32_t>());
                }
            }
        }
    } catch (const std::exception& e) {
        log::logw("Warning: Failed to parse jsleaklist file: ", e.what());
    }
}

void PropertyTagStage::load_property_tag_config() {
    // 内置默认：配置文件缺失或解析失败时兜底
    static const std::unordered_map<std::string, std::vector<BaseClassInfo>> kDefaults = {
        {"isDeleting_", {{"ViewBuildNodeBase", "Component"}}},
        {"_isDisposed", {{"BuilderNode", "BuilderNode"}, {"RenderNode", "RenderNode"}, {"FrameNode", "FrameNode"}}},
        {"isDestroyed", {{"Ability", "Ability"}, {"Window", "Window"}}},
    };
    constexpr const char* kDefaultDisplayType = "Others";

    const std::string rel = "config/jsleak/jsleak_config.json";

    // 固定目录：可执行文件同级的 config/jsleak/jsleak_config.json
    std::string config_path;
    const std::string exe_dir = platform::get_executable_dir();
    if (!exe_dir.empty()) {
        config_path = platform::join_path(exe_dir, rel);
    }

    if (config_path.empty() || !platform::file_exists(config_path)) {
        property_tag_base_classes_ = kDefaults;
        default_display_type_ = kDefaultDisplayType;
        return;
    }

    try {
        json j;
        {
            auto ifs = open_ifstream(config_path);
            ifs >> j;
        }

        // 外层域 propertyTag：表明以下均为属性标签阶段参数
        const json* root = nullptr;
        if (j.contains("propertyTag") && j["propertyTag"].is_object()) {
            root = &j["propertyTag"];
        }

        if (root && root->contains("tags") && (*root)["tags"].is_array()) {
            std::unordered_map<std::string, std::vector<BaseClassInfo>> parsed;
            for (const auto& item : (*root)["tags"]) {
                if (!item.is_object()) continue;
                if (!item.contains("propertyName") || !item["propertyName"].is_string()) continue;
                const std::string tag = item["propertyName"].get<std::string>();
                if (tag.empty()) continue;

                std::vector<BaseClassInfo> bases;
                if (item.contains("baseClasses") && item["baseClasses"].is_array()) {
                    for (const auto& b : item["baseClasses"]) {
                        BaseClassInfo info;
                        if (b.is_string()) {
                            info.name = b.get<std::string>();
                        } else if (b.is_object() && b.contains("name") && b["name"].is_string()) {
                            info.name = b["name"].get<std::string>();
                            if (b.contains("displayType") && b["displayType"].is_string()) {
                                info.display_type = b["displayType"].get<std::string>();
                            }
                        }
                        if (!info.name.empty()) {
                            bases.push_back(std::move(info));
                        }
                    }
                }
                parsed[tag] = std::move(bases);
            }
            property_tag_base_classes_ = parsed.empty() ? kDefaults : std::move(parsed);
        } else {
            property_tag_base_classes_ = kDefaults;
        }

        // 默认展示类型（可缺省）
        if (root && root->contains("defaultDisplayType") && (*root)["defaultDisplayType"].is_string()) {
            default_display_type_ = (*root)["defaultDisplayType"].get<std::string>();
        } else {
            default_display_type_ = kDefaultDisplayType;
        }
    } catch (const std::exception& e) {
        log::logw("Warning: Failed to parse property tag config: ", e.what());
        property_tag_base_classes_ = kDefaults;
        default_display_type_ = kDefaultDisplayType;
    }
}

} // namespace mem_analyzer
