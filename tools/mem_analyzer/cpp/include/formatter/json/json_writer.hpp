#pragma once
#include <string>
#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

namespace mem_analyzer {

void write_json_to_file(const std::string& file_path, const json& data);
std::string format_readable_timestamp(long long ms);

} // namespace mem_analyzer
