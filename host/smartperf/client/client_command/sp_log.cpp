/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#include "sp_log.h"

#include "securec.h"

#ifdef HI_LOG_ENABLE
#include "hilog/log.h"
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <mutex>
#include <ctime>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <regex>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <cstdio>
#else
#include <cstdio>
#endif

namespace OHOS {
namespace SmartPerf {
const int32_t LOG_MAX_LEN = 10000;
static const long long MAX_FILE_SIZE = 4 * 1024 * 1024;  // 4MB
static const int MAX_FILE_COUNT = 256;
static const std::string LOG_FILE_DIR = "/sdcard/Android/data/com.huawei.gameperceptio/files/log/SP_daemon/";
static std::mutex g_mtx;
static std::string g_currentLogFilePath;
static std::string g_currentDate;

static void SpLogOut(SpLogLevel logLevel, const char *logBuf)
{
#ifdef HI_LOG_ENABLE
    LogLevel hiLogLevel = LOG_INFO;
    switch (logLevel) {
        case SP_LOG_DEBUG:
            hiLogLevel = LOG_DEBUG;
            break;
        case SP_LOG_INFO:
            hiLogLevel = LOG_INFO;
            break;
        case SP_LOG_WARN:
            hiLogLevel = LOG_WARN;
            break;
        case SP_LOG_ERROR:
            hiLogLevel = LOG_ERROR;
            break;
        default:
            break;
    }
    (void)HiLogPrint(LOG_CORE, hiLogLevel, LOG_DOMAIN, "SP_daemon", "%{public}s", logBuf);
#else
    switch (logLevel) {
        case SP_LOG_DEBUG:
            printf("[D]%s\n", logBuf);
            break;
        case SP_LOG_INFO:
            printf("[I]%s\n", logBuf);
            break;
        case SP_LOG_WARN:
            printf("[W]%s\n", logBuf);
            break;
        case SP_LOG_ERROR:
            printf("[E]%s\n", logBuf);
            break;
        default:
            break;
    }
#endif
}

inline bool CreateDir(const std::string& dirPath)
{
    if (!std::__fs::filesystem::exists(dirPath)) {
        LOGD("Try create dir: %s", dirPath.c_str());
        try {
            // 尝试创建目录
            if (mkdir(dirPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
                // 如果 mkdir 失败，打印具体的错误信息
                LOGE("Failed to create log directory: %s", strerror(errno));
                return false;
            }

            // 检查目录是否成功创建
            if (!std::__fs::filesystem::exists(dirPath)) {
                LOGE("Failed to create log directory.");
                return false;
            }
        } catch (const std::exception& e) {
            LOGE("Exception while creating log directory: %s", e.what());
            return false;
        }
    }

    return true;
}

inline bool EnsureLogDirectoryExists()
{
    return CreateDir(LOG_FILE_DIR);
}

inline std::string GetCurrentDate()
{
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d");
    return oss.str();
}
inline std::string GetCurrentLogFilePath(int maxLogNumber)
{
    // 获取当前时分秒字符串
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::ostringstream oss;
    oss << LOG_FILE_DIR
        << "log."
        << maxLogNumber
        << "."
        << g_currentDate
        << "-"
        << std::put_time(&tm, "%H%M%S");
    return oss.str();
}
inline bool IsFilePathValid(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        return file.tellg() <= MAX_FILE_SIZE;
    }
    return false;
}

inline int CheckFileNameAndGetNumber(const std::string& fileName, const std::string& date, bool* currentDateFlag)
{
    // 定义正则表达式，匹配类似 log.23.20241129-160850 的格式
    std::regex pattern(R"(^log\.(\d+)\.(\d{8})-\d{6}$)");

    // 使用正则表达式匹配文件名
    std::smatch matches;
    if (std::regex_match(fileName, matches, pattern)) {
        // 提取文件名中的日期部分
        std::string fileDate = matches[2].str();

        // 判断日期是否匹配
        if (fileDate == date) {
            *currentDateFlag = true;
        }

        // 如果日期匹配，返回提取出的编号
        return std::stoi(matches[1].str());
    }

    // 如果文件名不符合规定格式，返回 0
    return 0;
}

inline bool Chmod(const std::string& sourceFilePath, mode_t mode)
{
    int retCode = chmod(sourceFilePath.c_str(), mode);
    if (retCode != 0) {
        LOGE("Failed to set %s permission, error code %d.", sourceFilePath.c_str(), retCode);
        return false;
    }

    return true;
}

#define CHMOD_FILE(path) Chmod((path), 0777)
inline void TarFile(const std::string& sourceFileName)
{
    // 创建压缩文件名（添加.tar.gz后缀）
    std::ostringstream compressedFilePath;
    compressedFilePath << LOG_FILE_DIR
                       << sourceFileName
                       << ".tar.gz";

    // 使用系统命令行工具tar进行文件压缩
    std::ostringstream tarCommand;
    tarCommand << "tar -czf "
               << compressedFilePath.str()
               << " -C "
               << LOG_FILE_DIR
               << " "
               << sourceFileName;
    LOGD("Running tar command: %s", tarCommand.str().c_str());

    int tarResult = system(tarCommand.str().c_str());
    if (tarResult != 0) {
        LOGE("Failed to compress file %s", sourceFileName.c_str());
        return;
    }

    CHMOD_FILE(compressedFilePath.str());

    // 删除原文件
    std::ostringstream deleteCommand;
    deleteCommand << "rm "
                  << LOG_FILE_DIR
                  << sourceFileName;
    LOGD("Running delete command: %s", deleteCommand.str().c_str());

    int deleteResult = system(deleteCommand.str().c_str());
    if (deleteResult != 0) {
        LOGE("Failed to delete original file %s", sourceFileName.c_str());
        return;
    }

    LOGD("Successfully compressed and deleted file: %s", sourceFileName.c_str());
    return;
}


inline bool GetLogFilePath()
{
    std::string date = GetCurrentDate();

    if ((date == g_currentDate) && IsFilePathValid(g_currentLogFilePath)) {
        return true;
    }
    LOGE("Current log file path invalid: %s", g_currentLogFilePath.c_str());

    g_currentDate = date;

    std::string fileName;
    bool currentDateFlag = false;
    int fileNumber = 0;
    for (const auto& entry : std::__fs::filesystem::directory_iterator(LOG_FILE_DIR)) {
        if (entry.is_regular_file()) {
            fileName = entry.path().filename().string();
            fileNumber = CheckFileNameAndGetNumber(fileName, date, &currentDateFlag);
            if (fileNumber != 0) {
                // 只可能存在一个未被压缩的日志文件，找到则退出循环
                break;
            }
        }
    }

    // 如果没有找到日志文件，则创建第一个
    if (fileNumber == 0) {
        g_currentLogFilePath = GetCurrentLogFilePath(1);
        return true;
    }

    // 找到了日志文件，将文件名转换为文件路径
    std::string filePath = LOG_FILE_DIR + fileName;

    // 判断:1.是否是当天的日志文件;2.文件是否合法
    if (currentDateFlag && IsFilePathValid(filePath)) {
        // 满足则写入现有文件
        g_currentLogFilePath = filePath;
        return true;
    }

    // 不满足则将其压缩
    TarFile(fileName);

    if (fileNumber >= MAX_FILE_COUNT) {
        LOGE("Log file full!");
        return false;
    }

    // 无论压缩是否成功，都创建编号+1的日志文件
    g_currentLogFilePath = GetCurrentLogFilePath(fileNumber + 1);
    return true;
}

int LogCheck(SpLogLevel level, const char *format, va_list *argList, char *buffer, size_t bufSize)
{
    int ret = 0;

    ret = memset_s(argList, sizeof(va_list), 0, sizeof(va_list));
    if (ret != 0) {
        SpLogOut(level, "SP log memset_s error.");
        return -1;
    }

    ret = vsprintf_s(buffer, bufSize, format, *argList);
    if (ret < 0) {
        SpLogOut(level, "SP log length error.");
        return -1;
    }

    return 0;
}

inline void WriteMessage(SpLogLevel logLevel, char* logMessage)
{
    // 如果文件不存在则需要在创建后提权
    bool chmodFlag = !std::__fs::filesystem::exists(g_currentLogFilePath);

    // 打开日志文件并写入
    std::ofstream logFile(g_currentLogFilePath, std::ios::app);
    if (logFile.is_open()) {
        if (chmodFlag) {
            LOGD("Create log file %s.", g_currentLogFilePath.c_str());
            if (!CHMOD_FILE(g_currentLogFilePath)) {
                logFile.close();
                return;
            }
        }

        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);

        std::ostringstream timeStamp;
        timeStamp << std::put_time(&tm, "[%H:%M:%S]");

        logFile << timeStamp.str()
                << logMessage
                << std::endl;

        logFile.close();
    } else {
        LOGE("Unable to open log file for writing: %s", g_currentLogFilePath.c_str());
    }

    return;
}

void SpLog(SpLogLevel logLevel, const char *fmt, bool isEditorLog, ...)
{
    char logBuf[LOG_MAX_LEN] = {0};
    int32_t ret = 0;
    va_list arg;
    ret = memset_s(&arg, sizeof(va_list), 0, sizeof(va_list));
    if (ret != 0) {
        SpLogOut(logLevel, "SP log memset_s error.");
        return;
    }
    va_start(arg, fmt);
    ret = vsprintf_s(logBuf, sizeof(logBuf), fmt, arg);
    va_end(arg);
    if (ret < 0) {
        SpLogOut(logLevel, "SP log length error.");
        return;
    }
    SpLogOut(logLevel, logBuf);

    if(!isEditorLog) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_mtx);

    if (LOG_FILE_DIR.empty()) {
        LOGE("Log file dir not set.");
        return;
    }

    if (!EnsureLogDirectoryExists()) {
        return;
    }

    if (!GetLogFilePath()) {
        return;
    }

    WriteMessage(logLevel, logBuf);

    return;
}
} // namespace SmartPerf
} // namespace OHOS