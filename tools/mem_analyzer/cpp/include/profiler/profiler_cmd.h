#pragma once

#include <string>

namespace mem_analyzer {

// Options parsed by the main CliParser for the `profiler` subcommand.
// Keeping the options in the overall CLI architecture means
// `mem-analyzer profiler --help` renders like any other subcommand
// (trace/tshandle/jsleak) instead of a separate help layout.
struct ProfilerCliOptions {
    std::string process_name;        // positional <bundle>  target bundle name (required unless --list-apps)
    int duration = 120;              // -t            capture duration in seconds (default 120)
    bool duration_specified = false; // whether -t was explicitly given on the command line
    std::string output_dir = "out";  // -d/--out-dir  output directory

    // Capture type: exactly one category.
    // Valid values: nativeheap/nh/native, snapshot/snap, globalhandle/global/gh,
    //               allhandle/handle/ah, localhandle/local/lh, nativeleaks/nl,
    //               fd, dma, thread, ashmem
    std::string category = "nativeheap";

    bool flag_report = false;        // -o/--auto-open  generate HTML report and open in browser
    bool flag_list_apps = false;     // --list-apps     list apps and exit

    // Deobfuscation: forwarded to the report subcommand so captured data is
    // restored before analysis (heapsnapshot via source map/nameCache, htrace via .so).
    std::string sourcemap_dir;       // --sourcemap source map archive directory
    std::string name_obfuscation;    // --namecache nameCache archive directory
    std::string so_dir;              // --so-dir native .so symbol directory (trace/tshandle only)
    std::string connect_key;         // --device value: device serial number
    bool connect_key_provided = false;   // whether --device was given explicitly
};

// Run the profiler subcommand with already-parsed options. Performs
// environment checks (hdc + device), loads the config, processes the
// template, and executes the hdc command.
// Returns the process exit code.
int run_profiler_cmd(const ProfilerCliOptions& opts);

} // namespace mem_analyzer
