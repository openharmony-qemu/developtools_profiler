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
#include <queue>
namespace OHOS {
namespace SmartPerf {
void GetLog::RemoveLogFile()
{
    char logFilePathChar[PATH_MAX] = {0x00};
    if ((realpath(logFilePath.c_str(), logFilePathChar) == nullptr)) {
        LOGI("Log file is not exist.");
        return;
    } 

    LOGI("Log file is exist, remove...");
    if (remove(logFilePathChar) == 0) {
        WLOGI("Deleted log file");
    } else {
        WLOGE("Failed to delete log file");
    }

    char hilogFileDirChar[PATH_MAX] = {0x00};
    if ((realpath(hilogFileDir.c_str(), hilogFileDirChar) == nullptr)) {
        LOGI("hilog directory is not exist.");
        return;
    }
    LOGI("hilog directory is exist, remove...");
    if (remove(hilogFileDirChar) == 0) {
        WLOGI("Deleted hilog file");
    } else {
        WLOGE("Failed to delete hilog file");
    }

    char daemonLogFileDirChar[PATH_MAX] = {0x00};
    if ((realpath(daemonLogFileDir.c_str(), daemonLogFileDirChar) == nullptr)) {
        LOGI("daemonLog directory is not exist.");
        return;
    }
    LOGI("daemonLog directory is exist, remove...");
    if (remove(daemonLogFileDirChar) == 0) {
        WLOGI("Deleted daemonLog file");
    } else {
        WLOGE("Failed to delete daemonLog file");
    }
}
void GetLog::GenerateDaemonLogFile()
{
    std::filesystem::path dirPath(LOG_FILE_DIR);    // Log file directory
    std::vector<std::filesystem::path> files;       // Log file vector to tar
    std::string cmdResult;

    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) == nullptr) {
        WLOGE("Failed to get current working directory");
        return;
    }

    // cd LOG_FILE_DIR
    std::filesystem::current_path(LOG_FILE_DIR);

    if (!SPUtils::FileAccess(daemonLogFileDir)) {
        std::string daemonLogFileDirPath = CMD_COMMAND_MAP.at(CmdCommand::CREAT_DIR) + daemonLogFileDir;
        if (!SPUtils::LoadCmd(daemonLogFileDirPath, cmdResult)) {
            WLOGE("%s capture not be created!", daemonLogFileDir.c_str());
            return;
        } else {
            WLOGI("%s created successfully!", daemonLogFileDir.c_str());
        }
    };

    // Use a min heap to keep track of the 10 most recent files
    std::priority_queue<std::filesystem::path, std::vector<std::filesystem::path>,
        std::function<bool(const std::filesystem::path&, const std::filesystem::path&)>> heap(
        [](const auto& a, const auto& b) {
            return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
        });

    // Get all log files in LOG_FILE_DIR
    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (std::filesystem::is_regular_file(entry)) {
            heap.push(entry.path());
            if (heap.size() > 10) {
                heap.pop(); // Remove oldest file if we exceed 10 files
            }
        }
    }

    // Transfer files from heap to vector (will be in reverse chronological order)
    while (!heap.empty()) {
        files.push_back(heap.top());
        heap.pop();
    }

    // Build tokar command with relative paths only
    std::string tarCommand = "cp ";
    for (const auto& file : files) {
        std::string filename = file.filename().string();
        tarCommand += filename + " ";
    }
    tarCommand += daemonLogFileDir;

    if (system(tarCommand.c_str()) != 0) {
        WLOGE("Failed to create tar archive");
        return;
    }
    WLOGI("Created tar archive of daemonLog files successfully");
}
std::time_t to_time_t(const std::filesystem::file_time_type& ftime) {
    auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(systemTime);
}

void GetLog::GenerateHilogFile()
{
    std::filesystem::path dirPath(systemHilogFileDir);    // System hilog file directory
    std::vector<std::filesystem::path> files;             // System hilog file vector to tar
    std::string cmdResult;

    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) == nullptr) {
        WLOGE("Failed to get current working directory");
        return;
    }

    if (!SPUtils::FileAccess(hilogFileDir)) {
        std::string hilogFileDirPath = CMD_COMMAND_MAP.at(CmdCommand::CREAT_DIR) + hilogFileDir;
        if (!SPUtils::LoadCmd(hilogFileDirPath, cmdResult)) {
            WLOGE("%s capture not be created!", hilogFileDir.c_str());
            return;
        } else {
            WLOGI("%s created successfully!", hilogFileDir.c_str());
        }
    };

    // Get all hilog files in systemHilogFileDir
    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (std::filesystem::is_regular_file(entry)) {
            std::string extension = entry.path().extension().string();
            // Check for .log and .zip files
            if (extension == ".log" || extension == ".zip") {
                files.push_back(entry.path());
            }
            // Check for .gz files with today's date
            else if (extension == ".gz") {
                auto fileTime = std::filesystem::last_write_time(entry.path());
                auto now = std::filesystem::file_time_type::clock::now();
                
                // Convert to system time
                auto fileTimeT = to_time_t(fileTime);
                auto nowT = to_time_t(now);
                
                // Convert to tm struct to compare dates
                auto fileTm = *std::localtime(&fileTimeT);
                auto nowTm = *std::localtime(&nowT);
                
                // Compare year, month and day
                if (fileTm.tm_year == nowTm.tm_year && 
                    fileTm.tm_mon == nowTm.tm_mon &&
                    fileTm.tm_mday == nowTm.tm_mday) {
                    files.push_back(entry.path());
                }
            }
        }
    }

    // Sort hilog files by last write time
    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
    });

    // Count .gz files and limit them to 10
    int gzCount = 0;
    auto it = files.begin();
    while (it != files.end()) {
        if (it->extension() == ".gz") {
            gzCount++;
            if (gzCount > 10) {
                it = files.erase(it);
                continue;
            }
        }
        ++it;
    }

    // cd LOG_FILE_DIR
    std::filesystem::current_path(systemHilogFileDir);
    // Build tokar command with relative paths only
    std::string tarCommand = "cp ";
    for (const auto& file : files) {
        std::string filename = file.filename().string();
        tarCommand += filename + " ";
    }
    tarCommand += hilogFileDir;
    if (system(tarCommand.c_str()) != 0) {
        WLOGE("Failed to create tar archive");
        return;
    }
    WLOGI("Created tar archive of hilog files successfully");
}
void GetLog::TarLogFile()
{
    GenerateDaemonLogFile();
    GenerateHilogFile();
    // Store current working directory, to restore later
    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) == nullptr) {
        WLOGE("Failed to get current working directory");
        return;
    }

    // cd LOG_FILE_DIR
    std::filesystem::current_path(LOG_FILE_DIR);

    // Check if directories exist
    if (!std::filesystem::exists("hilog") || !std::filesystem::exists("daemonLog")) {
        WLOGE("One or both directories do not exist");
        std::filesystem::current_path(currentPath);
        return;
    }

    // Build tar command with relative paths
    std::string tarCommand = "tar -czf " + logFilePath + " hilog daemonLog";

    if (system(tarCommand.c_str()) != 0) {
        WLOGE("Failed to create tar archive");
        std::filesystem::current_path(currentPath);
        return;
    }

    // Restore original working directory
    std::filesystem::current_path(currentPath);
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
    struct sockaddr_in socket_addr = {0};
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(logFileSocketPort);
    socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(logFileSocket, reinterpret_cast<struct sockaddr *>(&socket_addr), sizeof(socket_addr)) < 0) {
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
        return -1;
    }
    WLOGI("logfile exists, sending...");
    while (!logFile.eof()) {
        logFile.read(logSocketBuffer, sizeof(logSocketBuffer));
        std::streamsize bytesRead = logFile.gcount();
        if (send(logFileSocket, logSocketBuffer, bytesRead, 0) < 0) {
            WLOGE("Send log file failed");
            logFile.close();
            close(logFileSocket);
            return -1;
        }
    }
    logFile.close();
    close(logFileSocket);
    RemoveLogFile();
    WLOGI("Send log file success");
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