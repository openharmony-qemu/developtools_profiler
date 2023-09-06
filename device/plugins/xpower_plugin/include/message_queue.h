/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

#include "logging.h"

template <typename T>
class MessageQueue {
public:
    explicit MessageQueue(size_t maxSize);
    ~MessageQueue();
    bool WaitAndPop(T &value, const std::chrono::microseconds rel_time);
    void PushBack(const T &item);
    void shutDown();
    int Size();
    bool Empty();
    bool IsShutDown();

private:
    std::deque<T> dataQueue_;
    std::mutex mutex_;
    condition_variable fullCon_;
    condition_variable emptyCon_;
    bool shutDown_ = false;
    size_t maxSize_;
};

template <typename T>
MessageQueue<T>::MessageQueue(size_t maxSize)
{
  maxSize_ = maxSize;
}

template <typename T>
MessageQueue<T>::~MessageQueue()
{
}

template <typename T>
bool MessageQueue<T>::IsShutDown()
{
    return this->shutDown_;
}

template <typename T>
void MessageQueue<T>::shutDown()
{
    std::unique_lock<std::mutex> lock(mutex_);
    dataQueue_.clear();
    this->shutDown_ = true;
    fullCon_.notify_all();
    emptyCon_.notify_all();
}

template <typename T>
bool MessageQueue<T>::WaitAndPop(T &value, const std::chrono::microseconds realTime)
{
    // the relative timeout rel_time expires
    std::unique_lock<std::mutex> lock(mutex_);
    if (IsShutDown()) {
        return false;
    }
    if (emptyCon_.wait_for(lock, realTime, [&] { return !dataQueue_.empty(); })) {
        value = dataQueue_.front();
        dataQueue_.pop_front();
    } else {
        return false;
    }
    lock.unlock();
    fullCon_.notify_one();
    return true;
}

template <typename T>
bool MessageQueue<T>::WaitAndPopBatch(std::vector<T>& array,const std::chrono::microseconds realTime,size_t batchCount)
{
  std::unique_lock<std::mutex> lock(mutex_);
  if (IsShutDown()) {
      return false;
  }
  uint32_t queueSize = 0;
  if (emptyCon_.wait_for(lock, realTime,[&] {return !dataQueue_.empty();})) {
    queueSize = dataQueue_.size();
    size_t resultSize = queueSize > batchCount ? batchCount : queueSize;
    for (size_t i = 0; i < resultSize; i++) {
      T result = dataQueue_.front();
      dataQueue_.pop_front();
      array[i] = result;
    }
  } else {
    return false;
  }
  lock.unlock();
  fullCon_.notify_one();
  return true;
}

template <typename T>
void MessageQueue<T>::PushBack(const T &item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    while (dataQueue_.size() >= maxSize_) {
      fullCon_.wait(mlock);
    }
    dataQueue_.push_back(item);
    mlock.unlock();     // unlock before notificiation to minimize mutex con
    emptyCon_.notify_one(); // notify one waiting thread
}

template <typename T>
int MessageQueue<T>::Size()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    int size = dataQueue_.size();
    mlock.unlock();
    return size;
}

template <typename T>
bool MessageQueue<T>::Empty()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    bool isEmpty = dataQueue_.empty();
    mlock.unlock();
    return isEmpty;
}
#endif // MESSAGE_QUEUE_H
