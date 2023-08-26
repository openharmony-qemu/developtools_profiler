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

#ifndef RPC_HTTPSOCKET_H
#define RPC_HTTPSOCKET_H

#include <string>
namespace SysTuning {
namespace TraceStreamer {
class HttpSocket {
public:
    HttpSocket() {}
    HttpSocket(int32_t sockId, int32_t domain) : sockId_(sockId), domain_(domain) {}
    ~HttpSocket();

    bool CreateSocket(int32_t domain);
    bool Bind(int32_t port);
    bool Listen(int32_t maxConn);
    bool Accept(HttpSocket& client);
    bool Recv(void* data, size_t& len);
    bool Send(const void* data, size_t len);
    void Close();
    bool IsValid() const
    {
        return sockId_ != INVALID_SOCKET;
    }
    int32_t GetFd() const
    {
        return sockId_;
    }

private:
    int32_t sockId_ = -1;
    int32_t domain_ = 0;
    using SOCKET = int32_t;
#if !is_mingw
    const int32_t SOCKET_ERROR = -1;
#endif
    const SOCKET INVALID_SOCKET = -1;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // RPC_HTTPSOCKET_H
