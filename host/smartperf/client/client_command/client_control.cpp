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
int ClientControl::SocketStart(std::string agrs)
{
    std::string messageInit = message + args;
    const char* message_cstr = messageInit.c_str();
    OHOS::SmartPerf::ClientControl::InitSocket();
    send(client_socket, message_cstr, strlen(message_cstr), 0);
    read(client_socket, buffer, numBuff);
    send(client_socket, message1, strlen(message1), 0);
    read(client_socket, buffer, numBuff);
    std::cout << "SP_daemon " << buffer << std::endl;
    OHOS::SmartPerf::ClientControl::CloseSocket();
    return 0;
}
int ClientControl::SocketStop()
{
    OHOS::SmartPerf::ClientControl::InitSocket();
    send(client_socket, message2, strlen(message2), 0);
    read(client_socket, buffer, numBuff);
    std::cout << "SP_daemon " << buffer << std::endl;
    OHOS::SmartPerf::ClientControl::CloseSocket();
    return 0;
}
int ClientControl::InitSocket()
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1) {
        std::cout << "Faild to create socket" << std::endl;
        return -1;
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(prot_number);
    if (connect(client_socket, (struct socketaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cout << "Failed to connect to server" << std::endl;
    }
    return 0;
}
void ClientControl::StartSP_daemon()
{
    std::string result = "";
    SPUtils::LoadCmd("SP_daemon", result);
    sleep(1);
}
int ClientControl::CloseSocket()
{
    shutdown(client_socket, SHUT_RD);
    return 0;
}
}
}