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

#include "file_path_handler.h"
#include "common.h"
#include "ipc_skeleton.h"
#include "profiler_service.grpc.pb.h"
#include <sys/stat.h>
#include <cstring>
#include <memory>

const std::string SandboxFilePathHandler::SANDBOX_PATH_ROOT("/storage/Users/currentUser/");
const std::string NormalFilePathHandler::DEFAULT_OUTPUT_FILE("/data/local/tmp/hiprofiler_data.htrace");
const std::string NormalFilePathHandler::DEFAULT_OUTPUT_ROOT("/data/local/tmp/");

SandboxFilePathHandler::SandboxFilePathHandler()
{
}

std::string SandboxFilePathHandler::GetConfigFilePath(const std::string& configFile)
{
    if (configFile.find('/') != std::string::npos) {
        return configFile;
    }
    return SANDBOX_PATH_ROOT + configFile;
}

std::vector<std::string> SandboxFilePathHandler::GetValidPaths()
{
    return {SANDBOX_PATH_ROOT};
}

std::string SandboxFilePathHandler::GetDefaultOutputFile()
{
    std::string fileName = SANDBOX_PATH_ROOT + "hiprofiler_data.htrace";
    char* outputPathEnv = std::getenv("TMPDIR");
    if (outputPathEnv != nullptr && strlen(outputPathEnv) > 0) {
        fileName = outputPathEnv;
        fileName += "/hiprofiler_data.htrace";
    }
    return fileName;
}

std::string SandboxFilePathHandler::GetSandboxFileName(const std::string& outputFile,
                                                       ProfilerSessionConfig* sessionConfig)
{
    std::string fileName;
    if (!outputFile.empty()) {
        fileName = outputFile;
        sessionConfig->set_result_file(fileName);
    } else if (sessionConfig->result_file().empty()) {
        fileName = GetDefaultOutputFile();
        sessionConfig->set_result_file(fileName);
    } else {
        fileName = sessionConfig->result_file();
    }
    return fileName;
}

bool SandboxFilePathHandler::HandleSandboxFilename(const std::string& outputFile, ProfilerSessionConfig* sessionConfig)
{
    int32_t callingUid = OHOS::IPCSkeleton::GetCallingUid();
    if (callingUid <= APP_ID_THRESH) {
        return false; // Not a sandbox app
    }

    std::string fileName = GetSandboxFileName(outputFile, sessionConfig);
    if (fileName.substr(0, SANDBOX_PATH_ROOT.size()) == SANDBOX_PATH_ROOT) {
        size_t lastSlash = fileName.find_last_of('/');
        if (lastSlash == std::string::npos) {
            printf("HandleSandboxFilename filename invalid!\n");
            return false;
        }
        std::string pathName = fileName.substr(0, lastSlash + 1);
        struct stat st;
        if (stat(pathName.c_str(), &st) != 0) {
            printf("Filepath unable to write!\n");
            return false;
        }
        int32_t userId = callingUid / USER_ID_MOD;
        fileName.replace(0, SANDBOX_PATH_ROOT.size(),
                         "/storage/media/" + std::to_string(userId) + "/local/files/Docs/");
    } else {
        printf("Filepath unable to write! Please use /storage/Users/currentUser/ at the beginning.\n");
        return false;
    }
    sessionConfig->set_result_file(COMMON::CanonicalizeSpecPath(fileName.c_str()));
    return true;
}

bool SandboxFilePathHandler::HandleOutputFilePath(const std::string& outputFile, ProfilerSessionConfig* sessionConfig)
{
    return HandleSandboxFilename(outputFile, sessionConfig);
}

// NormalFilePathHandler implementation
std::string NormalFilePathHandler::GetConfigFilePath(const std::string& configFile)
{
    if (configFile.find('/') != std::string::npos) {
        return configFile;
    }
    return DEFAULT_OUTPUT_ROOT + configFile;
}

std::vector<std::string> NormalFilePathHandler::GetValidPaths()
{
    return {DEFAULT_OUTPUT_ROOT};
}

std::string NormalFilePathHandler::GetDefaultOutputFile()
{
    return DEFAULT_OUTPUT_FILE;
}

bool NormalFilePathHandler::HandleOutputFilePath(const std::string& outputFile, ProfilerSessionConfig* sessionConfig)
{
    if (!outputFile.empty()) {
        sessionConfig->set_result_file(outputFile);
    } else if (sessionConfig->result_file().empty()) {
        sessionConfig->set_result_file(DEFAULT_OUTPUT_FILE);
    }
    if (sessionConfig->result_file().substr(0, DEFAULT_OUTPUT_ROOT.size()) != DEFAULT_OUTPUT_ROOT) {
        printf("Filepath unable to write! Please use /data/local/tmp/ at the beginning.\n");
        return false;
    }
    return true;
}

// FilePathHandlerFactory implementation
std::unique_ptr<FilePathHandler> FilePathHandlerFactory::CreateHandler()
{
#if defined(is_sandbox) && is_sandbox
    int32_t callingUid = OHOS::IPCSkeleton::GetCallingUid();
    constexpr int APP_ID_THRESH = 20000000;
    if (callingUid > APP_ID_THRESH) {
        return std::make_unique<SandboxFilePathHandler>();
    }
#endif
    return std::make_unique<NormalFilePathHandler>();
}

