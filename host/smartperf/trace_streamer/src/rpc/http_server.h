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

#ifndef RPC_HTTPD_H
#define RPC_HTTPD_H

#include <atomic>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include "http_socket.h"
#include "rpc_server.h"
namespace SysTuning {
namespace TraceStreamer {
class HttpServer {
public:
    void RegisterRpcFunction(RpcServer* rpc);
    void Run(int32_t port = 9001);
    void Exit();

    static constexpr size_t MAXLEN_REQUEST = 2 * 1024 + 1024 * 1024; // header 2K + body 1M

private:
    struct ClientThread {
        HttpSocket sock_;
        std::thread thread_;
    };

    enum RequstParseStat { INIT = 0, OK, BAD, RECVING };

    struct RequestST {
        int32_t stat = RequstParseStat::INIT;
        std::string method;
        std::string uri;
        const uint8_t* body;
        size_t bodyLen;
    };

    bool CreateSocket(int32_t port);
    void ProcessClient(HttpSocket& client);
    void ProcessRequest(HttpSocket& client, RequestST& request);
    static void HttpResponse(HttpSocket& client, const std::string& status, bool hasBody = false);
    static void ParseRequest(const uint8_t* requst, size_t& len, RequestST& httpReq);
    void ClearDeadClientThread();
    static std::vector<std::string_view> StringSplit(std::string_view source, std::string_view split);
    void CloseAllThreads();
    bool ProcessAndParseReq(size_t& recvPos,
                            size_t& recvLen,
                            std::vector<uint8_t>& recvBuf,
                            RequestST& reqST,
                            HttpSocket& client);

    static const int32_t COUNT_SOCKET = 1;
    HttpSocket sockets_[COUNT_SOCKET]; // ipv4 and ipv6
    std::atomic_bool isExit_ = {false};
    std::vector<std::unique_ptr<ClientThread>> clientThreads_;
    using RpcFunction = std::function<bool(const uint8_t*, size_t, RpcServer::ResultCallBack)>;
    std::map<std::string_view, RpcFunction> rpcFunctions_;
    const int32_t pollTimeOut_ = 1000;
#ifdef _WIN32
    const uint32_t WS_VERSION_FIRST = 2;
    const uint32_t WS_VERSION_SEC = 2;
#endif
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // RPC_HTTPD_H
