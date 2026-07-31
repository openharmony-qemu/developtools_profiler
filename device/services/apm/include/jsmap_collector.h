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

#ifndef JSMAP_COLLECTOR_H
#define JSMAP_COLLECTOR_H

#include <cstdint>
#include <string>

#include "i_data_collector.h"

class JsmapCollector : public IDataCollector {
public:
    JsmapCollector(uint32_t pid, uint32_t tid) : pid_(pid), tid_(tid) {}
    ~JsmapCollector() = default;
    bool Collect(std::shared_ptr<IDataStorage> storage) override;

private:
    bool DumpJsmap();

    uint32_t pid_;
    uint32_t tid_;
};

#endif // JSMAP_COLLECTOR_H