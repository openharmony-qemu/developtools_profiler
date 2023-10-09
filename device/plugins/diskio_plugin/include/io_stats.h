/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef IO_STATS_H
#define IO_STATS_H

#include <deque>
#include <array>
#include <cinttypes>
#include <cstdio>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/mman.h>

#include "diskio_plugin_config.pb.h"
#include "diskio_plugin_result.pb.h"
#include "logging.h"

constexpr int NUM_TWO = 2;

class ProcStats {
public:
    std::string name_;
    uint64_t user_;
    uint64_t nice_;
    uint64_t system_;
    uint64_t idle_;
    uint64_t iowait_;
    uint64_t steal_;
    uint64_t hardirq_;
    uint64_t softirq_;
    uint64_t guest_;
    uint64_t guestNice_;

    ProcStats()
        : name_(""),
        user_(0),
        nice_(0),
        system_(0),
        idle_(0),
        iowait_(0),
        steal_(0),
        hardirq_(0),
        softirq_(0),
        guest_(0),
        guestNice_(0)
    {
    }

    ~ProcStats() {}

    uint64_t GetTotalTime()
    {
        return user_ + nice_ + system_ + idle_ + iowait_ + steal_ + hardirq_ + softirq_;
    }
};

class DiskStats {
public:
    std::string deviceName_;
    uint64_t major_;
    uint64_t minor_;

    uint64_t rSucc_;  // 成功完成读的总次数     rd_ios
    uint64_t rMerged_; // 合并读次数             rd_merges
    uint64_t rSectors_;   // 读扇区的次数           rd_sectors
    uint64_t timeOfRead_; // 读花的时间(ms)         rd_ticks

    uint64_t wSucc_;  // 成功完成写的总次数    wr_ios
    uint64_t wMerged_; // 合并写次数            wr_merges
    uint64_t wSectors_;    // 写扇区的次数          wr_sectors
    uint64_t timeOfWrite_; // 写花的时间(ms)        wr_ticks

    uint64_t dSucc_;
    uint64_t dMerged_;
    uint64_t dSectors_;
    uint64_t timeOfd_;

    uint64_t flushSucc_;
    uint64_t timeOfFlush_;

    uint64_t ios_;      // I/O的当前进度            ios_pgr
    uint64_t timeOfIo_; // I/O操作上的毫秒数        tot_ticks ?
    uint64_t weighted_; // 输入/输出操作花费的加权毫秒数 rq_ticks ?

    DiskStats()
        : deviceName_(""),
        major_(0),
        minor_(0),
        rSucc_(0),
        rMerged_(0),
        rSectors_(0),
        timeOfRead_(0),
        wSucc_(0),
        wMerged_(0),
        wSectors_(0),
        timeOfWrite_(0),
        dSucc_(0),
        dMerged_(0),
        dSectors_(0),
        timeOfd_(0),
        flushSucc_(0),
        timeOfFlush_(0),
        ios_(0),
        timeOfIo_(0),
        weighted_(0)
    {
    }

    ~DiskStats() {}
};

using CpuDatasPtr = std::shared_ptr<ProcStats>;
using DiskDatasPtr = std::shared_ptr<DiskStats>;
class IoStats {
public:
    IoStats(DiskioConfig::IoReportType type = DiskioConfig::UNSPECIFIED);
    ~IoStats() {}
    bool GetIoData();

    template <typename T> bool PutPluginStatsData(T& pluginStatsData)
    {
        PutCpuStatsData(pluginStatsData);
        if (type_ == DiskioConfig::IO_REPORT) {
            PutIoStatsData(pluginStatsData);
        } else if (type_ == DiskioConfig::IO_REPORT_EX) {
            ParseIoStatsEx();
        }
        return true;
    }

private:
    bool ParseCpuStats();
    bool GetCpuStats(std::string& line);
    bool ParseIoStats();
    bool GetIoStats(std::string& line);

    template <typename T> uint32_t PutCpuStatsData(T& pluginStatsData)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cpuDatas_.empty()) {
            return 0;
        }

        uint32_t count = 0;
        while (cpuDatas_.size() > 0) {
            auto cpuData = cpuDatas_.front();
            auto* cpuInfo = pluginStatsData.add_cpuinfo();
            CalcCpuStats(cpuData, *cpuInfo);
            cpuDatas_.pop_front();
            count++;
        }
        lock.unlock();
        return count;
    }

    template <typename T> void CalcCpuStats(const CpuDatasPtr& cpuData, T& cpuStatsInfo)
    {
        auto totalTime = cpuData->GetTotalTime();
        cpuStatsInfo.set_name(cpuData->name_);
        cpuStatsInfo.set_cpu_user(KeepTowDigits(cpuData->user_, totalTime));
        cpuStatsInfo.set_cpu_nice(KeepTowDigits(cpuData->nice_, totalTime));
        cpuStatsInfo.set_cpu_iowait(KeepTowDigits(cpuData->iowait_, totalTime));
        cpuStatsInfo.set_cpu_steal(KeepTowDigits(cpuData->steal_, totalTime));

        cpuStatsInfo.set_cpu_sys(KeepTowDigits(cpuData->system_ + cpuData->softirq_ + cpuData->hardirq_, totalTime));
        cpuStatsInfo.set_cpu_idle(KeepTowDigits(cpuData->idle_, totalTime));
    }

    double KeepTowDigits(const uint64_t& data, uint64_t div);

    template <typename T> uint32_t PutIoStatsData(T& pluginStatsData)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (ioDatas_.empty()) {
            return 0;
        }

        uint32_t count = 0;
        while (ioDatas_.size() > 0) {
            auto ioData = ioDatas_.front();
            auto* ioInfo = pluginStatsData.add_statsinfo();
            CalcIoStats(ioData, *ioInfo);
            ioDatas_.pop_front();
            count++;
        }
        lock.unlock();
        return count;
    }

    template <typename T> void CalcIoStats(const DiskDatasPtr& ioData, T& ioStatDataInfo)
    {
        ioStatDataInfo.set_name(ioData->deviceName_);
        // (成功完成读的总次数 + 写 + 丢弃) / sysTime_
        ioStatDataInfo.set_ios_per_sec(KeepTowDigits(ioData->rSucc_ + ioData->wSucc_ + ioData->dSucc_, sysTime_));

        // 读扇区的次数 / sysTime_
        ioStatDataInfo.set_rd_per_sec(KeepTowDigits(KeepTowDigits(ioData->rSectors_, sysTime_), NUM_TWO));
        ioStatDataInfo.set_wr_per_sec(KeepTowDigits(KeepTowDigits(ioData->wSectors_, sysTime_), NUM_TWO));
        ioStatDataInfo.set_dc_per_sec(KeepTowDigits(KeepTowDigits(ioData->dSectors_, sysTime_), NUM_TWO));

        // 读扇区的次数
        ioStatDataInfo.set_rd_kb(KeepTowDigits(ioData->rSectors_, NUM_TWO));
        ioStatDataInfo.set_wr_kb(KeepTowDigits(ioData->wSectors_, NUM_TWO));
        ioStatDataInfo.set_dc_kb(KeepTowDigits(ioData->dSectors_, NUM_TWO));
    }
    uint32_t OutputCpuData();
    bool OutputIoData();
    uint64_t GetSystime();
    bool ParseIoStatsEx();
    bool FindFirstNum(char** p);
    bool RemoveSpaces(char** p);
    uint32_t ParseLineFields(const std::string& line);
    uint32_t ParseLineFields(const std::string& line, std::string& name);

private:
    std::mutex mutex_;
    DiskioConfig::IoReportType type_;
    uint64_t sysTime_;
    std::deque<CpuDatasPtr> cpuDatas_;
    std::deque<DiskDatasPtr> ioDatas_;
    std::vector<uint64_t> fields_;
};

#endif // #ifndef IO_STATS_H
