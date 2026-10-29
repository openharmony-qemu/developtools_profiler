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

constexpr int JSON_INDENT_SPACES = 4;

void WriteJsonToFile(const std::string& filePath, const json& data)
{
    auto ofs = OpenOfstream(filePath);
    if (!ofs) {
        log::Loge("Error: Cannot write to ", filePath);
        return;
    }
    ofs << data.dump(JSON_INDENT_SPACES) << std::endl;
    ofs.close();
}

std::string FormatReadableTimestamp(long long ms)
{
    auto tp = std::chrono::system_clock::time_point{std::chrono::milliseconds(ms)};
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;
#if defined(_WIN32)
    (void)localtime_s(&tm, &t);
#else
    (void)localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace mem_analyzer
