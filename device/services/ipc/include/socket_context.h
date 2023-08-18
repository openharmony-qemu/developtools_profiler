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

#ifndef SOCKET_CONTEXT_H
#define SOCKET_CONTEXT_H

#include <cstdint>
#ifndef NO_PROTOBUF
#include <google/protobuf/message.h>
#endif
#include <thread>

#if defined(__i386__) || defined(__x86_64__)
const static char DEFAULT_UNIX_SOCKET_PATH[] = "hiprofiler_unix_socket";
const static char DEFAULT_UNIX_SOCKET_HOOK_PATH[] = "hook_unix_socket";
#else
const static char DEFAULT_UNIX_SOCKET_PATH[] = "hiprofiler_unix_socket";
const static char DEFAULT_UNIX_SOCKET_HOOK_PATH[] = "hook_unix_socket";

const static char DEFAULT_UNIX_SOCKET_FULL_PATH[] = "/dev/unix/socket/hiprofiler_unix_socket";
const static char DEFAULT_UNIX_SOCKET_HOOK_FULL_PATH[] = "/dev/unix/socket/hook_unix_socket";
#endif

class ServiceBase;

enum ClientState {
    CLIENT_STAT_WORKING,
    CLIENT_STAT_WAIT_THREAD_EXIT,
    CLIENT_STAT_THREAD_EXITED,
};

enum RawProtocol {
    RAW_PROTOCOL_POINTTO_SERVICE = 1,
};
struct RawPointToService {
    char serviceName_[64];
};

class SocketContext {
public:
    SocketContext();
    virtual ~SocketContext();

    bool SendRaw(uint32_t pnum, const int8_t* data, uint32_t size, int sockfd = -1);
#ifndef NO_PROTOBUF
    bool SendProtobuf(uint32_t pnum, google::protobuf::Message& pmsg);
#endif
    bool SendFileDescriptor(int fd);
    bool SendHookConfig(const uint8_t* config, size_t len);
    bool SendHeartBeat();
    int ReceiveFileDiscriptor();
    enum ClientState GetClientState() { return clientState_; }
protected:
    int socketHandle_;
    bool CreateRecvThread();
    enum ClientState clientState_;
    uint64_t lastProcMS_;

    ServiceBase* serviceBase_;

    virtual int RawProtocolProc(uint32_t pnum, const int8_t* buf, const uint32_t size);

private:
    static bool ReceiveData(int sock, uint8_t* databuf, uint32_t size);
    static void* UnixSocketRecv(void* pp);
    std::thread recvThread_;
};

#endif