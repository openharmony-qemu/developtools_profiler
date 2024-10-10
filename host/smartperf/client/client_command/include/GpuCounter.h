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
#include "GpuCounterCallback.h"
#include "thread"
#include "mutex"

namespace OHOS {
    namespace SmartPerf {
        class GpuCounterPlugin {
        public:
            uint32_t version;
            const char *pluginName;

            virtual int32_t StartGetGpuPerfInfo(int64_t duration, std::unique_ptr <GpuCounterCallback> callback) = 0;
            virtual int32_t StopGetGpuPerfInfo() = 0;
        };

        class GpuCounter : public SpProfiler {
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

            static GpuCounter &GetInstance()
            {
                static GpuCounter instance;
                return instance;
            }
            void StartCollect(GcCollectType type);
            void StopCollect();
            std::vector<std::string> &GetGpuCounterData();
            std::vector<std::string> &GetGpuCounterSaveReportData();
            std::mutex &GetRealtimeDataLock();
            std::string &GetGpuCounterRealtimeData();
            void AddGpuCounterRealtimeData(std::string dataString);
            void GetGpuRealtimeData(std::map<std::string, std::string> &dataMap);
            void SaveData(std::string path);
        private:
            GpuCounter() {};
            GpuCounter(const GpuCounter &);
            GpuCounter &operator = (const GpuCounter &);
            void* GetSoHandle();
            GcStatus gcStatus = GC_INIT;
            std::vector<std::string> gpuCounterData;
            std::vector<std::string> gpuCounterSaveReportData;
            std::mutex realtimeDataLock;
            std::string gpuCounterRealtimeData;
            const std::string PLUGIN_SO_PATH = "system/lib64/libgameservice_gpucounter_plugin.z.so";
            const std::string CREATE_PLUGIN = "onCreatePlugin";
        };
    };
}


#endif
