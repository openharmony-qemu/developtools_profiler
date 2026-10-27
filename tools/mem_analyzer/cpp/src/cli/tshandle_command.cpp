#include "cli/command.hpp"
#include "cli/pipeline_runner.hpp"
#include "config.hpp"
#include "core/module_registry.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"

namespace mem_analyzer {
namespace {

struct TshandleOptions {
    int top_n = defaults::TSHANDLE_TOP_N;
    std::string so_dir;
    std::string sourcemap_dir;
    std::string name_cache_dir;
    std::vector<std::string> files;
};

class TshandleCommand final : public ICommand {
    TshandleOptions opt_;

public:
    std::string name() const override { return "tshandle"; }
    std::string alias() const override { return "th"; }
    std::string description() const override { return "Process tshandle zip files"; }
    int order() const override { return 20; }

    void bind(CliParser& sub, GlobalOptions& g) override {
        bind_common_options(sub, g);
        sub.add_option("-n,--top-n", opt_.top_n, "Number of top symbols to output")
           .add_option("-S,--so-dir", opt_.so_dir,
                       "Directory of native .so symbol files for offline symbolization")
           .add_option("--sourcemap", opt_.sourcemap_dir,
                       "Source map file or directory (sourceMaps.map / *.map) to deobfuscate "
                       "heapsnapshot paths")
           .add_option("--namecache", opt_.name_cache_dir,
                       "nameCache file or directory (nameCache.json) to deobfuscate heapsnapshot "
                       "names")
           .add_positional("files", opt_.files, "Zip files to process");
    }

    int run(const GlobalOptions& g) override {
        log::logi(style::bold("=== Tshandle Command ==="));

        RunContext ctx = make_base_run_context(g);
        ctx.resources.so_dir = opt_.so_dir;
        // 反混淆参数交给 tshandle_parse：对解出的 rawheap 快照先还原（源码路径/行号）再分析
        ctx.resources.sourcemap_dir = opt_.sourcemap_dir;
        ctx.resources.name_cache_dir = opt_.name_cache_dir;
        // 与历史 Config::set_tshandle_top_n 一致：非正值回退默认
        ctx.tshandle.top_n = opt_.top_n > 0 ? opt_.top_n : defaults::TSHANDLE_TOP_N;

        bool all_ok = true;
        for (auto& f : opt_.files) {
            auto* module = ModuleRegistry::find_by_extension("zip");
            if (!module) {
                log::loge("Error: No parser found for zip files");
                all_ok = false;
                continue;
            }
            if (!process_file(f, *module, ctx)) all_ok = false;
        }
        log::logi(style::bold("=== Tshandle processing completed ==="));
        return all_ok ? 0 : 1;
    }
};

const bool registered = []() {
    CommandRegistry::add(std::make_shared<TshandleCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
