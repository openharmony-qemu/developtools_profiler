#include "cli/command.hpp"
#include "profiler/profiler_cmd.h"

namespace mem_analyzer {
namespace {

struct ProfilerOptions {
    std::string bundle;
    int duration = defaults::PROF_DURATION;
    bool duration_specified = false;  // -t 是否被用户显式指定
    std::string category = defaults::HTRACE_TYPE;
    bool list_apps = false;
    std::string so_dir;
    std::string sourcemap_dir;
    std::string name_cache_dir;
    std::string connect_key;           // --device value: device serial number
    bool connect_key_provided = false; // whether --device was given explicitly
};

class ProfilerCommand final : public ICommand {
    ProfilerOptions opt_;

public:
    std::string name() const override { return "profiler"; }
    std::string alias() const override { return "prof"; }
    std::string description() const override {
        return "Collect snapshot, htrace, Global/Local handle log";
    }
    int order() const override { return 40; }

    void bind(CliParser& sub, GlobalOptions& g) override {
        sub.set_usage("[options] [<bundle>]")
           .add_positional("bundle", opt_.bundle,
                           "Target bundle name (required for capture, not needed with "
                           "--list-apps)")
           .add_option("-c,--category", opt_.category,
                       "Capture type: nativeheap/nh/native, snapshot/snap, globalhandle/global/gh, "
                       "allhandle/handle/ah, localhandle/local/lh, nativeleaks/nl, fd, dma, thread, ashmem")
           .add_option("-t", opt_.duration, "Capture duration in seconds", &opt_.duration_specified)
           .add_option("--device", opt_.connect_key,
                       "Target device: device serial number (see 'hdc list targets'); optional "
                       "with 1 device, but required when 2 or more devices are connected or the "
                       "target cannot be determined",
                       &opt_.connect_key_provided)
           .add_option("-d,--out-dir", g.out_dir, "Output directory")
           .add_option("-S,--so-dir", opt_.so_dir,
                       "Directory of native .so symbol files for offline symbolization (ignored "
                       "for snapshot capture)")
           .add_option("--sourcemap", opt_.sourcemap_dir,
                       "Source map file or directory (sourceMaps.map / *.map) to deobfuscate "
                       "heapsnapshot paths")
           .add_option("--namecache", opt_.name_cache_dir,
                       "nameCache file or directory (nameCache.json) to deobfuscate heapsnapshot "
                       "names")
           .add_switch("-o,--auto-open", g.auto_open,
                       "Generate HTML analysis report and open it in browser")
           .add_switch("--list-apps", opt_.list_apps,
                       "List installed apps on device and exit");
        bind_debug_options(sub, g);
    }

    int run(const GlobalOptions& g) override {
        ProfilerCliOptions prof_opt;
        prof_opt.process_name = opt_.bundle;
        prof_opt.duration = opt_.duration;
        prof_opt.duration_specified = opt_.duration_specified;
        prof_opt.output_dir = g.out_dir;
        prof_opt.category = opt_.category;
        prof_opt.flag_report = g.auto_open;
        prof_opt.flag_list_apps = opt_.list_apps;
        prof_opt.sourcemap_dir = opt_.sourcemap_dir;
        prof_opt.name_obfuscation = opt_.name_cache_dir;
        prof_opt.so_dir = opt_.so_dir;
        // 仅在显式传了 --device 时才下发 key：区分“未传”（由门禁按设备数自动判定）
        // 与“显式传空串”（--device ""，属参数错误，须报错）。
        if (opt_.connect_key_provided) {
            prof_opt.connect_key_provided = true;
            prof_opt.connect_key = opt_.connect_key;
        }
        return run_profiler_cmd(prof_opt);
    }
};

const bool registered = []() {
    CommandRegistry::add(std::make_shared<ProfilerCommand>());
    return true;
}();

} // namespace
} // namespace mem_analyzer
