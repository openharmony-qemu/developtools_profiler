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
        GpuCounterCallbackImpl::GpuCounterCallbackImpl()
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
            firstData.remainTime = maxTime;
            realtimeGpuPerfInfoData = firstData;
            gpuCounter.push_back(firstData);
        }


        unsigned long long GpuCounterCallbackImpl::JoinSocketDataPercentFunction(uint32_t itemFirst, int32_t durationFirst,
            uint32_t itemSecond, int32_t durationSecond)
        {
            return (((unsigned long long)itemFirst * (unsigned long long)durationFirst) +
                ((unsigned long long)itemSecond * (unsigned long long)durationSecond)) /
                ((unsigned long long)durationFirst + (unsigned long long)durationSecond);
        }

        void GpuCounterCallbackImpl::JoinSocketDataValue(GpuPerfInfo *newData)
        {
            realtimeGpuPerfInfoData.gpuActive = JoinSocketDataPercentFunction(
                realtimeGpuPerfInfoData.gpuActive, realtimeGpuPerfInfoData.duration,
                newData->gpuActive, newData->duration);
            realtimeGpuPerfInfoData.drawCalls = JoinSocketDataPercentFunction(
                realtimeGpuPerfInfoData.drawCalls, realtimeGpuPerfInfoData.duration,
                newData->drawCalls, newData->duration);
            realtimeGpuPerfInfoData.primitives = JoinSocketDataPercentFunction(
                realtimeGpuPerfInfoData.primitives, realtimeGpuPerfInfoData.duration,
                newData->primitives, newData->duration);
            realtimeGpuPerfInfoData.vertexCounts = JoinSocketDataPercentFunction(
                realtimeGpuPerfInfoData.vertexCounts, realtimeGpuPerfInfoData.duration,
                newData->vertexCounts, newData->duration);
            realtimeGpuPerfInfoData.totalInstruments = JoinSocketDataPercentFunction(
                realtimeGpuPerfInfoData.totalInstruments, realtimeGpuPerfInfoData.duration,
                newData->totalInstruments, newData->duration);
            realtimeGpuPerfInfoData.memoryReadBandwidth = JoinSocketDataPercentFunction(
                realtimeGpuPerfInfoData.memoryReadBandwidth, realtimeGpuPerfInfoData.duration,
                newData->memoryReadBandwidth, newData->duration);
            realtimeGpuPerfInfoData.memoryWriteBandwidth = JoinSocketDataPercentFunction(
                realtimeGpuPerfInfoData.memoryWriteBandwidth, realtimeGpuPerfInfoData.duration,
                newData->memoryWriteBandwidth, newData->duration);
        }

        void GpuCounterCallbackImpl::JoinSocketDataPercent(GpuPerfInfo *newData)
        {
            realtimeGpuPerfInfoData.gpuLoadPercentage += newData->gpuLoadPercentage;
            realtimeGpuPerfInfoData.vertexLoadPercentage += newData->vertexLoadPercentage;
            realtimeGpuPerfInfoData.fragmentLoadPercentage += newData->fragmentLoadPercentage;
            realtimeGpuPerfInfoData.computeLoadPercentage += newData->computeLoadPercentage;
            realtimeGpuPerfInfoData.textureLoadPercentage += newData->textureLoadPercentage;
            realtimeGpuPerfInfoData.memoryBandwidthPercentage += newData->memoryBandwidthPercentage;
        }

        void GpuCounterCallbackImpl::JoinSocketData(GpuPerfInfo *newData)
        {
            JoinSocketDataValue(newData);
            JoinSocketDataPercent(newData);

            realtimeGpuPerfInfoData.duration += newData->duration;
        }

        void GpuCounterCallbackImpl::SplitSocketDataValue(int32_t interval)
        {
            GpuCounter &gpuCounter = GpuCounter::GetInstance();

            unsigned long long gpuActiveTargetValue =
                (unsigned long long)realtimeGpuPerfInfoData.gpuActive *
                (unsigned long long)interval / (unsigned long long)realtimeGpuPerfInfoData.duration;;
            unsigned long long drawCallsTargetValue =
                (unsigned long long)realtimeGpuPerfInfoData.drawCalls *
                (unsigned long long)interval / (unsigned long long)realtimeGpuPerfInfoData.duration;
            unsigned long long primitivesTargetValue =
                (unsigned long long)realtimeGpuPerfInfoData.primitives *
                (unsigned long long)interval / (unsigned long long)realtimeGpuPerfInfoData.duration;
            unsigned long long vertexCountsTargetValue =
                (unsigned long long)realtimeGpuPerfInfoData.vertexCounts *
                (unsigned long long)interval / (unsigned long long)realtimeGpuPerfInfoData.duration;
            unsigned long long totalInstrumentsTargetValue =
                (unsigned long long)realtimeGpuPerfInfoData.totalInstruments *
                (unsigned long long)interval / (unsigned long long)realtimeGpuPerfInfoData.duration;
            unsigned long long memoryReadBandwidthTargetValue =
                (unsigned long long)realtimeGpuPerfInfoData.memoryReadBandwidth *
                (unsigned long long)interval / (unsigned long long)realtimeGpuPerfInfoData.duration;
            unsigned long long memoryWriteBandwidthTargetValue =
                (unsigned long long)realtimeGpuPerfInfoData.memoryWriteBandwidth *
                (unsigned long long)interval / (unsigned long long)realtimeGpuPerfInfoData.duration;

            gpuCounter.GetGpuCounterRealtimeDataMap()["gpuActive"] =
                std::to_string(realtimeGpuPerfInfoData.gpuActive - gpuActiveTargetValue);
            gpuCounter.GetGpuCounterRealtimeDataMap()["drawCalls"] =
                std::to_string(realtimeGpuPerfInfoData.drawCalls - drawCallsTargetValue);
            gpuCounter.GetGpuCounterRealtimeDataMap()["primitives"] =
                std::to_string(realtimeGpuPerfInfoData.primitives - primitivesTargetValue);
            gpuCounter.GetGpuCounterRealtimeDataMap()["vertexCounts"] =
                std::to_string(realtimeGpuPerfInfoData.vertexCounts - vertexCountsTargetValue);
            gpuCounter.GetGpuCounterRealtimeDataMap()["totalInstruments"] =
                std::to_string(realtimeGpuPerfInfoData.totalInstruments - totalInstrumentsTargetValue);
            gpuCounter.GetGpuCounterRealtimeDataMap()["memoryReadBandwidth"] =
                std::to_string(realtimeGpuPerfInfoData.memoryReadBandwidth - memoryReadBandwidthTargetValue);
            gpuCounter.GetGpuCounterRealtimeDataMap()["memoryWriteBandwidth"] =
                std::to_string(realtimeGpuPerfInfoData.memoryWriteBandwidth - memoryWriteBandwidthTargetValue);

            realtimeGpuPerfInfoData.gpuActive = gpuActiveTargetValue;
            realtimeGpuPerfInfoData.drawCalls = drawCallsTargetValue;
            realtimeGpuPerfInfoData.primitives = primitivesTargetValue;
            realtimeGpuPerfInfoData.vertexCounts = vertexCountsTargetValue;
            realtimeGpuPerfInfoData.totalInstruments = totalInstrumentsTargetValue;
            realtimeGpuPerfInfoData.memoryReadBandwidth = memoryReadBandwidthTargetValue;
            realtimeGpuPerfInfoData.memoryWriteBandwidth = memoryWriteBandwidthTargetValue;
        }

        void GpuCounterCallbackImpl::SplitSocketDataPercent()
        {
            GpuCounter &gpuCounter = GpuCounter::GetInstance();

            gpuCounter.GetGpuCounterRealtimeDataMap()["gpuLoadPercentage"] =
                std::to_string(realtimeGpuPerfInfoData.gpuLoadPercentage);
            gpuCounter.GetGpuCounterRealtimeDataMap()["vertexLoadPercentage"] =
                std::to_string(realtimeGpuPerfInfoData.vertexLoadPercentage);
            gpuCounter.GetGpuCounterRealtimeDataMap()["fragmentLoadPercentage"] =
                std::to_string(realtimeGpuPerfInfoData.fragmentLoadPercentage);
            gpuCounter.GetGpuCounterRealtimeDataMap()["computeLoadPercentage"] =
                std::to_string(realtimeGpuPerfInfoData.computeLoadPercentage);
            gpuCounter.GetGpuCounterRealtimeDataMap()["textureLoadPercentage"] =
                std::to_string(realtimeGpuPerfInfoData.textureLoadPercentage);
            gpuCounter.GetGpuCounterRealtimeDataMap()["memoryBandwidthPercentage"] =
                std::to_string(realtimeGpuPerfInfoData.memoryBandwidthPercentage);
        }

        void GpuCounterCallbackImpl::SplitSocketData()
        {
            int32_t interval = realtimeGpuPerfInfoData.duration - maxDuration;
            GpuCounter &gpuCounter = GpuCounter::GetInstance();

            SplitSocketDataValue(interval);
            SplitSocketDataPercent();

            if ((interval == 0) || (realtimeGpuPerfInfoData.duration < interval)) {
                return;
            }

            realtimeGpuPerfInfoData.duration = interval;
            gpuCounter.GetGpuCounterRealtimeDataMap()["duration"] = std::to_string(maxDuration);
        }

        void GpuCounterCallbackImpl::GetRealTime(GpuPerfInfo *newData)
        {
            GpuCounter &gpuCounter = GpuCounter::GetInstance();

            JoinSocketData(newData);
            if ((realtimeGpuPerfInfoData.duration == 0) || (newData->duration == 0)) {
                return;
            }
            std::lock_guard<std::mutex> lock(gpuCounter.GetRealtimeDataLock());
            if (gpuCounter.GetGpuCounterRealtimeDataMap().size() == 0 && \
            realtimeGpuPerfInfoData.duration >= maxDuration) {
                SplitSocketData();
            }
        }

        int GpuCounterCallbackImpl::OnGpuData(std::vector <GpuPerfInfo> &gpuPerfInfos)
        {
            GpuCounter &gpuCounterInstance = GpuCounter::GetInstance();

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
                if (durationTime < collectInterval) {
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
                gpuCounterInstance.GetGpuCounterData().push_back(oss.str());

                GetRealTime(backData);
            }

            if (gpuPerfInfos[0].remainTime <= restartTime) {
                LOGI("remain time to restart");
                gpuCounterInstance.StartCollect(GpuCounter::GC_RESTART);
            }
            return 0;
        }
    }
}