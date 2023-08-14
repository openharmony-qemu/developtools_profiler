/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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
#include "ipc_unix_socket.h"

#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "hhlog.h"

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
IpcUnixSocketServer::IpcUnixSocketServer() {}

IpcUnixSocketServer::~IpcUnixSocketServer()
{
    Stop();
}

bool IpcUnixSocketServer::Start(const std::string &pathname)
{
    CHECK_TRUE(serverFd_ == -1, false, "Unix Socket Server is running");

    serverFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_TRUE(serverFd_ != -1, false, "create Unix Socket Server failed, %d: %s", errno, strerror(errno));

    unlink(pathname.c_str());
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    std::copy(pathname.c_str(), pathname.c_str() + pathname.size() + 1, addr.sun_path);
    if (bind(serverFd_, (struct sockaddr*)&addr, sizeof(sockaddr_un)) != 0) {
        HHLOGE(true, "bind failed, Unix Socket(%s), %d: %s", pathname.c_str(), errno, strerror(errno));
        close(serverFd_);
        return false;
    }
    if (listen(serverFd_, UNIX_SOCKET_LISTEN_COUNT) != 0) {
        HHLOGE(true, "listen failed, Unix Socket(%s), %d: %s", pathname.c_str(), errno, strerror(errno));
        close(serverFd_);
        unlink(pathname.c_str());
        return false;
    }
    pathName_ = pathname;

    isRunning_ = true;
    handleThread_ = std::thread(&IpcUnixSocketServer::handleThreadLoop, this);

    return true;
}

bool IpcUnixSocketServer::Stop()
{
    isRunning_ = false;
    if (serverFd_ != -1) {
        close(serverFd_);
        serverFd_ = -1;
    }
    if (clientFd_ != -1) {
        close(clientFd_);
        clientFd_ = -1;
    }
    if (handleThread_.joinable()) {
        handleThread_.join();
    }
    unlink(pathName_.c_str());
    return true;
}

bool IpcUnixSocketServer::SendMessage(const void *buf, size_t size)
{
    CHECK_TRUE(clientFd_ != -1, false, "no available Unix Socket");

    CHECK_TRUE(send(clientFd_, buf, size, 0) != -1, false,
               "send failed, Unix Socket(%d) %zu bytes, %d: %s", clientFd_, size, errno, strerror(errno));
    return true;
}

void IpcUnixSocketServer::handleThreadLoop()
{
    while (isRunning_) {
        struct pollfd pollFd {serverFd_, POLLIN, 0};
        const int timeout = 1000;
        int polled = TEMP_FAILURE_RETRY(poll(&pollFd, 1, timeout));
        if (polled == 0) { // timeout
            continue;
        } else if (polled < 0 || !(pollFd.revents & POLLIN)) {
            HHLOGE(true, "poll failed, Unix Socket(%d), %d: %s", serverFd_, errno, strerror(errno));
            close(serverFd_);
            serverFd_ = -1;
            break;
        }

        clientFd_ = accept(serverFd_, nullptr, nullptr);
        if (clientFd_ == -1) {
            HHLOGE(true, "accept failed, Unix Socket(%d), %d: %s", serverFd_, errno, strerror(errno));
            continue;
        }

        while (isRunning_ && clientFd_ != -1) {
            uint8_t buf[UNIX_SOCKET_BUFFER_SIZE] = {0};
            int recvSize = recv(clientFd_, buf, UNIX_SOCKET_BUFFER_SIZE, 0);
            if (recvSize > 0) {
                if (handleMessageFn_) {
                    handleMessageFn_(buf, recvSize);
                }
                continue;
            } else if (recvSize == 0) {
                HHLOGE(true, "recv failed, peer has closed");
            } else {
                HHLOGE(true, "recv failed, Unix Socket(%d), %d: %s", clientFd_, errno, strerror(errno));
            }
            close(clientFd_);
            clientFd_ = -1;
        }
    }
}

IpcUnixSocketClient::IpcUnixSocketClient() {}

IpcUnixSocketClient::~IpcUnixSocketClient()
{
    Disconnect();
}

bool IpcUnixSocketClient::Connect(const std::string &pathname)
{
    CHECK_TRUE(sockFd_ == -1, false, "Unix Socket has connected");

    sockFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_TRUE(sockFd_ != -1, false, "create Unix Socket Server failed, %d: %s", errno, strerror(errno));

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    std::copy(pathname.c_str(), pathname.c_str() + pathname.size() + 1, addr.sun_path);
    if (connect(sockFd_, (struct sockaddr*)&addr, sizeof(sockaddr_un)) == -1) {
        HHLOGE(true, "connect failed, %d: %s", errno, strerror(errno));
        sockFd_ = -1;
        return false;
    }

    return true;
}

void IpcUnixSocketClient::Disconnect()
{
    if (sockFd_ != -1) {
        close(sockFd_);
        sockFd_ = -1;
    }
}

bool IpcUnixSocketClient::SendMessage(const void *buf, size_t size)
{
    CHECK_TRUE(sockFd_ != -1, false, "Unix Socket disconnected");

    if (send(sockFd_, buf, size, 0) != -1) {
        return true;
    }
    HHLOGE(true, "send failed, Unix Socket(%d), %d: %s", sockFd_, errno, strerror(errno));
    return false;
}

bool IpcUnixSocketClient::RecvMessage(void *buf, size_t &size, uint32_t timeout)
{
    CHECK_TRUE(sockFd_ != -1, false, "Unix Socket disconnected");

    struct pollfd pollFd {sockFd_, POLLIN | POLLERR | POLLHUP, 0};
    int polled = poll(&pollFd, 1, timeout);
    if (polled == 0) { // timeout
        size = 0;
        return true;
    } else if (polled < 0 || !(pollFd.revents & POLLIN)) {
        HHLOGE(true, "poll failed, Unix Socket(%d), %d: %s", sockFd_, errno, strerror(errno));
        return false;
    }

    int recvSize = recv(sockFd_, buf, size, 0);
    if (recvSize > 0) {
        size = static_cast<size_t>(recvSize);
        return true;
    } else if (recvSize == 0) {
        HHLOGE(true, "recv failed, peer has closed");
    } else {
        HHLOGE(true, "recv failed, Unix Socket(%d), %d: %s", sockFd_, errno, strerror(errno));
    }

    return false;
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
