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

#include <string>
#include <sstream>
#include <mutex>
#include "include/sp_log.h"
#include "include/sp_utils.h"
#include "include/GpuCounter.h"
#include "include/GpuCounterCallback.h"

namespace OHOS {
    namespace SmartPerf {

#define DO_FOR_ALL_VALUE(macro) \
    do { \
        macro(gpuActive, "gpuActive"); \
        macro(drawCalls, "drawCalls"); \
        macro(primitives, "primitives"); \
        macro(vertexCounts, "vertexCounts"); \
        macro(totalInstruments, "totalInstruments"); \
        macro(memoryReadBandwidth, "memoryReadBandwidth"); \
        macro(memoryWriteBandwidth, "memoryWriteBandwidth"); \
    } while (0)
#define DO_FOR_ALL_PERCENTAGE(macro) \
    do { \
        macro(gpuLoadPercentage, "gpuLoadPercentage"); \
        macro(vertexLoadPercentage, "vertexLoadPercentage"); \
        macro(fragmentLoadPercentage, "fragmentLoadPercentage"); \
        macro(computeLoadPercentage, "computeLoadPercentage"); \
        macro(textureLoadPercentage, "textureLoadPercentage"); \
        macro(memoryBandwidthPercentage, "memoryBandwidthPercentage"); \
    } while (0)
        GpuCounterCallback::GpuCounterCallback()
        {
            GpuPerfInfo firstData;
            firstData.startTime = SPUtils::GetCurTime();
            firstData.duration = 0;
            firstData.gpuActive = 0;
            firstData.drawCalls = 0;
            firstData.primitives = 0;
            firstData.vertexCounts = 0;
            firstData.totalInstruments = 0;
            firstData.gpuLoadPercentage = 0;
            firstData.vertexLoadPercentage = 0;
            firstData.fragmentLoadPercentage = 0;
            firstData.computeLoadPercentage = 0;
            firstData.textureLoadPercentage = 0;
            firstData.memoryReadBandwidth = 0;
            firstData.memoryWriteBandwidth = 0;
            firstData.memoryBandwidthPercentage = 0;
            firstData.remainTime = MAX_TIME;
            realtimeGpuPerfInfoData = firstData;
            gpuCounter.push_back(firstData);
        }

        void GpuCounterCallback::JoinSocketData(GpuPerfInfo *newData)
        {
#define JOIN_PERCENTAGE(field, fieldString) \
    do { \
        unsigned long long targetValue = \
        (((unsigned long long)realtimeGpuPerfInfoData.field * (unsigned long long)realtimeGpuPerfInfoData.duration) + \
            ((unsigned long long)newData->field * (unsigned long long)newData->duration)) / \
            ((unsigned long long)realtimeGpuPerfInfoData.duration + (unsigned long long)newData->duration); \
        realtimeGpuPerfInfoData.field = targetValue; \
    } while (0)
#define JOIN_VALUE(field, fieldString) \
    do { \
        realtimeGpuPerfInfoData.field += newData->field; \
    } while (0)
            DO_FOR_ALL_VALUE(JOIN_VALUE);
            DO_FOR_ALL_PERCENTAGE(JOIN_PERCENTAGE);
            realtimeGpuPerfInfoData.duration += newData->duration;
        }
        void GpuCounterCallback::SplitSocketData()
        {
            int32_t interval = realtimeGpuPerfInfoData.duration - MAX_DURATION;
            GpuCounter &GpuCounter = GpuCounter::GetInstance();
#define SPLIT_VALUE(field, fieldString) \
    do {                   \
        unsigned long long targetValue = (unsigned long long)realtimeGpuPerfInfoData.field * \
            (unsigned long long)interval / \
            (unsigned long long)realtimeGpuPerfInfoData.duration; \
       GpuCounter.GetGpuCounterRealtimeDataMap()[fieldString] = \
       std::to_string(realtimeGpuPerfInfoData.field - targetValue); \
       realtimeGpuPerfInfoData.field = targetValue; \
    } while (0)

#define COPY_PERCENTAGE(field, fieldString) \
    do { \
        GpuCounter.GetGpuCounterRealtimeDataMap()[fieldString] = std::to_string(realtimeGpuPerfInfoData.field); \
    } while (0)
            if ((interval == 0) || (realtimeGpuPerfInfoData.duration < interval)) {
                return;
            }
            DO_FOR_ALL_VALUE(SPLIT_VALUE);
            DO_FOR_ALL_PERCENTAGE(COPY_PERCENTAGE);
            realtimeGpuPerfInfoData.duration = interval;
            GpuCounter.GetGpuCounterRealtimeDataMap()["duration"] = std::to_string(MAX_DURATION);
        }

        void GpuCounterCallback::GetRealTime(GpuPerfInfo *newData)
        {
            GpuCounter &GpuCounter = GpuCounter::GetInstance();

            JoinSocketData(newData);
            if ((realtimeGpuPerfInfoData.duration == 0) || (newData->duration == 0)) {
                return;
            }
            std::lock_guard<std::mutex> lock(GpuCounter.GetRealtimeDataLock());
            if (GpuCounter.GetGpuCounterRealtimeDataMap().size() == 0 && \
            realtimeGpuPerfInfoData.duration >= MAX_DURATION) {
                SplitSocketData();
            }
        }

        int GpuCounterCallback::OnGpuData(std::vector <GpuPerfInfo> &gpuPerfInfos)
        {
            GpuCounter &GpuCounter = GpuCounter::GetInstance();

            for (auto gpuPerfInfo : gpuPerfInfos) {
                unsigned int gpuCounterBackSize = gpuCounter.size();
                gpuCounter.push_back(gpuPerfInfo);
                unsigned int gpuCounterSize = gpuCounter.size();
                if (gpuCounterSize <= gpuCounterBackSize) {
                    LOGE("gpuCounter data len error!");
                    return -1;
                }
                GpuPerfInfo *newData = &gpuCounter[gpuCounterSize - 1];
                GpuPerfInfo *backData = &gpuCounter[gpuCounterSize - 2];
                long long durationTime = newData->startTime - backData->startTime;

                // 如果两次数据间隔过短，则舍弃新数据
                if (durationTime < COLLECT_INTERVAL) {
                    LOGE("Start time(%lld, %lld) make duration time(%lld) too short",
                         newData->startTime, backData->startTime, durationTime);
                    gpuCounter.pop_back();
                    continue;
                }

                backData->duration = durationTime;

                std::ostringstream oss;
                oss << backData->startTime << ","
                    << backData->duration << ","
                    << backData->gpuActive << ","
                    << backData->drawCalls << ","
                    << backData->primitives << ","
                    << backData->vertexCounts << ","
                    << backData->totalInstruments << ","
                    << backData->gpuLoadPercentage << ","
                    << backData->vertexLoadPercentage << ","
                    << backData->fragmentLoadPercentage << ","
                    << backData->computeLoadPercentage << ","
                    << backData->textureLoadPercentage << ","
                    << backData->memoryReadBandwidth << ","
                    << backData->memoryWriteBandwidth << ","
                    << backData->memoryBandwidthPercentage << ",";
                GpuCounter.GetGpuCounterData().push_back(oss.str());

                GetRealTime(backData);
            }

            if (gpuPerfInfos[0].remainTime <= RESTART_TIME) {
                LOGI("remain time to restart");
                GpuCounter.StartCollect(GpuCounter::GC_RESTART);
            }
            return 0;
        }
    }
}