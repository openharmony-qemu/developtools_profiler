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
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <filesystem>
#include <vector>
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

}
void GetLog::TarLogFile()
{
    std::filesystem::path dirPath(LOG_FILE_DIR);    // Log file directory
    std::vector<std::filesystem::path> files;       // Log file vector to tar

    // Get all log files in LOG_FILE_DIR
    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (std::filesystem::is_regular_file(entry)) {
            files.push_back(entry.path());
        }
    }

    // Sort log files by last write time
    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
    });

    // Limit log files to 10
    if (files.size() > 10) {
        files.resize(10);
    }

    // Store current working directory, to restore later
    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) == nullptr) {
        WLOGE("Failed to get current working directory");
        return;
    }

    // cd LOG_FILE_DIR
    std::filesystem::current_path(LOG_FILE_DIR);

    // Build tokar command with relative paths only
    std::string tarCommand = "tar -czf " + logFilePath;
    for (const auto& file : files) {
        std::string filename = file.filename().string();
        tarCommand += " " + filename;
    }

    if (system(tarCommand.c_str()) != 0) {
        WLOGE("Failed to create tar archive");
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
    
    // Get file size
    logFile.seekg(0, std::ios::end);
    std::streamsize fileSize = logFile.tellg();
    logFile.seekg(0, std::ios::beg);
    
    std::streamsize totalBytesSent = 0;
    while (!logFile.eof()) {
        logFile.read(logSocketBuffer, sizeof(logSocketBuffer));
        std::streamsize bytesRead = logFile.gcount();
        if (send(logFileSocket, logSocketBuffer, bytesRead, 0) < 0) {
            WLOGE("Send log file failed");
            logFile.close();
            close(logFileSocket);
            return -1;
        }
        totalBytesSent += bytesRead;
    }
    logFile.close();
    
    if (totalBytesSent != fileSize) {
        WLOGE("File size verification failed: sent %ld bytes, file size %ld bytes", 
              totalBytesSent, fileSize);
        return -1;
    }
    WLOGI("File sent successfully: %ld bytes", totalBytesSent);
    close(logFileSocket);
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