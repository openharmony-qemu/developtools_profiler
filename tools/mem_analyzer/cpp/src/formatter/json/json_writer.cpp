#include "formatter/json/json_writer.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace mem_analyzer {

void write_json_to_file(const std::string& file_path, const json& data) {
    auto ofs = open_ofstream(file_path);
    if (!ofs) {
        log::loge("Error: Cannot write to ", file_path);
        return;
    }
    ofs << data.dump(4) << std::endl;
    ofs.close();
}

std::string format_readable_timestamp(long long ms) {
    auto tp = std::chrono::system_clock::time_point{std::chrono::milliseconds(ms)};
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace mem_analyzer
