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

#ifndef ZIP_STORAGE_H
#define ZIP_STORAGE_H

#include <cstdint>
#include <string>
#include <filesystem>
#include "contrib/minizip/zip.h"
#include "i_data_storage.h"

namespace fs = std::filesystem;

class ZipStorage : public IDataStorage {
public:
    ZipStorage(std::string zipFilePath);
    ~ZipStorage();
    bool AddFile(const std::string &filepath) override;

private:
    bool WriteFileToZip(std::ifstream &file);
    bool GetRealPath(const std::string& path, std::string& realPath);

    std::string zipFilePath_;
    zipFile zipFile_ = nullptr;
};

#endif // ZIP_STORAGE_H