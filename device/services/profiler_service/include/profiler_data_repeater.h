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

template <typename T>
class ProfilerDataRepeater {
public:
    explicit ProfilerDataRepeater(size_t maxSize);
    ~ProfilerDataRepeater();

    bool PutPluginData(const std::shared_ptr<T>& pluginData);

    std::shared_ptr<T> TakePluginData();

    int TakePluginData(std::vector<std::shared_ptr<T>>& pluginDataVec);

    void Close();

    void Reset();

    size_t Size();

    void ClearQueue();

private:
    std::mutex mutex_;
    std::condition_variable slotCondVar_;
    std::condition_variable itemCondVar_;
    std::deque<std::shared_ptr<T>> dataQueue_;
    size_t maxSize_;
    bool closed_;

    DISALLOW_COPY_AND_MOVE(ProfilerDataRepeater);
};

template <typename T>
ProfilerDataRepeater<T>::ProfilerDataRepeater(size_t maxSize)
{
    maxSize_ = maxSize;
    closed_ = false;
}

template <typename T>
ProfilerDataRepeater<T>::~ProfilerDataRepeater()
{
    Close();
}

template <typename T>
size_t ProfilerDataRepeater<T>::Size()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return dataQueue_.size();
}

template <typename T>
void ProfilerDataRepeater<T>::Reset()
{
    std::unique_lock<std::mutex> lock(mutex_);
    closed_ = false;
}

template <typename T>
void ProfilerDataRepeater<T>::Close()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        dataQueue_.clear();
        closed_ = true;
    }
    slotCondVar_.notify_all();
    itemCondVar_.notify_all();
}

template <typename T>
bool ProfilerDataRepeater<T>::PutPluginData(const std::shared_ptr<T>& pluginData)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if ((pluginData == nullptr) && (dataQueue_.size() > 0)) {
        PROFILER_LOG_INFO(LOG_CORE, "no need put nullptr if queue has data, dataQueue_.size() = %zu",
                          dataQueue_.size());
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

template <typename T>
std::shared_ptr<T> ProfilerDataRepeater<T>::TakePluginData()
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

template <typename T>
int ProfilerDataRepeater<T>::TakePluginData(std::vector<std::shared_ptr<T>>& pluginDataVec)
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

template <typename T>
void ProfilerDataRepeater<T>::ClearQueue()
{
    std::unique_lock<std::mutex> lock(mutex_);
    dataQueue_.clear();
}

using ProfilerDataRepeaterPtr = STD_PTR(shared, ProfilerDataRepeater<ProfilerPluginData>);

#endif // PROFILER_DATA_REPEATER_H