/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PROFILER_DATA_REPEATER_H
#define PROFILER_DATA_REPEATER_H

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include "logging.h"
#include "nocopyable.h"
#include "profiler_service_types.pb.h"

using ProfilerPluginDataPtr = STD_PTR(shared, ProfilerPluginData);

class ProfilerDataRepeater {
public:
    explicit ProfilerDataRepeater(size_t maxSize);
    ~ProfilerDataRepeater();

    bool PutPluginData(const ProfilerPluginDataPtr& pluginData);

    ProfilerPluginDataPtr TakePluginData();

    int TakePluginData(std::vector<ProfilerPluginDataPtr>& pluginDataVec);

    void Close();

    void Reset();

    size_t Size();

    void ClearQueue();

private:
    std::mutex mutex_;
    std::condition_variable slotCondVar_;
    std::condition_variable itemCondVar_;
    std::deque<ProfilerPluginDataPtr> dataQueue_;
    size_t maxSize_;
    bool closed_;

    DISALLOW_COPY_AND_MOVE(ProfilerDataRepeater);
};

using ProfilerDataRepeaterPtr = STD_PTR(shared, ProfilerDataRepeater);

#endif // PROFILER_DATA_REPEATER_H