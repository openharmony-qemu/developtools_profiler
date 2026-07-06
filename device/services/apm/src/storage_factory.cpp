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

#include "storage_factory.h"

#include "common.h"
#include "logging.h"
#include "i_data_storage.h"
#include "zip_storage.h"

namespace {
const std::string ZIP_FILE_SUFFIX = ".zip";
}

bool StorageFactory::StringEndsWith(const std::string &string, const std::string &suffix)
{
    if (string.empty() || suffix.empty() || string.length() < suffix.length()) {
        return false;
    }
    return string.rfind(suffix) == (string.length() - suffix.length());
}

std::shared_ptr<IDataStorage> StorageFactory::Create(std::string &destination)
{
    CHECK_TRUE(!destination.empty(), nullptr, "empty destination");
    if (StringEndsWith(destination, ZIP_FILE_SUFFIX)) {
        auto retFile = COMMON::CheckNotExistsFilePath(destination);
        if (retFile.first) {
            return std::static_pointer_cast<IDataStorage>(std::make_shared<ZipStorage>(retFile.second));
        }
    }
    return nullptr;
}