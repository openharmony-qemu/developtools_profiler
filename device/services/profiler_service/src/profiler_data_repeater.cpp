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
#include "profiler_data_repeater.h"

ProfilerDataRepeater::ProfilerDataRepeater(size_t maxSize)
{
    maxSize_ = maxSize;
    closed_ = false;
}

ProfilerDataRepeater::~ProfilerDataRepeater()
{
    Close();
}

size_t ProfilerDataRepeater::Size()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return dataQueue_.size();
}

void ProfilerDataRepeater::Reset()
{
    std::unique_lock<std::mutex> lock(mutex_);
    closed_ = false;
}

void ProfilerDataRepeater::Close()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        dataQueue_.clear();
        closed_ = true;
    }
    slotCondVar_.notify_all();
    itemCondVar_.notify_all();
}

bool ProfilerDataRepeater::PutPluginData(const ProfilerPluginDataPtr& pluginData)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if ((pluginData == nullptr) && (dataQueue_.size() > 0)) {
        HILOG_INFO(LOG_CORE, "no need put nullptr if queue has data, dataQueue_.size() = %zu", dataQueue_.size());
        return true;
    }

    while (dataQueue_.size() >= maxSize_ && !closed_) {
        slotCondVar_.wait(lock);
    }
    if (closed_) {
        return false;
    }

    dataQueue_.push_back(pluginData);
    lock.unlock();

    itemCondVar_.notify_one();
    return true;
}

ProfilerPluginDataPtr ProfilerDataRepeater::TakePluginData()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (dataQueue_.empty() && !closed_) {
        itemCondVar_.wait(lock);
    }
    if (closed_) {
        return nullptr;
    }

    auto result = dataQueue_.front();
    dataQueue_.pop_front();
    lock.unlock();

    slotCondVar_.notify_one();
    return result;
}

int ProfilerDataRepeater::TakePluginData(std::vector<ProfilerPluginDataPtr>& pluginDataVec)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (dataQueue_.empty() && !closed_) {
        itemCondVar_.wait(lock);
    }
    if (closed_) {
        return -1;
    }

    int count = 0;
    while (dataQueue_.size() > 0) {
        auto result = dataQueue_.front();
        pluginDataVec.push_back(result);
        dataQueue_.pop_front();
        count++;
    }
    lock.unlock();

    slotCondVar_.notify_one();
    return count;
}

void ProfilerDataRepeater::ClearQueue()
{
    std::unique_lock<std::mutex> lock(mutex_);
    dataQueue_.clear();
}
