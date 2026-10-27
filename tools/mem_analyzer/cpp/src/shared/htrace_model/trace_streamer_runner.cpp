#include "shared/htrace_model/trace_streamer_runner.hpp"

#include "utils/logger.hpp"
#include "utils/platform.hpp"

#include <string>
#include <vector>

namespace mem_analyzer {

bool run_trace_streamer(const std::string& htrace_path,
                        const std::string& db_path,
                        const std::string& so_dir,
                        std::string* error) {
    const std::string streamer = platform::get_trace_streamer_path();
    if (streamer.empty() || !platform::file_exists(streamer)) {
        if (error) *error = "trace_streamer not found at: " + streamer;
        return false;
    }

    std::vector<std::string> args = {"-e",
                                     platform::absolute_path(db_path),
                                     platform::absolute_path(htrace_path)};
    if (!so_dir.empty()) {
        args.push_back("--So_dir");
        args.push_back(platform::absolute_path(so_dir));
    }

    log::logd("[trace_streamer] ", streamer, " ", platform::absolute_path(db_path), " ",
              platform::absolute_path(htrace_path),
              (so_dir.empty() ? std::string() : " --So_dir " + platform::absolute_path(so_dir)));

    const int result = platform::run_process_with_spinner(streamer, args, "Running trace_streamer");
    if (result != 0) {
        if (error) *error = "trace_streamer conversion failed with code: " + std::to_string(result);
        return false;
    }
    return true;
}

} // namespace mem_analyzer
