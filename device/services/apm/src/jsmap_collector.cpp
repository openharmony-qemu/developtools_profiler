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

#include "jsmap_collector.h"

#include <chrono>
#include <thread>
#include <unistd.h>
#include "app_mgr_interface.h"
#include "app_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "app_jsheap_mem_info.h"
#include "common.h"
#include "logging.h"
#include "file_monitor.h"

namespace {
const std::string SRC_JSMAP_FILE_DIR = "/data/log/faultlog/temp";
const std::string DEST_JSMAP_FILE_DIR = "/data/local/tmp";
constexpr int32_t WAIT_FOR_JSMAP_FILE_READY_MS = 20 * 1000;
}

bool JsmapCollector::Collect(std::shared_ptr<IDataStorage> storage)
{
    PROFILER_LOG_DEBUG(LOG_CORE,  "start collect jshandle map, pid: %u", pid_);
    CHECK_NOTNULL(storage, false, "storage is NULL");
    CHECK_TRUE(pid_ > 0, false, "invalid pid");

    std::string prefix = "js_map-" + std::to_string(pid_) + "-";
    auto filer = [&prefix](std::string& filename) {
        return filename.length() >= prefix.length() && filename.compare(0, prefix.length(), prefix) == 0;
    };
    FileMonitor monitor(SRC_JSMAP_FILE_DIR, filer);
    if (!DumpJsmap() || !monitor.Start()) {
        PROFILER_LOG_ERROR(LOG_CORE,  "DumpJsHandleMap failed");
        monitor.Finalize();
        return false;
    }
    std::string resultFilename = monitor.GetResultFile(WAIT_FOR_JSMAP_FILE_READY_MS);
    monitor.Finalize();
    CHECK_TRUE(!resultFilename.empty(), false, "Dump jshandle map file fail");
    PROFILER_LOG_DEBUG(LOG_CORE,  "Dump jshandle map file succless, pid: %u", pid_);
    std::string srcPath = SRC_JSMAP_FILE_DIR + "/" + resultFilename;
    std::string destPath = DEST_JSMAP_FILE_DIR + "/" + resultFilename + ".txt";
    CHECK_TRUE(COMMON::CopyFileFastFromPath(srcPath, destPath), false, "copy rawheap file fail");
    if (!storage->AddFile(destPath)) {
        PROFILER_LOG_ERROR(LOG_CORE,  "storage rawheap file fail");
        (void)unlink(destPath.c_str());
        return false;
    }
    (void)unlink(destPath.c_str());
    return true;
}

bool JsmapCollector::DumpJsmap()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> samgr =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_NOTNULL(samgr, false, "samgr is NULL");
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager =
        OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(samgr->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID));
    CHECK_NOTNULL(appManager, false, "appManager is NULL");

    OHOS::AppExecFwk::JsHandleMapInfo info;
    info.pid = pid_;
    info.tid = tid_;

    int ret = appManager->DumpJsHandleMap(info);
    if (ret != 0) {
        PROFILER_LOG_ERROR(LOG_CORE,  "Call DumpJsHandleMap failed, ret:%d", ret);
        return false;
    }
    return true;
}