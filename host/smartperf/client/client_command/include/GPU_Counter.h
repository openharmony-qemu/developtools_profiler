/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef GPU_COUNTER_H
#define GPU_COUNTER_H

#include "string"
#include "vector"
#include "sp_profiler.h"
#include "thread"
#include "mutex"

namespace OHOS {
    namespace SmartPerf {
        class GPU_Counter : public SpProfiler {
        public:
            enum GcStatus {
                GC_INIT = 0,
                GC_RUNNING,
            };

            enum GcCollectType {
                GC_START = 0,
                GC_RESTART,
            };

        public:
            std::map<std::string, std::string> ItemData() override;

            static GPU_Counter &GetInstance()
            {
                static GPU_Counter instance;
                return instance;
            }
            void StartCollect(GcCollectType type);
            void StopCollect(std::string path);
            std::vector<std::string> &GetGpuCounterData();
            std::mutex &GetRealtimeDataLock();
            std::map<std::string, std::string> &GetGpuCounterRealtimeDataMap();
            void GetGpuRealtimeData(std::map<std::string, std::string> &dataMap);
        private:
            GPU_Counter() {};
            GPU_Counter(const GPU_Counter &);
            GPU_Counter &operator = (const GPU_Counter &);
            GcStatus gcStatus = GC_INIT;
            std::vector<std::string> gpuCounterData;    //保存 std::vector<OHOS::GameService::GpuPerfInfo> gpuCounter; 的数据
            std::mutex realtimeDataLock;
            std::map<std::string, std::string> gpuCounterRealtimeDataMap;
            void SaveData(std::string path);
        };
    };
}


#endif
