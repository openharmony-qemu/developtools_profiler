/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "http_socket.h"
#include <cstring>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include "log.h"
namespace SysTuning {
namespace TraceStreamer {
HttpSocket::~HttpSocket()
{
    Close();
}
bool HttpSocket::CreateSocket(int32_t domain)
{
    SOCKET sockId = socket(domain, SOCK_STREAM, 0);
    if (sockId == INVALID_SOCKET) {
        TS_LOGE("CreateSocket socket error, domain %d: %d:%s", domain, errno, strerror(errno));
        return false;
    }
    sockId_ = sockId;
    if (domain == AF_INET || domain == AF_INET6) {
        int32_t enable = 1;
        if (setsockopt(sockId, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&enable), sizeof(enable)) ==
            SOCKET_ERROR) {
            Close();
            return false;
        }
        if (domain == AF_INET6) {
            if (setsockopt(sockId, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&enable), sizeof(enable)) ==
                SOCKET_ERROR) {
                Close();
                return false;
            }
        }
    }
    domain_ = domain;
    TS_LOGI("CreateSocket socket ok, socket %d domain %d", sockId_, domain);
    return true;
}

bool HttpSocket::Bind(int32_t port)
{
    if (sockId_ == INVALID_SOCKET) {
        TS_LOGE("the socket not created");
        return false;
    }

    if (domain_ == AF_INET) {
        struct sockaddr_in addr;
        std::fill(reinterpret_cast<char*>(&addr), reinterpret_cast<char*>(&addr) + sizeof(addr), 0);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htons(INADDR_ANY);
        addr.sin_port = htons(static_cast<uint16_t>(port));
        if (bind(sockId_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            TS_LOGE("bind ipv4 socket error, port %d: %d:%s", port, errno, strerror(errno));
            return false;
        }
    } else if (domain_ == AF_INET6) {
        struct sockaddr_in6 addr;
        std::fill(reinterpret_cast<char*>(&addr), reinterpret_cast<char*>(&addr) + sizeof(addr), 0);
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(static_cast<uint16_t>(port));
        if (bind(sockId_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            TS_LOGE("bind ipv6 socket error, port %d: %d:%s", port, errno, strerror(errno));
            return false;
        }
    } else {
        return false;
    }
    TS_LOGI("bind socket ok, port %d", port);
    return true;
}

bool HttpSocket::Listen(int32_t maxConn)
{
    if (listen(sockId_, maxConn) == SOCKET_ERROR) {
        TS_LOGE("listen socket error: %d:%s", errno, strerror(errno));
        return false;
    }
    TS_LOGI("listen socket ok, maxConn %d", maxConn);
    return true;
}

bool HttpSocket::Accept(HttpSocket& client)
{
    int32_t clientId = accept(sockId_, nullptr, nullptr);
    if (clientId == INVALID_SOCKET) {
        TS_LOGE("accept socket error: %d:%s", errno, strerror(errno));
        return false;
    }

    client.domain_ = domain_;
    client.sockId_ = clientId;
    TS_LOGI("accept client socket id %d domain %d", clientId, domain_);
    return true;
}

bool HttpSocket::Recv(void* data, size_t& len)
{
#ifdef _WIN32
    ssize_t recvLen = recv(sockId_, static_cast<char*>(data), len, 0);
#else
    ssize_t recvLen = recv(sockId_, data, len, 0);
#endif
    if (recvLen == SOCKET_ERROR) {
        if (errno == EAGAIN) {
            recvLen = 0;
        } else {
            TS_LOGE("recv from socket(%d) error: %d:%s", sockId_, errno, strerror(errno));
            return false;
        }
    } else if (recvLen == 0) {
        TS_LOGI("client socket(%d) closed", sockId_);
        return false;
    }
    len = recvLen;
    TS_LOGD("Recv from socket(%d) len %zu", sockId_, len);
    return true;
}

bool HttpSocket::Send(const void* data, size_t len)
{
#ifdef _WIN32
    ssize_t sendLen = send(sockId_, static_cast<const char*>(data), len, 0);
#else
    ssize_t sendLen = send(sockId_, data, len, 0);
#endif
    if (sendLen == SOCKET_ERROR) {
        TS_LOGE("send to socket(%d) error: %d:%s", sockId_, errno, strerror(errno));
        return false;
    }
    TS_LOGI("send to socket(%d) len %zu", sockId_, len);
    return true;
}

void HttpSocket::Close()
{
    if (sockId_ == INVALID_SOCKET) {
        return;
    }
    TS_LOGI("close socket(%d)", sockId_);
#ifdef _WIN32
    if (closesocket(sockId_) == SOCKET_ERROR) {
#else
    if (close(sockId_) == SOCKET_ERROR) {
#endif
        TS_LOGE("close socket(%d) error: %d:%s", sockId_, errno, strerror(errno));
    }
    sockId_ = INVALID_SOCKET;
}
} // namespace TraceStreamer
} // namespace SysTuning
