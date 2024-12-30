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
    } else {
        LOGI("Log file is exist, remove...");
        if (remove(logFilePathChar) == 0) {
            WLOGI("Deleted log file");
        } else {
            WLOGE("Failed to delete log file");
        }
    }
}
void GetLog::TarLogFile()
{
    std::filesystem::path dirPath(LOG_FILE_DIR);
    std::vector<std::filesystem::path> files;

    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (std::filesystem::is_regular_file(entry)) {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
    });

    if (files.size() > 10) {
        files.resize(10);
    }

    std::string tarCommand = "tar -czf " + logFilePath;
    for (const auto& file : files) {
        std::string fileName = file.string();
        fileName = fileName.substr(fileName.find_last_of('/'));
        tarCommand += " " + fileName;
    }

    if (system(tarCommand.c_str()) != 0) {
        WLOGE("Failed to create tar archive");
        return;
    }
}
std::map<std::string, std::string> GetLog::ItemData()
{
    // TODO: 打包日志文件
    RemoveLogFile();
    TarLogFile();

    return std::map<std::string, std::string>();
}

int GetLog::LogFileSocketConnect()
{
    logFileSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (logFileSocket < 0) {
        return -1;
    }
    struct sockaddr_in socket_addr = {0};
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(logFIleSocketPort);
    socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(logFileSocket, reinterpret_cast<struct sockaddr *>(&socket_addr), sizeof(socket_addr)) < 0) {
        return -1;
    }
    return 0;
}
int GetLog::SendLogFile()
{
    std::ifstream logFile(logFilePath, std::ios::binary);
    if (!logFile.is_open()) {
        WLOGE("Open log file failed");
        close(logFileSocket);
        return -1;
    }

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
    return 0;
}   
} // namespace SmartPerf
} // namespace OHOS