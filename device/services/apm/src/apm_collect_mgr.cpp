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
#include "apm_collect_mgr.h"

#include "logging.h"
#include "storage_factory.h"

ApmCollectMgr::ApmCollectMgr(std::string destination) : destination_(destination)
{
    storage_ = StorageFactory::Create(destination);
}

ApmCollectMgr::~ApmCollectMgr()
{
    collectors_.clear();
    storage_ = nullptr;
}

bool ApmCollectMgr::RegisterCollector(std::shared_ptr<IDataCollector> collector)
{
    CHECK_NOTNULL(collector, false, "collector is NULL");
    collectors_.emplace_back(collector);
    return true;
}

bool ApmCollectMgr::Collect()
{
    CHECK_NOTNULL(storage_, false, "storage is NULL");
    CHECK_TRUE(collectors_.size() > 0, false, "none collector exists");
    for (auto& collector : collectors_) {
        if (!collector || !collector->Collect(storage_)) {
            PROFILER_LOG_ERROR(LOG_CORE, "collector collect fail");
            return false;
        }
    }
    return true;
}