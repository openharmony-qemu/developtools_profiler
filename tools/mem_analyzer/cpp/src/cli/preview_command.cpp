#include "cli/command.hpp"
#include "config_mgr/preview_features_mgr.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"

#include <iostream>

namespace mem_analyzer {
namespace {

/// 隐藏子命令：预览特性开关（内部使用，不出现在 --help/usage 中）
class PreviewCommand final : public ICommand {
    std::string action_;

public:
    std::string name() const override { return "preview"; }
    std::string description() const override {
        return "Enable/disable preview features (internal)";
    }
    bool hidden() const override { return true; }
    int order() const override { return 90; }

    void bind(CliParser& sub, GlobalOptions&) override {
        sub.add_positional("action", action_, "enable | disable | status (default: status)");
    }

    int run(const GlobalOptions&) override {
        auto& mgr = PreviewFeaturesMgr::instance();

        if (action_.empty() || action_ == "status") {
            std::cout << "Preview features:" << std::endl;
            for (const auto& f : PreviewFeaturesMgr::registry()) {
                std::cout << "  " << f.name << ": " << (mgr.enabled(f.name) ? "enabled" : "disabled")
                          << " - " << f.description << std::endl;
            }
            return 0;
        }
        if (action_ != "enable" && action_ != "disable") {
            log::loge("Error: preview expects 'enable', 'disable' or 'status'");
            return 1;
        }

        const bool enable = (action_ == "enable");
        std::string err;
        if (!mgr.set_enabled(enable, err)) {
            log::loge("Error: ", err);
            return 1;
        }
        std::cout << "Preview features " << (enable ? "enabled" : "disabled") << "." << std::endl;
        return 0;
    }
};

const bool registered = []() {
    CommandRegistry::add(std::make_shared<PreviewCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
