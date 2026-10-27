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
 *   timer.stage("Loading data_dict");     // prints:   [DB] ── Loading data_dict
 *   // ... work ...
 *   timer.tick("data_dict");              // prints:   [DB] data_dict: 123ms
 *   timer.stage("Building call stacks");  // prints:   [DB] ── Building call stacks
 *   // ... work ...
 *   timer.tick("call_stacks", "1000 st"); // prints:   [DB] call_stacks: 456ms (1000 st)
 */
class StageTimer {
public:
    using clock = std::chrono::steady_clock;
    using ms = std::chrono::milliseconds;

    explicit StageTimer(const std::string& tag) : tag_(tag), start_(clock::now()), last_(start_) {}

    /// Print a stage entry marker and start timing for it
    void stage(const std::string& msg) {
        log::logi_minor("  [", tag_, "] -- ", msg);
        last_ = clock::now();
    }

    /// Print elapsed since last tick/stage (or start), and update last_
    void tick(const std::string& label, const std::string& extra = "") {
        auto now = clock::now();
        auto elapsed = std::chrono::duration_cast<ms>(now - last_).count();
        if (extra.empty()) {
            log::logi_minor("    [", tag_, "] ", label, ": ", elapsed, "ms");
        } else {
            log::logi_minor("    [", tag_, "] ", label, ": ", elapsed, "ms (", extra, ")");
        }
        last_ = now;
    }

    /// Print total since start, no extra label needed
    void done() {
        auto now = clock::now();
        auto elapsed = std::chrono::duration_cast<ms>(now - start_).count();
        log::logi_minor("    [", tag_, "] done: ", elapsed, "ms");
        last_ = now;
    }

    /// Get elapsed since start (no print)
    long long elapsed_ms() const {
        return std::chrono::duration_cast<ms>(clock::now() - start_).count();
    }

    /// Get elapsed since last tick (no print)
    long long since_last_ms() const {
        return std::chrono::duration_cast<ms>(clock::now() - last_).count();
    }

private:
    std::string tag_;
    clock::time_point start_;
    clock::time_point last_;
};

} // namespace mem_analyzer
