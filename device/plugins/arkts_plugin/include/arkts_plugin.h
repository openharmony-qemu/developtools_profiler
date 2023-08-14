/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef ARKTS_PLUGIN_H
#define ARKTS_PLUGIN_H

#include "arkts_plugin_config.pb.h"
#include "plugin_module_api.h"
#include "schedule_task_manager.h"

struct WebSocketFrame {
    uint8_t fin;
    uint8_t opCode;
    uint8_t mask;
    char maskingKey[5];
    uint64_t payloadLen;
    std::unique_ptr<char[]> payload;
};

class ArkTSPlugin {
public:
    ArkTSPlugin() = default;
    ~ArkTSPlugin() = default;
    int32_t Start(const uint8_t* configData, uint32_t configSize);
    int32_t Stop();
    void SetWriter(WriterStruct* writer);

private:
    void Snapshot();
    void FlushData();
    bool ClientConnectUnixWebSocket(const std::string& sockName, uint32_t timeoutLimit = 0);
    bool ClientSendWSUpgradeReq();
    bool ClientRecvWSUpgradeRsp();
    bool ClientSendReq(const std::string& message);
    void Close();
    bool SetWebSocketTimeOut(int32_t fd, uint32_t timeoutLimit);
    std::string Decode();
    bool HandleFrame(WebSocketFrame& wsFrame);
    bool DecodeMessage(WebSocketFrame& wsFrame);
    uint64_t NetToHostLongLong(char* buf, uint32_t len);
    bool Recv(int32_t client, char* buf, size_t totalLen, int32_t flags) const;

private:
    ArkTSConfig protoConfig_;
    WriterStruct* resultWriter_{nullptr};
    int32_t pid_{0};
    std::vector<char> buffer_;
    std::string snapshotCmd_;
    std::string timelineCmd_;
    ScheduleTaskManager scheduleTaskManager_;
    int32_t client_{-1};
    enum SocketState : uint8_t {
        UNINITED,
        INITED,
        CONNECTED,
    };
    std::atomic<SocketState> socketState_{SocketState::UNINITED};
};

#endif // ARKTS_PLUGIN_H