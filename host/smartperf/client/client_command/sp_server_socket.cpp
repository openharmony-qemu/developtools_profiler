/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include "include/sp_server_socket.h"
namespace OHOS {
namespace SmartPerf {
SpServerSocket::SpServerSocket() {}

SpServerSocket::~SpServerSocket()
{
    Close();
}

int SpServerSocket::Init(ProtoType type)
{
    if (type == ProtoType::TCP) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        sockPort = tcpPort;
    }
    if (type == ProtoType::UDP) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        sockPort = udpPort;
    }
    if (sock < 0) {
        std::cout << "Socket Create Failed:" << sock << std::endl;
        return -1;
    }
    local.sin_family = AF_INET;
    local.sin_port = htons(sockPort);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(sock, reinterpret_cast<struct sockaddr *>(&local), sizeof(local)) < 0) {
        std::cout << "Socket Bind failed:" << sock << std::endl;
        return -1;
    }
    if (type == ProtoType::TCP) {
        if (listen(sock, listenCount) < 0) {
            std::cout << "Socket Listen failed:" << sock << std::endl;
            return -1;
        }
    }
    return 0;
}

int SpServerSocket::Accept()
{
    connFd = accept(sock, nullptr, nullptr);
    return connFd;
}

int SpServerSocket::Sendto(std::string &sendBuf)
{
    socklen_t len = sizeof(sockaddr_in);
    sendto(sock, sendBuf.c_str(), sendBuf.size(), 0, reinterpret_cast<struct sockaddr*>(&client), len);
    return 0;
}

int SpServerSocket::Send(std::string sendBuf)
{
    int sendBytes = send(connFd, sendBuf.c_str(), sendBuf.size(), 0);
    if (sendBytes < 0) {
        std::cout << "Send:Error" << std::endl;
        return -1;
    }
    return 0;
}

void SpServerSocket::Close() const
{
    shutdown(sock, SHUT_RD);
}

int SpServerSocket::Recvfrom()
{
    bzero(rbuf, sizeof(rbuf));
    socklen_t len = sizeof(sockaddr_in);
    int l = recvfrom(sock, rbuf, sizeof(rbuf) - 1, 0, reinterpret_cast<struct sockaddr*>(&client), &len);
    if (l > 0) {
        std::cout << "Client:" << rbuf << std::endl;
    }
    return l;
}

int SpServerSocket::Recv()
{
    bzero(rbuf, sizeof(rbuf));
    int l = recv(connFd, rbuf, sizeof(rbuf) - 1, 0);
    if (l > 0) {
        std::cout << "Client:" << rbuf << std::endl;
    } else {
        perror("recv error:");
        return -1;
    }
    return l;
}

std::string SpServerSocket::RecvBuf() const
{
    std::string recvBuf = rbuf;
    return recvBuf;
}
}
}
