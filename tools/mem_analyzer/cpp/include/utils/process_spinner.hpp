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