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
 * Description: FlowController implements
 */
#include "flow_controller.h"

#include <algorithm>
#include <cinttypes>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <regex>

#include "hitrace_ops.h"
#include "file_utils.h"
#include "ftrace_field_parser.h"
#include "ftrace_fs_ops.h"
#include "logging.h"

namespace {
#ifndef PAGE_SIZE
    constexpr uint32_t PAGE_SIZE = 4096;
#endif
    constexpr int KB_PER_PAGE = PAGE_SIZE / 1024;
    constexpr uint32_t BYTE_PER_KB = 1024;
    constexpr uint32_t MAX_FLUSH_INTERVAL = 1800 * 1000;
    constexpr uint32_t MAX_FLUSH_THRESHOLD = 128 * 1024 * 1024;
    constexpr uint32_t MAX_TRACE_PERIOD_MS = 720 * 1000;
    constexpr uint32_t MAX_BUFFER_SIZE_KB = 64 * 1024; // 64MB
    constexpr uint32_t MIN_BUFFER_SIZE_KB = 1024;      // 1 MB
    constexpr uint32_t DEFAULT_TRACE_PERIOD_MS = 250;  // 250 ms
    constexpr uint32_t MAX_BLOCK_SIZE_PAGES = 4096;    // 16 MB
    constexpr uint32_t MIN_BLOCK_SIZE_PAGES = 256;     // 1  MB
    constexpr uint32_t PARSE_CMDLINE_COUNT = 1000;
    const std::set<std::string> g_availableClocks = { "boot", "global", "local", "mono" };
    constexpr uint32_t SAVED_CMDLINE_SIZE_SMALL = 1024; // save cmdline sizes for cpu num less than 8
    constexpr uint32_t SAVED_CMDLINE_SIZE_LARGE = 4096; // save cmdline sizes for cpu num no less than 8
    constexpr int OCTA_CORE_CPU = 8; // 8 core
} // namespace

FTRACE_NS_BEGIN
std::unique_ptr<TraceOps> FlowController::GetTraceOps()
{
    std::vector<std::unique_ptr<TraceOps>> traceOps;
    traceOps.emplace_back(std::make_unique<HitraceOps>());

    for (size_t i = 0; i < traceOps.size(); i++) {
        auto& ops = traceOps[i];
        CHECK_TRUE(ops != nullptr, nullptr, "traceOps[%zu] is null!", i);
        if (ops->IsSupported()) {
            return std::move(ops);
        }
    }
    return nullptr;
}

FlowController::FlowController()
{
    ftraceParser_ = std::make_unique<FtraceParser>();
    ksymsParser_ = std::make_unique<KernelSymbolsParser>();
    ftraceSupported_ = FtraceFsOps::GetInstance().GetFtraceRoot().size() > 0;
}

FlowController::~FlowController(void)
{
    HILOG_INFO(LOG_CORE, "FlowController destroy!");
}

int FlowController::SetWriter(const WriterStructPtr& writer)
{
    CHECK_TRUE(ftraceSupported_, -1, "current kernel not support ftrace!");
    CHECK_TRUE(resultWriter_ == nullptr, 0, "writer already setted!");

    CHECK_NOTNULL(writer, -1, "writer null!");
    auto transmiter = std::make_unique<ResultTransporter>("Transporter", writer);
    CHECK_NOTNULL(transmiter, -1, "create ResultTransporter FAILED!");

    // get CPU core numbers
    int nprocs = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    CHECK_TRUE(nprocs > 0, -1, "get processor number failed!");
    platformCpuNum_ = nprocs;

    auto traceOps = GetTraceOps();
    CHECK_NOTNULL(traceOps, -1, "get trace ops failed!");

    // init FtraceParser
    CHECK_NOTNULL(ftraceParser_, 0, "FtraceParser create FAILED!");
    CHECK_TRUE(ftraceParser_->Init(), -1, "ftrace parser init failed!");

    // init KernelSymbolsParser
    CHECK_NOTNULL(ksymsParser_, 0, "KernelSymbolsParser create FAILED!");
    ksymsParser_->Parse(FtraceFsOps::GetInstance().GetKernelSymbols());

    CHECK_TRUE(AddPlatformEventsToParser(), -1, "add platform events to parser failed!");
    // disable all trace events
    DisableAllCategories();

    resultWriter_ = writer;
    tansporter_ = std::move(transmiter);
    traceOps_ = std::move(traceOps);
    return 0;
}

bool FlowController::CreateRawDataReaders()
{
    for (int i = 0; i < platformCpuNum_; i++) {
        auto reader = std::make_unique<FtraceDataReader>(FtraceFsOps::GetInstance().GetRawTracePath(i));
        CHECK_NOTNULL(reader, false, "create reader %d FAILED!", i);
        ftraceReaders_.emplace_back(std::move(reader));
    }
    return true;
}

bool FlowController::CreatePagedMemoryPool()
{
    HILOG_INFO(LOG_CORE, "create memory pool, buffer_size_kb = %u", bufferSizeKb_);
    size_t bufferSizePages = bufferSizeKb_ / KB_PER_PAGE;
    size_t pagesPerBlock = bufferSizePages / static_cast<size_t>(platformCpuNum_);
    if (pagesPerBlock < MIN_BLOCK_SIZE_PAGES) {
        pagesPerBlock = MIN_BLOCK_SIZE_PAGES;
    }
    if (pagesPerBlock > MAX_BLOCK_SIZE_PAGES) {
        pagesPerBlock = MAX_BLOCK_SIZE_PAGES;
    }
    memPool_ = std::make_unique<PagedMemPool>(pagesPerBlock, platformCpuNum_);
    CHECK_NOTNULL(memPool_, false, "create PagedMemPool FAILED!");
    return true;
}

bool FlowController::CreateRawDataBuffers()
{
    for (int i = 0; i < platformCpuNum_; i++) {
        using u8ptr = std::unique_ptr<uint8_t>::pointer;
        auto buffer = std::shared_ptr<uint8_t>(reinterpret_cast<u8ptr>(memPool_->Allocate()),
            [&](u8ptr block) { this->memPool_->Recycle(block); });
        CHECK_NOTNULL(buffer, false, "create buffer %d failed!", i);
        ftraceBuffers_.push_back(buffer);
    };
    return true;
}

bool FlowController::CreateRawDataCaches()
{
    char fileName[] = "/data/local/tmp/ftrace_rawdata.XXXXXX";
    CHECK_TRUE(mkstemp(fileName) >= 0, false, "Create temp file failed!");
    rawDataFile_ = std::shared_ptr<FILE>(fopen(fileName, "wb+"), [](FILE* fp) { fclose(fp); });
    unlink(fileName);
    return true;
}

bool FlowController::ParseBasicData()
{
    // get clock times
    if (getClockTimes_) {
        CHECK_TRUE(ReportClockTimes(), -1, "report clock times FAILED!");
    }

    // parse kernel symbols
    if (parseKsyms_) {
        CHECK_TRUE(ParseKernelSymbols(), -1, "parse kernel symbols FAILED!");
    }

    // parse per cpu stats
    CHECK_TRUE(ParsePerCpuStatus(TRACE_START), -1, "parse TRACE_START stats failed!");

    return 0;
}

int FlowController::StartCapture(void)
{
    CHECK_TRUE(ftraceSupported_, -1, "current kernel not support ftrace!");
    CHECK_NOTNULL(ftraceParser_, -1, "create FtraceParser FAILED!");
    CHECK_NOTNULL(ksymsParser_, -1, "create KernelSymbolsParser FAILED!");
    CHECK_NOTNULL(tansporter_, -1, "crated ResultTransporter FAILED!");
    CHECK_NOTNULL(traceOps_, -1, "create TraceOps FAILED!");

    CHECK_TRUE(ParseBasicData() == 0, -1, "parse basic data failed!");

    // create memory pool, and raw data readers, buffers, caches.
    CHECK_TRUE(CreatePagedMemoryPool(), -1, "create paged memory pool failed!");
    CHECK_TRUE(CreateRawDataReaders(), -1, "create raw data readers failed!");
    CHECK_TRUE(CreateRawDataBuffers(), -1, "create raw data buffers failed!");

    // clear old trace
    FtraceFsOps::GetInstance().ClearTraceBuffer();

    uint32_t savedCmdlinesSize = platformCpuNum_ < OCTA_CORE_CPU ? SAVED_CMDLINE_SIZE_SMALL : SAVED_CMDLINE_SIZE_LARGE;
    if (!FtraceFsOps::GetInstance().SetSavedCmdLinesSize(savedCmdlinesSize)) {
        HILOG_ERROR(LOG_CORE, "SetSavedCmdLinesSize %u fail.", savedCmdlinesSize);
    }

    // enable additional record options
    FtraceFsOps::GetInstance().SetRecordCmdOption(true);
    FtraceFsOps::GetInstance().SetRecordTgidOption(true);

    // start ftrace event data polling thread
    keepRunning_ = true;

    if (parseMode_ == TracePluginConfig_ParseMode_NORMAL) {
        pollThread_ = std::thread(&FlowController::CaptureWorkOnNomalMode, this);
    } else if (parseMode_ == TracePluginConfig_ParseMode_DELAY_PARSE) {
        CHECK_TRUE(CreateRawDataCaches(), -1, "create raw data caches failed!");
        pollThread_ = std::thread(&FlowController::CaptureWorkOnDelayMode, this);
    } else {
        HILOG_ERROR(LOG_CORE, "ParseMode is Illegal parameter!");
        return -1;
    }

    // set trace_clock
    traceOps_->SetTraceClock(traceClock_);
    // enable ftrace event switches
    if (traceCategories_.size() > 0) {
        traceOps_->EnableCategories(traceCategories_, hitraceTime_);
    }

    // hitrace will reset trace buffer size, so reset user config after hitrace
    FtraceFsOps::GetInstance().SetBufferSizeKb(bufferSizeKb_);
    EnableTraceEvents();
    return 0;
}

void FlowController::CaptureWorkOnNomalMode()
{
    pthread_setname_np(pthread_self(), "TraceReader");
    HILOG_INFO(LOG_CORE, "FlowController::CaptureWorkOnNomalMode start!");

    auto tracePeriod = std::chrono::milliseconds(tracePeriodMs_);
    std::vector<long> rawDataBytes(platformCpuNum_, 0);

    while (keepRunning_) {
        std::this_thread::sleep_for(tracePeriod);

        // read data from percpu trace_pipe_raw, consume kernel ring buffers
        for (size_t i = 0; i < rawDataBytes.size(); i++) {
            if (flushCacheData_ && !keepRunning_) {
                HILOG_INFO(LOG_CORE, "flushCacheData_ is true, return");
                return;
            }
            long nbytes = ReadEventData(i);
            rawDataBytes[i] = nbytes;
        }

        // parse ftrace metadata
        ftraceParser_->ParseSavedCmdlines(FtraceFsOps::GetInstance().GetSavedCmdLines());

        // parse ftrace percpu event data
        for (size_t i = 0; i < rawDataBytes.size(); i++) {
            if (flushCacheData_ && !keepRunning_) {
                HILOG_INFO(LOG_CORE, "flushCacheData_ is true, return");
                return;
            }
            if (rawDataBytes[i] == 0) {
                HILOG_INFO(LOG_CORE, "Get raw data from CPU%zu is 0 bytes.", i);
                continue;
            }
            CHECK_TRUE(ParseEventDataOnNomalMode(i, rawDataBytes[i]), NO_RETVAL, "ParseEventData failed!");
        }
    }

    tansporter_->Flush();
    HILOG_DEBUG(LOG_CORE, "FlowController::CaptureWorkOnNomalMode done!");
}

void FlowController::CaptureWorkOnDelayMode()
{
    pthread_setname_np(pthread_self(), "TraceReader");
    HILOG_INFO(LOG_CORE, "FlowController::CaptureWorkOnDelayMode start!");

    auto tracePeriod = std::chrono::milliseconds(tracePeriodMs_);
    int writeDataCount = 0;
    while (keepRunning_) {
        std::this_thread::sleep_for(tracePeriod);

        // read data from percpu trace_pipe_raw, consume kernel ring buffers
        for (int cpuIdx = 0; cpuIdx < platformCpuNum_; cpuIdx++) {
            if (flushCacheData_ && !keepRunning_) {
                HILOG_INFO(LOG_CORE, "flushCacheData_ is true, return");
                return;
            }
            long nbytes = ReadEventData(cpuIdx);
            if (nbytes == 0) {
                HILOG_INFO(LOG_CORE, "Get raw data from CPU%d is 0 bytes.", cpuIdx);
                continue;
            }
            fwrite(&cpuIdx, sizeof(uint8_t), 1, rawDataFile_.get());
            fwrite(&nbytes, sizeof(long), 1, rawDataFile_.get());
            fwrite(ftraceBuffers_[cpuIdx].get(), sizeof(uint8_t), nbytes, rawDataFile_.get());
        }
        writeDataCount++;
        if (writeDataCount == PARSE_CMDLINE_COUNT) {
            // parse ftrace metadata
            ftraceParser_->ParseSavedCmdlines(FtraceFsOps::GetInstance().GetSavedCmdLines());
            writeDataCount = 0;
        }
    }

    CHECK_TRUE(ParseEventDataOnDelayMode(), NO_RETVAL, "ParseEventData failed!");
    tansporter_->Flush();
    HILOG_DEBUG(LOG_CORE, "FlowController::CaptureWorkOnDelayMode done!");
}

long FlowController::ReadEventData(int cpuid)
{
    auto buffer = ftraceBuffers_[cpuid].get();
    auto reader = ftraceReaders_[cpuid].get();
    auto bufferSize = static_cast<long>(memPool_->GetBlockSize());

    long nbytes = 0;
    long used = 0;
    long rest = bufferSize;
    while ((nbytes = reader->Read(&buffer[used], rest)) > 0 && used < bufferSize) {
        CHECK_TRUE(used % PAGE_SIZE == 0, used, "used invalid!");
        used += nbytes;
        rest -= nbytes;
    }
    return used;
}

bool FlowController::ParseEventDataOnNomalMode(int cpuid, long dataSize)
{
    auto buffer = ftraceBuffers_[cpuid].get();
    auto endPtr = buffer + dataSize;

    for (auto page = buffer; page < endPtr; page += PAGE_SIZE) {
        CHECK_TRUE(ParseFtraceEvent(cpuid, page), false, "parse raw event for cpu-%d failed!", cpuid);
    }
    return true;
}

bool FlowController::ParseEventDataOnDelayMode()
{
    CHECK_TRUE(fseek(rawDataFile_.get(), 0, SEEK_SET) == 0, false, "fseek failed!");
    while (!feof(rawDataFile_.get())) {
        uint8_t cpuId = 0;
        long dataBytes = 0;
        fread(&cpuId, sizeof(uint8_t), 1, rawDataFile_.get());
        fread(&dataBytes, sizeof(long), 1, rawDataFile_.get());
        for (long i = 0; i < dataBytes; i += PAGE_SIZE) {
            uint8_t page[PAGE_SIZE] = {0};
            fread(page, sizeof(uint8_t), PAGE_SIZE, rawDataFile_.get());
            CHECK_TRUE(ParseFtraceEvent(cpuId, page), false, "ParseFtraceEvent failed!");
        }
    }
    return true;
}

int FlowController::StopCapture(void)
{
    CHECK_TRUE(ftraceSupported_, -1, "current kernel not support ftrace!");
    CHECK_NOTNULL(tansporter_, -1, "crate ResultTransporter FAILED!");
    CHECK_NOTNULL(traceOps_, -1, "create TraceOps FAILED!");

    CHECK_TRUE(requestEvents_.size() != 0 || traceApps_.size() != 0 || traceCategories_.size() != 0, -1,
               "StopCapture: ftrace event is not set, return false");

    // disable ftrace event switches
    DisableTraceEvents();

    // stop ftrace event data polling thread
    keepRunning_ = false;
    if (pollThread_.joinable()) {
        HILOG_INFO(LOG_CORE, "join thread start!\n");
        pollThread_.join();
        HILOG_INFO(LOG_CORE, "join thread  done!\n");
    }

    // parse per cpu stats
    CHECK_TRUE(ParsePerCpuStatus(TRACE_END), -1, "parse TRACE_END stats failed!");

    // disable userspace trace triggers
    // because trace cmd will read trace buffer,
    // so we to this action after polling thread exit.
    if (traceCategories_.size() > 0) {
        traceOps_->DisableCategories();
    }
    tansporter_->Flush();

    // release resources
    ftraceReaders_.clear();   // release ftrace data readers
    ftraceBuffers_.clear();   // release ftrace event read buffers
    memPool_.reset();         // release memory pool
    return 0;
}

bool FlowController::ParsePerCpuStatus(int stage)
{
    auto tracePacket = std::make_unique<TracePluginResult>();
    CHECK_NOTNULL(tracePacket, false, "create TracePluginResult FAILED!");

    auto cpuStatsMsg = tracePacket->add_ftrace_cpu_stats();
    if (stage == TRACE_START) {
        cpuStatsMsg->set_status(FtraceCpuStatsMsg_Status_TRACE_START);
    } else {
        cpuStatsMsg->set_status(FtraceCpuStatsMsg_Status_TRACE_END);
    }

    std::string traceClock = FtraceFsOps::GetInstance().GetTraceClock();
    if (traceClock.size() > 0) {
        cpuStatsMsg->set_trace_clock(traceClock);
    }

    for (int i = 0; i < platformCpuNum_; i++) {
        HILOG_INFO(LOG_CORE, "[%d] ParsePerCpuStatus %d!", i, stage);
        PerCpuStats stats = {};
        stats.cpuIndex = i;
        ftraceParser_->ParsePerCpuStatus(stats, FtraceFsOps::GetInstance().GetPerCpuStats(i));
        auto perCpuMsg = cpuStatsMsg->add_per_cpu_stats();
        perCpuMsg->set_cpu(stats.cpuIndex);
        perCpuMsg->set_entries(stats.entries);
        perCpuMsg->set_overrun(stats.overrun);
        perCpuMsg->set_commit_overrun(stats.commitOverrun);
        perCpuMsg->set_bytes(stats.bytes);
        perCpuMsg->set_oldest_event_ts(stats.oldestEventTs);
        perCpuMsg->set_now_ts(stats.nowTs);
        perCpuMsg->set_dropped_events(stats.droppedEvents);
        perCpuMsg->set_read_events(stats.readEvents);
    }

    return tansporter_->Submit(std::move(tracePacket));
}

bool FlowController::ReportClockTimes()
{
    auto traceResult = std::make_unique<TracePluginResult>();
    CHECK_NOTNULL(traceResult, false, "create TracePluginResult FAILED!");

    std::map<clockid_t, ClockDetailMsg::ClockId> clocksMap = {
        {CLOCK_REALTIME, ClockDetailMsg::REALTIME},
        {CLOCK_REALTIME_COARSE, ClockDetailMsg::REALTIME_COARSE},
        {CLOCK_MONOTONIC, ClockDetailMsg::MONOTONIC},
        {CLOCK_MONOTONIC_COARSE, ClockDetailMsg::MONOTONIC_COARSE},
        {CLOCK_MONOTONIC_RAW, ClockDetailMsg::MONOTONIC_RAW},
        {CLOCK_BOOTTIME, ClockDetailMsg::BOOTTIME},
    };
    for (auto& entry : clocksMap) {
        struct timespec ts = {};
        clock_gettime(entry.first, &ts);
        auto clockMsg = traceResult->add_clocks_detail();
        CHECK_NOTNULL(clockMsg, false, "add clock_detail failed for %d!", entry.first);
        clockMsg->set_id(entry.second);
        auto timeMsg = clockMsg->mutable_time();
        timeMsg->set_tv_sec(ts.tv_sec);
        timeMsg->set_tv_nsec(ts.tv_nsec);

        struct timespec tsResolution = {};
        clock_getres(entry.first, &tsResolution);
        auto resolutionMsg = clockMsg->mutable_resolution();
        resolutionMsg->set_tv_sec(tsResolution.tv_sec);
        resolutionMsg->set_tv_nsec(tsResolution.tv_nsec);
    }
    return tansporter_->Submit(std::move(traceResult));
}

bool FlowController::ParseKernelSymbols()
{
    auto traceResult = std::make_unique<TracePluginResult>();
    CHECK_NOTNULL(traceResult, false, "create TracePluginResult FAILED!");

    ksymsParser_->Accept([&traceResult](const KernelSymbol& symbol) {
        auto symbolDetail = traceResult->add_symbols_detail();
        symbolDetail->set_symbol_addr(symbol.addr);
        symbolDetail->set_symbol_name(symbol.name);
    });
    HILOG_INFO(LOG_CORE, "parse kernel symbol message done!");
    return tansporter_->Submit(std::move(traceResult));
}

bool FlowController::ParseFtraceEvent(int cpuid, uint8_t page[])
{
    auto tracePacket = std::make_unique<TracePluginResult>();
    CHECK_NOTNULL(tracePacket, false, "create TracePluginResult FAILED!");

    auto cpudetail = tracePacket->add_ftrace_cpu_detail();
    cpudetail->set_cpu(static_cast<uint32_t>(cpuid));

    CHECK_TRUE(ftraceParser_->ParsePage(*cpudetail, page, PAGE_SIZE), false, "parse page failed!");

    return tansporter_->Submit(std::move(tracePacket));
}

bool FlowController::AddPlatformEventsToParser(void)
{
    CHECK_TRUE(ftraceSupported_, false, "current kernel not support ftrace!");

    HILOG_INFO(LOG_CORE, "Add platform events to parser start!");
    for (auto& typeName : FtraceFsOps::GetInstance().GetPlatformEvents()) {
        std::string type = typeName.first;
        std::string name = typeName.second;
        if (ftraceParser_->SetupEvent(type, name)) {
            supportedEvents_.push_back(typeName);
        }
    }
    HILOG_INFO(LOG_CORE, "Add platform events to parser done, events: %zu!", supportedEvents_.size());
    return true;
}

int FlowController::LoadConfig(const uint8_t configData[], uint32_t size)
{
    CHECK_TRUE(size > 0, -1, "config data size is zero!");
    CHECK_NOTNULL(configData, -1, "config data is null!");
    CHECK_TRUE(ftraceSupported_, -1, "current kernel not support ftrace!");
    CHECK_NOTNULL(tansporter_, -1, "ResultTransporter crated FAILED!");

    TracePluginConfig traceConfig;
    CHECK_TRUE(traceConfig.ParseFromArray(configData, size), -1, "parse %u bytes configData failed!", size);

    // sort and save user requested trace events
    std::set<std::string> events(traceConfig.ftrace_events().begin(), traceConfig.ftrace_events().end());
    for (auto ftraceEvent : events) {
        requestEvents_.push_back(ftraceEvent);
    }

    traceApps_.assign(traceConfig.hitrace_apps().begin(), traceConfig.hitrace_apps().end());
    traceCategories_.assign(traceConfig.hitrace_categories().begin(), traceConfig.hitrace_categories().end());

    CHECK_TRUE(requestEvents_.size() != 0 || traceApps_.size() != 0 || traceCategories_.size() != 0, -1,
               "LoadConfig: ftrace event is not set, return false");

    // setup trace clock
    if (g_availableClocks.count(traceConfig.clock()) > 0) {
        traceClock_ = traceConfig.clock();
        FtraceFsOps::GetInstance().SetTraceClock(traceConfig.clock());
    }

    // setup parse kernel symbol option
    parseKsyms_ = traceConfig.parse_ksyms();
    parseMode_ = traceConfig.parse_mode();
    // setup trace buffer size
    SetupTraceBufferSize(traceConfig.buffer_size_kb());

    // setup transporter flush params
    SetupTransporterFlushParams(traceConfig.flush_interval_ms(), traceConfig.flush_threshold_kb());

    // generate raw data file names
    GenerateRawDataFileNames(traceConfig.raw_data_prefix());

    // setup trace period param
    SetupTraceReadPeriod(traceConfig.trace_period_ms());
    flushCacheData_ = traceConfig.discard_cache_data();
    hitraceTime_ = traceConfig.hitrace_time();
    return 0;
}

void FlowController::SetupTraceBufferSize(uint32_t sizeKb)
{
    if (sizeKb < MIN_BUFFER_SIZE_KB) {
        bufferSizeKb_ = MIN_BUFFER_SIZE_KB;
    } else if (sizeKb > MAX_BUFFER_SIZE_KB) {
        bufferSizeKb_ = MAX_BUFFER_SIZE_KB;
    } else {
        bufferSizeKb_ = sizeKb / KB_PER_PAGE * KB_PER_PAGE;
    }
}

void FlowController::SetupTransporterFlushParams(uint32_t flushInterval, uint32_t flushThresholdKb)
{
    if (flushInterval > 0 && flushInterval <= MAX_FLUSH_INTERVAL) {
        tansporter_->SetFlushInterval(flushInterval);
    }
    if (flushThresholdKb > 0 && flushThresholdKb <= MAX_FLUSH_THRESHOLD) {
        tansporter_->SetFlushThreshold(flushThresholdKb * BYTE_PER_KB);
    }
}

void FlowController::GenerateRawDataFileNames(const std::string& prefix)
{
    if (prefix.size() > 0) {
        for (int i = 0; i < platformCpuNum_; i++) {
            std::string path = prefix + std::to_string(i);
            rawDataDumpPath_.push_back(path);
        }
    }
}

void FlowController::SetupTraceReadPeriod(uint32_t tracePeriod)
{
    if (tracePeriod > 0 && tracePeriod <= MAX_TRACE_PERIOD_MS) {
        tracePeriodMs_ = tracePeriod;
    } else {
        tracePeriodMs_ = DEFAULT_TRACE_PERIOD_MS;
    }
}

void FlowController::EnableTraceEvents(void)
{
    std::unordered_set<std::string> userEventSet(requestEvents_.begin(), requestEvents_.end());
    for (auto& event : supportedEvents_) {
        std::string type = event.first;
        std::string name = event.second;
        if (userEventSet.count(type + "/" + name)) { // user config format
            if (FtraceFsOps::GetInstance().EnableEvent(type, name)) {
                FtraceFsOps::GetInstance().AppendSetEvent(type, name);
                enabledEvents_.push_back(event);
            }
        }
    }
    FtraceFsOps::GetInstance().EnableTracing();
}

void FlowController::DisableTraceEvents(void)
{
    FtraceFsOps::GetInstance().DisableTracing();
    for (auto& event : enabledEvents_) {
        std::string type = event.first;
        std::string name = event.second;
        FtraceFsOps::GetInstance().DisableEvent(type, name);
    }
    enabledEvents_.clear();
}

void FlowController::DisableAllCategories(void)
{
    for (auto& event : supportedEvents_) {
        std::string type = event.first;
        std::string name = event.second;
        FtraceFsOps::GetInstance().DisableCategories(type);
    }
}
FTRACE_NS_END
