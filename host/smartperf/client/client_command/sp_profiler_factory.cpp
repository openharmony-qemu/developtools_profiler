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
#include "include/CPU.h"
#include "include/DDR.h"
#include "include/GPU.h"
#include "include/FPS.h"
#include "include/RAM.h"
#include "include/Power.h"
#include "include/Temperature.h"
#include "include/ByTrace.h"
#include "include/sp_utils.h"
#include "include/sp_profiler_factory.h"
#include "include/Dubai.h"
namespace OHOS {
namespace SmartPerf {
SpProfiler *SpProfilerFactory::GetProfilerItem(MessageType messageType)
{
    SpProfiler *profiler = nullptr;
    switch (messageType) {
        case MessageType::GET_CPU_NUM:
        case MessageType::GET_CPU_FREQ:
        case MessageType::GET_CPU_LOAD:
            profiler = &CPU::GetInstance();
            break;
        case MessageType::GET_FPS_AND_JITTERS:
            profiler = &FPS::GetInstance();
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
        case MessageType::GET_TEMPERATURE:
            profiler = &Temperature::GetInstance();
            break;
        case MessageType::GET_POWER:
            profiler = &Power::GetInstance();
            break;
        case MessageType::CATCH_TRACE_START:
            ByTrace::GetInstance().ThreadGetTrace();
            break;
        case MessageType::CATCH_TRACE_FINISH: {
            ByTrace::GetInstance().ThreadEndTrace();
            std::string curTime = std::to_string(SPUtils::GetCurTime());
            ByTrace::GetInstance().ThreadFinishTrace(curTime);
            break; 
        } 
        case MessageType::GET_CAPTURE:
            FPS::GetInstance().SetCaptureOn();
            break; 
        case MessageType::SET_DUBAI_DB: {
            Dubai::DumpDubaiBegin();
            Dubai::DumpDubaiFinish();
            Dubai::MoveDubaiDb();
            break;   
        }              
        default:
            break;
    }
    return profiler;
}
void SpProfilerFactory::SetProfilerPkg(std::string pkg)
{
    FPS &fps = FPS::GetInstance();
    fps.SetPackageName(pkg);
}
void SpProfilerFactory::SetProfilerPid(std::string pid)
{
    RAM &ram = RAM::GetInstance();
    ram.SetProcessId(pid);
}
SpProfiler *SpProfilerFactory::GetCmdProfilerItem(CommandType commandType)
{
    SpProfiler *profiler = nullptr;
    switch (commandType) {
        case CommandType::CT_C:
            profiler = &CPU::GetInstance();
            break;
        case CommandType::CT_G:
            profiler = &GPU::GetInstance();
            break;
        case CommandType::CT_F:
        case CommandType::CT_F1:
        case CommandType::CT_F2:
            profiler = &FPS::GetInstance();
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
            profiler = &RAM::GetInstance();
            break;
        case CommandType::CT_TTRACE:
            FPS::GetInstance().SetTraceCatch();
            break;
        case CommandType::CT_SNAPSHOT:
            FPS::GetInstance().SetCaptureOn();
            break; 
        case CommandType::CT_HW:
            break;            
        default:
            break;
    }
    return profiler;
}
}
}
