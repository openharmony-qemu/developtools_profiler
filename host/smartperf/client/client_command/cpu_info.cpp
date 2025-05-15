/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cpu_info.h"
#include "sp_utils.h"

#include <string>
#include <regex>
#include <string_view>
#include <iomanip>
#include <unistd.h>
namespace OHOS {
namespace SmartPerf {
namespace {
const std::string HIPERF_CMD = "/bin/hiperf stat -e hw-instructions,hw-cpu-cycles -d 1 -i 800 ";
}
std::map<std::string, std::string> CPUInfo::ItemData()
{
    Stop();
    std::map<std::string, std::string> result;
    std::istringstream stream(buffer_);
    std::string line;
    std::regex cpu_cycles_regex(R"(\s*([\d,]+)\s+hw-cpu-cycles)");
    std::regex instructions_regex(R"(\s*([\d,]+)\s+hw-instructions)");
    std::regex cpi_regex(R"(\|\s*([\d.]+)\s+cycles per instruction)");

    uint64_t cpu_cycles_total = 0;
    uint64_t instructions_total = 0;
    double cpi_total = 0.0;
    size_t cpu_cycles_count = 0;
    size_t instructions_count = 0;
    size_t cpi_count = 0;

    while (std::getline(stream, line)) {
        std::smatch match;

        if (std::regex_search(line, match, cpu_cycles_regex)) {
            std::string number_str = match[1];
            number_str.erase(std::remove(number_str.begin(), number_str.end(), ','), number_str.end());
            cpu_cycles_total += SPUtilesTye::StringToSometype<uint64_t>(number_str);
            ++cpu_cycles_count;
        }

        if (std::regex_search(line, match, instructions_regex)) {
            std::string number_str = match[1];
            number_str.erase(std::remove(number_str.begin(), number_str.end(), ','), number_str.end());
            instructions_total += SPUtilesTye::StringToSometype<uint64_t>(number_str);
            ++instructions_count;
        }

        if (std::regex_search(line, match, cpi_regex)) {
            cpi_total += SPUtilesTye::StringToSometype<double>(match[1]);
            ++cpi_count;
        }
    }

    cpu_cycles_count == 0 ? "" :
        result["hw-cpu-cycles"] = std::to_string(static_cast<double>(cpu_cycles_total) / cpu_cycles_count);
    instructions_count == 0 ? "" :
        result["hw-instructions"] = std::to_string(static_cast<double>(instructions_total) / instructions_count);
    cpi_count == 0 ? "" :
        result["cycles per instruction"] = std::to_string(cpi_total / cpi_count);

    Start();
    return result;
}

void CPUInfo::StartExecutionOnce()
{
    Stop();
    hiperfCmd_ = HIPERF_CMD;
    if (pids_.empty()) {
        hiperfCmd_ += "-a";
    } else {
        hiperfCmd_ += "-p " + pids_[0];
    }

    running_ = true;
    th_ = std::thread([this]() {
        while (running_) {
            buffer_.clear();
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(hiperfCmd_.c_str(), "r"), pclose);
            constexpr int lineSize = 1024;
            std::array<char, lineSize> chunk;
            while (fgets(chunk.data(), chunk.size(), pipe.get()) != nullptr) {
                buffer_.append(chunk.data());
            }

            if (!running_) {
                return;
            }
            std::unique_lock<std::mutex> lock(mtx_);
            cond_.wait(lock);
        }
    });
    sleep(1);
}

void CPUInfo::FinishtExecutionOnce()
{
    running_ = false;
    Stop();
    cond_.notify_all();
    if (th_.joinable()) {
        th_.join();
    }
}

void CPUInfo::SetPids(std::string& pids)
{
    SPUtils::StrSplit(pids, " ", pids_);
}

void CPUInfo::Start()
{
    cond_.notify_all();
}

void CPUInfo::Stop()
{
    std::system("killall hiperf > /data/local/tmp/cpu_temp_error 2>&1");
    std::remove("/data/local/tmp/cpu_temp_error");
}
}
}