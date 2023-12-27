/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <iostream>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include "include/client_control.h"
#include "include/sp_utils.h"
#include "include/startup_delay.h"
namespace OHOS {
namespace SmartPerf {
int ClientControl::SocketStart(std::string args)
{
    std::string messageInit = message + args;
    OHOS::SmartPerf::ClientControl::InitSocket();
    send(clientSocket,  messageInit.c_str(), strlen(messageInit.c_str()), 0);
    read(clientSocket, buffer, numBuff);
    send(clientSocket, message1, strlen(message1), 0);
    read(clientSocket, buffer, numBuff);
    std::cout << "SP_daemon " << buffer << std::endl;
    OHOS::SmartPerf::ClientControl::CloseSocket();
    return 0;
}
int ClientControl::SocketStop()
{
    OHOS::SmartPerf::ClientControl::InitSocket();
    send(clientSocket, message2, strlen(message2), 0);
    read(clientSocket, buffer, numBuff);
    std::cout << "SP_daemon " << buffer << std::endl;
    OHOS::SmartPerf::ClientControl::CloseSocket();
    return 0;
}
int ClientControl::InitSocket()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cout << "Faild to create socket" << std::endl;
        return -1;
    }
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(protNumber);
    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        std::cout << "Failed to connect to server" << std::endl;
    }
    return 0;
}
void ClientControl::StartSPDaemon()
{
    std::string result = "";
    SPUtils::LoadCmd("SP_daemon", result);
    sleep(1);
}
int ClientControl::CloseSocket()
{
    OHOS::SmartPerf::StartUpDelay sd;
    bool spTcp = sd.GetSpTcp();
    if (!spTcp) {
        std::string result = "";
        SPUtils::LoadCmd("SP_daemon", result);
        sleep(1);
    }
    shutdown(clientSocket, SHUT_RD);
    return 0;
}
}
}