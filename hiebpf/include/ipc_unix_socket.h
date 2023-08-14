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
#ifndef HIEBPF_IPC_UNIX_SOCKET_H_
#define HIEBPF_IPC_UNIX_SOCKET_H_

#include <functional>
#include <string>
#include <thread>

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
const std::string UNIX_SOCKET_DEFAULT_PATHNAME = "/dev/unix/socket/hiebpf";
const size_t UNIX_SOCKET_BUFFER_SIZE = 64;
// this Unix socket server don't support multiple clients
const int UNIX_SOCKET_LISTEN_COUNT = 1;

class IpcUnixSocketServer {
public:
    IpcUnixSocketServer();
    ~IpcUnixSocketServer();

    bool Start(const std::string &pathname = UNIX_SOCKET_DEFAULT_PATHNAME);
    bool Stop();

    using HandleCallBack = std::function<void(const void *buf, size_t size)>;
    void SetHandleCallback(HandleCallBack fn)
    {
        handleMessageFn_ = fn;
    }

    // SendMessage can be called by HandleCallBack
    bool SendMessage(const void *buf, size_t size);

private:
    std::string pathName_;
    std::thread handleThread_;
    HandleCallBack handleMessageFn_;
    int serverFd_ = -1;
    int clientFd_ = -1;
    bool isRunning_ = false;

    void handleThreadLoop();
};

class IpcUnixSocketClient {
public:
    IpcUnixSocketClient();
    ~IpcUnixSocketClient();

    bool Connect(const std::string &pathname = UNIX_SOCKET_DEFAULT_PATHNAME);
    void Disconnect();

    bool SendMessage(const void *buf, size_t size);
    /* size is size of buf when input, is size of recved data when output.
       timeout is milliseconds */
    bool RecvMessage(void *buf, size_t &size, uint32_t timeout);

private:
    int sockFd_ = -1;
};
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
#endif // HIEBPF_IPC_UNIX_SOCKET_H_
