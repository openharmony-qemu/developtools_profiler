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

std::vector<std::shared_ptr<AnalysisModule>>& modules()
{
    static std::vector<std::shared_ptr<AnalysisModule>> sModules;
    return sModules;
}

} // namespace

void ModuleRegistry::Add(std::shared_ptr<AnalysisModule> module)
{
    if (!module) {
        return;
    }
    modules().push_back(std::move(module));
}

AnalysisModule* ModuleRegistry::find(const std::string& name)
{
    for (auto& m : modules()) {
        if (m->Name() == name) {
            return m.get();
        }
    }
    return nullptr;
}

AnalysisModule* ModuleRegistry::FindByExtension(const std::string& ext)
{
    std::string extLower = ext;
    for (auto& c : extLower) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    if (!extLower.empty() && extLower.front() == '.') {
        extLower = extLower.substr(1);
    }

    for (auto& m : modules()) {
        if (m->SupportsExtension(extLower)) {
            return m.get();
        }
    }
    return nullptr;
}

AnalysisModule* ModuleRegistry::Detect(const std::string& filePath)
{
    auto pos = filePath.find_last_of('.');
    if (pos == std::string::npos) {
        return nullptr;
    }
    return FindByExtension(filePath.substr(pos + 1));
}

std::vector<AnalysisModule*> ModuleRegistry::All()
{
    std::vector<AnalysisModule*> result;
    for (auto& m : modules()) {
        result.push_back(m.get());
    }
    return result;
}

std::vector<std::string> ModuleRegistry::SelfCheck()
{
    std::vector<std::string> issues;

    for (auto* module : all()) {
        const ModuleCapability cap = module->Capability();
        const std::string name = module->Name();

        auto checkMode = [&](const char* mode, const ModeCapability& mc) {
            if (!mc.Supported()) {
                // 明确不支持该模式：此时默认格式是死数据，留着会误导下一个改的人
                if (!mc.defaultFormat.empty()) {
                    issues.push_back(name + ": " + mode + " declares no formats but has default '" +
                                     mc.defaultFormat + "'");
                }
                return;
            }
            // 声明了格式列表时，默认格式必须非空且是其一
            if (mc.defaultFormat.empty()) {
                issues.push_back(name + ": missing default " + mode + " format");
            } else if (std::find(mc.formats.begin(), mc.formats.end(), mc.defaultFormat) ==
                       mc.formats.end())
                       {
                issues.push_back(name + ": default " + mode + " format '" + mc.defaultFormat +
                                 "' is not in the declared format list");
            }
        };

        checkMode("parse", cap.parse);
        checkMode("analysis", cap.analysis);
    }
    return issues;
}

namespace {
// 数据域注册。新增一个域 = 建目录 + 在这里加一行 + CMakeLists 登记源文件。
const bool REGISTERED = []() {
    ModuleRegistry::Add(std::make_shared<HtraceModule>());
    ModuleRegistry::Add(std::make_shared<TshandleModule>());
    ModuleRegistry::Add(std::make_shared<JsleakModule>());
    return true;
}();
} // namespace

} // namespace mem_analyzer
