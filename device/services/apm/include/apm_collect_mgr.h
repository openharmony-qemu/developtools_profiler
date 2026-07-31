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

#ifndef APM_COLLECT_MGR_H
#define APM_COLLECT_MGR_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "i_data_storage.h"
#include "i_data_collector.h"

class ApmCollectMgr {
public:
    ApmCollectMgr(std::string destination);
    ~ApmCollectMgr();
    bool RegisterCollector(std::shared_ptr<IDataCollector> collector);
    bool Collect();

private:
    std::string destination_;
    std::shared_ptr<IDataStorage> storage_;
    std::vector<std::shared_ptr<IDataCollector>> collectors_;
};

#endif // APM_COLLECT_MGR_H
