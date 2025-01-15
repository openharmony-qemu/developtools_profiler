/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#include "socket_context.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "logging.h"
#include "securec.h"
#include "service_base.h"
#include "service_entry.h"

namespace {
const int PROTO_SIZE_MAX = 1024 * 1024;
const int MEMORY_BLOCK_UNIT = 4096;
const int HEARTBEAT_MSG = -1;
} // namespace

SocketContext::SocketContext()
{
    socketHandle_ = -1;
    clientState_ = CLIENT_STAT_WORKING;
    lastProcMS_ = 0;
    serviceBase_ = nullptr;
}

SocketContext::~SocketContext()
{
    if (socketHandle_ >= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "SocketContext socketHandle_ = %d", socketHandle_);

        int ret = shutdown(socketHandle_, SHUT_RDWR);
        if (ret < 0) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "shutdown socket err = %d %s", errno, buf);
        }
    }
    int socketTemp = socketHandle_;
    socketHandle_ = -1;
    if (recvThread_.joinable()) {
        recvThread_.join();
    }
    socketHandle_ = socketTemp;
    PROFILER_LOG_ERROR(LOG_CORE, "~SocketContext recvThread join success");
    if (socketHandle_ >= 0) {
        int ret = close(socketHandle_);
        if (ret < 0) {
            const int bufSize = 256;
            char buf[bufSize] = { 0 };
            strerror_r(errno, buf, bufSize);
            PROFILER_LOG_ERROR(LOG_CORE, "close socket err = %d %s", errno, buf);
        }
        socketHandle_ = -1;
    }
}

int SocketContext::RawProtocolProc(uint32_t pnum, const int8_t* buf, const uint32_t size)
{
    return -1;
}

bool SocketContext::ReceiveData(int sock, uint8_t* databuf, uint32_t size)
{
    uint32_t p = 0;
    if (sock < 0) {
        return false;
    }
    if (size == 0) {
        return false;
    }
    const uint32_t timesToTry = 2; // try 2 times
    uint32_t nTimes = 0;
    while (p < size) {
        int ret = recv(sock, &databuf[p], size - p, 0);
        if (ret > 0) {
            p += ret;
        } else if (ret == -1 && errno == EAGAIN) {
            if (++nTimes >= timesToTry) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

void* SocketContext::UnixSocketRecv(void* pp, void (*callback)())
{
    pthread_setname_np(pthread_self(), "UnixSocketRecv");

    uint32_t bufferSize = MEMORY_BLOCK_UNIT;

    SocketContext* pssr = (SocketContext*)pp;
    std::vector<unsigned char> buf(bufferSize);

    struct ProtocolHead* pph = (struct ProtocolHead*)buf.data();
    uint32_t head_size = sizeof(struct ProtocolHead);

    CHECK_TRUE(pssr->socketHandle_ != -1, nullptr, "UnixSocketRecv pssr->socketHandle_ ==-1");
    while (pssr->socketHandle_ >= 0) {
        if (!ReceiveData(pssr->socketHandle_, buf.data(), head_size)) {
            PROFILER_LOG_DEBUG(LOG_CORE, "====== IPC LOST CONNECT ======");
            break;
        }
        if (pph->protoSize > bufferSize) {
            if (pph->protoSize > PROTO_SIZE_MAX) {
                PROFILER_LOG_ERROR(LOG_CORE, "buffer size out of range %d/%d", pph->protoSize, PROTO_SIZE_MAX);
                break;
            }
            bufferSize = (pph->protoSize / MEMORY_BLOCK_UNIT + 1) * MEMORY_BLOCK_UNIT;
            buf.resize(bufferSize);
            pph = (struct ProtocolHead*)buf.data();
        }
        if (pph->protoSize < head_size) {
            PROFILER_LOG_ERROR(LOG_CORE, "pph->protoSize is less than head_size!");
            break;
        }
        if (!ReceiveData(pssr->socketHandle_, buf.data() + head_size, pph->protoSize - head_size)) {
            PROFILER_LOG_DEBUG(LOG_CORE, "====== IPC LOST CONNECT ======");
            break;
        }
        switch (pph->protoType & PROTOCOL_TYPE_FILTER) {
            case PROTOCOL_TYPE_RAW: {
                pssr->RawProtocolProc(pph->protoType & (~PROTOCOL_TYPE_FILTER), pph->datas, pph->protoSize - head_size);
                break;
            }
            case PROTOCOL_TYPE_PROTOBUF:
                if (pssr->serviceBase_ != nullptr) {
                    pssr->serviceBase_->ProtocolProc(*pssr, pph->protoType & (~PROTOCOL_TYPE_FILTER), pph->datas,
                                                     pph->protoSize - head_size);
                }
                break;
            default:
                PROFILER_LOG_ERROR(LOG_CORE, "unknown protocol %d", pph->protoType);
                break;
        }
    }
    if (callback) {
        callback();
    }
    pssr->clientState_ = CLIENT_STAT_THREAD_EXITED;
    PROFILER_LOG_DEBUG(LOG_CORE, "UnixSocketRecv thread exit");
    return nullptr;
}

bool SocketContext::CreateRecvThread(void (*callback)())
{
    recvThread_ = std::thread([this, callback] { this->UnixSocketRecv(this, callback); });
    CHECK_TRUE(recvThread_.get_id() != std::thread::id(), false, "CreateRecvThread FAIL");
    return true;
}

bool SocketContext::SendRaw(uint32_t pnum, const int8_t* data, uint32_t size, int sockfd)
{
    CHECK_NOTNULL(data, false, "SendRaw data null");
    if (sockfd == -1) {
        sockfd = socketHandle_;
    }
    struct ProtocolHead phead;
    phead.protoType = PROTOCOL_TYPE_RAW | pnum;
    phead.protoSize = size + sizeof(struct ProtocolHead);
    CHECK_TRUE(send(sockfd, reinterpret_cast<int8_t*>(&phead), sizeof(struct ProtocolHead), 0) != -1, false,
               "SendRaw Send Head ERR :%d", errno);

    CHECK_TRUE(send(sockfd, data, size, 0) != -1, false, "SendRaw Send Data ERR : %d", errno);
    return true;
}

#ifndef NO_PROTOBUF
bool SocketContext::SendProtobuf(uint32_t pnum, google::protobuf::Message& pmsg)
{
    int size = pmsg.ByteSizeLong();
    CHECK_TRUE(size > 0, false, "%s:size less than or equal to 0", __func__);
    int8_t* data = reinterpret_cast<int8_t*>(malloc(size));
    if (data == nullptr) {
        return false;
    }
    pmsg.SerializeToArray(data, size);
    struct ProtocolHead phead;
    phead.protoType = PROTOCOL_TYPE_PROTOBUF | pnum;
    phead.protoSize = size + sizeof(struct ProtocolHead);
    send(socketHandle_, reinterpret_cast<int8_t*>(&phead), sizeof(struct ProtocolHead), 0);
    send(socketHandle_, data, size, 0);

    free(data);
    return true;
}
#endif

bool SocketContext::SendHookConfig(const uint8_t* config, size_t len)
{
    if (config == nullptr || len == 0) {
        return false;
    }

    struct ProtocolHead phead;
    phead.protoType = PROTOCOL_TYPE_PROTOBUF;
    phead.protoSize = len + sizeof(struct ProtocolHead);
    send(socketHandle_, reinterpret_cast<int8_t*>(&phead), sizeof(struct ProtocolHead), 0);
    send(socketHandle_, config, len, 0);
    return true;
}

bool SocketContext::SendHeartBeat()
{
    struct ProtocolHead phead;
    phead.protoType = PROTOCOL_TYPE_PROTOBUF;
    phead.protoSize = sizeof(HEARTBEAT_MSG) + sizeof(struct ProtocolHead);

    if (send(socketHandle_, reinterpret_cast<int8_t*>(&phead), sizeof(struct ProtocolHead), 0) == -1) {
        return false;
    }

    if (send(socketHandle_, &HEARTBEAT_MSG, sizeof(HEARTBEAT_MSG), 0) == -1) {
        return false;
    }

    return true;
}

bool SocketContext::IsConnected()
{
    char buf[1];
    ssize_t recv_bytes = recv(socketHandle_, buf, sizeof(buf), MSG_DONTWAIT);
    if (recv_bytes == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "Peer Process Exceptionally Disconnected");
        return false;
    } else if (recv_bytes > 0) {
        // This is not supposed to happen because currently native_deomon does not send data to the client.
        return true;
    } else {
        return errno == EAGAIN || errno == EWOULDBLOCK;
    }
}

bool SocketContext::SendFileDescriptor(int fd)
{
    struct msghdr msg = {0};
    struct cmsghdr* cmsg = nullptr;
    char buf[CMSG_SPACE(1 * sizeof(int))] = {0};
    char data;
    if (memset_s(buf, sizeof(buf), 0, sizeof(buf)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memset_s error!");
    }
    struct iovec io = {.iov_base = &data, .iov_len = 1};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int) * 1);
    if (memcpy_s(CMSG_DATA(cmsg), 1 * sizeof(int), &fd, 1 * sizeof(int)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s error");
    }

    CHECK_TRUE(sendmsg(socketHandle_, &msg, 0) != -1, false, "SendFileDescriptor FAIL");
    return true;
}

int SocketContext::ReceiveFileDiscriptor()
{
    struct msghdr msg = {0};
    struct cmsghdr* cmsg = nullptr;
    char buf[CMSG_SPACE(1 * sizeof(int))] = {0};
    char data;
    struct iovec io = {.iov_base = &data, .iov_len = 1};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    CHECK_TRUE(recvmsg(socketHandle_, &msg, 0) != -1, -1, "ReceiveFileDiscriptor FAIL");
    cmsg = CMSG_FIRSTHDR(&msg);

    return cmsg ? *(int*)CMSG_DATA(cmsg) : -1;
}
