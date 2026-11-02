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
#include "jsleak/property_tag_stage.h"
#include "utils/file_utils.h"
#include "utils/platform.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

namespace mem_analyzer {

namespace {
constexpr int VISITED_SET_INITIAL_CAPACITY = 16;

void ParseLeakObjList(const json& j, std::unordered_set<uint32_t>& hashSet)
{
    if (!j.contains("leakObjList") || !j["leakObjList"].is_array()) {
        return;
    }
    for (const auto& obj : j["leakObjList"]) {
        if (obj.contains("hash")) {
            hashSet.insert(obj["hash"].get<uint32_t>());
        }
    }
}
} // namespace

PropertyTagStage::PropertyTagStage(const std::string& jsleakListFile)
{
    LoadPropertyTagConfig();
    if (!jsleakListFile.empty()) {
        LoadJsleakListFile(jsleakListFile);
    }
}

const std::string& PropertyTagStage::Mode() const
{
    static const std::string MODE = "property-tag";
    return MODE;
}

const char* PropertyTagStage::PreviewId() const
{
    return nullptr;
}

const char* PropertyTagStage::JsonKey() const
{
    return "propertyTagObjects";
}

void PropertyTagStage::Run(const HeapSnapshot& heap)
{
    GeneratePropertyTagObjs(heap);
    log::Logok("  Analysis completed. Found ", propertyTagObjs.size(),
        " suspicious object types.");
}

json PropertyTagStage::Analyze(const HeapSnapshot& heap, const std::function<bool(uint32_t)>& includeNode)
        {
    propertyTagObjs.clear();
    GeneratePropertyTagObjs(heap, includeNode);
    return ToJson();
}

json PropertyTagStage::ToJson() const
{
    nlohmann::ordered_json output = nlohmann::ordered_json::object();

    for (const auto& [name, objInfo] : propertyTagObjs) {
        nlohmann::ordered_json objJson;
        objJson["totalRetainedSize"] = objInfo.totalRetainedSize;
        objJson["totalNodeCount"] = objInfo.totalNodeCount;
        // 为空时也始终输出，保持输出结构稳定
        objJson["propertyName"] = objInfo.propertyName;
        objJson["displayType"] = objInfo.concernDisplayType.empty() ?
                                      defaultDisplayType :
                                      objInfo.concernDisplayType;
        objJson["hashValues"] = objInfo.hashValues;

        objJson["referenceChains"] = RefChainsToJson(objInfo.referenceChains);
        output[name] = std::move(objJson);
    }

    return output;
}

std::string PropertyTagStage::CheckDestoryedTag(uint32_t uNi, const HeapSnapshot& heap)
{
    std::string foundEdgeName;

    heap.ForEachEdge(uNi, [this, &foundEdgeName, &heap](const HeapSnapshot::EdgeInfo& edge, size_t) {
        std::string edgeName;
        if (edge.nameOrIndex < static_cast<uint32_t>(heap.strings.size())) {
            edgeName = heap.strings[edge.nameOrIndex];
        }
        if (propertyTagBaseClasses.find(edgeName) != propertyTagBaseClasses.end()) {
            std::string targetName = heap.GetNodeName(edge.toNode);
            if (targetName.find("true") != std::string::npos) {
                foundEdgeName = edgeName;
            }
        }
    });

    return foundEdgeName;
}

BaseClassInfo PropertyTagStage::FindConcernType(uint32_t uNi,
    const std::vector<BaseClassInfo>& baseClasses, const HeapSnapshot& heap)
{
    if (baseClasses.empty()) {
        return {};
    }

    std::unordered_set<uint32_t> visited;
    visited.reserve(VISITED_SET_INITIAL_CAPACITY);
    uint32_t cur = uNi;
    bool lookingForHclass = true;

    while (cur < static_cast<uint32_t>(heap.nodeCount) && visited.insert(cur).second) {
        // 当前节点名做包含匹配，命中即返回（沿继承链从近到远，首个命中优先）
        std::string name = heap.GetNodeName(cur);
        for (const auto& base : baseClasses) {
            if (!base.name.empty() && name.find(base.name) != std::string::npos) {
                return base;
            }
        }

        // 沿原型链推进：node → hclass → Proto/__proto__ → 父原型 → hclass → ...
        bool advanced = false;
        heap.ForEachEdge(cur, [&advanced, &cur, &heap, &lookingForHclass](const HeapSnapshot::EdgeInfo& edge, size_t) {
            if (advanced) {
                return;
            }
            if (edge.nameOrIndex >= static_cast<uint32_t>(heap.strings.size())) {
                return;
            }
            const std::string& edgeName = heap.strings[edge.nameOrIndex];
            if (lookingForHclass) {
                if (edgeName == "hclass") {
                    cur = edge.toNode;
                    advanced = true;
                    lookingForHclass = false;
                }
            } else {
                if (edgeName == "Proto" || edgeName == "__proto__") {
                    cur = edge.toNode;
                    advanced = true;
                    lookingForHclass = true;
                }
            }
        });
        if (!advanced) {
            break;
        }
    }
    return {};
}

void PropertyTagStage::AssemblePropertyTagObj(uint32_t uNi, const HeapSnapshot& heap,
    const std::string& propertyName, const BaseClassInfo& baseClass, uint32_t hashValue)
{
    std::string refChain = heap.FormatRefChain(uNi);
    uint64_t retainedSize = static_cast<uint64_t>(heap.GetRetainedSize(uNi));

    auto& objInfo = propertyTagObjs[heap.GetExtendedNodeName(uNi)];
    objInfo.totalRetainedSize += retainedSize;
    objInfo.totalNodeCount++;
    if (!propertyName.empty() && objInfo.propertyName.empty()) {
        objInfo.propertyName = propertyName;
    }
    // concern_type 记录技术名；display_type 记录展示类型（未配置则回退 name；未命中基类则用 default_display_type_）
    if (!baseClass.name.empty() && objInfo.concernType.empty()) {
        objInfo.concernType = baseClass.name;
        objInfo.concernDisplayType = baseClass.displayType.empty() ? baseClass.name : baseClass.displayType;
    } else if (objInfo.concernDisplayType.empty()) {
        objInfo.concernDisplayType = defaultDisplayType;
    }
    if (hashValue != 0) {
        objInfo.hashValues.push_back(hashValue);
    }
    auto& rc = objInfo.referenceChains[refChain];
    if (rc.frames.empty()) {
        rc.frames = heap.FormatRefChainFrames(uNi);
    }
    rc.nodeIds.push_back(heap.GetNodeId(uNi));
    rc.totalRetainedSize += retainedSize;
}

void PropertyTagStage::GeneratePropertyTagObjs(
    const HeapSnapshot& heap,
    const std::function<bool(uint32_t)>& includeNode)
        {
    for (int ni = 0; ni < heap.nodeCount; ni++) {
        uint32_t uNi = static_cast<uint32_t>(ni);
        if (includeNode && !includeNode(uNi)) {
            continue;
        }

        // 1) 先过滤属性名：命中配置的属性标签才继续（避免对每个节点都查继承链）
        std::string propertyName = CheckDestoryedTag(uNi, heap);

        // 2) 命中属性标签后，再沿继承链查首个命中的目标组件基类（关注类型）
        BaseClassInfo baseClass;
        if (!propertyName.empty()) {
            auto it = propertyTagBaseClasses.find(propertyName);
            if (it != propertyTagBaseClasses.end() && !it->second.empty()) {
                baseClass = FindConcernType(uNi, it->second, heap);
            }
        }

        // 3) Check if hash value matches.
        uint32_t hashValue = 0;
        if (!jsleakHashSet.empty()) {
            hashValue = heap.GetNodeInternalHash(uNi);
            if (hashValue != 0 && jsleakHashSet.find(hashValue) == jsleakHashSet.end()) {
                hashValue = 0;
            }
        }

        if (propertyName.empty() && hashValue == 0) {
            continue;
        }

        AssemblePropertyTagObj(uNi, heap, propertyName, baseClass, hashValue);
    }
}

void PropertyTagStage::LoadJsleakListFile(const std::string& filePath)
{
    // open_ifstream 打不开时会抛 std::system_error，不会返回未打开的流。
    // 必须在这里捕获：-l 是可选参数，路径不可读不应终止整轮分析
    // （此前 Windows 上该异常会一路逃到 JsleakModule::Run，直接丢整份报告）。
    std::ifstream file;
    try {
        file = OpenIfstream(filePath);
    } catch (const std::exception&) {
        log::Logw("Warning: Cannot open jsleaklist file: ", filePath);
        return;
    }
    if (!file.is_open()) {
        log::Logw("Warning: Cannot open jsleaklist file: ", filePath);
        return;
    }
    try {
        json j = json::parse(file);
        ParseLeakObjList(j, jsleakHashSet);
    } catch (const std::exception& e) {
        log::Logw("Warning: Failed to parse jsleaklist file: ", e.what());
    }
}

/**
 * Parse base class info from a JSON item (string or object form).
 */
static std::vector<BaseClassInfo> ParseBaseClassList(const json& item)
{
    std::vector<BaseClassInfo> bases;
    if (!item.contains("baseClasses") || !item["baseClasses"].is_array()) {
        return bases;
    }
    for (const auto& b : item["baseClasses"]) {
        BaseClassInfo info;
        if (b.is_string()) {
            info.name = b.get<std::string>();
        } else if (b.is_object() && b.contains("name") && b["name"].is_string()) {
            info.name = b["name"].get<std::string>();
            if (b.contains("displayType") && b["displayType"].is_string()) {
                info.displayType = b["displayType"].get<std::string>();
            }
        }
        if (!info.name.empty()) {
            bases.push_back(std::move(info));
        }
    }
    return bases;
}

/**
 * Parse the tags array from a propertyTag JSON root.
 */
static std::unordered_map<std::string, std::vector<BaseClassInfo>> ParsePropertyTagEntries(const json& root)
{
    std::unordered_map<std::string, std::vector<BaseClassInfo>> parsed;
    for (const auto& item : root["tags"]) {
        if (!item.is_object()) {
            continue;
        }
        if (!item.contains("propertyName") || !item["propertyName"].is_string()) {
            continue;
        }
        const std::string tag = item["propertyName"].get<std::string>();
        if (tag.empty()) {
            continue;
        }
        parsed[tag] = ParseBaseClassList(item);
    }
    return parsed;
}

void PropertyTagStage::LoadPropertyTagConfig()
{
    // 内置默认：配置文件缺失或解析失败时兜底
    static const std::unordered_map<std::string, std::vector<BaseClassInfo>> DEFAULTS = {
        {"isDeleting_", {{"ViewBuildNodeBase", "Component"}}},
        {"_isDisposed", {{"BuilderNode", "BuilderNode"}, {"RenderNode", "RenderNode"}, {"FrameNode", "FrameNode"}}},
        {"isDestroyed", {{"Ability", "Ability"}, {"Window", "Window"}}},
    };
    constexpr const char* DEFAULT_DISPLAY_TYPE = "Others";

    const std::string rel = "config/jsleak/jsleak_config.json";

    // 固定目录：可执行文件同级的 config/jsleak/jsleak_config.json
    std::string configPath;
    const std::string exeDir = platform::GetExecutableDir();
    if (!exeDir.empty()) {
        configPath = platform::JoinPath(exeDir, rel);
    }

    if (configPath.empty() || !platform::FileExists(configPath)) {
        propertyTagBaseClasses = DEFAULTS;
        defaultDisplayType = DEFAULT_DISPLAY_TYPE;
        return;
    }

    try {
        json j;
        {
            auto ifs = OpenIfstream(configPath);
            ifs >> j;
        }

        // 外层域 propertyTag：表明以下均为属性标签阶段参数
        const json* root = nullptr;
        if (j.contains("propertyTag") && j["propertyTag"].is_object()) {
            root = &j["propertyTag"];
        }

        if (root && root->contains("tags") && (*root)["tags"].is_array()) {
            auto parsed = ParsePropertyTagEntries(*root);
            propertyTagBaseClasses = parsed.empty() ? DEFAULTS : std::move(parsed);
        } else {
            propertyTagBaseClasses = DEFAULTS;
        }

        // 默认展示类型（可缺省）
        if (root && root->contains("defaultDisplayType") && (*root)["defaultDisplayType"].is_string()) {
            defaultDisplayType = (*root)["defaultDisplayType"].get<std::string>();
        } else {
            defaultDisplayType = DEFAULT_DISPLAY_TYPE;
        }
    } catch (const std::exception& e) {
        log::Logw("Warning: Failed to parse property tag config: ", e.what());
        propertyTagBaseClasses = DEFAULTS;
        defaultDisplayType = DEFAULT_DISPLAY_TYPE;
    }
}

} // namespace mem_analyzer
