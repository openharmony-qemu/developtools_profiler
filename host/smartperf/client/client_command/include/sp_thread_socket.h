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
#include <functional>
#include "sp_profiler_factory.h"
#include "sp_server_socket.h"
#include "sp_utils.h"
#include "sp_task.h"
#include "control_call_cmd.h"
namespace OHOS {
namespace SmartPerf {
class SpThreadSocket {
public:
    std::string MapToString(std::map<std::string, std::string> dataMap) const
    {
        std::string result;
        int i = 0;
        std::string splitStr = "";
        for (auto iter = dataMap.cbegin(); iter != dataMap.cend(); ++iter) {
            printf("%s = %s\n", iter->first.c_str(), iter->second.c_str());
            if (i > 0) {
                splitStr = "$$";
            }
            result += splitStr + iter->first.c_str() + "||" + iter->second.c_str();
            i++;
        }
        return result;
    }
    std::string SplitMsg(const std::string recvBuf) const
    {
        std::vector<std::string> sps;
        SPUtils::StrSplit(recvBuf, "::", sps);
        return sps[1];
    }

    void Process(ProtoType type) const
    {
        std::cout << "Socket Process called!" << std::endl;
        SpServerSocket spSocket;
        spSocket.Init(type);
        if (type == ProtoType::TCP) {
            std::cout << "Socket TCP Init called!" << std::endl;
            while (1) {
                int procFd = spSocket.Accept();
                std::cout << "Socket TCP procFd: " << procFd << std::endl;
                while (procFd > 0) {
                    spSocket.Recv();
                    std::string recvStr = spSocket.RecvBuf();
                    std::cout << "Socket TCP Recv: " << recvStr << std::endl;
                    // 解析消息 分发处理
                    DealMsg(recvStr, spSocket);
                }
            }
        }
        if (type == ProtoType::UDP) {
            while (1) {
                spSocket.Recvfrom();
                HandleMsg(spSocket);
            }
        }
        std::cout << "Socket Process finished!" << std::endl;
        spSocket.Close();
    }
    // TCP
    void DealMsg(std::string recvStr, SpServerSocket &spSocket) const
    {
        // init::-SessionId 12345678 -Interval 1000 -PKG ohos.samples.ecg -c -g -t -p -f -r
        if (recvStr.find("init::") != std::string::npos) {
            ErrCode code = SPTask::GetInstance().InitTask(SplitMsg(recvStr));
            if (code == ErrCode::OK) {
                spSocket.Send("init::True");
            } else if (code == ErrCode::FAILED) {
                spSocket.Send("init::False");
            }
        } else if (recvStr.find("start::") != std::string::npos) {
            auto lambdaTask = [&spSocket](std::string data) { spSocket.Send(data); };
            ErrCode code = SPTask::GetInstance().StartTask(lambdaTask);
            if (code == ErrCode::OK) {
                spSocket.Send("start::True");
            } else if (code == ErrCode::FAILED) {
                spSocket.Send("start::False");
            }
        } else if (recvStr.find("stop::") != std::string::npos) {
            SPTask::GetInstance().StopTask();
            spSocket.Send("stop::True");
        } else if (recvStr.find("SP_daemon -editor") != std::string::npos) {
            std::vector<std::string> vec;
            int size = recvStr.size();
            int j = 0;
            for (int i = 0; i < size; i++) {
                if (recvStr[i] == ' ') {
                    vec.push_back(recvStr.substr(j, i - j));
                    j = i+1;
                }
            }
            vec.push_back(recvStr.substr(j, size - j));
            OHOS::SmartPerf::ControlCallCmd controlCallCmd;
            std::string result = controlCallCmd.GetResult(vec.size(), vec);
            spSocket.Send(result);
        }
    }
    // UDP
    void HandleMsg(SpServerSocket &spSocket) const
    {
        auto iterator = messageMap.begin();
        while (iterator != messageMap.end()) {
            std::string recvBuf = spSocket.RecvBuf();
            if (SPUtils::IsSubString(recvBuf, iterator->second)) {
                SpProfiler *profiler = SpProfilerFactory::GetProfilerItem(iterator->first);
                if (profiler == nullptr && (iterator->first == MessageType::SET_PKG_NAME)) {
                    std::string curPkgName = SplitMsg(recvBuf);
                    SpProfilerFactory::SetProfilerPkg(curPkgName);
                    std::string pidCmd = "pidof " + curPkgName;
                    std::string pidResult;
                    if (SPUtils::LoadCmd(pidCmd, pidResult)) {
                        SpProfilerFactory::SetProfilerPid(pidResult);
                    }
                    spSocket.Sendto(curPkgName);
                } else if (profiler == nullptr && (iterator->first == MessageType::SET_PROCESS_ID)) {
                    SpProfilerFactory::SetProfilerPid(SplitMsg(recvBuf));
                } else if (profiler == nullptr) {
                    std::string returnStr = iterator->second;
                    spSocket.Sendto(returnStr);
                } else {
                    std::map<std::string, std::string> data = profiler->ItemData();
                    std::string sendData = MapToString(data);
                    spSocket.Sendto(sendData);
                }
                break;
            }
            ++iterator;
        }
    }
};
}
}
#endif