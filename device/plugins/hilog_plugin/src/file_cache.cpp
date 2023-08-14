/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include "file_cache.h"

FileCache::FileCache(const std::string& path) : path_(path), fp_(nullptr)
{
    HILOG_INFO(LOG_CORE, "FileCache: path(%s)!", path_.c_str());
}

FileCache::~FileCache()
{
    fp_ = nullptr;
    return;
}

bool FileCache::Open(const std::string& file)
{
    char realPath[PATH_MAX] = {0};
    CHECK_TRUE(!(path_.empty() || (path_.length() >= PATH_MAX) || (realpath(path_.c_str(), realPath) == nullptr)),
               false, "%s:path is invalid: %s, errno=%d", __func__, path_.c_str(), errno);
    std::string targetFile = std::string(realPath) + std::string("/") + file;
    fp_ = fopen(targetFile.c_str(), "wb+");
    CHECK_NOTNULL(fp_, false, "FileCache: open(%s) Failed, errno(%d)", targetFile.c_str(), errno);
    return true;
}

long FileCache::Write(char* bytes, int32_t len)
{
    CHECK_TRUE((len >= 0) && (bytes != nullptr), -1, "FileCache:%s param invalid!", __func__);
    CHECK_NOTNULL(fp_, -1, "FileCache:%s fp_ invalid!", __func__);

    // write data bytes
    int32_t dataLen = len;
    int32_t writedLen = 0;
    while (writedLen < dataLen) {
        size_t len = fwrite(bytes, sizeof(char), dataLen - writedLen, fp_);
        CHECK_TRUE(len >= 0, -1, "FileCache: write failed, error(%d)!", errno);
        writedLen += len;
    }

    return dataLen;
}

long FileCache::Read(char* content)
{
    CHECK_NOTNULL(content, -1, "FileCache:%s param invalid!", __func__);
    CHECK_NOTNULL(fp_, -1, "FileCache:%s fp_ invalid!", __func__);
    uint64_t readLen = 0;

    // read data bytes
    int ret = fseek(fp_, 0, SEEK_END);
    CHECK_TRUE(ret == 0, -1, "FileCache:%s fseek_end failed, error(%d)!", __func__, errno);
    uint64_t dataLen = static_cast<uint64_t>(ftell(fp_));
    CHECK_TRUE(dataLen > 0, -1, "FileCache:%s ftell failed, error(%d)!", __func__, errno);
    ret = fseek(fp_, 0, SEEK_SET);
    CHECK_TRUE(ret == 0, -1, "FileCache:%s fseek_set failed, error(%d)!", __func__, errno);

    while (readLen < dataLen) {
        size_t len = static_cast<size_t>(fread(content, sizeof(char), dataLen - readLen, fp_));
        CHECK_TRUE(len >= 0, -1, "FileCache:%s read failed, error(%d)!", __func__, errno);
        readLen += len;
    }

    return dataLen;
}

bool FileCache::Close()
{
    CHECK_NOTNULL(fp_, false, "FileCache: %s fp is null", __func__);

    fclose(fp_);
    fp_ = nullptr;

    return true;
}
