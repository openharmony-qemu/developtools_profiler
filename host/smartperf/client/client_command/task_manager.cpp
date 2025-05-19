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

#include "task_manager.h"
#include "common.h"
#include <chrono>
#include <sys/types.h>
#include <unistd.h>

#include "include/heartbeat.h"
#include "include/sp_utils.h"
#include "include/sp_csv_util.h"
#include "include/sp_profiler_factory.h"
#include "include/sp_thread_socket.h"
#include "include/startup_delay.h"
#include "include/ByTrace.h"
#include "include/smartperf_command.h"
#include "include/sp_log.h"
#include "include/RAM.h"
#include "include/common.h"
#include "include/FPS.h"
#include "include/sp_task.h"
#include "include/profiler_fps.h"
#include "include/CPU.h"
#include "include/navigation.h"
#include "include/AI_schedule.h"
#include "include/Threads.h"
#include "include/FileDescriptor.h"
#include <sys/syscall.h>

namespace {
OHOS::SmartPerf::TaskManager* g_mgr = nullptr;
thread_local OHOS::SmartPerf::ThreadLocal g_datas;
constexpr int32_t SAVE_DATA_INTERVAL_MINUTE = 5;
}

namespace OHOS::SmartPerf {
ThreadLocal::ThreadLocal()
{
    mgr_ = g_mgr;
    if (mgr_ == nullptr) {
        return;
    }
    mgr_->RegisterThreadLocal(this);
}

ThreadLocal::~ThreadLocal()
{
    if (mgr_ == nullptr || (datasA_.empty() && datasB_.empty())) {
        return;
    }
    if (switch_) {
        mgr_->CollectData(datasA_);
    } else {
        mgr_->CollectData(datasB_);
    }
}

TaskManager::TaskManager()
{
    g_mgr = this;
}

TaskManager::~TaskManager()
{
    Stop();
}

void TaskManager::AddTask(const std::unordered_map<std::string, ArgumentParser::ArgValue>& argv)
{
    for (auto& [key, val] : argv) {
        auto iter = COMMAND_MAP.find(key);
        if (iter == COMMAND_MAP.end()) {
            continue;
        }

        if (key == "-LOW_POWER") {
            FPS::GetInstance().hapLowFpsFlag = true;
        }
        if (key == "-fc") {
            FPS::GetInstance().hapNeedCatonInfo = true;
        }
        switch (iter->second) {
            case CommandType::CT_N: {
                collectCount_ = std::get<int>(val);
                continue;
            }
            case CommandType::CT_PKG:
            case CommandType::CT_PID: {
                GetProcessInfo(iter->second, val);
                continue;
            }
            case CommandType::CT_PRINT: {
                printDataInfo_ = true;
                continue;
            }
            default:
                break;
        }

        SpProfiler* pro = SpProfilerFactory::GetCmdProfilerItem(iter->second, true);
        if (pro == nullptr) {
            continue;
        }
        pro->ipcCallback_ = ipcCallback_;
        if (iter->second == CommandType::CT_C || iter->second == CommandType::CT_P) {
            priorityTask_.emplace_back(pro);
        } else {
            normalTask_.emplace_back(pro);
        }
    }
}

void TaskManager::AddTask(const std::string& argv)
{
    LOGD("AddTask argv (%s)", argv.c_str());
    ArgumentParser parameter;
    parameter.Parse(argv);
    AddTask(parameter.Values());
}

void TaskManager::AddTask(SpProfiler* task, bool priority)
{
    if (task == nullptr) {
        return;
    }
    priority ? priorityTask_.emplace_back(task) : normalTask_.emplace_back(task);
}

void TaskManager::SetFilePath(const std::string& fileName, bool removeCurrentFile)
{
    fileName_ = fileName;
    saveFlag_ = true;
    if (removeCurrentFile) {
        std::remove(fileName_.c_str());
    }
}

std::map<std::string, std::string> TaskManager::TaskFun(SpProfiler* pro, uint32_t batch, bool record)
{
    auto mapRes = pro->ItemData();
    if (record) {
        if (g_datas.switch_) {
            g_datas.datasA_[batch].insert(mapRes.begin(), mapRes.end());
        } else {
            g_datas.datasB_[batch].insert(mapRes.begin(), mapRes.end());
        }
    }
    return mapRes;
}

void TaskManager::StartSaveFileThread()
{
    if (scheduleSaveDataTh_.joinable()) {
        return;
    }
    scheduleSaveDataTh_ = std::thread([this]() {
        while (running_ && recordData_) {
            std::unique_lock<std::mutex> lock(scheduleSaveDataMtx_);
            scheduleSaveDataCond_.wait(lock);
            if (!running_) {
                return;
            }
            ScheduleSaveData();
        }
    });
}

void TaskManager::SaveRegularly(std::chrono::steady_clock::time_point& loopEnd)
{
    if (!recordData_) {
        return;
    }

    if ((loopEnd - currentTimePoint_) >= std::chrono::minutes(SAVE_DATA_INTERVAL_MINUTE)) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (savingFile_) {
            return;
        }
        savingFile_ = true;
        for (auto& item : threadLocals_) {
            if (item == nullptr) {
                continue;
            }
            item->switch_ = !item->switch_;
        }
        scheduleSaveDataCond_.notify_one();
        currentTimePoint_ = loopEnd;
    }
}

void TaskManager::Start()
{
    if (priorityTask_.empty() && normalTask_.empty()) {
        return;
    }

    ProcessOnceTask(true);
    running_ = true;
    mainLoop_ = std::thread([this]() {
        while (running_) {
            auto loopStart = std::chrono::steady_clock::now();
            bool recordData = recordData_.load();
            if (dataIndex_++ == collectCount_) {
                break;
            }

            std::map<std::string, std::string> currDatas;
            currDatas.emplace("timestamp", std::to_string(SPUtils::GetCurTime()));
            if (recordData) {
                datas_.emplace(dataIndex_,
                    std::map<std::string, std::string>{{"timestamp", std::to_string(SPUtils::GetCurTime())}});
            }

            for (auto& item : priorityTask_) {
                auto ret = threadPool_.PushTask(&TaskManager::TaskFun, this, item, dataIndex_, recordData).get();
                currDatas.insert(std::make_move_iterator(ret.begin()), std::make_move_iterator(ret.end()));
            }

            std::vector<std::future<std::map<std::string, std::string>>> result;
            for (auto& item : normalTask_) {
                result.emplace_back(threadPool_.PushTask(&TaskManager::TaskFun, this, item, dataIndex_, recordData));
            }

            for (auto& item : result) {
                auto ret = item.get();
                currDatas.insert(std::make_move_iterator(ret.begin()), std::make_move_iterator(ret.end()));
            }

            if (nextTime_ != nullptr) {
                *nextTime_ = SPUtils::GetCurTime();
            }

            ProcessCurrentBatch(currDatas);
            auto loopEnd = std::chrono::steady_clock::now();
            SaveRegularly(loopEnd);
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(loopEnd - loopStart);
            if (elapsed < std::chrono::seconds(1)) {
                std::this_thread::sleep_for(std::chrono::seconds(1) - elapsed);
            }
        }
        ProcessOnceTask(false);
        running_ = false;
        finishCond_.notify_all();
        scheduleSaveDataCond_.notify_all();
    });
}

void TaskManager::WriteToCSV()
{
    std::unique_lock<std::mutex> lock(mtx_);
    if (datas_.empty() || !saveFlag_) {
        return;
    }

    if (!dataFile_.is_open()) {
        dataFile_.open(fileName_, std::ios::app);
        if (!dataFile_.is_open()) {
            return;
        }
        for (const auto& [id, m] : datas_) {
            for (const auto& [k, _] : m) {
                titles_.insert(k);
            }
        }
    
        for (const auto& field : titles_) {
            dataFile_ << field << ",";
        }
        dataFile_ << "\n";
    }

    for (auto& [_, v] : datas_) {
        for (auto& item : titles_) {
            auto it = v.find(item);
            if (it != v.end()) {
                dataFile_ << it->second;
            }
            dataFile_ << ",";
        }
        dataFile_ << "\n";
    }
    dataFile_.flush();

    if (!running_) {
        dataFile_.close();
    }
}

void TaskManager::SetStopFlag()
{
    running_ = false;
}

void TaskManager::Stop(bool pause)
{
    SetStopFlag();
    if (mainLoop_.joinable()) {
        mainLoop_.join();
    }
    if (scheduleSaveDataTh_.joinable()) {
        scheduleSaveDataTh_.join();
    }

    if (!pause) {
        threadPool_.Stop();
    }
}

void TaskManager::Wait()
{
    std::unique_lock<std::mutex> lock(finishMtx_);
    finishCond_.wait(lock, [this] { return !running_.load(); });
}

void TaskManager::CollectData(std::map<uint32_t, std::map<std::string, std::string>>& datas)
{
    std::unique_lock<std::mutex> lock(mtx_);
    for (auto& [k, v] : datas) {
        datas_[k].insert(std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
    }
}

void TaskManager::GetProcessInfo(CommandType type, const ArgumentParser::ArgValue& value)
{
    if (type == CommandType::CT_PKG) {
        if (auto pval = std::get_if<std::string>(&value)) {
            processName_ = *pval;
        } else {
            std::cerr << "Invalid type for -PKG argument\n";
            return;
        }
        SpProfilerFactory::SetProfilerPkg(processName_);
        FPS &fps = FPS::GetInstance();
        fps.isGameApp = SPUtils::GetIsGameApp(processName_);
        fps.firstDump = true;
        fps.catonNum = 0;

        OHOS::SmartPerf::StartUpDelay sp;
        sp.GetPidByPkg(processName_, &processIds_);
    }

    if (type == CommandType::CT_PID) {
        if (auto pval = std::get_if<int32_t>(&value)) {
            processId_ = *pval;
        } else {
            std::cerr << "Invalid type for -PID argument\n";
            return;
        }
        processIds_ = std::to_string(processId_);
    }

    FPS::GetInstance().SetProcessId(processIds_);
    FPS::GetInstance().SetPackageName(processName_);
    ProfilerFPS::GetInstance().SetProcessId(processIds_);
    ProfilerFPS::GetInstance().SetPackageName(processName_);
    RAM::GetInstance().SetProcessId(processIds_);
    CPU::GetInstance().SetProcessId(processIds_);
    CPU::GetInstance().SetPackageName(processName_);
    Navigation::GetInstance().SetProcessId(processIds_);
    AISchedule::GetInstance().SetProcessId(processIds_);
    Threads::GetInstance().SetProcessId(processIds_);
    FileDescriptor::GetInstance().SetProcessId(processIds_);
}

void TaskManager::RegisterThreadLocal(ThreadLocal* local)
{
    std::unique_lock<std::mutex> lock(mtx_);
    threadLocals_.emplace_back(local);
}

std::string TaskManager::MapToString(std::map<std::string, std::string>& myMap)
{
    if (hapCollect_) {
        std::string appCollectMap = "t_index_info$$";
        std::vector<std::string> keysToFind = {"fps", "refreshrate", "currentNow", "ddrFrequency", "cpu0Frequency",
            "cpu1Frequency", "cpu2Frequency", "cpu3Frequency", "cpu4Frequency", "cpu5Frequency", "cpu6Frequency",
            "cpu7Frequency", "cpu8Frequency", "cpu9Frequency", "cpu10Frequency", "cpu11Frequency", "gpuFrequency",
            "pss", "shell_frame", "shell_back", "soc_thermal", "cpu0Usage", "cpu1Usage", "cpu2Usage", "cpu3Usage",
            "cpu4Usage", "cpu5Usage", "cpu6Usage", "cpu7Usage", "cpu8Usage", "cpu9Usage", "cpu10Usage", "cpu11Usage",
            "gpuLoad"};
        for (const auto& key : keysToFind) {
            if (auto iter = myMap.find(key); iter != myMap.end()) {
                appCollectMap += iter->first + ":" + iter->second + ",";
            }
        }
        return appCollectMap;
    } else {
        std::string str = "{ ";
        for (auto it = myMap.begin(); it != myMap.end(); ++it) {
            str += "\"" + it->first + "\": " + it->second + ", ";
        }
        const int subLen = 2;
        str.erase(str.end() - subLen, str.end());
        str += " }";
        return str;
    }
}

void TaskManager::ProcessCurrentBatch(std::map<std::string, std::string>& data)
{
    if (printDataInfo_) {
        std::cerr << std::endl;
        uint32_t index = 0;
        for (auto& [k, v] : data) {
            std::cerr << "order:" << index++ << " ";
            std::cerr << k << "=" << v << std::endl;
        }
    }

    if (ipcDataRecv_) {
        ipcCallback_(MapToString(data));
    }
}

void TaskManager::ScheduleSaveData()
{
    for (auto& item : threadLocals_) {
        if (item == nullptr) {
            continue;
        }

        std::map<uint32_t, std::map<std::string, std::string>>& datas = item->switch_ ? item->datasB_ : item->datasA_;
        for (auto& [k, v] : datas) {
            datas_[k].insert(std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
        }
        std::map<uint32_t, std::map<std::string, std::string>>().swap(datas);
    }

    WriteToCSV();
    std::map<uint32_t, std::map<std::string, std::string>>().swap(datas_);
    savingFile_ = false;
}

void TaskManager::EnableIPCCallback()
{
    ipcDataRecv_ = true;
}

void TaskManager::SetIPCCallback(std::function<void(std::string)> callback)
{
    ipcCallback_ = callback;
}

void TaskManager::DisableIPCCallback()
{
    ipcDataRecv_ = false;
}

void TaskManager::SetHapFlag(bool flag)
{
    hapCollect_ = flag;
}

void TaskManager::SetNextTime(long long& nextTime)
{
    nextTime_ = &nextTime;
}

void TaskManager::ProcessOnceTask(bool start)
{
    for (auto& item : priorityTask_) {
        if (item == nullptr) {
            continue;
        }
        start ? item->StartExecutionOnce() : item->FinishtExecutionOnce();
    }

    for (auto& item : normalTask_) {
        if (item == nullptr) {
            continue;
        }
        start ? item->StartExecutionOnce() : item->FinishtExecutionOnce();
    }
}

void TaskManager::SetRecordState(bool record)
{
    if (record) {
        currentTimePoint_ = std::chrono::steady_clock::now();
        StartSaveFileThread();
        recordData_ = record;
    } else {
        auto time = currentTimePoint_ + std::chrono::minutes(SAVE_DATA_INTERVAL_MINUTE + 1);
        SaveRegularly(time);
        recordData_ = record;

        if (scheduleSaveDataTh_.joinable()) {
            scheduleSaveDataTh_.join();
        }
        dataFile_.close();
    }
}
}