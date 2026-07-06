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
#include <thread>
#include <unistd.h>
#include "app_mgr_interface.h"
#include "app_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "app_jsheap_mem_info.h"
#include "common.h"
#include "logging.h"

namespace {
const std::string SRC_RAWHEAP_FILE_DIR = "/data/log/faultlog/temp";
const std::string DEST_RAWHEAP_FILE_DIR = "/data/log/faultlog/temp";
}

bool RawheapCollector::Collect(std::shared_ptr<IDataStorage> storage)
{
    PROFILER_LOG_DEBUG(LOG_CORE,  "start collect rawheap, pid: %u", pid_);
    CHECK_NOTNULL(storage, false, "storage is NULL");
    CHECK_TRUE(pid_ > 0, false, "invalid pid");
    CHECK_TRUE(tid_ > 0, false, "invalid pid");
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
    CHECK_TRUE(ret == 0, false, "DumpJsHeapMemory failed, ret:%d", ret);

    int waitSeconds = 0;
    std::string file;
    std::string prefix = "jsheap-" + std::to_string(pid_) + "-" + std::to_string(tid_) + "-";
    while (waitSeconds < 5 && file.empty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        file = COMMON::GetLatestFileStartWithPrefix(SRC_RAWHEAP_FILE_DIR, prefix);
        waitSeconds++;
    }
    CHECK_TRUE(!file.empty(), false, "Dump rawheap file fail");
    PROFILER_LOG_DEBUG(LOG_CORE,  "Dump rawheap file succless, pid: %u", pid_);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::string srcPath = SRC_RAWHEAP_FILE_DIR + "/" + file;
    std::string destPath = DEST_RAWHEAP_FILE_DIR + "/" + file + ".rawheap";
    CHECK_TRUE(COMMON::CopyFileFastFromPath(srcPath, destPath), false, "copy rawheap file fail");
    CHECK_TRUE(storage->AddFile(destPath), false, "storage rawheap file fail");
    return true;
}