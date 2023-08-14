/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
 *
 * Description: FlowController define
 */
#ifndef FLOW_CONTROLLER_H
#define FLOW_CONTROLLER_H
#include "ftrace_data_reader.h"
#include "ftrace_namespace.h"
#include "ftrace_parser.h"
#include "kernel_symbols_parser.h"
#include "paged_mem_pool.h"
#include "plugin_module_api.h"
#include "result_transporter.h"
#include "trace_ops.h"
#include "trace_plugin_config.pb.h"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>

using WriterStructPtr = std::unique_ptr<WriterStruct>::pointer;

FTRACE_NS_BEGIN
class FlowController {
public:
    FlowController(void);
    ~FlowController(void);

    int SetWriter(const WriterStructPtr& writer);
    int LoadConfig(const uint8_t configData[], uint32_t size);

    int StartCapture(void);
    int StopCapture(void);
    bool ParseBasicData(void);

private:
    DISALLOW_COPY_AND_MOVE(FlowController);
    bool CreateRawDataReaders();
    bool CreatePagedMemoryPool();
    bool CreateRawDataBuffers();
    bool CreateRawDataCaches();
    void SetupTraceBufferSize(uint32_t sizeKb);
    void SetupTransporterFlushParams(uint32_t intervalMs, uint32_t thresholdKb);
    void GenerateRawDataFileNames(const std::string& prefix);
    void SetupTraceReadPeriod(uint32_t periodMs);
    void CaptureWorkOnNomalMode();
    void CaptureWorkOnDelayMode();
    long ReadEventData(int cpuid);
    bool ParseEventDataOnNomalMode(int cpuid, long dataSize);
    bool ParseEventDataOnDelayMode();

    bool AddPlatformEventsToParser(void);
    void EnableTraceEvents(void);
    void DisableTraceEvents(void);
    void DisableAllCategories(void);

    bool ReportClockTimes(void);
    bool ParseKernelSymbols(void);
    bool ParsePerCpuStatus(int stage);
    bool ParseFtraceEvent(int cpuid, uint8_t page[]);

    static std::unique_ptr<TraceOps> GetTraceOps();

    using EventTypeName = std::pair<std::string, std::string>;
    std::vector<EventTypeName> supportedEvents_ = {};
    std::vector<EventTypeName> enabledEvents_ = {};

    std::unique_ptr<PagedMemPool> memPool_ = nullptr;
    std::unique_ptr<KernelSymbolsParser> ksymsParser_ = nullptr;
    std::unique_ptr<FtraceParser> ftraceParser_ = nullptr;
    std::unique_ptr<ResultTransporter> tansporter_ = nullptr;
    std::unique_ptr<TraceOps> traceOps_ = nullptr;
    std::shared_ptr<FILE> rawDataFile_ = nullptr;
    std::vector<std::unique_ptr<FtraceDataReader>> ftraceReaders_ = {};
    std::vector<std::shared_ptr<uint8_t>> ftraceBuffers_;
    std::atomic<bool> keepRunning_ = false;
    std::thread pollThread_ = {};

    // for trace plugin config fields
    std::vector<std::string> requestEvents_ = {};   // 1
    std::vector<std::string> traceCategories_ = {}; // 2
    std::vector<std::string> traceApps_ = {};       // 3
    std::vector<std::string> rawDataDumpPath_ = {}; // 13
    uint32_t tracePeriodMs_ = 0;                    // 10
    uint32_t bufferSizeKb_ = 0;                     // 6
    bool parseKsyms_ = false;                       // 7
    TracePluginConfig_ParseMode parseMode_ = TracePluginConfig_ParseMode_NORMAL;

    WriterStructPtr resultWriter_ = nullptr;
    int platformCpuNum_ = 0;
    bool getClockTimes_ = true;

    bool ftraceSupported_ = false;
    bool flushCacheData_ = false;
    unsigned int hitraceTime_ = 0;
    std::string traceClock_;
};
FTRACE_NS_END
#endif // FLOW_CONTROLLER_H
