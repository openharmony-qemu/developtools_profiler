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

#include "rawheap_collector.h"

#include <chrono>
#include <cstdio>
#include <thread>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include "app_mgr_interface.h"
#include "app_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "app_jsheap_mem_info.h"
#include "common.h"
#include "logging.h"
#include "file_monitor.h"

namespace {
const std::string SRC_RAWHEAP_FILE_DIR = "/data/log/faultlog/temp";
const std::string DEST_RAWHEAP_FILE_DIR = "/data/local/tmp";
#if defined(__arm__)
const std::string METADATA_PATH = "/system/lib/module/arkcompiler/metadata.json";
#else
const std::string METADATA_PATH = "/system/lib64/module/arkcompiler/metadata.json";
#endif
constexpr int32_t WAIT_FOR_RAWHEAP_FILE_READY_MS = 60 * 1000;
}

bool RawheapCollector::Collect(std::shared_ptr<IDataStorage> storage)
{
    PROFILER_LOG_DEBUG(LOG_CORE,  "start collect rawheap, pid: %u", pid_);
    CHECK_NOTNULL(storage, false, "storage is NULL");
    CHECK_TRUE(pid_ > 0, false, "invalid pid");

    std::string prefix = "jsheap-" + std::to_string(pid_) + "-" + std::to_string(tid_) + "-";
    std::string suffix = ".rawheap";
    auto filer = [&prefix, &suffix](std::string& filename) {
        return filename.length() >= prefix.length() + suffix.length() &&
            filename.compare(0, prefix.length(), prefix) == 0 &&
            filename.compare(filename.length() - suffix.length(), suffix.length(), suffix) == 0;
    };
    FileMonitor monitor(SRC_RAWHEAP_FILE_DIR, filer);
    if (!DumpRawheap() || !monitor.Start()) {
        PROFILER_LOG_ERROR(LOG_CORE,  "DumpJsHeapMemory failed");
        monitor.Finalize();
        return false;
    }
    std::string resultFilename = monitor.GetResultFile(WAIT_FOR_RAWHEAP_FILE_READY_MS);
    monitor.Finalize();
    CHECK_TRUE(!resultFilename.empty(), false, "Dump rawheap file fail");
    PROFILER_LOG_DEBUG(LOG_CORE,  "Dump rawheap file success, pid: %u", pid_);
    std::string srcPath = SRC_RAWHEAP_FILE_DIR + "/" + resultFilename;
    std::string destPath = DEST_RAWHEAP_FILE_DIR + "/" + resultFilename;
    CHECK_TRUE(COMMON::CopyFileFastFromPath(srcPath, destPath), false, "copy rawheap file fail");
    if (!AppendMetaData(destPath) || !storage->AddFile(destPath)) {
        PROFILER_LOG_ERROR(LOG_CORE,  "storage rawheap file fail");
        (void)unlink(destPath.c_str());
        return false;
    }
    (void)unlink(destPath.c_str());
    return true;
}

bool RawheapCollector::DumpRawheap()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> samgr =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_NOTNULL(samgr, false, "samgr is NULL");
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager =
        OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(samgr->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID));
    CHECK_NOTNULL(appManager, false, "appManager is NULL");

    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = pid_;
    info.tid = tid_;
    info.needGc = false;
    info.needSnapshot = true;
    info.needLeakobj = false;
    info.needBinary = true;
    info.needClearNodeIdCache = false;
    info.needProcDump = false;
    int ret = appManager->DumpJsHeapMemory(info);
    if (ret != 0) {
        PROFILER_LOG_ERROR(LOG_CORE,  "Call DumpJsHeapMemory failed, ret:%d", ret);
        return false;
    }
    return true;
}

bool RawheapCollector::AppendMetaData(const std::string& filepath)
{
    struct stat fileInfo;
    CHECK_TRUE(stat(filepath.c_str(), &fileInfo) == 0, false, "get file stat fail");
    uint32_t fileSize = static_cast<uint32_t>(fileInfo.st_size);
    CHECK_TRUE(fileSize > 0, false, "file is empty");

    struct stat metadataInfo;
    CHECK_TRUE(stat(METADATA_PATH.c_str(), &metadataInfo) == 0, false, "get metadata stat fail");
    uint32_t metaSize = static_cast<uint32_t>(metadataInfo.st_size);

    std::ifstream finMetadata(METADATA_PATH);
    CHECK_TRUE(finMetadata.is_open(), false, "open meta file fail");
    std::ofstream fout;
    fout.open(filepath.c_str(), std::ios::out | std::ios::app);
    if (!fout.is_open()) {
        PROFILER_LOG_ERROR(LOG_CORE,  "open file fail: %s", filepath.c_str());
        finMetadata.close();
        return false;
    }
    fout << finMetadata.rdbuf();
    fout.write(reinterpret_cast<const char *>(&fileSize), sizeof(fileSize));
    fout.write(reinterpret_cast<const char *>(&metaSize), sizeof(metaSize));
    fout.close();
    finMetadata.close();
    PROFILER_LOG_DEBUG(LOG_CORE,  "append metadata done");
    return true;
}