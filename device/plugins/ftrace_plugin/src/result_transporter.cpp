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
 *
 * Description: ResultTransporter class implements
 */
#include "result_transporter.h"

#include <chrono>
#include <cinttypes>
#include <pthread.h>
#include <unistd.h>
#include "logging.h"

namespace {
constexpr auto DEFAULT_FLUSH_INTERVAL = std::chrono::milliseconds(1000);
constexpr uint32_t DEFAULT_FLUSH_THRESHOLD = 1024 * 1024;
} // namespace

FTRACE_NS_BEGIN
ResultTransporter::ResultTransporter(const std::string& name, WriterStructPtr writer)
    : name_(name), flushThreshold_(DEFAULT_FLUSH_THRESHOLD), flushInterval_(DEFAULT_FLUSH_INTERVAL), writer_(writer)
{
}

ResultTransporter::~ResultTransporter(void)
{
    HILOG_INFO(LOG_CORE, "ResultTransporter destroy!");
}

void ResultTransporter::SetFlushInterval(int ms)
{
    HILOG_INFO(LOG_CORE, "ResultTransporter set flush interval to %d", ms);
    flushInterval_ = std::chrono::milliseconds(ms);
}

void ResultTransporter::SetFlushThreshold(uint32_t nbytes)
{
    HILOG_INFO(LOG_CORE, "ResultTransporter set flush threshold to %u", nbytes);
    flushThreshold_ = nbytes;
}

bool ResultTransporter::IsFlushTime() const
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<decltype(flushInterval_)>(currentTime - lastTime);
    if (elapsedTime < flushInterval_) {
        return false;
    }
    lastTime = currentTime;
    return true;
}

long ResultTransporter::Write(ResultPtr&& packet)
{
    if (writer_ == nullptr || writer_->write == nullptr) {
        return 0;
    }

    size_t size = packet->ByteSizeLong();
    buffer_.resize(size);
    CHECK_TRUE(buffer_.size() == size, -1,
               "%s: buffer resize failed, size: %zu, buffer size: %zu, errno: %d(%s)",
               __func__, size, buffer_.size(), errno, strerror(errno));

    int ret = packet->SerializeToArray(buffer_.data(), buffer_.size());
    CHECK_TRUE(ret > 0, ret, "%s: SerializeToArray failed with %d, size: %zu", __func__, ret, size);

    writer_->write(writer_, buffer_.data(), buffer_.size());
    return buffer_.size();
}

void ResultTransporter::Flush()
{
    if (writer_ == nullptr || writer_->flush == nullptr) {
        return;
    }
    writer_->flush(writer_);

    auto count = bytesCount_.load();
    auto pending = bytesPending_.load();
    bytesPending_ = 0;
    HILOG_DEBUG(LOG_CORE, "ResultTransporter TX stats B: %" PRIu64 ", P: %u", count, pending);
}

bool ResultTransporter::Submit(ResultPtr&& packet)
{
    std::unique_lock<std::mutex> lock(mutex_);
    long nbytes = Write(std::move(packet));
    if (nbytes < 0) {
        HILOG_ERROR(LOG_CORE, "send result FAILED!");
        lock.unlock();
        return false;
    }
    bytesCount_ += nbytes;
    bytesPending_ += nbytes;

    if (IsFlushTime() || bytesPending_ >= flushThreshold_) {
        Flush();
    }
    lock.unlock();
    return true;
}
FTRACE_NS_END
