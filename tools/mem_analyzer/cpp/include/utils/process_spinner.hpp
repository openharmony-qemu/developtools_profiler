#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>

namespace mem_analyzer {

class ProcessSpinner {
public:
    explicit ProcessSpinner(const std::string& description);
    ~ProcessSpinner();
    void done(bool success = true);

    ProcessSpinner(const ProcessSpinner&) = delete;
    ProcessSpinner& operator=(const ProcessSpinner&) = delete;

private:
    void run();

    std::string description_;
    std::atomic<bool> running_{true};
    std::thread thread_;
    std::chrono::steady_clock::time_point start_;
};

} // namespace mem_analyzer