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

bool is_registered(const std::string& feature_id) {
    for (const auto& f : PreviewFeaturesMgr::registry()) {
        if (f.name == feature_id) return true;
    }
    return false;
}
} // namespace

// ===== Singleton =====

PreviewFeaturesMgr& PreviewFeaturesMgr::instance() {
    static PreviewFeaturesMgr inst;
    return inst;
}

// ===== 特性注册表 =====

const std::vector<PreviewFeatureInfo>& PreviewFeaturesMgr::registry() {
    // 受预览特性开关逐项管控的功能清单，新增预览特性时在此登记
    static const std::vector<PreviewFeatureInfo> kFeatures = {
        {"napi_wrap", "Analyze shortest reference chains of JSWrappedNapiObject nodes in jsleak snapshot reports"},
    };
    return kFeatures;
}

// ===== 配置文件定位 =====

std::string PreviewFeaturesMgr::resolve_config_path() {
    // 固定目录：可执行文件同级的 config/preview_features.json
    // （该文件需要可写，save() 会在目录缺失时按需创建）
    const std::string rel = platform::join_path(kConfigDir, kConfigFileName);
    const std::string exe_dir = platform::get_executable_dir();
    if (exe_dir.empty()) return rel;
    return platform::join_path(exe_dir, rel);
}

// ===== 加载 / 保存 =====

bool PreviewFeaturesMgr::load(std::string& err) {
    config_path_ = resolve_config_path();
    enabled_features_.clear();  // 默认全部关闭

    if (!platform::file_exists(config_path_)) {
        return true;  // 无配置文件：保持默认（全部关闭）
    }

    try {
        auto ifs = open_ifstream(config_path_);
        json j;
        ifs >> j;
        if (!j.contains("enabled")) return true;

        const auto& enabled = j["enabled"];
        if (enabled.is_boolean()) {
            // 兼容旧格式：布尔值表示「全部预览特性」的总开关
            if (enabled.get<bool>()) {
                for (const auto& f : registry()) enabled_features_.insert(f.name);
            }
        } else if (enabled.is_array()) {
            // 仅保留 registry 中登记过的 id，避免配置残留导致误判
            for (const auto& item : enabled) {
                if (!item.is_string()) continue;
                const std::string id = item.get<std::string>();
                if (is_registered(id)) enabled_features_.insert(id);
            }
        }
    } catch (const std::exception& e) {
        err = "failed to read " + config_path_ + ": " + e.what();
        return false;
    }
    return true;
}

bool PreviewFeaturesMgr::save(std::string& err) const {
    // 按 registry 顺序输出已使能的特性 id，保证文件内容稳定
    json enabled = json::array();
    for (const auto& f : registry()) {
        if (enabled_features_.count(f.name)) enabled.push_back(f.name);
    }
    json j;
    j["enabled"] = std::move(enabled);

    const std::string dir = platform::dirname(config_path_);
    if (!dir.empty() && !platform::directory_exists(dir)) {
        std::error_code ec;
        fs::create_directories(path_from_utf8(dir), ec);
        if (ec) {
            err = "failed to create directory " + dir + ": " + ec.message();
            return false;
        }
    }

    try {
        auto ofs = open_ofstream(config_path_);
        ofs << j.dump(2) << std::endl;
        if (!ofs) {
            err = "failed to write " + config_path_;
            return false;
        }
    } catch (const std::exception& e) {
        err = "failed to write " + config_path_ + ": " + e.what();
        return false;
    }
    return true;
}

// ===== 查询 / 修改 =====

bool PreviewFeaturesMgr::enabled(const std::string& feature_id) const {
    if (!is_registered(feature_id)) return false;
    return enabled_features_.count(feature_id) > 0;
}

bool PreviewFeaturesMgr::set_enabled(bool enabled, std::string& err) {
    if (config_path_.empty()) {
        config_path_ = resolve_config_path();
    }
    enabled_features_.clear();
    if (enabled) {
        for (const auto& f : registry()) enabled_features_.insert(f.name);
    }
    return save(err);
}

} // namespace mem_analyzer
