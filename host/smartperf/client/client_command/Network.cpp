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
#include "include/Network.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <string>
#include <climits>
#include "include/sp_utils.h"
#define LARGE_BUFF_MAX_LEN (256)
namespace OHOS {
namespace SmartPerf {
std::map<std::string, std::string> Network::ItemData()
{
    std::map<std::string, std::string> result;
    std::map<std::string, std::string> networkInfo = Network::GetNetworkInfo();
    result = networkInfo;
    return result;
}

std::map<std::string, std::string> Network::GetNetworkInfo() const
{
    std::cout << "GetNetwork is called" << std::endl;
    std::map<std::string, std::string> networkInfo;
    long long allTx = 0;
    long long allRx = 0;
    long long curTx = 0;
    long long curRx = 0;
    char buff[LARGE_BUFF_MAX_LEN];
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == nullptr) {
        std::cout << "net work node is not accessed" << std::endl;
        return networkInfo;
    } else {
        std::cout << "net work node is accessed" << std::endl;
    }
    while (fgets(buff, LARGE_BUFF_MAX_LEN, fp)) {
        if (strstr(buff, "rmnet") || strstr(buff, "eth") || strstr(buff, "wlan")) {
            if (sscanf(buff, "%*s%llu%*llu%*llu%*llu%*llu%*llu%*llu%*llu%llu%*llu%*llu%*llu%*llu%*llu%*llu%*llu",
                &curRx, &curTx) < 0) {
                return networkInfo;
            }
            allTx += curTx;
            allRx += curRx;
        }
    }
    fclose(fp);
    networkInfo["networkUp"] = std::to_string(allTx);
    networkInfo["networkDown"] = std::to_string(allRx);
    return networkInfo;
}
}
}
