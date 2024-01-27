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
#include "include/Network.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cctype>
#include "sys/time.h"
#include "securec.h"
#include "include/sp_utils.h"
#include "include/sp_log.h"
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

std::map<std::string, std::string> Network::GetNetworkInfo()
{
    std::map<std::string, std::string> networkInfo;
    char buff[LARGE_BUFF_MAX_LEN];
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == nullptr) {
        std::cout << "net work node is not accessed" << std::endl;
        return networkInfo;
    }
    while (fgets(buff, LARGE_BUFF_MAX_LEN, fp)) {
        if (strstr(buff, "rmnet") || strstr(buff, "eth") || strstr(buff, "wlan")) {
            if (sscanf_s(buff, "%*s%lld%*lld%*lld%*lld%*lld%*lld%*lld%*lld%lld%*lld%*lld%*lld%*lld%*lld%*lld%*lld",
                &curRx, &curTx) < 0) {
                return networkInfo;
            }
            allTx += curTx;
            allRx += curRx;
        }
    }
    fclose(fp);
    if (isFirst) {
        networkInfo["networkUp"] = std::to_string(prevTx);
        networkInfo["networkDown"] = std::to_string(prevRx);
        isFirst = false;
        prevTx = allTx;
        prevRx = allRx;
        allTx = 0;
        allRx = 0;
        return networkInfo;
    }
    if ((allTx == 0 && allRx == 0) || (allTx <= prevTx && allRx <= prevRx)) {
        networkInfo["networkUp"] = "0";
        networkInfo["networkDown"] = "0";
        prevTx = allTx;
        prevRx = allRx;
        allTx = 0;
        allRx = 0;
        return networkInfo;
    }
    diffTx = allTx - prevTx;
    prevTx = allTx;
    diffRx = allRx - prevRx;
    prevRx = allRx;
    allTx = 0;
    allRx = 0;
    networkInfo["networkUp"] = std::to_string(diffTx);
    networkInfo["networkDown"] = std::to_string(diffRx);
    return networkInfo;
}
}
}
