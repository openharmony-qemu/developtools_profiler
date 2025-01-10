/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "include/GetLog.h"
#include "include/sp_utils.h"
#include "include/smartperf_command.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <chrono>
#include <ctime>
namespace OHOS {
namespace SmartPerf {
void GetLog::CreateDir(const std::string &dirPath)
{
    if (!SPUtils::FileAccess(dirPath)) {
        std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::CREAT_DIR) + dirPath;
        std::string cmdResult;
        if (!SPUtils::LoadCmd(cmd, cmdResult)) {
            WLOGE("%s capture not be created!", dirPath.c_str());
        } else {
            WLOGI("%s created successfully!", dirPath.c_str());
        }
    }
}

void GetLog::RemoveDirOrFile(const std::string &dirPath)
{
    char pathChar[PATH_MAX] = {0x00};
    if ((realpath(dirPath.c_str(), pathChar) == nullptr)) {
        WLOGI("%s is not exist.", dirPath.c_str());
        return;
    }
    LOGD("%s is exist, remove...", dirPath.c_str());

    std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::REMOVE) + dirPath;
    std::string cmdResult;
    if (!SPUtils::LoadCmd(cmd, cmdResult)) {
        WLOGE("%s capture not be removed!", dirPath.c_str());
    } else {
        WLOGI("%s removed successfully!", dirPath.c_str());
    }
}

void GetLog::CopyFiles(std::string cpStr)
{
    std::string cmd = CMD_COMMAND_MAP.at(CmdCommand::CP) + cpStr;
    std::string cmdResult;
    if (!SPUtils::LoadCmd(cmd, cmdResult)) {
        WLOGE("Failed to copy files: %s", cpStr.c_str());
    }
}

void GetLog::TarFiles(std::string tarStr)
{
    std::string tarCommand = CMD_COMMAND_MAP.at(CmdCommand::TAR) + tarStr;
    std::string cmdResult;
    if (!SPUtils::LoadCmd(tarCommand, cmdResult)) {
        WLOGE("Failed to tar log files");
    }
}

void GetLog::RemoveLogFile()
{
    // Process before and after send
    RemoveDirOrFile(logFilePath);
    RemoveDirOrFile(daemonLogFileDir);

    currentLogSize = 0;
}

void GetLog::GenerateDaemonLogFile()
{
    const std::string preLogFileName = "log.";
    std::filesystem::path dirPath(LOG_FILE_DIR);    // Log file directory
    std::vector<std::filesystem::path> files;       // Log file vector to tar

    CreateDir(daemonLogFileDir);                    // Create daemonLog directory

    // Save current working directory to restore it later
    // Change directory to handle relative paths in tar operations
    char originPath[PATH_MAX] = {0x00};
    if (getcwd(originPath, sizeof(originPath)) == nullptr) {
        WLOGE("Failed to get current working directory");
        return;
    }
    std::filesystem::current_path(LOG_FILE_DIR);

    // Get all log files in LOG_FILE_DIR
    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (std::filesystem::is_regular_file(entry)) {
            if (entry.path().filename().string().substr(0, preLogFileName.length()) != preLogFileName) {
                continue;  // Skip files that don't start with "log."
            }
            files.push_back(entry.path());
        }
    }

    // Sort log files by last write time
    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
    });

    // Build tar command with relative paths only, respecting size limit
    std::string cpCommand = "";
    for (const auto& file : files) {
        uintmax_t fileSize = std::filesystem::file_size(file);
        if (currentLogSize + fileSize > logMaxSize) {
            break; // Stop if adding this file would exceed the limit
        }
        currentLogSize += fileSize;
        std::string filename = file.filename().string();
        cpCommand += filename + " ";
    }
    cpCommand += daemonLogFileDir;
    CopyFiles(cpCommand);

    std::filesystem::current_path(originPath);
    WLOGI("Created tar archive of daemonLog files successfully");
}

void GetLog::TarLogFile()
{
    GenerateDaemonLogFile();

    char originPath[PATH_MAX] = {0x00};
    if (getcwd(originPath, sizeof(originPath)) == nullptr) {
        WLOGE("Failed to get current working directory");
        return;
    }

    // cd LOG_FILE_DIR
    std::filesystem::current_path(LOG_FILE_DIR);

    // Check if directories exist
    if (!std::filesystem::exists("daemonLog")) {
        WLOGE("One or both directories do not exist");
        std::filesystem::current_path(originPath);
        return;
    }

    // Build tar command with relative paths
    std::string tarCommand = logFilePath + " daemonLog";
    TarFiles(tarCommand);

    // Restore original working directory
    std::filesystem::current_path(originPath);
    WLOGI("Created tar archive of log files successfully");
}

std::map<std::string, std::string> GetLog::ItemData()
{
    // Remove old log tar file
    RemoveLogFile();
    // Create tar archive of log files
    TarLogFile();
    // Return empty map to satisfy interface
    return std::map<std::string, std::string>();
}

int GetLog::LogFileSocketConnect()
{
    logFileSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (logFileSocket < 0) {
        WLOGE("Create log file socket failed, errno: %d", errno);
        return -1;
    }
    struct sockaddr_in socketAddr = {0};
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_port = htons(logFileSocketPort);
    socketAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(logFileSocket, reinterpret_cast<struct sockaddr *>(&socketAddr), sizeof(socketAddr)) < 0) {
        WLOGE("Connect log file socket failed, errno: %d", errno);
        return -1;
    }
    WLOGI("Connect log file socket success, socket: %d", logFileSocket);
    return logFileSocket;
}

int GetLog::SendLogFile()
{
    std::ifstream logFile(logFilePath, std::ios::binary);
    if (!logFile.is_open()) {
        WLOGE("Open log file failed");
        close(logFileSocket);
        logFileSocket = -1;
        return -1;
    }
    WLOGI("logfile exists, sending...");
    // Get file size
    logFile.seekg(0, std::ios::end);
    std::streamsize fileSize = logFile.tellg();
    logFile.seekg(0, std::ios::beg);

    std::streamsize totalSent = 0;
    while (!logFile.eof()) {
        logFile.read(logSocketBuffer, sizeof(logSocketBuffer));
        std::streamsize bytesRead = logFile.gcount();
        ssize_t bytesSent = send(logFileSocket, logSocketBuffer, bytesRead, 0);
        if (bytesSent < 0) {
            WLOGE("Send log file failed");
            logFile.close();
            close(logFileSocket);
            logFileSocket = -1;
            return -1;
        }
        totalSent += bytesSent;
        if (bytesSent != bytesRead) {
            WLOGE("Incomplete send: sent %zd bytes out of %zd", bytesSent, bytesRead);
            logFile.close();
            close(logFileSocket);
            logFileSocket = -1;
            return -1;
        }
    }

    if (totalSent != fileSize) {
        WLOGE("File size mismatch: sent %zd bytes, file size %zd", totalSent, fileSize);
        return -1;
    }
    logFile.close();
    close(logFileSocket);
    logFileSocket = -1;
    WLOGI("Send log file success, bytes: %zd", totalSent);
    RemoveLogFile();
    return 0;
}

void GetLog::SetLogFileSocketPort(int port)
{
    logFileSocketPort = port;
}

int GetLog::GetLogFileSocketPort() const
{
    return logFileSocketPort;
}

void GetLog::SetLogFileSocket(int socket)
{
    logFileSocket = socket;
}

int GetLog::GetLogFileSocket() const
{
    return logFileSocket;
}
} // namespace SmartPerf
} // namespace OHOS