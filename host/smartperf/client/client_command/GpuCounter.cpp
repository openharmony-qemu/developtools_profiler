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

#include "chrono"
#include "string"
#include "thread"
#include "fstream"
#include <iostream>
#include <dlfcn.h>
#include "include/sp_log.h"
#include "include/GpuCounter.h"
#include "include/GpuCounterCallback.h"
#include "include/service_plugin.h"

namespace OHOS {
    namespace SmartPerf {
        std::map<std::string, std::string> GpuCounter::ItemData()
        {
            return std::map<std::string, std::string>();
        }

        void GpuCounter::StartCollect(GcCollectType type)
        {
            std::unique_ptr<GpuCounterCallback> gpuCounterCallback = std::make_unique<GpuCounterCallbackImpl>();

            const int duration = 1000;

            ServicePluginHandler &servicePluginHandler = ServicePluginHandler::GetInstance();
            void* handle = servicePluginHandler.GetSoHandler(ServicePluginHandler::ServicePluginType::GAME_PLUGIN);
            if (!handle) {
                WLOGE("Get service plugin handler failed.");
                return;
            }

            typedef GameServicePlugin *(*GetServicePlugin)();
            GetServicePlugin servicePlugin = (GetServicePlugin)dlsym(handle, createPlugin.c_str());
            if (!servicePlugin) {
                WLOGE("GameServicePlugin Error loading symbol");
                return;
            }

            if (type == GC_START && gcStatus == GC_INIT) {
                gpuCounterData.clear();
                gpuCounterRealtimeData.clear();
                int ret = servicePlugin()->StartGetGpuPerfInfo(duration, std::move(gpuCounterCallback));
                if (ret == 0) {
                    gcStatus = GC_RUNNING;
                } else {
                    WLOGE("GpuCounter call gameService error, ret = %d", ret);
                }
            } else if (type == GC_RESTART && gcStatus == GC_RUNNING) {
                int ret = servicePlugin()->StartGetGpuPerfInfo(duration, std::move(gpuCounterCallback));
                if (ret != 0) {
                    WLOGE("GpuCounter call gameService error, ret = %d", ret);
                }
            } else {
                WLOGE("GpuCounter state error, type: %d, state: %d", type, gcStatus);
            }
        }

        void GpuCounter::SaveData(std::string path)
        {
            if (gcStatus != GC_RUNNING || gpuCounterData.size() <= 0) {
                return;
            }
            char gpuCounterDataDirChar[PATH_MAX] = {0x00};
            if (realpath(path.c_str(), gpuCounterDataDirChar) == nullptr) {
                WLOGE("data dir %s is nullptr", path.c_str());
                return;
            }
            std::string gpuCounterDataPath = std::string(gpuCounterDataDirChar) + "/gpu_counter.csv";
            std::ofstream outFile;
            std::mutex mtx;
            mtx.lock();
            outFile.open(gpuCounterDataPath.c_str(), std::ios::out | std::ios::trunc);
            if (!outFile.is_open()) {
                WLOGE("open GpuCounter data file failed.");
                return;
            }
            std::string title = "startTime,"
                "duration,"
                "gpuActive,"
                "drawCalls,"
                "primitives,"
                "vertexCounts,"
                "totalInstruments,"
                "gpuLoadPercentage,"
                "vertexLoadPercentage,"
                "fragmentLoadPercentage,"
                "computeLoadPercentage,"
                "textureLoadPercentage,"
                "memoryReadBandwidth,"
                "memoryWriteBandwidth,"
                "memoryBandwidthPercentage\r";
            outFile << title << std::endl;
            for (unsigned int i = 0; i < gpuCounterSaveReportData.size() - 1; i++) {
                outFile << gpuCounterSaveReportData[i] << std::endl;
            }
            outFile.close();
            mtx.unlock();
        }

        std::vector<std::string> &GpuCounter::GetGpuCounterData()
        {
            return gpuCounterData;
        }

        std::vector<std::string> &GpuCounter::GetGpuCounterSaveReportData()
        {
            return gpuCounterSaveReportData;
        }

        std::mutex &GpuCounter::GetRealtimeDataLock()
        {
            return realtimeDataLock;
        }

        std::string &GpuCounter::GetGpuCounterRealtimeData()
        {
            return gpuCounterRealtimeData;
        }
        void GpuCounter::AddGpuCounterRealtimeData(std::string dataString)
        {
            gpuCounterRealtimeData += dataString;
        }

        void GpuCounter::GetGpuRealtimeData(std::map<std::string, std::string> &dataMap)
        {
            if (gpuCounterRealtimeData.size() > 0) {
                std::map<std::string, std::string> gpuCounterRealtimeDataMap;
                gpuCounterRealtimeDataMap["gpuCounterData"] = gpuCounterRealtimeData;
                realtimeDataLock.lock();
                dataMap.insert(gpuCounterRealtimeDataMap.begin(), gpuCounterRealtimeDataMap.end());
                realtimeDataLock.unlock();
                gpuCounterRealtimeData.clear();
            }
        }

        void GpuCounter::StopCollect()
        {
            if (gcStatus != GC_RUNNING) {
                return;
            }
            ServicePluginHandler &servicePluginHandler = ServicePluginHandler::GetInstance();
            void* handle = servicePluginHandler.GetSoHandler(ServicePluginHandler::ServicePluginType::GAME_PLUGIN);
            if (!handle) {
                WLOGE("Get service plugin handler failed.");
                return;
            }

            typedef GameServicePlugin *(*GetServicePlugin)();
            GetServicePlugin servicePlugin = (GetServicePlugin)dlsym(handle, createPlugin.c_str());
            if (!servicePlugin) {
                WLOGE("GameServicePlugin Error loading symbol");
                return;
            }

            int ret = servicePlugin()->StopGetGpuPerfInfo();
            if (ret == 0) {
                gcStatus = GC_INIT;
            }
        }
    }
}