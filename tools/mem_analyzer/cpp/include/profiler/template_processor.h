#pragma once

#include <string>
#include <optional>

namespace mem_analyzer {

// User-supplied options for the profiler subcommand.
struct ProfilerOptions {
    std::optional<std::string> p;  // process name
    std::optional<long long>    t;  // duration in seconds
    std::optional<std::string> o;  // output directory
    std::string config_option;     // e.g. "-g", "-l", "-s", "-n"
};

// The device-side capture command resolved from a config template: the
// command line (first line, placeholders replaced) plus the device config
// content, duration, target process and remote output path needed to
// launch the capture.
struct CaptureCommand {
    bool ok = false;
    std::string error;

    // The command part (first line) after placeholder replacement.
    std::string command;

    // The config content (everything after the first line) for hiprofiler commands.
    std::string device_config_content;

    // The full file content after placeholder replacement.
    std::string content;

    long long duration = 0;

    // The resolved target process name.
    std::string process_name;

    // Device-side result file path extracted from the -o argument.
    std::string remote_output_path;
};

// Read the file at `config_file_path` and replace $(p:default) / $(t:default)
// / $(ts:) placeholders: p/t come from `opts`, ts expands to the local time
// stamp in YYYYMMDD_HHMMSS format.
CaptureCommand process_template(const std::string& config_file_path,
                                const ProfilerOptions& opts);

} // namespace mem_analyzer
