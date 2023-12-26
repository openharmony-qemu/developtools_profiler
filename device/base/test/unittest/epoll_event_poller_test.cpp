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
#include <atomic>
#include <chrono>
#include <cinttypes>
#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <memory>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "epoll_event_poller.h"
#include "event_notifier.h"
#include "logging.h"

using namespace testing::ext;

namespace {
constexpr int DEFAULT_POLL_INTERVAL = 1000;
}

namespace {
class EpollEventPollerTest : public testing::Test {
protected:
    std::unique_ptr<EpollEventPoller> eventPoller;
    void SetUp() override
    {
        eventPoller = std::make_unique<EpollEventPoller>(DEFAULT_POLL_INTERVAL);
    }

    void TearDown() override {}
};

/**
 * @tc.name: EpollEventPollerTest
 * @tc.desc: CtorDtor.
 * @tc.type: FUNC
 */
HWTEST_F(EpollEventPollerTest, CtorDtor, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);
}

/**
 * @tc.name: EpollEventPollerTest
 * @tc.desc: InitFinalize.
 * @tc.type: FUNC
 */
HWTEST_F(EpollEventPollerTest, InitFinalize, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);

    HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitFinalize start!");
    EXPECT_TRUE(eventPoller->Init());
    EXPECT_TRUE(eventPoller->Finalize());
}

/**
 * @tc.name: EpollEventPollerTest
 * @tc.desc: InitFinalize.
 * @tc.type: FUNC
 */
HWTEST_F(EpollEventPollerTest, InitOnly, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);
    HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitOnly start!");
    EXPECT_TRUE(eventPoller->Init());
}

/**
 * @tc.name: EpollEventPollerTest
 * @tc.desc: Init 2 times Finalize 1 time.
 * @tc.type: FUNC
 */
HWTEST_F(EpollEventPollerTest, Init2Finalize1, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);

    HILOG_INFO(LOG_CORE, "EpollEventPollerTest.Init2Finalize1 start!");
    EXPECT_TRUE(eventPoller->Init());
    EXPECT_FALSE(eventPoller->Init());
    EXPECT_TRUE(eventPoller->Finalize());
}

/**
 * @tc.name: EpollEventPollerTest
 * @tc.desc: InitStartStopFinalize.
 * @tc.type: FUNC
 */
HWTEST_F(EpollEventPollerTest, InitStartStopFinalize, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);

    HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitStartStopFinalize start!");
    EXPECT_TRUE(eventPoller->Init());
    EXPECT_TRUE(eventPoller->Start());
    EXPECT_TRUE(eventPoller->Stop());
    EXPECT_TRUE(eventPoller->Finalize());
}

/**
 * @tc.name: EpollEventPollerTest
 * @tc.desc: InitStartStop.
 * @tc.type: FUNC
 */
HWTEST_F(EpollEventPollerTest, InitStartStop, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);

    HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitStartStop start!");
    EXPECT_TRUE(eventPoller->Init());
    EXPECT_TRUE(eventPoller->Start());
    EXPECT_TRUE(eventPoller->Stop());
}

/**
 * @tc.name: EpollEventPollerTest
 * @tc.desc: InitStart.
 * @tc.type: FUNC
 */
HWTEST_F(EpollEventPollerTest, InitStart, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);

    HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitStart start!");
    EXPECT_TRUE(eventPoller->Init());
    EXPECT_TRUE(eventPoller->Start());
}

/**
 * @tc.name: EpollEventPollerTest
 * @tc.desc: AddFd.
 * @tc.type: FUNC
 */
HWTEST_F(EpollEventPollerTest, InitStartAddFd, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);

    HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitStartAddFd start!");
    EXPECT_TRUE(eventPoller->Init());
    EXPECT_TRUE(eventPoller->Start());

    int eventFd = eventfd(0, O_CLOEXEC | O_NONBLOCK);
    uint64_t readValue = 0;
    auto onReadable = [&readValue, &eventFd]() {
        read(eventFd, &readValue, sizeof(readValue));
        HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitStartAddFd read %" PRIu64, readValue);
    };
    EXPECT_TRUE(eventPoller->AddFileDescriptor(eventFd, onReadable));

    uint64_t writeValue = 1234;
    int writeSize = sizeof(writeValue); // run -t UT -ss developtools
    EXPECT_EQ(write(eventFd, &writeValue, writeSize), writeSize);

    std::this_thread::yield();
    usleep(15 * 1000);
    EXPECT_EQ(readValue, writeValue);
    eventPoller->RemoveFileDescriptor(eventFd);
    EXPECT_TRUE(eventPoller->Stop());
    EXPECT_TRUE(eventPoller->Finalize());

    close(eventFd);
}

HWTEST_F(EpollEventPollerTest, InitStartAddEventFd, TestSize.Level1)
{
    ASSERT_NE(eventPoller, nullptr);

    HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitStartAddFd start!");
    EXPECT_TRUE(eventPoller->Init());
    EXPECT_TRUE(eventPoller->Start());

    auto notifier = EventNotifier::Create(0, EventNotifier::NONBLOCK);
    uint64_t readValue = 0;
    auto onReadable = [&readValue, &notifier]() {
        readValue = notifier->Take();
        HILOG_INFO(LOG_CORE, "EpollEventPollerTest.InitStartAddFd read %" PRIu64, readValue);
    };
    EXPECT_TRUE(eventPoller->AddFileDescriptor(notifier->GetFd(), onReadable));

    uint64_t writeValue = 1234;
    pid_t pid = fork();
    EXPECT_GE(pid, 0);
    if (pid == 0) {
        int evFd = dup(notifier->GetFd());
        auto evNotifier = EventNotifier::CreateWithFd(evFd);
        evNotifier->Post(writeValue);
        _exit(0);
    } else if (pid > 0) {
        int wstatus = 0;
        waitpid(pid, &wstatus, 0);
    }

    std::this_thread::yield();
    usleep(15 * 1000);
    EXPECT_EQ(readValue, writeValue);

    EXPECT_TRUE(eventPoller->Stop());
    EXPECT_TRUE(eventPoller->Finalize());
}
} // namespace