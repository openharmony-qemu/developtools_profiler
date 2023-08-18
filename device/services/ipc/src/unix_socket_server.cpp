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

#include "unix_socket_server.h"

#include <cstdio>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/un.h>

#include "init_socket.h"
#include "logging.h"
#include "securec.h"

UnixSocketServer::UnixSocketServer()
{
    sAddrName_ = "";
    socketHandle_ = -1;
    serviceEntry_ = nullptr;
}

UnixSocketServer::~UnixSocketServer()
{
    if (socketHandle_ != -1) {
        HILOG_DEBUG(LOG_CORE, "close UnixSocketServer");
        close(socketHandle_);
        socketHandle_ = -1;
        unlink(sAddrName_.c_str());
    }

    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }
    HILOG_DEBUG(LOG_CORE, "acceptThread finish");
    if (socketClients_.size() > 0) {
        HILOG_DEBUG(LOG_CORE, "socketClients_.size() = %zu delete map", socketClients_.size());
        socketClients_.clear();
    }
}

void UnixSocketServer::UnixSocketAccept()
{
    pthread_setname_np(pthread_self(), "UnixSocketAccept");
    CHECK_TRUE(socketHandle_ != -1, NO_RETVAL, "Unix Socket Accept socketHandle_ == -1");
    int epfd = epoll_create(1);
    struct epoll_event evt;
    evt.data.fd = socketHandle_;
    evt.events = EPOLLIN | EPOLLET | EPOLLHUP;
    CHECK_TRUE(epoll_ctl(epfd, EPOLL_CTL_ADD, socketHandle_, &evt) != -1, NO_RETVAL, "Unix Socket Server Exit");
    while (socketHandle_ != -1) {
        struct epoll_event events[10];
        int nfds = epoll_wait(epfd, &events[0], 1, 1000);  // timeout value set 1000.
        if (nfds > 0) {
            if (events[0].events & EPOLLIN) {
                int clientSocket = accept(socketHandle_, nullptr, nullptr);
                HILOG_INFO(LOG_CORE, "Accept A Client %d", clientSocket);

                struct epoll_event clientEvt;
                clientEvt.data.fd = clientSocket;
                clientEvt.events = EPOLLHUP;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientSocket, &clientEvt);

                if (socketClients_.find(clientSocket) == socketClients_.end()) {
                    HILOG_DEBUG(LOG_CORE, "new socketClients_ socketClients_.size() = %zu", socketClients_.size());
                    socketClients_[clientSocket] = std::make_shared<ClientConnection>(clientSocket, *serviceEntry_);
                } else {
                    HILOG_ERROR(LOG_CORE, "Client %d exist", clientSocket);
                }
            } else if (events[0].events & EPOLLHUP) {
                struct epoll_event delEvt;
                delEvt.data.fd = events[0].data.fd;
                delEvt.events = EPOLLHUP;
                epoll_ctl(epfd, EPOLL_CTL_DEL, events[0].data.fd, &delEvt);

                if (socketClients_.find(events[0].data.fd) != socketClients_.end()) {
                    HILOG_DEBUG(LOG_CORE, "socketClients disconnect socketClients_.size() = %zu", socketClients_.size());
                    socketClients_.erase(events[0].data.fd);
                } else {
                    HILOG_ERROR(LOG_CORE, "Client %d not exist", events[0].data.fd);
                }
            }
        }
    }
    close(epfd);
}

namespace {
const int UNIX_SOCKET_LISTEN_COUNT = 5;
}
bool UnixSocketServer::StartServer(const std::string& addrname, ServiceEntry& p)
{
    CHECK_TRUE(socketHandle_ == -1, false, "StartServer FAIL socketHandle_ != -1");
    int sock = -1;
    if (getuid() == 0) {
        struct sockaddr_un addr;
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        CHECK_TRUE(sock != -1, false, "StartServer FAIL create socket ERR : %d", errno);

        if (memset_s(&addr, sizeof(struct sockaddr_un), 0, sizeof(struct sockaddr_un)) != EOK) {
            HILOG_ERROR(LOG_CORE, "memset_s error!");
        }
        addr.sun_family = AF_UNIX;
        if (strncpy_s(addr.sun_path, sizeof(addr.sun_path), addrname.c_str(), sizeof(addr.sun_path) - 1) != EOK) {
            HILOG_ERROR(LOG_CORE, "strncpy_s error!");
        }
        unlink(addrname.c_str());
        CHECK_TRUE(bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == 0, close(sock) != 0,
                   "StartServer FAIL bind ERR : %d", errno);

        std::string chmodCmd = "chmod 666 " + addrname;
        HILOG_INFO(LOG_CORE, "chmod command : %s", chmodCmd.c_str());
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(chmodCmd.c_str(), "r"), pclose);
    } else {
        sock = GetControlSocket(addrname.c_str());
        CHECK_TRUE(sock != -1, false, "StartServer FAIL GetControlSocket retrun : %d", sock);
    }

    CHECK_TRUE(listen(sock, UNIX_SOCKET_LISTEN_COUNT) != -1, close(sock) != 0 && unlink(addrname.c_str()) == 0,
               "StartServer FAIL listen ERR : %d", errno);

    socketHandle_ = sock;
    acceptThread_ = std::thread(&UnixSocketServer::UnixSocketAccept, this);
    if (acceptThread_.get_id() == std::thread::id()) {
        close(socketHandle_);
        unlink(addrname.c_str());
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        HILOG_ERROR(LOG_CORE, "StartServer FAIL pthread_create ERR : %s", buf);
        socketHandle_ = -1;
        return false;
    }

    serviceEntry_ = &p;
    sAddrName_ = addrname;
    return true;
}
