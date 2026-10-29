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
#pragma once
#include <chrono>
#include <string>

#include "utils/logger.hpp"

namespace mem_analyzer {

/**
 * Simple timer for measuring and printing stage durations.
 *
 * Usage:
 *   StageTimer timer("DB");
 *   timer.Stage("Loading data_dict");     // prints:   [DB] ── Loading data_dict
 *   // ... work ...
 *   timer.Tick("data_dict");              // prints:   [DB] data_dict: 123ms
 *   timer.Stage("Building call stacks");  // prints:   [DB] ── Building call stacks
 *   // ... work ...
 *   timer.Tick("call_stacks", "1000 st"); // prints:   [DB] call_stacks: 456ms (1000 st)
 */
class StageTimer {
public:
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::milliseconds;

    explicit StageTimer(const std::string& tag) : tag_(tag), start_(Clock::now()), last_(start_) {}

    /// Print a stage entry marker and start timing for it
    void Stage(const std::string& msg)
    {
        log::LogiMinor("  [", tag_, "] -- ", msg);
        last_ = Clock::now();
    }

    /// Print elapsed since last tick/stage (or start), and update last_
    void Tick(const std::string& label, const std::string& extra = "")
    {
        auto now = Clock::now();
        auto elapsed = std::chrono::duration_cast<Ms>(now - last_).count();
        if (extra.empty()) {
            log::LogiMinor("    [", tag_, "] ", label, ": ", elapsed, "ms");
        } else {
            log::LogiMinor("    [", tag_, "] ", label, ": ", elapsed, "ms (", extra, ")");
        }
        last_ = now;
    }

    /// Print total since start, no extra label needed
    void Done()
    {
        auto now = Clock::now();
        auto elapsed = std::chrono::duration_cast<Ms>(now - start_).count();
        log::LogiMinor("    [", tag_, "] done: ", elapsed, "ms");
        last_ = now;
    }

    /// Get elapsed since start (no print)
    long long ElapsedMs() const
    {
        return std::chrono::duration_cast<Ms>(Clock::now() - start_).count();
    }

    /// Get elapsed since last tick (no print)
    long long SinceLastMs() const
    {
        return std::chrono::duration_cast<Ms>(Clock::now() - last_).count();
    }

private:
    std::string tag_;
    Clock::time_point start_;
    Clock::time_point last_;
};

} // namespace mem_analyzer
