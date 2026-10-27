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
#include "core/module_registry.hpp"

#include <algorithm>
#include <cctype>
#include <memory>

#include "htrace/htrace_module.hpp"
#include "jsleak/jsleak_module.hpp"
#include "tshandle/tshandle_module.hpp"

namespace mem_analyzer {
namespace {

std::vector<std::shared_ptr<AnalysisModule>>& modules() {
    static std::vector<std::shared_ptr<AnalysisModule>> s_modules;
    return s_modules;
}

} // namespace

void ModuleRegistry::add(std::shared_ptr<AnalysisModule> module) {
    if (!module) return;
    modules().push_back(std::move(module));
}

AnalysisModule* ModuleRegistry::find(const std::string& name) {
    for (auto& m : modules()) {
        if (m->name() == name) return m.get();
    }
    return nullptr;
}

AnalysisModule* ModuleRegistry::find_by_extension(const std::string& ext) {
    std::string ext_lower = ext;
    for (auto& c : ext_lower) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    if (!ext_lower.empty() && ext_lower.front() == '.') ext_lower = ext_lower.substr(1);

    for (auto& m : modules()) {
        if (m->supports_extension(ext_lower)) return m.get();
    }
    return nullptr;
}

AnalysisModule* ModuleRegistry::detect(const std::string& file_path) {
    auto pos = file_path.find_last_of('.');
    if (pos == std::string::npos) return nullptr;
    return find_by_extension(file_path.substr(pos + 1));
}

std::vector<AnalysisModule*> ModuleRegistry::all() {
    std::vector<AnalysisModule*> result;
    for (auto& m : modules()) result.push_back(m.get());
    return result;
}

std::vector<std::string> ModuleRegistry::self_check() {
    std::vector<std::string> issues;

    for (auto* module : all()) {
        const ModuleCapability cap = module->capability();
        const std::string name = module->name();

        auto check_mode = [&](const char* mode, const ModeCapability& mc) {
            if (!mc.supported()) {
                // 明确不支持该模式：此时默认格式是死数据，留着会误导下一个改的人
                if (!mc.default_format.empty()) {
                    issues.push_back(name + ": " + mode + " declares no formats but has default '" +
                                     mc.default_format + "'");
                }
                return;
            }
            // 声明了格式列表时，默认格式必须非空且是其一
            if (mc.default_format.empty()) {
                issues.push_back(name + ": missing default " + mode + " format");
            } else if (std::find(mc.formats.begin(), mc.formats.end(), mc.default_format) ==
                       mc.formats.end()) {
                issues.push_back(name + ": default " + mode + " format '" + mc.default_format +
                                 "' is not in the declared format list");
            }
        };

        check_mode("parse", cap.parse);
        check_mode("analysis", cap.analysis);
    }
    return issues;
}

namespace {
// 数据域注册。新增一个域 = 建目录 + 在这里加一行 + CMakeLists 登记源文件。
const bool registered = []() {
    ModuleRegistry::add(std::make_shared<HtraceModule>());
    ModuleRegistry::add(std::make_shared<TshandleModule>());
    ModuleRegistry::add(std::make_shared<JsleakModule>());
    return true;
}();
} // namespace

} // namespace mem_analyzer
