/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef PROFILER_SESSION_MANAGER_H
#define PROFILER_SESSION_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "profiler_service.grpc.pb.h"

class ProfilerSessionManager {
public:
    static ProfilerSessionManager& GetInstance();

    ProfilerSessionManager(const ProfilerSessionManager&) = delete;
    ProfilerSessionManager& operator=(const ProfilerSessionManager&) = delete;

    bool Capture(const std::string& config, const std::string& durationSeconds, const std::string& outputFile);
    uint32_t CaptureLongRunning(const std::string& config, const std::string& durationSeconds,
                                const std::string& outputFile);
    bool StopSessionById(uint32_t sessionId);
    bool StopAllSessions();

    bool GetCapabilities(std::string& capabilities, bool printResult = false);

    bool CheckServiceConnection();

    void RequestExit();

private:
    ProfilerSessionManager();
    ~ProfilerSessionManager();

    struct SessionConfig {
        uint32_t sessionId = 0;
        uint32_t durationMs = 0;
        std::string outputFile;
    };

    std::unique_ptr<CreateSessionRequest> MakeCreateRequest(const std::string& config,
                                                            const std::string& keepSecond,
                                                            const std::string& outputFile);
    uint32_t CreateSession(std::unique_ptr<IProfilerService::Stub>& profilerStub,
                           const std::string& config, const std::string& keepSecond,
                           const std::string& outputFile);
    bool StartSession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t sessionId);
    bool StopSession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t sessionId,
                     bool stopAll = false);
    bool DestroySession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t sessionId,
                        bool destroyAll = false);
    std::unique_ptr<IProfilerService::Stub> GetProfilerServiceStub();

    void KeepSessionAlive(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t sessionId);

    std::atomic<bool> exitRequested_;
    std::atomic<bool> keepSessionRunning_;
    std::mutex keepSessionMutex_;
    std::condition_variable keepSessionCv_;
    std::unique_ptr<std::thread> keepSessionThread_;

    std::mutex sessionMutex_;
    std::condition_variable sessionCv_;
};

#endif // PROFILER_SESSION_MANAGER_H

