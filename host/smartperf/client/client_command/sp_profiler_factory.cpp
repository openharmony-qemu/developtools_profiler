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
#include <iostream>
#include "include/AI_schedule.h"
#include "include/CPU.h"
#include "include/DDR.h"
#include "include/GetLog.h"
#include "include/GPU.h"
#include "include/FPS.h"
#include "include/profiler_fps.h"
#include "include/RAM.h"
#include "include/Network.h"
#include "include/Power.h"
#include "include/Temperature.h"
#include "include/ByTrace.h"
#include "include/sp_utils.h"
#include "include/sp_profiler_factory.h"
#include "include/Capture.h"
#include "include/navigation.h"
#include "include/sp_log.h"
#include "include/FileDescriptor.h"
#include "include/Threads.h"

namespace OHOS {
namespace SmartPerf {
SpProfiler *SpProfilerFactory::GetProfilerItem(MessageType messageType)
{
    SpProfiler* profiler = nullptr;
    switch (messageType) {
        case MessageType::GET_CPU_FREQ_LOAD:
            profiler = &CPU::GetInstance();
            break;
        case MessageType::GET_FPS_AND_JITTERS:
        case MessageType::GET_CUR_FPS:
            profiler = &ProfilerFPS::GetInstance();
            break;
        case MessageType::GET_GPU_FREQ:
        case MessageType::GET_GPU_LOAD:
            profiler = &GPU::GetInstance();
            break;
        case MessageType::GET_DDR_FREQ:
            profiler = &DDR::GetInstance();
            break;
        case MessageType::GET_RAM_INFO:
            profiler = &RAM::GetInstance();
            break;
        case MessageType::GET_LOG:
            profiler = &GetLog::GetInstance();
            break;
        case MessageType::GET_PROCESS_THREADS:
            profiler = &Threads::GetInstance();
            break;
        case MessageType::GET_PROCESS_FDS:
            profiler = &FileDescriptor::GetInstance();
            break;
        default:
            break;
    }
    if (profiler == nullptr) {
        profiler = GetProfilerItemContinue(messageType);
    }
    return profiler;
}

SpProfiler *SpProfilerFactory::GetProfilerItemContinue(MessageType messageType)
{
    SpProfiler* profiler = nullptr;
    switch (messageType) {
        case MessageType::GET_TEMPERATURE:
            profiler = &Temperature::GetInstance();
            break;
        case MessageType::GET_POWER:
            profiler = &Power::GetInstance();
            break;
        case MessageType::CATCH_TRACE_CONFIG:
            ProfilerFPS::GetInstance().SetTraceCatch();
            break;
        case MessageType::GET_CAPTURE:
            Capture::GetInstance().SocketMessage();
            profiler = &Capture::GetInstance();
            break;
        case MessageType::CATCH_NETWORK_TRAFFIC:
        case MessageType::GET_NETWORK_TRAFFIC:
            profiler = &Network::GetInstance();
            break;
        default:
            break;
    }
    return profiler;
}

void SpProfilerFactory::SetProfilerPkg(const std::string &pkg)
{
    LOGD("SpProfilerFactory setPKG:%s", pkg.c_str());
    FPS &fps = FPS::GetInstance();
    fps.SetPackageName(pkg);
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    profilerFps.SetPackageName(pkg);
    RAM &ram = RAM::GetInstance();
    ram.SetPackageName(pkg);
    CPU &cpu = CPU::GetInstance();
    cpu.SetPackageName(pkg);
    Threads &threads = Threads::GetInstance();
    threads.SetPackageName(pkg);
    FileDescriptor &fds = FileDescriptor::GetInstance();
    fds.SetPackageName(pkg);
}

void SpProfilerFactory::SetProfilerPidByPkg(std::string &pid, std::string pids)
{
    LOGD("SpProfilerFactory setPID:%s", pid.c_str());
    std::string bundleName;
    SPUtils::GetBundleName(pid, bundleName);
    FPS &fps = FPS::GetInstance();
    fps.SetProcessId(pid);
    fps.SetPackageName(bundleName);
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    profilerFps.SetProcessId(pid);
    profilerFps.SetPackageName(bundleName);
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId(pids.empty() ? pid : pids);
    CPU &cpu = CPU::GetInstance();
    cpu.SetProcessId(pids.empty() ? pid : pids);
    cpu.SetPackageName(bundleName);
    Navigation &nav = Navigation::GetInstance();
    nav.SetProcessId(pid);
    AISchedule &as = AISchedule::GetInstance();
    as.SetProcessId(pid);
    Threads &threads = Threads::GetInstance();
    threads.SetProcessId(pids.empty() ? pid : pids);
    FileDescriptor &fds = FileDescriptor::GetInstance();
    fds.SetProcessId(pids.empty() ? pid : pids);
}

void SpProfilerFactory::SetProfilerLayer(const std::string &layer)
{
    FPS &fps = FPS::GetInstance();
    fps.SetLayerName(layer);
}

void SpProfilerFactory::SetProfilerGameLayer(const std::string &isGameView)
{
    ProfilerFPS &profilerFps = ProfilerFPS::GetInstance();
    profilerFps.SetGameLayer(isGameView);
}

void SpProfilerFactory::SetByTrace(std::string message)
{
    std::vector<std::string> values;
    std::string delimiter = "||";
    std::string delim = "=";
    SPUtils::StrSplit(message, delimiter, values);
    int mSum = 0;
    int mInterval = 0;
    long long mThreshold = 0;
    int lowFps = 0;
    for (std::string vItem : values) {
        std::vector<std::string> vItems;
        SPUtils::StrSplit(vItem, delim, vItems);
        if (vItems[0] == "traceSum") {
            mSum = SPUtilesTye::StringToSometype<int>(vItems[1]);
        }
        if (vItems[0] == "fpsJitterTime") {
            mThreshold = SPUtilesTye::StringToSometype<int>(vItems[1]);
        }
        if (vItems[0] == "catchInterval") {
            mInterval = SPUtilesTye::StringToSometype<int>(vItems[1]);
        }
        if (vItems[0] == "lowFps") {
            lowFps = SPUtilesTye::StringToSometype<int>(vItems[1]);
        }
    }
    const ByTrace &bTrace = ByTrace::GetInstance();
    if (message.find("traceSum") != std::string::npos) {
        int mCurNum = 1;
        bTrace.SetTraceConfig(mSum, mInterval, mThreshold, lowFps, mCurNum);
    }
}
SpProfiler *SpProfilerFactory::GetCmdProfilerItem(CommandType commandType, bool cmdFlag)
{
    SpProfiler *profiler = nullptr;
    switch (commandType) {
        case CommandType::CT_C:
            if (cmdFlag) {
                profiler = &CPU::GetInstance();
            }
            break;
        case CommandType::CT_G:
            profiler = &GPU::GetInstance();
            break;
        case CommandType::CT_F:
            if (cmdFlag) {
                profiler = &FPS::GetInstance();
            }
            break;
        case CommandType::CT_D:
            profiler = &DDR::GetInstance();
            break;
        case CommandType::CT_P:
            profiler = &Power::GetInstance();
            break;
        case CommandType::CT_T:
            profiler = &Temperature::GetInstance();
            break;
        case CommandType::CT_R:
            if (cmdFlag) {
                profiler = &RAM::GetInstance();
            }
            break;
        case CommandType::CT_NET:
            profiler = &Network::GetInstance();
            break;
        case CommandType::CT_NAV:
            profiler = &Navigation::GetInstance();
            break;
        case CommandType::CT_TTRACE:
            ProfilerFPS::GetInstance().SetTraceCatch();
            break;
        case CommandType::CT_AS:
            profiler = &AISchedule::GetInstance();
            break;
        default:
            break;
    }
    if (profiler == nullptr) {
        profiler = GetCmdProfilerItemContinue(commandType, cmdFlag);
    }
    return profiler;
}

SpProfiler *SpProfilerFactory::GetCmdProfilerItemContinue(CommandType commandType, bool cmdFlag)
{
    SpProfiler *profiler = nullptr;
    switch (commandType) {
        case CommandType::CT_SNAPSHOT:
            if (cmdFlag) {
                profiler = &Capture::GetInstance();
            }
            break;
        case CommandType::CT_THREADS:
            profiler = &Threads::GetInstance().GetInstance();
            break;
        case CommandType::CT_FDS:
            if (cmdFlag) {
                profiler = &FileDescriptor::GetInstance().GetInstance();
            }
            break;
        default:
            break;
    }
    return profiler;
}
}
}
