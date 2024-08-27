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

#include "include/GPU_Counter.h"

namespace OHOS {
    namespace SmartPerf {
        std::map<std::string, std::string> GPU_Counter::ItemData()
        {
            return std::map<std::string, std::string>();
        }

        void GPU_Counter::StartCollect(GcCollectType type)
        {
        }

        void GPU_Counter::SaveData(std::string path)
        {
        }

        std::vector<std::string> &GPU_Counter::GetGpuCounterData()
        {
            static std::vector<std::string> defaultVec;
            return defaultVec;
        }

        std::mutex &GPU_Counter::GetRealtimeDataLock()
        {
            static std::mutex defaultMutex;
            return defaultMutex;
        }

        std::map<std::string, std::string> &GPU_Counter::GetGpuCounterRealtimeDataMap()
        {
            static std::map<std::string, std::string> defaultMap;
            return defaultMap;
        }

        void GPU_Counter::GetGpuRealtimeData(std::map<std::string, std::string> &dataMap)
        {
        }

        void GPU_Counter::StopCollect(std::string path)
        {
        }
    }
}