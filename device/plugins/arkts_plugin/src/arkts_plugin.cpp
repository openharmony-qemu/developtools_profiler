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

#include "arkts_plugin.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <regex>
#include <sys/un.h>
#include <unistd.h>

#include "arkts_plugin_result.pb.h"
#include "logging.h"
#include "securec.h"

namespace {
const std::string PANDA = "PandaDebugger";
const std::string SNAPSHOT_HEAD =
    R"({"id":1,"method":"HeapProfiler.takeHeapSnapshot","params":{"reportProgress":true,"captureNumericValue":)";
const std::string SNAPSHOT_TAIL = R"(,"exposeInternals":false}})";
const std::string TIMELINE_HEAD =
    R"({"id":1,"method":"HeapProfiler.startTrackingHeapObjects","params":{"trackAllocations":)";
const std::string TIMELINE_TAIL = "}}";
const std::string TIMELINE_STOP =
    R"({"id":2,"method":"HeapProfiler.stopTrackingHeapObjects","params":{"reportProgress":true}})";
const std::string CPU_PROFILER_INTERVAL_HEAD =
    R"({"id":3,"method":"Profiler.setSamplingInterval","params":{"interval":)";
const std::string CPU_PROFILER_INTERVAL_TAIL = R"(}})";
const std::string CPU_PROFILER_START = R"({"id":3,"method":"Profiler.start","params":{}})";
const std::string CPU_PROFILER_STOP = R"({"id":3,"method":"Profiler.stop","params":{}})";
constexpr uint8_t TIMELINE_START_SUCCESS = 0x1;
constexpr uint8_t CPU_PROFILER_START_SUCCESS = 0x2;
const std::string RESPONSE_FLAG_HEAD = R"({"id":)";
const std::string RESPONSE_FLAG_TAIL = R"(,"result":{}})";
const std::string REGEX_PATTERN = R"("id":(\d+))";
const std::string ARKTS_SCHEDULE = R"(ArkTS_Snapshot)";
enum class HeapType : int32_t {
    INVALID = -1,
    SNAPSHOT,
    TIMELINE,
};
constexpr char CLIENT_WEBSOCKET_UPGRADE_REQ[] =
    "GET / HTTP/1.1\r\n"
    "Connection: Upgrade\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Upgrade: websocket\r\n"
    "Sec-WebSocket-Version: 13\r\n"
    "Accept-Encoding: gzip, deflate, br\r\n"
    "Sec-WebSocket-Key: 64b4B+s5JDlgkdg7NekJ+g==\r\n"
    "Sec-WebSocket-Extensions: permessage-deflate\r\n";
constexpr int32_t CLIENT_WEBSOCKET_UPGRADE_RSP_LEN = 129;
constexpr int32_t SOCKET_MASK_LEN = 4;
constexpr char MASK_KEY[SOCKET_MASK_LEN + 1] = "abcd";
constexpr uint32_t TIME_OUT = 5;
constexpr uint32_t TIME_BASE = 1000;
constexpr int32_t SOCKET_SUCCESS = 0;
constexpr int32_t SOCKET_HEADER_LEN = 2;
constexpr int32_t PAYLOAD_LEN = 2;
constexpr int32_t EXTEND_PAYLOAD_LEN = 8;
constexpr uint32_t CPU_PROFILER_INTERVAL_DEFAULT = 1000;
} // namespace

int32_t ArkTSPlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    if (protoConfig_.ParseFromArray(configData, configSize) <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:parseFromArray failed!", __func__);
        return -1;
    }

    if (!protoConfig_.split_outfile_name().empty()) {
        splitTraceWriter_ = std::make_shared<TraceFileWriter>(protoConfig_.split_outfile_name());
        splitTraceWriter_->WriteStandalonePluginData(
            std::string(g_pluginModule.name) + "_config",
            std::string(reinterpret_cast<const char *>(configData),
                        configSize));
        splitTraceWriter_->SetTimeSource();
    }

    pid_ = protoConfig_.pid();
    if (pid_ <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: pid is less than or equal to 0", __func__);
        return -1;
    }

    if (!ClientConnectUnixWebSocket(std::to_string(pid_) + PANDA, TIME_OUT)) {
        return -1;
    }

    if (!ClientSendWSUpgradeReq()) {
        return -1;
    }

    if (!ClientRecvWSUpgradeRsp()) {
        return -1;
    }

    if (protoConfig_.enable_cpu_profiler()) {
        if (EnableCpuProfiler() != 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "arkts plugin cpu profiler start failed");
            return -1;
        }
    }

    switch (static_cast<int32_t>(protoConfig_.type())) {
        case static_cast<int32_t>(HeapType::SNAPSHOT): {
            return EnableSnapshot();
        }
        case static_cast<int32_t>(HeapType::TIMELINE): {
            return EnableTimeline();
        }
        case static_cast<int32_t>(HeapType::INVALID): {
            PROFILER_LOG_INFO(LOG_CORE, "arkts plugin memory type is INVALID");
            return 0;
        }
        default: {
            PROFILER_LOG_ERROR(LOG_CORE, "arkts plugin start type error");
            return -1;
        }
    }
}

int32_t ArkTSPlugin::EnableTimeline()
{
    std::string timelineCmd = TIMELINE_HEAD + (protoConfig_.track_allocations() ? "true" : "false") + TIMELINE_TAIL;
    if (!ClientSendReq(timelineCmd)) {
        return -1;
    }
    FlushData(timelineCmd);
    commandResult_ |= TIMELINE_START_SUCCESS;
    return 0;
}

int32_t ArkTSPlugin::EnableSnapshot()
{
    if (protoConfig_.interval() == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:scheduleTask interval == 0 error!", __func__);
        return -1;
    }
    snapshotCmd_ = SNAPSHOT_HEAD + (protoConfig_.capture_numeric_value() ? "true" : "false") + SNAPSHOT_TAIL;
    auto callback = std::bind(&ArkTSPlugin::Snapshot, this);
    snapshotScheduleTaskFd_ = scheduleTaskManager_.ScheduleTask(callback, protoConfig_.interval() * TIME_BASE);
    if (snapshotScheduleTaskFd_ == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s:scheduleTask failed!", __func__);
        return -1;
    }
    return 0;
}

int32_t ArkTSPlugin::EnableCpuProfiler()
{
    std::string interval = CPU_PROFILER_INTERVAL_HEAD
        + (protoConfig_.cpu_profiler_interval() == 0 ?
        std::to_string(CPU_PROFILER_INTERVAL_DEFAULT) : std::to_string(protoConfig_.cpu_profiler_interval()))
        + CPU_PROFILER_INTERVAL_TAIL;
    if (!ClientSendReq(interval)) {
        return -1;
    }
    FlushData(interval);

    if (!ClientSendReq(CPU_PROFILER_START)) {
        return -1;
    }
    FlushData(CPU_PROFILER_START);
    commandResult_ |= CPU_PROFILER_START_SUCCESS;
    return 0;
}

int32_t ArkTSPlugin::Stop()
{
    switch (static_cast<int32_t>(protoConfig_.type())) {
        case static_cast<int32_t>(HeapType::SNAPSHOT): {
            scheduleTaskManager_.UnscheduleTask(snapshotScheduleTaskFd_);
            break;
        }
        case static_cast<int32_t>(HeapType::TIMELINE): {
            if (commandResult_ & TIMELINE_START_SUCCESS) {
                if (!ClientSendReq(TIMELINE_STOP)) {
                    break;
                }
                FlushData(TIMELINE_STOP);
            }
            break;
        }
        case static_cast<int32_t>(HeapType::INVALID): {
            break;
        }
        default: {
            PROFILER_LOG_ERROR(LOG_CORE, "arkts plugin stop type error");
            break;
        }
    }

    if (protoConfig_.enable_cpu_profiler() && (commandResult_ & CPU_PROFILER_START_SUCCESS)) {
        if (ClientSendReq(CPU_PROFILER_STOP)) {
            FlushData();
        }
    }
    Close();

    if (!protoConfig_.split_outfile_name().empty()) { // write split file.
        CHECK_NOTNULL(splitTraceWriter_, -1, "%s: writer is nullptr, WriteStandaloneFile failed", __func__);
        splitTraceWriter_->SetDurationTime();
        splitTraceWriter_->Finish();
        splitTraceWriter_.reset();
        splitTraceWriter_ = nullptr;
    }
    return 0;
}

void ArkTSPlugin::SetWriter(WriterStruct* writer)
{
    resultWriter_ = writer;
}

void ArkTSPlugin::Snapshot()
{
    CHECK_NOTNULL(resultWriter_, NO_RETVAL, "%s: resultWriter_ nullptr", __func__);
    if (!ClientSendReq(snapshotCmd_)) {
        return;
    }
    FlushData(snapshotCmd_);
}

void ArkTSPlugin::FlushData(const std::string& command)
{
    std::string endFlag;
    if (!command.empty()) {
        std::regex pattern(REGEX_PATTERN);
        std::smatch match;
        if (std::regex_search(command, match, pattern)) {
            endFlag = RESPONSE_FLAG_HEAD + match[1].str() + RESPONSE_FLAG_TAIL;
        }
    }
    if (!protoConfig_.split_outfile_name().empty()) {
        CHECK_NOTNULL(splitTraceWriter_, NO_RETVAL, "%s: writer is nullptr, WriteStandaloneFile failed", __func__);
    }

    while (true) {
        std::string recv = Decode();
        if (recv.empty()) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: recv is empty", __func__);
            break;
        }
        ArkTSResult data;
        data.set_result(recv.c_str(), recv.size());
        buffer_.resize(data.ByteSizeLong());
        data.SerializeToArray(buffer_.data(), buffer_.size());

        if (protoConfig_.split_outfile_name().empty()) {
            resultWriter_->write(resultWriter_, buffer_.data(), buffer_.size());
            resultWriter_->flush(resultWriter_);
        } else { // write split file.
            splitTraceWriter_->WriteStandalonePluginData(
                std::string(g_pluginModule.name),
                std::string(buffer_.data(), buffer_.size()),
                std::string(g_pluginModule.version));
        }

        if (endFlag.empty() || recv == endFlag) {
            break;
        }
    }

    if (!protoConfig_.split_outfile_name().empty()) {
        splitTraceWriter_->Flush();
    }
}

bool ArkTSPlugin::ClientConnectUnixWebSocket(const std::string& sockName, uint32_t timeoutLimit)
{
    if (socketState_ != SocketState::UNINITED) {
        PROFILER_LOG_ERROR(LOG_CORE, "client has inited");
        return true;
    }

    client_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_ < SOCKET_SUCCESS) {
        PROFILER_LOG_ERROR(LOG_CORE, "client socket failed, error = %d, , desc = %s", errno, strerror(errno));
        return false;
    }

    // set send and recv timeout limit
    if (!SetWebSocketTimeOut(client_, timeoutLimit)) {
        PROFILER_LOG_ERROR(LOG_CORE, "client SetWebSocketTimeOut failed, error = %d, desc = %s",
                           errno, strerror(errno));
        close(client_);
        client_ = -1;
        return false;
    }

    struct sockaddr_un serverAddr;
    if (memset_s(&serverAddr, sizeof(serverAddr), 0, sizeof(serverAddr)) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "client memset_s serverAddr failed, error = %d, desc = %s",
                           errno, strerror(errno));
        close(client_);
        client_ = -1;
        return false;
    }
    serverAddr.sun_family = AF_UNIX;
    if (strcpy_s(serverAddr.sun_path + 1, sizeof(serverAddr.sun_path) - 1, sockName.c_str()) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "client strcpy_s serverAddr.sun_path failed, error = %d, , desc = %s",
                           errno, strerror(errno));
        close(client_);
        client_ = -1;
        return false;
    }
    serverAddr.sun_path[0] = '\0';

    uint32_t len = offsetof(struct sockaddr_un, sun_path) + strlen(sockName.c_str()) + 1;
    int ret = connect(client_, reinterpret_cast<struct sockaddr*>(&serverAddr), static_cast<int32_t>(len));
    if (ret != SOCKET_SUCCESS) {
        PROFILER_LOG_ERROR(LOG_CORE, "client connect failed, error, error = %d, , desc = %s", errno, strerror(errno));
        close(client_);
        client_ = -1;
        return false;
    }
    socketState_ = SocketState::INITED;
    PROFILER_LOG_INFO(LOG_CORE, "client connect success...");
    return true;
}

bool ArkTSPlugin::ClientSendWSUpgradeReq()
{
    if (socketState_ == SocketState::UNINITED) {
        PROFILER_LOG_ERROR(LOG_CORE, "client has not inited");
        return false;
    }
    if (socketState_ == SocketState::CONNECTED) {
        PROFILER_LOG_ERROR(LOG_CORE, "client has connected");
        return true;
    }

    int msgLen = strlen(CLIENT_WEBSOCKET_UPGRADE_REQ);
    int32_t sendLen = send(client_, CLIENT_WEBSOCKET_UPGRADE_REQ, msgLen, 0);
    if (sendLen != msgLen) {
        PROFILER_LOG_ERROR(LOG_CORE, "client send wsupgrade req failed, error = %d, desc = %s", errno, strerror(errno));
        socketState_ = SocketState::UNINITED;
        shutdown(client_, SHUT_RDWR);
        close(client_);
        client_ = -1;
        return false;
    }
    PROFILER_LOG_INFO(LOG_CORE, "client send wsupgrade req success");
    return true;
}

bool ArkTSPlugin::ClientRecvWSUpgradeRsp()
{
    if (socketState_ == SocketState::UNINITED) {
        PROFILER_LOG_ERROR(LOG_CORE, "client has not inited");
        return false;
    }
    if (socketState_ == SocketState::CONNECTED) {
        PROFILER_LOG_ERROR(LOG_CORE, "ClientRecvWSUpgradeRsp::client has connected");
        return true;
    }

    char recvBuf[CLIENT_WEBSOCKET_UPGRADE_RSP_LEN + 1] = {0};
    int32_t bufLen = recv(client_, recvBuf, CLIENT_WEBSOCKET_UPGRADE_RSP_LEN, 0);
    if (bufLen != CLIENT_WEBSOCKET_UPGRADE_RSP_LEN) {
        PROFILER_LOG_ERROR(LOG_CORE, "client recv wsupgrade rsp failed, error = %d, desc = %s", errno, strerror(errno));
        socketState_ = SocketState::UNINITED;
        shutdown(client_, SHUT_RDWR);
        close(client_);
        client_ = -1;
        return false;
    }
    socketState_ = SocketState::CONNECTED;
    PROFILER_LOG_INFO(LOG_CORE, "client recv wsupgrade rsp success");
    return true;
}

bool ArkTSPlugin::ClientSendReq(const std::string& message)
{
    if (socketState_ != SocketState::CONNECTED) {
        PROFILER_LOG_ERROR(LOG_CORE, "client has not connected");
        return false;
    }

    uint32_t msgLen = message.length();
    std::unique_ptr<char[]> msgBuf = std::make_unique<char[]>(msgLen + 15); // 15: the maximum expand length
    char* sendBuf = msgBuf.get();
    uint32_t sendMsgLen = 0;
    sendBuf[0] = 0x81; // 0x81: the text message sent by the server should start with '0x81'.
    uint32_t mask = 1;
    // Depending on the length of the messages, client will use shift operation to get the res
    // and store them in the buffer.
    if (msgLen <= 125) {                     // 125: situation 1 when message's length <= 125
        sendBuf[1] = msgLen | (mask << 7);   // 7: mask need shift left by 7 bits
        sendMsgLen = 2;                      // 2: the length of header frame is 2;
    } else if (msgLen < 65536) {             // 65536: message's length
        sendBuf[1] = 126 | (mask << 7);      // 126: payloadLen according to the spec; 7: mask shift left by 7 bits
        sendBuf[2] = ((msgLen >> 8) & 0xff); // 8: shift right by 8 bits => res * (256^1)
        sendBuf[3] = (msgLen & 0xff);        // 3: store len's data => res * (256^0)
        sendMsgLen = 4;                      // 4: the length of header frame is 4
    } else {
        sendBuf[1] = 127 | (mask << 7);    // 127: payloadLen according to the spec; 7: mask shift left by 7 bits
        for (int32_t i = 2; i <= 5; i++) { // 2 ~ 5: unused bits
            sendBuf[i] = 0;
        }
        sendBuf[6] = ((msgLen & 0xff000000) >> 24); // 6: shift 24 bits => res * (256^3)
        sendBuf[7] = ((msgLen & 0x00ff0000) >> 16); // 7: shift 16 bits => res * (256^2)
        sendBuf[8] = ((msgLen & 0x0000ff00) >> 8);  // 8: shift 8 bits => res * (256^1)
        sendBuf[9] = (msgLen & 0x000000ff);         // 9: res * (256^0)
        sendMsgLen = 10;                            // 10: the length of header frame is 10
    }

    if (memcpy_s(sendBuf + sendMsgLen, SOCKET_MASK_LEN, MASK_KEY, SOCKET_MASK_LEN) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "client memcpy_s MASK_KEY failed, error = %d, desc = %s", errno, strerror(errno));
        return false;
    }
    sendMsgLen += SOCKET_MASK_LEN;

    std::string maskMessage;
    for (uint64_t i = 0; i < msgLen; i++) {
        uint64_t j = i % SOCKET_MASK_LEN;
        maskMessage.push_back(message[i] ^ MASK_KEY[j]);
    }
    if (memcpy_s(sendBuf + sendMsgLen, msgLen, maskMessage.c_str(), msgLen) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "client memcpy_s maskMessage failed, error = %d, desc = %s",
                           errno, strerror(errno));
        return false;
    }
    msgBuf[sendMsgLen + msgLen] = '\0';

    if (send(client_, sendBuf, sendMsgLen + msgLen, 0) != static_cast<int>(sendMsgLen + msgLen)) {
        PROFILER_LOG_ERROR(LOG_CORE, "client send msg req failed, error = %d, desc = %s", errno, strerror(errno));
        return false;
    }
    PROFILER_LOG_INFO(LOG_CORE, "ClientRecvWSUpgradeRsp::client send msg req success...");
    return true;
}

void ArkTSPlugin::Close()
{
    if (socketState_ == SocketState::UNINITED) {
        PROFILER_LOG_ERROR(LOG_CORE, "client has not inited");
        return;
    }
    shutdown(client_, SHUT_RDWR);
    close(client_);
    client_ = -1;
    socketState_ = SocketState::UNINITED;
}

bool ArkTSPlugin::SetWebSocketTimeOut(int32_t fd, uint32_t timeoutLimit)
{
    if (timeoutLimit > 0) {
        struct timeval timeout = {timeoutLimit, 0};
        if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != SOCKET_SUCCESS) {
            return false;
        }
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != SOCKET_SUCCESS) {
            return false;
        }
    }
    return true;
}

std::string ArkTSPlugin::Decode()
{
    if (socketState_ != SocketState::CONNECTED) {
        PROFILER_LOG_ERROR(LOG_CORE, "client has not connected");
        return {};
    }
    char recvbuf[SOCKET_HEADER_LEN + 1];
    if (!Recv(client_, recvbuf, SOCKET_HEADER_LEN, 0)) {
        PROFILER_LOG_ERROR(LOG_CORE, "Decode failed, client websocket disconnect");
        socketState_ = SocketState::INITED;
        shutdown(client_, SHUT_RDWR);
        close(client_);
        client_ = -1;
        return {};
    }
    recvbuf[SOCKET_HEADER_LEN] = '\0';
    WebSocketFrame wsFrame;
    int32_t index = 0;
    wsFrame.fin = static_cast<uint8_t>(recvbuf[index] >> 7); // 7: shift right by 7 bits to get the fin
    wsFrame.opCode = static_cast<uint8_t>(recvbuf[index] & 0xf);
    if (wsFrame.opCode == 0x1) { // 0x1: 0x1 means a text frame
        index++;
        wsFrame.mask = static_cast<uint8_t>((recvbuf[index] >> 7) & 0x1); // 7: to get the mask
        wsFrame.payloadLen = recvbuf[index] & 0x7f;
        HandleFrame(wsFrame);
        return wsFrame.payload.get();
    }
    return std::string();
}

uint64_t ArkTSPlugin::NetToHostLongLong(char* buf, uint32_t len)
{
    uint64_t result = 0;
    for (uint32_t i = 0; i < len; i++) {
        result |= static_cast<unsigned char>(buf[i]);
        if ((i + 1) < len) {
            result <<= 8; // 8: result need shift left 8 bits in order to big endian convert to int
        }
    }
    return result;
}

bool ArkTSPlugin::HandleFrame(WebSocketFrame& wsFrame)
{
    if (wsFrame.payloadLen == 126) { // 126: the payloadLen read from frame
        char recvbuf[PAYLOAD_LEN + 1] = {0};
        if (!Recv(client_, recvbuf, PAYLOAD_LEN, 0)) {
            PROFILER_LOG_ERROR(LOG_CORE, "HandleFrame: Recv payloadLen == 126 failed");
            return false;
        }
        recvbuf[PAYLOAD_LEN] = '\0';
        uint16_t msgLen = 0;
        if (memcpy_s(&msgLen, sizeof(recvbuf), recvbuf, sizeof(recvbuf) - 1) != EOK) {
            return false;
        }
        wsFrame.payloadLen = ntohs(msgLen);
    } else if (wsFrame.payloadLen > 126) { // 126: the payloadLen read from frame
        char recvbuf[EXTEND_PAYLOAD_LEN + 1] = {0};
        if (!Recv(client_, recvbuf, EXTEND_PAYLOAD_LEN, 0)) {
            PROFILER_LOG_ERROR(LOG_CORE, "HandleFrame: Recv payloadLen > 127 failed");
            return false;
        }
        recvbuf[EXTEND_PAYLOAD_LEN] = '\0';
        wsFrame.payloadLen = NetToHostLongLong(recvbuf, EXTEND_PAYLOAD_LEN);
    }
    return DecodeMessage(wsFrame);
}

bool ArkTSPlugin::DecodeMessage(WebSocketFrame& wsFrame)
{
    if (wsFrame.payloadLen == 0 || wsFrame.payloadLen > UINT64_MAX) {
        return false;
    }
    wsFrame.payload = std::make_unique<char[]>(wsFrame.payloadLen + 1);
    if (wsFrame.mask == 1) {
        CHECK_TRUE(Recv(client_, wsFrame.maskingKey, SOCKET_MASK_LEN, 0), false,
                   "DecodeMessage: Recv maskingKey failed");
        wsFrame.maskingKey[SOCKET_MASK_LEN] = '\0';

        char buf[wsFrame.payloadLen + 1];
        CHECK_TRUE(Recv(client_, buf, wsFrame.payloadLen, 0), false, "DecodeMessage: Recv message with mask failed");
        buf[wsFrame.payloadLen] = '\0';

        for (uint64_t i = 0; i < wsFrame.payloadLen; i++) {
            uint64_t j = i % SOCKET_MASK_LEN;
            wsFrame.payload.get()[i] = buf[i] ^ wsFrame.maskingKey[j];
        }
    } else {
        if (!Recv(client_, wsFrame.payload.get(), wsFrame.payloadLen, 0)) {
            return false;
        }
    }
    wsFrame.payload.get()[wsFrame.payloadLen] = '\0';
    return true;
}

bool ArkTSPlugin::Recv(int32_t client, char* buf, size_t totalLen, int32_t flags) const
{
    size_t recvLen = 0;
    while (recvLen < totalLen) {
        ssize_t len = recv(client, buf + recvLen, totalLen - recvLen, flags);
        CHECK_TRUE(len > 0, false, "Recv payload in while failed, websocket disconnect");
        recvLen += static_cast<size_t>(len);
    }
    buf[totalLen] = '\0';
    return true;
}