/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
 */

#include <climits>
#include <cstdint>
#include <dlfcn.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <malloc.h>
#include <new>
#include <pthread.h>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <sys/prctl.h>
#include <unordered_map>
#include <unordered_set>
#include "logging.h"
#include "parameters.h"
#include "preload_client.h"
#include "preload_socket_client.h"

bool GetPreloadSoList(const char* appName, char* buf, size_t len)
{
    if (appName == nullptr || buf == nullptr || len == 0) {
        HILOG_BASE_ERROR(LOG_CORE, "param invalid");
        return false;
    }

    std::shared_ptr<PreloadSocketClient> client = std::make_shared<PreloadSocketClient>(appName);
    if (!client->GetConnectState()) {
        HILOG_BASE_ERROR(LOG_CORE, "connect to native daemon failed");
        return false;
    }

    if (!client->Wait()) {
        HILOG_BASE_ERROR(LOG_CORE, "wait native daemon socket msg timeout");
        return false;
    }

    std::string path = client->GetDiagPath();
    HILOG_BASE_INFO(LOG_CORE, "get diag path from native_daemon:%{public}s", path.c_str());

    if (path.size() >= len) {
        HILOG_BASE_ERROR(LOG_CORE, "diag path size too big");
        return false;
    }

    if (memcpy_s(buf, len, path.c_str(), path.size()) != 0) {
        HILOG_BASE_ERROR(LOG_CORE, "memcpy_s arkts diag path failed");
        return false;
    }
    buf[path.size()] = '\0';

    return true;
}
