/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include "ipc_generator_impl.h"

IpcGeneratorImpl::IpcGeneratorImpl() {}

IpcGeneratorImpl::~IpcGeneratorImpl() {}

namespace {
const std::string BASE_HEADER_STRING = R"(
#pragma once

#include "#HEAD_FILE_NAME#.pb.h"
#include "service_base.h"
#include <cstdint>
#include <mutex>

class SocketContext;
class UnixSocketClient;

#PROTOCOL_ENUM#

class #SERVICE_CLASS_NAME#:public ServiceBase
{
public:
    #SERVICE_CLASS_NAME#();
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
#RESPONSE_DEFINE#
};

class #CLIENT_CLASS_NAME#:public ServiceBase
{
public:
    #CLIENT_CLASS_NAME#();

    std::shared_ptr<UnixSocketClient> unixSocketClient_;
    bool Connect(const std::string addrname);
    bool ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size) override;
    google::protobuf::Message *presponse;
    uint32_t waitingFor;
#VIRTUAL_RESPONSE_FUNC#
};
)";

const std::string BASE_SOURCE_STRING = R"(
#include "#HEAD_FILE_NAME#.ipc.h"
#include "#HEAD_FILE_NAME#.pb.h"
#include "socket_context.h"
#include "unix_socket_client.h"
#include "unix_socket_server.h"
#include <unistd.h>

namespace {
    constexpr uint32_t WAIT_FOR_EVER = 24 * 60 * 60 * 1000;
}

#SERVICE_CLASS_NAME#::#SERVICE_CLASS_NAME#()
{
    serviceName_ = "#SERVICE_NAME#";
}

#RESPONSE_IMPLEMENT#

#SERVICE_PROTOCOL_PROC_FUNC#
bool #SERVICE_CLASS_NAME#::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    switch (pnum) {
#SERVICE_PROTOCOL_PROC#
    }
    return false;
}

#CLIENT_CLASS_NAME#::#CLIENT_CLASS_NAME#()
{
    unixSocketClient_ = nullptr;
    serviceName_ = "#SERVICE_NAME#";
}
bool #CLIENT_CLASS_NAME#::Connect(const std::string addrname)
{
    if (unixSocketClient_ != nullptr) {
        return false;
    }
    unixSocketClient_ = std::make_shared<UnixSocketClient>();
    if (!unixSocketClient_->Connect(addrname, *this)) {
        printf("Socket Connect failed\n");
        unixSocketClient_ = nullptr;
        return false;
    }
    return true;
}

#CLIENT_SEND_REQUEST_PROC_FUNC#

#CLIENT_SEND_PROTOCOL_PROC_FUNC#
bool #CLIENT_CLASS_NAME#::ProtocolProc(SocketContext &context, uint32_t pnum, const int8_t *buf, const uint32_t size)
{
    switch (pnum) {
#CLIENT_PROTOCOL_PROC#
    }
    if (waitingFor == pnum) {
        waitingFor = -1;
        mWait_.unlock();
    }
    return false;
}
)";

std::string SwapName(std::string s)
{
    std::string ret = "";
    bool b = true;
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '_') {
            b = true;
        } else if (b && c >= 'a' && c <= 'z') {
            ret += (c + 'A' - 'a');
            b = false;
        } else {
            ret += c;
        }
    }
    return ret;
}
std::string ReplaceStr(const std::string& base, const std::string& _from, const std::string& _to)
{
    std::string ret = base;
    while (true) {
        size_t pos = ret.find(_from, 0);
        if (pos == std::string::npos) {
            break;
        }
        ret.replace(pos, _from.length(), _to);
    }
    return ret;
}
} // namespace

std::string IpcGeneratorImpl::SetNames(std::string fileName, std::string packageName)
{
    fileName_ = fileName;
    packageName_ = packageName + "::";
    headFileName_ = "";

    for (size_t i = 0; i < fileName.length(); i++) {
        if (fileName.c_str()[i] == '.') {
            break;
        }
        headFileName_ += fileName.c_str()[i];
    }
    baseName_ = SwapName(headFileName_);

    serviceCount_ = 0;

    serviceList_.clear();
    enumMessageDict_.clear();

    return headFileName_;
}

bool IpcGeneratorImpl::AddService(std::string serviceName)
{
    for (int i = 0; i < serviceCount_; i++) {
        if (serviceList_[i].serviceName_ == serviceName) {
            return false;
        }
    }
    serviceList_[serviceCount_].serviceName_ = serviceName;
    serviceCount_++;
    return true;
}

bool IpcGeneratorImpl::AddServiceMethod(std::string serviceName,
                                        std::string methodName,
                                        std::string requestName,
                                        std::string responseName)
{
    for (int i = 0; i < serviceCount_; i++) {
        if (serviceList_[i].serviceName_ == serviceName) {
            return serviceList_[i].AddMethod(methodName, requestName, responseName);
        }
    }
    return false;
}

void IpcGeneratorImpl::GenerateHeader(std::string& header_str)
{
    for (int i = 0; i < serviceCount_; i++) {
        std::string server_class_name = serviceList_[i].serviceName_ + "Server";
        header_str = ReplaceStr(header_str, "#SERVICE_CLASS_NAME#", server_class_name);

        std::string tmp1 = "";
        std::string tmp2 = "";
        for (int j = 0; j < serviceList_[i].methodCount_; j++) {
            tmp1 += "\tvirtual bool " + serviceList_[i].methodList_[j] + "(SocketContext &context," + packageName_ +
                    serviceList_[i].requestList_[j] + " &request," + packageName_ + serviceList_[i].responseList_[j] +
                    " &response);\n";

            tmp2 += "\tbool SendResponse" + serviceList_[i].responseList_[j] + "(SocketContext &context," +
                    packageName_ + serviceList_[i].responseList_[j] + " &response);\n";
        }
        tmp1 += "\n" + tmp2;
        header_str = ReplaceStr(header_str, "#RESPONSE_DEFINE#", tmp1);

        std::string client_class_name = serviceList_[i].serviceName_ + "Client";
        header_str = ReplaceStr(header_str, "#CLIENT_CLASS_NAME#", client_class_name);

        tmp1 = "";
        for (int j = 0; j < serviceList_[i].methodCount_; j++) {
            tmp1 += "\tbool " + serviceList_[i].methodList_[j] + "(" + packageName_ + serviceList_[i].requestList_[j];
            tmp1 += " &request," + packageName_ + serviceList_[i].responseList_[j];
            tmp1 += " &response,uint32_t timeout_ms=5000);\n";
            tmp1 += "\tbool " + serviceList_[i].methodList_[j] + "(" + packageName_ + serviceList_[i].requestList_[j];
            tmp1 += " &request);\n";
        }
        tmp1 += "\n";
        for (int j = 0; j < serviceList_[i].methodCount_; j++) {
            tmp1 += "\tvirtual bool On" + serviceList_[i].responseList_[j] + "(SocketContext &context," + packageName_;
            tmp1 += serviceList_[i].responseList_[j] + " &response);\n";
        }

        header_str = ReplaceStr(header_str, "#VIRTUAL_RESPONSE_FUNC#", tmp1);
    }
}

std::string IpcGeneratorImpl::GenHeader()
{
    std::string header_str = BASE_HEADER_STRING;
    std::string tmp1;
    header_str = ReplaceStr(header_str, "#HEAD_FILE_NAME#", headFileName_);
    const int numTwo = 2;

    if (serviceCount_ > 0) {
        tmp1 = "enum {\n";
        for (int i = 0; i < serviceCount_; i++) {
            for (int j = 0; j < serviceList_[i].methodCount_; j++) {
                tmp1 += "\tIpcProtocol" + baseName_ + serviceList_[i].requestList_[j];
                tmp1 += "=" + std::to_string(j * numTwo) + ",\n";
                tmp1 += "\tIpcProtocol" + baseName_ + serviceList_[i].responseList_[j];
                tmp1 += "=" + std::to_string(j * numTwo + 1) + ",\n";
            }
        }
        tmp1 += "};";
    } else {
        tmp1 = "";
    }
    header_str = ReplaceStr(header_str, "#PROTOCOL_ENUM#", tmp1);

    GenerateHeader(header_str);
    header_str = ReplaceStr(header_str, "\t", "    ");
    return header_str;
}

namespace {
const std::string SEND_RESPONSE_IMPL_STRING = R"(
bool #SERVER_CLASS_NAME#::SendResponse#RESPONSE_NAME#(SocketContext &context, 
                                                      #PACKAGE_NAME##RESPONSE_NAME# &response) {
    context.SendProtobuf(#ENUM_STR#, response);
    return false;
}
)";
}
std::string IpcGeneratorImpl::GenSendResponseImpl(int servicep, const std::string& server_class_name)
{
    std::string ret = "";
    for (int j = 0; j < serviceList_[servicep].methodCount_; j++) {
        std::string enum_str = "IpcProtocol" + baseName_ + serviceList_[servicep].responseList_[j];
        std::string tmp = ReplaceStr(SEND_RESPONSE_IMPL_STRING, "#SERVER_CLASS_NAME#", server_class_name);
        tmp = ReplaceStr(tmp, "#RESPONSE_NAME#", serviceList_[servicep].responseList_[j]);
        tmp = ReplaceStr(tmp, "#PACKAGE_NAME#", packageName_);
        tmp = ReplaceStr(tmp, "#ENUM_STR#", enum_str);
        ret += tmp;
    }
    return ret;
}
namespace {
const std::string ON_RESPONSE_IMPL_STRING = R"(
bool #CLIENT_CLASS_NAME#::On#RESPONSE_NAME#(SocketContext &context, #PACKAGE_NAME##RESPONSE_NAME# &response) {
    return false;
}
)";
}
std::string IpcGeneratorImpl::GenOnResponseImpl(int servicep, const std::string& client_class_name)
{
    std::string ret = "";
    for (int j = 0; j < serviceList_[servicep].methodCount_; j++) {
        std::string tmp = ReplaceStr(ON_RESPONSE_IMPL_STRING, "#CLIENT_CLASS_NAME#", client_class_name);
        tmp = ReplaceStr(tmp, "#RESPONSE_NAME#", serviceList_[servicep].responseList_[j]);
        tmp = ReplaceStr(tmp, "#PACKAGE_NAME#", packageName_);
        ret += tmp;
    }
    return ret;
}
namespace {
const std::string SERVICE_CALL_IMPL_STRING = R"(
bool #SERVER_CLASS_NAME#::#METHOD_NAME#(SocketContext &context,
                                        #PACKAGE_NAME##REQUEST_NAME# &request,
                                        #PACKAGE_NAME##RESPONSE_NAME# &response) {
    return false;
}
)";
}
std::string IpcGeneratorImpl::GenServiceCallImpl(int servicep, const std::string& server_class_name)
{
    std::string ret = "";
    for (int j = 0; j < serviceList_[servicep].methodCount_; j++) {
        std::string tmp = ReplaceStr(SERVICE_CALL_IMPL_STRING, "#SERVER_CLASS_NAME#", server_class_name);
        tmp = ReplaceStr(tmp, "#SERVER_CLASS_NAME#", server_class_name);
        tmp = ReplaceStr(tmp, "#METHOD_NAME#", serviceList_[servicep].methodList_[j]);
        tmp = ReplaceStr(tmp, "#REQUEST_NAME#", serviceList_[servicep].requestList_[j]);
        tmp = ReplaceStr(tmp, "#RESPONSE_NAME#", serviceList_[servicep].responseList_[j]);
        tmp = ReplaceStr(tmp, "#PACKAGE_NAME#", packageName_);
        ret += tmp;
    }
    return ret;
}
namespace {
const std::string CLIENT_PROC_IMPL_STRING = R"(
    case IpcProtocol#BASE_NAME##REQUEST_NAME#:{
        #PACKAGE_NAME##REQUEST_NAME# request;
        #PACKAGE_NAME##RESPONSE_NAME# response;
        request.ParseFromArray(buf, size);
        if (#METHOD_NAME#(context, request, response)) {
            context.SendProtobuf(IpcProtocol#BASE_NAME##RESPONSE_NAME#, response);
        }
    }
        break;
)";
const std::string CLIENT_PROC_NOTIFYRESULT_STRING = R"(
    case IpcProtocol#BASE_NAME##REQUEST_NAME#:{
        #PACKAGE_NAME##REQUEST_NAME# request;
        #PACKAGE_NAME##RESPONSE_NAME# response;
        request.ParseFromArray(buf, size);
        #METHOD_NAME#(context, request, response);
    }
        break;
)";
}
std::string IpcGeneratorImpl::GenClientProcImpl(int servicep)
{
    std::string ret = "";
    for (int j = 0; j < serviceList_[servicep].methodCount_; j++) {
        std::string tmp = ReplaceStr(CLIENT_PROC_IMPL_STRING, "#BASE_NAME#", baseName_);
        if (serviceList_[servicep].methodList_[j] == "NotifyResult") {
            tmp = ReplaceStr(CLIENT_PROC_NOTIFYRESULT_STRING, "#BASE_NAME#", baseName_);
        }
        tmp = ReplaceStr(tmp, "#PACKAGE_NAME#", packageName_);
        tmp = ReplaceStr(tmp, "#METHOD_NAME#", serviceList_[servicep].methodList_[j]);
        tmp = ReplaceStr(tmp, "#REQUEST_NAME#", serviceList_[servicep].requestList_[j]);
        tmp = ReplaceStr(tmp, "#RESPONSE_NAME#", serviceList_[servicep].responseList_[j]);
        tmp = ReplaceStr(tmp, "#PACKAGE_NAME#", packageName_);
        ret += tmp;
    }
    return ret;
}
namespace {
const std::string CLIENT_REQUEST_IMPL_STRING = R"(
bool #CLIENT_CLASS_NAME#::#METHOD_NAME#(#PACKAGE_NAME##REQUEST_NAME# &request,
                                        #PACKAGE_NAME##RESPONSE_NAME# &response,
                                        uint32_t timeout_ms)
{
    mWait_.lock();
    if (timeout_ms<=0) {
        timeout_ms=WAIT_FOR_EVER;
    }
    waitingFor=IpcProtocol#BASE_NAME##RESPONSE_NAME#;
    presponse=&response;
    if (unixSocketClient_!=nullptr) {
        unixSocketClient_->SendProtobuf(IpcProtocol#BASE_NAME##REQUEST_NAME#, request);
    }
    if (mWait_.try_lock_for(std::chrono::milliseconds(timeout_ms))) {
        mWait_.unlock();
        return true;
    }
    waitingFor=-1;
    mWait_.unlock();
    return false;
}
bool #CLIENT_CLASS_NAME#::#METHOD_NAME#(#PACKAGE_NAME##REQUEST_NAME# &request)
{
    unixSocketClient_->SendProtobuf(IpcProtocol#BASE_NAME##REQUEST_NAME#, request);
    return true;
}
)";
}
std::string IpcGeneratorImpl::GenClientRequestImpl(int servicep, const std::string& client_class_name)
{
    std::string ret = "";
    for (int j = 0; j < serviceList_[servicep].methodCount_; j++) {
        std::string tmp = ReplaceStr(CLIENT_REQUEST_IMPL_STRING, "#CLIENT_CLASS_NAME#", client_class_name);
        tmp = ReplaceStr(tmp, "#METHOD_NAME#", serviceList_[servicep].methodList_[j]);
        tmp = ReplaceStr(tmp, "#PACKAGE_NAME#", packageName_);
        tmp = ReplaceStr(tmp, "#REQUEST_NAME#", serviceList_[servicep].requestList_[j]);
        tmp = ReplaceStr(tmp, "#RESPONSE_NAME#", serviceList_[servicep].responseList_[j]);
        tmp = ReplaceStr(tmp, "#BASE_NAME#", baseName_);
        ret += tmp;
    }
    return ret;
}
namespace {
const std::string SERVICE_PROC_IMPL_STRING = R"(
    case IpcProtocol#BASE_NAME##RESPONSE_NAME#:
        {
            if (waitingFor==pnum) {
                presponse->ParseFromArray(buf, size);
            }
            else {
                #PACKAGE_NAME##RESPONSE_NAME# response#NUM#;
                response#NUM#.ParseFromArray(buf, size);
                On#RESPONSE_NAME#(context, response#NUM#);
            }
        }
        break;
)";
}
std::string IpcGeneratorImpl::GenServiceProcImpl(int servicep)
{
    std::string ret = "";
    for (int j = 0; j < serviceList_[servicep].methodCount_; j++) {
        std::string tmp = ReplaceStr(SERVICE_PROC_IMPL_STRING, "#BASE_NAME#", baseName_);
        tmp = ReplaceStr(tmp, "#RESPONSE_NAME#", serviceList_[servicep].responseList_[j]);
        tmp = ReplaceStr(tmp, "#PACKAGE_NAME#", packageName_);
        tmp = ReplaceStr(tmp, "#NUM#", std::to_string(j + 1));

        ret += tmp;
    }
    return ret;
}

std::string IpcGeneratorImpl::GenSource()
{
    std::string source_str = BASE_SOURCE_STRING;

    source_str = ReplaceStr(source_str, "#HEAD_FILE_NAME#", headFileName_);

    for (int i = 0; i < serviceCount_; i++) {
        std::string server_class_name = serviceList_[i].serviceName_ + "Server";
        source_str = ReplaceStr(source_str, "#SERVICE_CLASS_NAME#", server_class_name);
        source_str = ReplaceStr(source_str, "#SERVICE_NAME#", serviceList_[i].serviceName_);
        std::string client_class_name = serviceList_[i].serviceName_ + "Client";
        source_str = ReplaceStr(source_str, "#CLIENT_CLASS_NAME#", client_class_name);

        source_str = ReplaceStr(source_str, "#RESPONSE_IMPLEMENT#", GenSendResponseImpl(i, server_class_name));
        source_str = ReplaceStr(source_str, "#CLIENT_SEND_REQUEST_PROC_FUNC#", GenOnResponseImpl(i, client_class_name));

        source_str = ReplaceStr(source_str, "#SERVICE_PROTOCOL_PROC_FUNC#", GenServiceCallImpl(i, server_class_name));
        source_str = ReplaceStr(source_str, "#SERVICE_PROTOCOL_PROC#", GenClientProcImpl(i));
        source_str = ReplaceStr(source_str, "#SERVICE_NAME#", serviceList_[i].serviceName_);

        source_str = ReplaceStr(source_str, "#CLIENT_PROTOCOL_PROC#", GenServiceProcImpl(i));
        source_str =
            ReplaceStr(source_str, "#CLIENT_SEND_PROTOCOL_PROC_FUNC#", GenClientRequestImpl(i, client_class_name));
    }

    source_str = ReplaceStr(source_str, "\t", "    ");
    return source_str;
}
