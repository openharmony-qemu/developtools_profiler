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
#ifndef SP_THREAD_SOCKET_H
#define SP_THREAD_SOCKET_H
#include "sp_server_socket.h"
namespace OHOS {
namespace SmartPerf {
enum class SocketConnectType {
    CMD_SOCKET,
    EDITOR_SOCKET,
};

enum class SocketErrorType {
    OK,
    TOKEN_CHECK_FAILED,
    INIT_FAILED,
    START_FAILED,
    STOP_FAILED,
    START_RECORD_FAILED,
    STOP_RECORD_FAILED,
};
class SpThreadSocket {
public:
    static SpThreadSocket &GetInstance()
    {
        static SpThreadSocket instance;
        return instance;
    }

    std::string MapToString(std::map<std::string, std::string> dataMap) const;
    std::string SplitMsg(const std::string &recvBuf) const;
    void Process(ProtoType type);
    SocketErrorType CheckToken(std::string recvStr, SpServerSocket &spSocket, std::string recvStrNoToken) const;
    void TypeTcp(SpServerSocket &spSocket);
    void InitRecv(std::string recvStr, SpServerSocket &spSocket, SocketConnectType type) const;
    void StartRecv(SpServerSocket &spSocket);
    void StartRecvRealtime(SpServerSocket &spSocket) const;
    void StopRecvRealtime(SpServerSocket &spSocket);
    void StartRecvRecord(SpServerSocket &spSocket) const;
    void StopRecvRecord(SpServerSocket &spSocket) const;
    void SendTokenFailedMessage(SpServerSocket &socket, std::string &message) const;
    void DealMsg(std::string recvStr, SpServerSocket &spSocket, SocketErrorType tokenStatus);
    void EditorRecv(std::string recvStr, const SpServerSocket &spSocket) const;
    void BackDesktop() const;
    void HandleMsg(SpServerSocket &spSocket) const;
    void HandleUDPMsg(SpServerSocket &spSocket, std::map<std::string, std::string> data, std::string retCode,
        std::unordered_map<MessageType, std::string>::const_iterator iterator) const;
    void SocketHeartbeat() const;
    void FetchCpuStats(SpServerSocket &spSocket, std::map<std::string, std::string> data) const;
    void HandleNullMsg(SpServerSocket &spSocket, SpProfiler *profiler, std::string retCode, std::string recvBuf,
        std::unordered_map<MessageType, std::string>::const_iterator iterator) const;
    void HandleNullAddMsg(SpServerSocket &spSocket, SpProfiler *profiler, std::string retCode, std::string recvBuf,
        std::unordered_map<MessageType, std::string>::const_iterator iterator) const;
    std::string SocketErrorTypeToString(SocketErrorType errorType) const;
    std::map<std::string, std::string> GetLogProcess(SpProfiler *profilerItem, std::string buffer) const;
    void ResetValue(std::string retCode) const;
    void GetProcessIdByPkgName(std::unordered_map<MessageType, std::string>::const_iterator iterator) const;

private:
    bool flagRunning = false;
    bool socketConnect = true;
};
}
}
#endif