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
 *
 * Description: FileUtils implements
 */
#include "file_utils.h"

#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex>
#include "logging.h"

namespace {
constexpr size_t DEFAULT_READ_SIZE = 4096;
}

std::string FileUtils::ReadFile(int fd)
{
    std::string content;
    size_t count = 0;
    while (true) {
        if (content.size() - count < DEFAULT_READ_SIZE) {
            content.resize(content.size() + DEFAULT_READ_SIZE);
        }
        ssize_t nBytes = TEMP_FAILURE_RETRY(read(fd, &content[count], content.size() - count));
        if (nBytes == -1 && errno == EAGAIN) {
            continue;
        }
        if (nBytes <= 0) {
            break;
        }
        count += static_cast<size_t>(nBytes);
    }
    content.resize(count);
    return content;
}

std::string FileUtils::ReadFile(const std::string& path)
{
    char realPath[PATH_MAX + 1] = {0};
    CHECK_TRUE((path.length() < PATH_MAX) && (realpath(path.c_str(), realPath) != nullptr), "",
               "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
    FILE* fp = fopen(realPath, "r");
    if (fp == nullptr) {
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_WARN(LOG_CORE, "open file %s FAILED: %s!", path.c_str(), buf);
        return "";
    }
    char buffer[DEFAULT_READ_SIZE] = {0};
    size_t itemsRead = 0;
    std::string content;
    while ((itemsRead = fread(buffer, 1, DEFAULT_READ_SIZE - 1, fp)) > 0) {
        content.append(buffer, itemsRead);
    }
    if (ferror(fp)) {
        PROFILER_LOG_ERROR(LOG_CORE, "read file error");
        fclose(fp);
        return "";
    }
    if (fclose(fp) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "fclose file error");
        return content;
    }
    return content;
}

int FileUtils::WriteFile(const std::string& path, const std::string& content)
{
    return WriteFile(path, content, "w");
}

int FileUtils::WriteFile(const std::string& path, const std::string& content, const std::string mode)
{
    CHECK_TRUE(!path.empty() && (path.length() < PATH_MAX), -1,
               "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
    std::regex dirNameRegex("[.~-]");
    std::regex fileNameRegex("[\\/:*?\"<>|]");
    size_t pos = path.rfind("/");
    if (pos != std::string::npos) {
        std::string dirName = path.substr(0, pos+1);
        size_t index = path.length() > (pos + 1) ? path.length() - pos - 1 : 0;
        std::string fileName = path.substr(pos+1, index);
        CHECK_TRUE(!(std::regex_search(dirName, dirNameRegex) || std::regex_search(fileName, fileNameRegex)), -1,
                   "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
    } else {
        CHECK_TRUE(!std::regex_search(path, fileNameRegex), -1,
                   "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
    }
    FILE* fp = fopen(path.c_str(), mode.c_str());
    CHECK_TRUE(fp != nullptr, -1, "open %s failed, %d", path.c_str(), errno);
    size_t ret = fwrite(content.data(), sizeof(char), content.size(), fp);
    if (ret != content.size()) {
        PROFILER_LOG_ERROR(LOG_CORE, "write %s failed, %d", path.c_str(), errno);
        (void)fclose(fp);
        return -1;
    }
    if (fclose(fp) != 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "close file %s failed, %d", path.c_str(), errno);
        return -1;
    }
    return ret;
}

std::vector<std::string> FileUtils::ListDir(const std::string& dirPath)
{
    std::vector<std::string> result;
    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        return result;
    }

    struct dirent* ent = nullptr;
    while ((ent = readdir(dir)) != nullptr) {
        std::string name = ent->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        result.push_back(name);
    }
    closedir(dir);
    return result;
}
