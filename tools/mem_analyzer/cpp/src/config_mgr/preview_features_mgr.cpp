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
#include "config_mgr/preview_features_mgr.hpp"
#include "utils/file_utils.hpp"
#include "utils/platform.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

using json = nlohmann::json;

namespace mem_analyzer {

namespace {
constexpr const char* kConfigDir = "config";
constexpr const char* kConfigFileName = "preview_features.json";

bool IsRegistered(const std::string& featureId)
{
    for (const auto& f : PreviewFeaturesMgr::registry()) {
        if (f.name == featureId) {
            return true;
        }
    }
    return false;
}
} // namespace

// ===== Singleton =====

PreviewFeaturesMgr& PreviewFeaturesMgr::instance()
{
    static PreviewFeaturesMgr inst;
    return inst;
}

// ===== 特性注册表 =====

const std::vector<PreviewFeatureInfo>& PreviewFeaturesMgr::registry()
{
    // 受预览特性开关逐项管控的功能清单，新增预览特性时在此登记
    static const std::vector<PreviewFeatureInfo> kFeatures = {
        {"napi_wrap", "Analyze shortest reference chains of JSWrappedNapiObject nodes in jsleak snapshot reports"},
    };
    return kFeatures;
}

// ===== 配置文件定位 =====

std::string PreviewFeaturesMgr::ResolveConfigPath()
{
    // 固定目录：可执行文件同级的 config/preview_features.json
    // （该文件需要可写，save() 会在目录缺失时按需创建）
    const std::string rel = platform::JoinPath(kConfigDir, kConfigFileName);
    const std::string exeDir = platform::GetExecutableDir();
    if (exeDir.empty()) {
        return rel;
    }
    return platform::JoinPath(exeDir, rel);
}

// ===== 加载 / 保存 =====

bool PreviewFeaturesMgr::load(std::string& err)
{
    configPath_ = ResolveConfigPath();
    enabledFeatures_.clear();  // 默认全部关闭

    if (!platform::FileExists(configPath_)) {
        return true;  // 无配置文件：保持默认（全部关闭）
    }

    try {
        auto ifs = OpenIfstream(configPath_);
        json j;
        ifs >> j;
        if (!j.contains("enabled")) {
            return true;
        }

        const auto& enabled = j["enabled"];
        if (enabled.is_boolean()) {
            // 兼容旧格式：布尔值表示「全部预览特性」的总开关
            if (enabled.get<bool>()) {
                for (const auto& f : registry()) {
                    enabledFeatures_.insert(f.name);
                }
            }
        } else if (enabled.is_array()) {
            // 仅保留 registry 中登记过的 id，避免配置残留导致误判
            for (const auto& item : enabled) {
                if (!item.is_string()) {
                    continue;
                }
                const std::string id = item.get<std::string>();
                if (IsRegistered(id)) {
                    enabledFeatures_.insert(id);
                }
            }
        }
    } catch (const std::exception& e) {
        err = "failed to read " + configPath_ + ": " + e.what();
        return false;
    }
    return true;
}

bool PreviewFeaturesMgr::save(std::string& err) const
{
    // 按 registry 顺序输出已使能的特性 id，保证文件内容稳定
    json enabled = json::array();
    for (const auto& f : registry()) {
        if (enabledFeatures_.count(f.name)) {
            enabled.push_back(f.name);
        }
    }
    json j;
    j["enabled"] = std::move(enabled);

    const std::string dir = platform::dirname(configPath_);
    if (!dir.empty() && !platform::DirectoryExists(dir)) {
        std::error_code ec;
        fs::create_directories(PathFromUtf8(dir), ec);
        if (ec) {
            err = "failed to create directory " + dir + ": " + ec.message();
            return false;
        }
    }

    try {
        auto ofs = OpenOfstream(configPath_);
        ofs << j.dump(2) << std::endl;
        if (!ofs) {
            err = "failed to write " + configPath_;
            return false;
        }
    } catch (const std::exception& e) {
        err = "failed to write " + configPath_ + ": " + e.what();
        return false;
    }
    return true;
}

// ===== 查询 / 修改 =====

bool PreviewFeaturesMgr::enabled(const std::string& featureId) const
{
    if (!IsRegistered(featureId)) {
        return false;
    }
    return enabledFeatures_.count(featureId) > 0;
}

bool PreviewFeaturesMgr::SetEnabled(bool enabled, std::string& err)
{
    if (configPath_.empty()) {
        configPath_ = ResolveConfigPath();
    }
    enabledFeatures_.clear();
    if (enabled) {
        for (const auto& f : registry()) {
            enabledFeatures_.insert(f.name);
        }
    }
    return save(err);
}

} // namespace mem_analyzer
