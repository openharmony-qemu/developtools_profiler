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

#include "http_server.h"
#include <chrono>
#include <csignal>
#include <cstdint>
#include <strings.h> // for macx
#include <thread>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <poll.h>
#include <sys/socket.h>
#endif
#include "log.h"
#include "string_to_numerical.h"
namespace SysTuning {
namespace TraceStreamer {
void HttpServer::RegisterRpcFunction(RpcServer* rpc)
{
    rpcFunctions_.clear();

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    auto parsedata = std::bind(&RpcServer::ParseData, rpc, _1, _2, _3);
    rpcFunctions_["/parsedata"] = parsedata;

    auto parsedataover = std::bind(&RpcServer::ParseDataOver, rpc, _1, _2, _3);
    rpcFunctions_["/parsedataover"] = parsedataover;

    auto sqlquery = std::bind(&RpcServer::SqlQuery, rpc, _1, _2, _3);
    rpcFunctions_["/sqlquery"] = sqlquery;

    auto sqloperate = std::bind(&RpcServer::SqlOperate, rpc, _1, _2, _3);
    rpcFunctions_["/sqloperate"] = sqloperate;

    auto reset = std::bind(&RpcServer::Reset, rpc, _1, _2, _3);
    rpcFunctions_["/reset"] = reset;
}

void HttpServer::CloseAllThreads()
{
    for (const auto& it : clientThreads_) {
        if (it->thread_.joinable()) {
            it->thread_.join();
            it->sock_.Close();
        }
    }
}

#ifdef _WIN32
void HttpServer::Run(int32_t port)
{
    WSADATA ws{};
    if (WSAStartup(MAKEWORD(WS_VERSION_FIRST, WS_VERSION_SEC), &ws) != 0) {
        return;
    }
    if (!CreateSocket(port)) {
        return;
    }
    WSAEVENT events[COUNT_SOCKET];
    for (int32_t i = 0; i < COUNT_SOCKET; i++) {
        if ((events[i] = WSACreateEvent()) == WSA_INVALID_EVENT) {
            TS_LOGE("WSACreateEvent error %d", WSAGetLastError());
            return;
        }
        WSAEventSelect(sockets_[i].GetFd(), events[i], FD_ACCEPT | FD_CLOSE);
    }

    while (!isExit_) {
        ClearDeadClientThread();

        int32_t index = WSAWaitForMultipleEvents(COUNT_SOCKET, events, false, pollTimeOut_, false);
        if (index == WSA_WAIT_FAILED) {
            TS_LOGE("WSAWaitForMultipleEvents error %d", WSAGetLastError());
            break;
        } else if (index == WSA_WAIT_TIMEOUT) {
            continue;
        }

        index = index - WSA_WAIT_EVENT_0;
        WSANETWORKEVENTS event;
        WSAEnumNetworkEvents(sockets_[index].GetFd(), events[index], &event);
        if (event.lNetworkEvents & FD_ACCEPT) {
            if (event.iErrorCode[FD_ACCEPT_BIT] != 0) {
                continue;
            }

            std::unique_ptr<ClientThread> client = std::make_unique<ClientThread>();
            if (sockets_[index].Accept(client->sock_)) {
                client->thread_ = std::thread(&HttpServer::ProcessClient, this, std::ref(client->sock_));
                clientThreads_.push_back(std::move(client));
            } else {
                TS_LOGE("http socket accept error");
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
    CloseAllThreads();
    clientThreads_.clear();

    WSACleanup();
}
#else
void HttpServer::Run(int32_t port)
{
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        return;
    }

    if (!CreateSocket(port)) {
        return;
    }
    TS_LOGI("http server running");
    struct pollfd fds[COUNT_SOCKET];
    for (int32_t i = 0; i < COUNT_SOCKET; i++) {
        fds[i] = {sockets_[i].GetFd(), POLLIN, 0};
    }
    while (!isExit_) {
        ClearDeadClientThread();
        if (poll(fds, sizeof(fds) / sizeof(pollfd), pollTimeOut_) <= 0) {
            continue; // try again
        }

        for (int32_t i = 0; i < 1; i++) {
            if (fds[i].revents != POLLIN) {
                continue;
            }
            std::unique_ptr<ClientThread> client = std::make_unique<ClientThread>();
            if (sockets_[i].Accept(client->sock_)) {
                client->thread_ = std::thread(&HttpServer::ProcessClient, this, std::ref(client->sock_));
                clientThreads_.push_back(std::move(client));
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
    CloseAllThreads();
    clientThreads_.clear();

    for (int32_t i = 0; i < COUNT_SOCKET; i++) {
        sockets_[i].Close();
    }
    TS_LOGI("http server exit");
}
#endif

void HttpServer::Exit()
{
    isExit_ = true;
    for (int32_t i = 0; i < COUNT_SOCKET; i++) {
        sockets_[i].Close();
    }
}

bool HttpServer::CreateSocket(int32_t port)
{
    for (int32_t i = 0; i < COUNT_SOCKET; i++) {
        if (!sockets_[i].CreateSocket(i == 0 ? AF_INET : AF_INET6)) {
            TS_LOGE("Create http socket error");
            return false;
        }
        if (!sockets_[i].Bind(port)) {
            TS_LOGE("bind http socket error");
            return false;
        }
        if (!sockets_[i].Listen(SOMAXCONN)) {
            TS_LOGE("listen http socket error");
            return false;
        }
    }

    return true;
}

void HttpServer::ClearDeadClientThread()
{
    for (auto it = clientThreads_.begin(); it != clientThreads_.end();) {
        if (it->get()->sock_.GetFd() != -1) {
            it++;
            continue;
        }
        if (it->get()->thread_.joinable()) {
            it->get()->thread_.join();
        }
        it = clientThreads_.erase(it);
    }
}

bool HttpServer::ProcessAndParseReq(size_t& recvPos,
                                    size_t& recvLen,
                                    std::vector<uint8_t>& recvBuf,
                                    RequestST& reqST,
                                    HttpSocket& client)
{
    if (!client.Recv(recvBuf.data() + recvPos, recvLen)) {
        return false;
    }
    recvPos += recvLen;
    ParseRequest(recvBuf.data(), recvPos, reqST);
    recvLen = recvBuf.size() - recvPos;
    if (reqST.stat == RequstParseStat::RECVING) {
        return true;
    }
    ProcessRequest(client, reqST);
    reqST.stat = RequstParseStat::INIT;
    return true;
}

#ifdef _WIN32
void HttpServer::ProcessClient(HttpSocket& client)
{
    std::vector<uint8_t> recvBuf(MAXLEN_REQUEST);
    size_t recvLen = recvBuf.size();
    size_t recvPos = 0;
    RequestST reqST;
    WSAEVENT recvEvent = WSACreateEvent();
    if (recvEvent == WSA_INVALID_EVENT) {
        TS_LOGE("WSACreateEvent error %d", WSAGetLastError());
        return;
    }
    WSAEventSelect(client.GetFd(), recvEvent, FD_READ | FD_CLOSE);
    while (!isExit_) {
        int32_t index = WSAWaitForMultipleEvents(1, &recvEvent, false, pollTimeOut_, false);
        if (index == WSA_WAIT_FAILED) {
            TS_LOGE("WSAWaitForMultipleEvents error %d", WSAGetLastError());
            break;
        } else if (index == WSA_WAIT_TIMEOUT) {
            if (reqST.stat != RequstParseStat::INIT) {
                ProcessRequest(client, reqST);
                reqST.stat = RequstParseStat::INIT;
                recvPos = 0;
                recvLen = recvBuf.size();
            }
            continue;
        }

        WSANETWORKEVENTS event;
        WSAEnumNetworkEvents(client.GetFd(), recvEvent, &event);
        if (event.lNetworkEvents & FD_READ) {
            if (event.iErrorCode[FD_READ_BIT] != 0) {
                continue;
            }
            if (!ProcessAndParseReq(recvPos, recvLen, recvBuf, reqST, client)) {
                break;
            }
        } else if (event.lNetworkEvents & FD_CLOSE) {
            TS_LOGI("client close socket(%d)", client.GetFd());
            break;
        }
    }
    TS_LOGI("recive client thread exit. socket(%d)", client.GetFd());

    client.Close();
}
#else
void HttpServer::ProcessClient(HttpSocket& client)
{
    std::vector<uint8_t> recvBuf(MAXLEN_REQUEST);
    size_t recvLen = recvBuf.size();
    size_t recvPos = 0;
    RequestST reqST;

    struct pollfd fd = {client.GetFd(), POLLIN, 0};
    while (!isExit_) {
        int32_t pollRet = poll(&fd, sizeof(fd) / sizeof(pollfd), pollTimeOut_);
        if (pollRet < 0) {
            TS_LOGE("poll client socket(%d) error: %d:%s", client.GetFd(), errno, strerror(errno));
            break;
        }
        if (pollRet == 0) {
            if (reqST.stat != RequstParseStat::INIT) {
                ProcessRequest(client, reqST);
                reqST.stat = RequstParseStat::INIT;
                recvPos = 0;
                recvLen = recvBuf.size();
            }
            continue;
        }
        if (!ProcessAndParseReq(recvPos, recvLen, recvBuf, reqST, client)) {
            break;
        }
    }
    TS_LOGI("recive client thread exit. socket(%d)", client.GetFd());

    client.Close();
    TS_LOGI("thread exit");
}
#endif

void HttpServer::ProcessRequest(HttpSocket& client, RequestST& request)
{
    if (request.stat == RequstParseStat::RECVING) {
        TS_LOGE("http request data missing, client %d\n", client.GetFd());
        HttpResponse(client, "408 Request Time-out\r\n");
        return;
    } else if (request.stat != RequstParseStat::OK) {
        TS_LOGE("bad http request, client %d\n", client.GetFd());
        HttpResponse(client, "400 Bad Request\r\n");
        return;
    }
    if (request.method == "OPTIONS") {
        HttpResponse(client,
                     "204 No Content\r\n"
                     "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                     "Access-Control-Allow-Headers: *\r\n"
                     "Access-Control-Max-Age: 86400\r\n");
        return;
    } else if (request.method != "POST" && request.method != "GET") {
        TS_LOGE("method(%s) not allowed, client %d", request.method.c_str(), client.GetFd());
        HttpResponse(client, "405 Method Not Allowed\r\n");
        return;
    }
    auto it = rpcFunctions_.find(request.uri);
    if (it == rpcFunctions_.end()) {
        TS_LOGE("http uri(%s) not found, client %d", request.uri.c_str(), client.GetFd());
        HttpResponse(client, "404 Not Found\r\n");
        return;
    }
    HttpResponse(client, "200 OK\r\n", true);
    auto resultCallback = [&client](const std::string& result, int32_t) {
        std::stringstream chunkLenbuff;
        chunkLenbuff << std::hex << result.size() << "\r\n";
        if (!client.Send(chunkLenbuff.str().data(), chunkLenbuff.str().size())) {
            TS_LOGE("send client socket(%d) error", client.GetFd());
            return;
        }
        if (!client.Send(result.data(), result.size())) {
            TS_LOGE("send client socket(%d) error", client.GetFd());
            return;
        }
        if (!client.Send("\r\n", strlen("\r\n"))) {
            TS_LOGE("send client socket(%d) error", client.GetFd());
            return;
        }
    };
    it->second(request.body, request.bodyLen, resultCallback);
    if (!client.Send("0\r\n\r\n", strlen("0\r\n\r\n"))) { // chunk tail
        TS_LOGE("send client socket(%d) error", client.GetFd());
    }
}

void HttpServer::ParseRequest(const uint8_t* requst, size_t& len, RequestST& httpReq)
{
    std::string_view reqStr(reinterpret_cast<const char*>(requst), len);
    size_t bodyPos = reqStr.find("\r\n\r\n");
    if (bodyPos == 0) {
        len = 0;
        httpReq.stat = RequstParseStat::BAD;
        return;
    } else if (bodyPos == std::string_view::npos) {
        httpReq.stat = RequstParseStat::RECVING;
        return;
    }
    std::string_view header = reqStr.substr(0, bodyPos);
    bodyPos += strlen("\r\n\r\n");
    httpReq.bodyLen = reqStr.size() - bodyPos;

    std::vector<std::string_view> headerlines = StringSplit(header, "\r\n");
    // at least 1 line in headerlines, such as "GET /parsedata HTTP/1.1"
    std::vector<std::string_view> requestItems = StringSplit(headerlines[0], " ");
    const size_t indexHttpMethod = 0;
    const size_t indexHttpUri = 1;
    const size_t indexHttpVersion = 2;
    const size_t countRequestItems = 3;
    if (requestItems.size() != countRequestItems || requestItems[indexHttpVersion] != "HTTP/1.1") {
        len = 0;
        httpReq.stat = RequstParseStat::BAD;
        return;
    }
    httpReq.method = requestItems[indexHttpMethod];
    httpReq.uri = requestItems[indexHttpUri];

    for (size_t i = 1; i < headerlines.size(); i++) {
        size_t tagPos = headerlines[i].find(":");
        if (tagPos == std::string_view::npos) {
            len = 0;
            httpReq.stat = RequstParseStat::BAD;
            return;
        }
        std::string_view tag = headerlines[i].substr(0, tagPos);
        if (strncasecmp(tag.data(), "Content-Length", tag.size()) == 0) {
            std::string value(headerlines[i].data() + tagPos + strlen(":"),
                              headerlines[i].size() - tagPos - strlen(":"));
            size_t conterntLen = atoi(value.c_str());
            if (conterntLen > httpReq.bodyLen) {
                httpReq.stat = RequstParseStat::RECVING;
                return;
            } else if (conterntLen < httpReq.bodyLen) {
                httpReq.bodyLen = conterntLen;
            }
        }
    }

    if (httpReq.bodyLen > 0) {
        httpReq.body = (requst + bodyPos);
    }
    httpReq.stat = RequstParseStat::OK;
    len -= (bodyPos + httpReq.bodyLen);
    return;
}

void HttpServer::HttpResponse(HttpSocket& client, const std::string& status, bool hasBody)
{
    std::string res;
    const size_t maxLenResponse = 1024;
    res.reserve(maxLenResponse);
    res += "HTTP/1.1 ";
    res += status;

    res += "Connection: Keep-Alive\r\n";
    if (hasBody) {
        res += "Content-Type: application/json\r\n";
        res += "Transfer-Encoding: chunked\r\n";
    }
    res += "\r\n";
    if (!client.Send(res.data(), res.size())) {
        TS_LOGE("send client socket(%d) error", client.GetFd());
    }
}

std::vector<std::string_view> HttpServer::StringSplit(std::string_view source, std::string_view split)
{
    std::vector<std::string_view> result;
    if (!split.empty()) {
        size_t pos = 0;
        while ((pos = source.find(split)) != std::string_view::npos) {
            // split
            std::string_view token = source.substr(0, pos);
            if (!token.empty()) {
                result.push_back(token);
            }
            source = source.substr(pos + split.size(), source.size() - token.size() - split.size());
        }
    }
    // add last token
    if (!source.empty()) {
        result.push_back(source);
    }
    return result;
}
} // namespace TraceStreamer
} // namespace SysTuning
