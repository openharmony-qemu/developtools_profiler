/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

#include "logging.h"
#include "common.h"
#include "zip_storage.h"

namespace {
static const int32_t MAX_ZIP_BUFFER_SIZE = 4096;
}

ZipStorage::ZipStorage(std::string zipFilePath)
{
    zipFilePath_ = zipFilePath;
}

ZipStorage::~ZipStorage()
{
    if (zipFile_ != nullptr) {
        zipClose(zipFile_, NULL);
        zipFile_ = nullptr;
    }
}

bool ZipStorage::AddFile(const std::string &filepath)
{
    PROFILER_LOG_DEBUG(LOG_CORE,  "Start storage file:%s", filepath.c_str());
    if (zipFile_ == nullptr) {
        CleanStaleData();
        zipFile_ = zipOpen64(zipFilePath_.c_str(), APPEND_STATUS_CREATE);
        if (zipFile_ == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "zip file handle open failed");
            return false;
        }
    }
    fs::path fsFilePath(filepath);
    CHECK_TRUE(fs::is_regular_file(fsFilePath), false, "not regular file:%s", filepath.c_str());
    std::string realFilePath;
    CHECK_TRUE(GetRealPath(filepath, realFilePath), false, "invalid realFilePath:%s", filepath.c_str());
    zip_fileinfo zipInfo;
    CHECK_TRUE(memset_s(&zipInfo, sizeof(zipInfo), 0, sizeof(zipInfo)) == EOK, false, "init zip info fail");
    std::ifstream file(realFilePath, std::ios::binary);
    CHECK_TRUE(file.is_open(), false, "open file failed,real path:%s", realFilePath.c_str());
    int ret = zipOpenNewFileInZip64(zipFile_, fsFilePath.filename().string().c_str(), &zipInfo, nullptr, 0, nullptr,
        0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 1);
    if (ret != ZIP_OK) {
        PROFILER_LOG_ERROR(LOG_CORE, "open file in zip failed, ret:%d, zipPath:%s", ret, fsFilePath.string().c_str());
        file.close();
        return false;
    }
    bool result = true;
    if (fs::file_size(realFilePath) > 0) {
        result = WriteFileToZip(file);
    }
    zipCloseFileInZip(zipFile_);
    file.close();
    return result;
}

bool ZipStorage::WriteFileToZip(std::ifstream &file)
{
    char buffer[MAX_ZIP_BUFFER_SIZE] = {0};
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        if (file.fail() && !file.eof()) {
            PROFILER_LOG_ERROR(LOG_CORE, "read file error");
            return false;
        }
        auto bytesRead = file.gcount();
        if (bytesRead < 0) {
            long size = bytesRead;
            PROFILER_LOG_ERROR(LOG_CORE,  "read file bytes error,bytesRead:%ld", size);
            return false;
        }
        if (bytesRead == 0) {
            return true;
        }
        if (zipWriteInFileInZip(zipFile_, buffer, bytesRead) < 0) {
            PROFILER_LOG_ERROR(LOG_CORE,  "write file in zip failed");
            return false;
        }
    }
    return true;
}

bool ZipStorage::GetRealPath(const std::string& path, std::string& realPath)
{
    if (path.length() >= PATH_MAX) {
        return false;
    }
    char buffer[PATH_MAX] = {0};
    if (realpath(path.c_str(), buffer) == nullptr) {
        return false;
    }
    realPath = std::string(buffer);
    return true;
}

void ZipStorage::CleanStaleData()
{
    uid_t uid = getuid();
    std::string parentDir = fs::path(zipFilePath_).parent_path().string();
    auto filter = [](const std::string &filename) {
        struct stat st;
        std::string filepath = parentDir + "/" + filename;
        if (stat(filepath.c_str(), &st) != 0) {
            return false;
        }
        return st.st_uid == uid && COMMON::StringMatch(filename, "", ".zip");
    };
    CHECK_TRUE(COMMON::DeleteFiles(parentDir, filter), NO_RETVAL, "clean stale apm data fail");
}