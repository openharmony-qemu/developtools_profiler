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

namespace {
constexpr int UNIX_SOCKET_LISTEN_COUNT = 5;
constexpr int EPOLL_MAX_TASK_COUNT = 10;
constexpr int EPOLL_WAIT_TIMEOUT = 1000;
constexpr int RETRY_MAX_COUNT = 5;
}

UnixSocketServer::UnixSocketServer()
{
    sAddrName_ = "";
    socketHandle_ = -1;
    serviceEntry_ = nullptr;
}

UnixSocketServer::~UnixSocketServer()
{
    socketHandle_ = -1;

    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }
    PROFILER_LOG_DEBUG(LOG_CORE, "acceptThread finish");
    if (epfd_ != -1) {
        close(epfd_);
        epfd_ = -1;
    }
    std::unique_lock<std::mutex> lock(mtx_);
    if (socketClients_.size() > 0) {
        PROFILER_LOG_DEBUG(LOG_CORE, "socketClients_.size() = %zu delete map", socketClients_.size());
        socketClients_.clear();
    }
}

void UnixSocketServer::RemoveContext(int fd)
{
    std::unique_lock<std::mutex> lock(mtx_);
    if (socketClients_.find(fd) != socketClients_.end()) {
        PROFILER_LOG_DEBUG(LOG_CORE, "RemoveContext socketClients disconnect socketClients_.size() = %zu",
                           socketClients_.size());
        if (epfd_ != -1) {
            struct epoll_event delEvt;
            delEvt.data.fd = fd;
            delEvt.events = EPOLLHUP;
            if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, &delEvt) == -1) {
                PROFILER_LOG_ERROR(LOG_CORE, "RemoveContext epoll_ctl failed, errno: %s", strerror(errno));
            }
        }
        socketClients_.erase(fd);
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "RemoveContext Client %d not exist", fd);
    }
}

void UnixSocketServer::UnixSocketAccept(void (*callback)(int))
{
    pthread_setname_np(pthread_self(), "UnixSocketAccept");
    CHECK_TRUE(socketHandle_ != -1, NO_RETVAL, "Unix Socket Accept socketHandle_ == -1");
    epfd_ = epoll_create(1);
    struct epoll_event evt;
    evt.data.fd = socketHandle_;
    evt.events = EPOLLIN | EPOLLHUP;
    CHECK_TRUE(epoll_ctl(epfd_, EPOLL_CTL_ADD, socketHandle_, &evt) != -1, NO_RETVAL, "Unix Socket Server Exit");
    struct epoll_event events[EPOLL_MAX_TASK_COUNT];
    int retryCount = 0;
    while (socketHandle_ != -1) {
        int nfds = epoll_wait(epfd_, events, EPOLL_MAX_TASK_COUNT, EPOLL_WAIT_TIMEOUT);  // timeout value set 1000.
        if (nfds == -1) {
            if (errno == EINTR && retryCount < RETRY_MAX_COUNT) {
                ++retryCount;
                continue;
            } else {
                PROFILER_LOG_ERROR(LOG_CORE, "UnixSocketServer epoll_wait failed, errno: %s", strerror(errno));
                return;
            }
        }
        for (int32_t i = 0; i < nfds; ++i) {
            if (events[i].events & EPOLLIN) {
                int clientSocket = accept(socketHandle_, nullptr, nullptr);
                CHECK_TRUE(clientSocket != -1, NO_RETVAL, "Accept Failed");
                PROFILER_LOG_INFO(LOG_CORE, "Accept A Client %d", clientSocket);

                struct epoll_event clientEvt;
                clientEvt.data.fd = clientSocket;
                clientEvt.events = EPOLLHUP;
                CHECK_TRUE(epoll_ctl(epfd_, EPOLL_CTL_ADD, clientSocket, &clientEvt) != -1,
                    NO_RETVAL, "Unix Socket Server Exit");
                std::unique_lock<std::mutex> lock(mtx_);
                if (socketClients_.find(clientSocket) == socketClients_.end()) {
                    PROFILER_LOG_DEBUG(LOG_CORE, "new socketClients_ socketClients_.size() = %zu",
                                       socketClients_.size());
                    socketClients_[clientSocket] = std::make_shared<ClientConnection>(clientSocket, *serviceEntry_);
                } else {
                    PROFILER_LOG_ERROR(LOG_CORE, "Client %d exist", clientSocket);
                }
            } else if (events[i].events & EPOLLHUP) {
                std::unique_lock<std::mutex> lock(mtx_);
                if (socketClients_.find(events[i].data.fd) != socketClients_.end()) {
                    struct epoll_event delEvt;
                    delEvt.data.fd = events[i].data.fd;
                    delEvt.events = EPOLLHUP;
                    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, events[i].data.fd, &delEvt) == -1) {
                        PROFILER_LOG_ERROR(LOG_CORE, "UnixSocketServer epoll_ctl failed, errno: %s", strerror(errno));
                    }
                    lock.unlock();
                    if (callback != nullptr) {
                        callback(events[i].data.fd);
                    }
                    std::unique_lock<std::mutex> socketMapLock(mtx_);
                    PROFILER_LOG_DEBUG(LOG_CORE, "socketClients disconnect socketClients_.size() = %zu",
                                       socketClients_.size());
                    socketClients_.erase(events[i].data.fd);
                } else {
                    PROFILER_LOG_ERROR(LOG_CORE, "Client %d not exist", events[i].data.fd);
                }
            }
        }
    }
}

bool UnixSocketServer::StartServer(const std::string& addrname, ServiceEntry& p, void (*callback)(int))
{
    CHECK_TRUE(socketHandle_ == -1, false, "StartServer FAIL socketHandle_ != -1");
    int sock = GetControlSocket(addrname.c_str());
    CHECK_TRUE(sock != -1, false, "StartServer FAIL GetControlSocket return : %d", sock);
    if (listen(sock, UNIX_SOCKET_LISTEN_COUNT) == -1) {
        close(sock);
        unlink(addrname.c_str());
        PROFILER_LOG_ERROR(LOG_CORE, "StartServer FAIL listen ERR : %d", errno);
        return false;
    }
    socketHandle_ = sock;
    acceptThread_ = std::thread([this, callback] { this->UnixSocketAccept(callback); });
    if (acceptThread_.get_id() == std::thread::id()) {
        close(socketHandle_);
        unlink(addrname.c_str());
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "StartServer FAIL pthread_create ERR : %s", buf);
        socketHandle_ = -1;
        return false;
    }
    serviceEntry_ = &p;
    sAddrName_ = addrname;
    return true;
}
