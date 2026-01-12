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

#ifndef FILE_PATH_HANDLER_H
#define FILE_PATH_HANDLER_H

#include <string>
#include <vector>
#include <memory>
#include "profiler_service_types.pb.h"

// Forward declaration for protobuf types
class SessionConfig;

class FilePathHandler {
public:
    virtual ~FilePathHandler() = default;
    virtual bool HandleOutputFilePath(const std::string& outputFile, ProfilerSessionConfig* sessionConfig) = 0;
    virtual std::string GetConfigFilePath(const std::string& configFile) = 0;
    virtual std::vector<std::string> GetValidPaths() = 0;

protected:
    virtual std::string GetDefaultOutputFile() = 0;
};

class SandboxFilePathHandler : public FilePathHandler {
public:
    SandboxFilePathHandler();
    ~SandboxFilePathHandler() override = default;

    bool HandleOutputFilePath(const std::string& outputFile, ProfilerSessionConfig* sessionConfig) override;
    std::string GetConfigFilePath(const std::string& configFile) override;
    std::vector<std::string> GetValidPaths() override;

protected:
    std::string GetDefaultOutputFile() override;

private:
    std::string GetSandboxFileName(const std::string& outputFile, ProfilerSessionConfig* sessionConfig);
    bool HandleSandboxFilename(const std::string& outputFile, ProfilerSessionConfig* sessionConfig);

    static constexpr int USER_ID_MOD = 200000;
    static constexpr int APP_ID_THRESH = 20000000;
    static const std::string SANDBOX_PATH_ROOT;

    friend class FilePathHandlerFactory;
};

class NormalFilePathHandler : public FilePathHandler {
public:
    NormalFilePathHandler() = default;
    ~NormalFilePathHandler() override = default;

    bool HandleOutputFilePath(const std::string& outputFile, ProfilerSessionConfig* sessionConfig) override;
    std::string GetConfigFilePath(const std::string& configFile) override;
    std::vector<std::string> GetValidPaths() override;

protected:
    std::string GetDefaultOutputFile() override;

private:
    static const std::string DEFAULT_OUTPUT_FILE;
    static const std::string DEFAULT_OUTPUT_ROOT;
};

class FilePathHandlerFactory {
public:
    static std::unique_ptr<FilePathHandler> CreateHandler();
};

#endif // FILE_PATH_HANDLER_H
