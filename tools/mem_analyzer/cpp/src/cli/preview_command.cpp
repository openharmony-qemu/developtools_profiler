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
#include "cli/command.h"
#include "config_mgr/preview_features_mgr.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"

#include <iostream>

namespace mem_analyzer {
namespace {

/**
 * 隐藏子命令：预览特性开关（内部使用，不出现在 --help/usage 中）
 */
class PreviewCommand final : public ICommand {
    std::string action_;

public:
    std::string Name() const override { return "preview"; }
    std::string Description() const override
    {
        return "Enable/disable preview features (internal)";
    }
    bool Hidden() const override { return true; }
    int Order() const override { return 90; }

    void Bind(CliParser& sub, GlobalOptions&) override
    {
        sub.AddPositional("action", action_, "enable | disable | status (default: status)");
    }

    int Run(const GlobalOptions&) override
    {
        auto& mgr = PreviewFeaturesMgr::Instance();

        if (action_.empty() || action_ == "status") {
            std::cout << "Preview features:" << std::endl;
            for (const auto& f : PreviewFeaturesMgr::Registry()) {
                std::cout << "  " << f.name << ": " << (mgr.Enabled(f.name) ? "enabled" : "disabled") <<
                          " - " << f.description << std::endl;
            }
            return 0;
        }
        if (action_ != "enable" && action_ != "disable") {
            log::Loge("Error: preview expects 'enable', 'disable' or 'status'");
            return 1;
        }

        const bool enable = (action_ == "enable");
        std::string err;
        if (!mgr.SetEnabled(enable, err)) {
            log::Loge("Error: ", err);
            return 1;
        }
        std::cout << "Preview features " << (enable ? "enabled" : "disabled") << "." << std::endl;
        return 0;
    }
};

const bool REGISTERED = []() {
    CommandRegistry::Add(std::make_shared<PreviewCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
