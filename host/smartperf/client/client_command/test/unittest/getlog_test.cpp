/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <gtest/gtest.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "sdk_data_recv.h"
#include "sp_task.h"
#include "GetLog.h"
#include "sp_log.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonGetLogTest : public testing::Test {
public:
    std::thread tcpServer;
    int tcpPort = -1;
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        if (tcpServer.joinable()) {
            tcpServer.join();
        }
    }

    void GetLogCommand()
    {
        int tcpServerFd = socket(AF_INET, SOCK_STREAM, 0);
        if (tcpServerFd < 0) {
            WLOGE("TCP socket creation failed");
            return;
        }

        struct sockaddr_in tcpServerAddr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = inet_addr("127.0.0.1"),
            .sin_port = 0
        };

        socklen_t len = sizeof(tcpServerAddr);
        if (::bind(tcpServerFd, reinterpret_cast<struct sockaddr *>(&tcpServerAddr), sizeof(tcpServerAddr)) < 0 ||
            getsockname(tcpServerFd, reinterpret_cast<struct sockaddr *>(&tcpServerAddr), &len) < 0 ||
            listen(tcpServerFd, 1) < 0) {
            close(tcpServerFd);
            return;
        }
        tcpPort = ntohs(tcpServerAddr.sin_port);

        int clientFd = accept(tcpServerFd, nullptr, nullptr);
        if (clientFd < 0) {
            WLOGE("Accept failed");
            close(tcpServerFd);
            return;
        }
        WLOGI("Accept clientFd: %d\n", clientFd);
        FILE* fp = fopen("/data/local/tmp/getlogtest.tar.gz", "wb");
        if (!fp) {
            WLOGE("File open failed");
            close(clientFd);
            close(tcpServerFd);
            return;
        }

        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = recv(clientFd, buffer, sizeof(buffer), 0)) > 0) {
            size_t written = fwrite(buffer, 1, bytesRead, fp);
            if (written != static_cast<size_t>(bytesRead)) {
                WLOGE("Failed to write all bytes: expected %zd, wrote %zu", bytesRead, written);
                break;
            }
        }

        if (fclose(fp) != 0) {
            WLOGE("Failed to close file");
        }
        close(clientFd);
        close(tcpServerFd);
    }
};

/**
 * @tc.name: GetLog::SendFileTestCase
 * @tc.desc: Test SendFileTestCase
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonGetLogTest, SendFileTestCase, TestSize.Level1)
{
    std::map<std::string, std::string> result = GetLog::GetInstance().ItemData();
    tcpServer = std::thread([this]() {
        GetLogCommand();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_NE(tcpPort, -1);
    GetLog::GetInstance().SetLogFileSocketPort(tcpPort);
    int connectStatus = GetLog::GetInstance().LogFileSocketConnect();
    ASSERT_GT(connectStatus, 0);
    int sendStatus = GetLog::GetInstance().SendLogFile();
    ASSERT_GE(sendStatus, 0);
    tcpServer.join();
}

/**
 * @tc.name: GetLog::SetGetPortTestCase
 * @tc.desc: Test set and get port function
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonGetLogTest, SetGetPortTestCase, TestSize.Level1)
{
    GetLog::GetInstance().SetLogFileSocket(12345);
    EXPECT_EQ(GetLog::GetInstance().GetLogFileSocket(), 12345);
    GetLog::GetInstance().SetLogFileSocketPort(12345);
    EXPECT_EQ(GetLog::GetInstance().GetLogFileSocketPort(), 12345);
}
} // namespace SmartPerf
}