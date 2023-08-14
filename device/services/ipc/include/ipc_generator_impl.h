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

#ifndef IPC_GENERATOR_IMPL_H
#define IPC_GENERATOR_IMPL_H

#include <map>
#include <ostream>

class IpcServices {
public:
    IpcServices()
    {
        methodCount_ = 0;
    }
    ~IpcServices() {}
    std::string serviceName_;
    std::map<int, std::string> methodList_;
    std::map<int, std::string> requestList_;
    std::map<int, std::string> responseList_;
    int methodCount_;
    bool AddMethod(std::string method, std::string request, std::string response)
    {
        for (int i = 0; i < methodCount_; i++) {
            if (methodList_[i] == method) {
                return false;
            }
        }
        methodList_[methodCount_] = method;
        requestList_[methodCount_] = request;
        responseList_[methodCount_] = response;
        methodCount_++;
        return true;
    }
};

class IpcGeneratorImpl {
public:
    IpcGeneratorImpl();
    ~IpcGeneratorImpl();

    std::string SetNames(std::string fileName, std::string packageName);
    bool AddService(std::string serviceName);
    bool AddServiceMethod(std::string serviceName,
                          std::string methodName,
                          std::string requestName,
                          std::string responseName);

    std::string GenHeader();
    std::string GenSource();

private:
    std::string fileName_ = "";
    std::string baseName_ = "";
    std::string packageName_ = "";
    std::string headFileName_ = "";

    void GenerateHeader(std::string& header_str);

    std::string GenSendResponseImpl(int servicep, const std::string& server_class_name);
    std::string GenOnResponseImpl(int servicep, const std::string& client_class_name);

    std::string GenServiceCallImpl(int servicep, const std::string& server_class_name);
    std::string GenClientProcImpl(int servicep);
    std::string GenClientRequestImpl(int servicep, const std::string& client_class_name);
    std::string GenServiceProcImpl(int servicep);

    std::map<int, IpcServices> serviceList_;
    int serviceCount_ = 0;

    std::map<int, std::string> enumMessageDict_;
};

#endif