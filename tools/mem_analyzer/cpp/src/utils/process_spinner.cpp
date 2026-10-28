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
#include "utils/process_spinner.hpp"
#include "utils/terminal_style.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace mem_analyzer {

static const char* FRAMES[] = {"\u280B","\u2819","\u2839","\u2838","\u283C","\u2834","\u2826","\u2827","\u2807","\u280F"};
static constexpr int FRAME_COUNT = 10;
static constexpr int INTERVAL_MS  = 100;

ProcessSpinner::ProcessSpinner(const std::string& description)
    : description_(description),
      start_(std::chrono::steady_clock::now())
{
    if (style::IsTty()) {
        thread_ = std::thread(&ProcessSpinner::Run, this);
    }
}

ProcessSpinner::~ProcessSpinner()
{
    if (running_.load()) {
        done(false);
    }
}

void ProcessSpinner::Done(bool success)
{
    running_.store(false);
    if (thread_.joinable()) {
        thread_.join();
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_).count();
    double sec = elapsed / 1000.0;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << sec;

    auto status = success ? style::green("done") : style::red("failed");
    auto time   = style::dim("(" + oss.str() + "s)");

    if (style::IsTty()) {
        std::cout << "\r  " << description_ << "...  " <<
                  status << " " << time << "     " << std::endl;
    } else {
        std::cout << "  " << description_ << "...  " <<
                  (success ? "done" : "failed") <<
                  " (" << oss.str() << "s)" << std::endl;
    }
}

void ProcessSpinner::Run()
{
    int frame = 0;
    while (running_.load()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_).count();
        double sec = elapsed / 1000.0;

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sec << "s";

        std::cout << "\r  " << description_ << "...  " <<
                  FRAMES[frame] << "  (" << oss.str() << ")   " << std::flush;

        frame = (frame + 1) % FRAME_COUNT;
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_MS));
    }
}

} // namespace mem_analyzer