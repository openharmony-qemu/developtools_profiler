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

#include "file_utils.h"
#include "ftrace_field_parser.h"
#include "ftrace_fs_ops.h"
#include "logging.h"
#include "parameters.h"

namespace {
using namespace OHOS::Developtools::Profiler;
#ifndef PAGE_SIZE
    constexpr uint32_t PAGE_SIZE = 4096;
#endif
    constexpr int KB_PER_PAGE = PAGE_SIZE / 1024;
    constexpr uint32_t BYTE_PER_KB = 1024;
    constexpr uint32_t MAX_FLUSH_INTERVAL = 1800 * 1000;
    constexpr uint32_t MAX_FLUSH_THRESHOLD = 128 * 1024 * 1024;
    constexpr uint32_t MAX_TRACE_PERIOD_MS = 720 * 1000;
    constexpr uint32_t MAX_BUFFER_SIZE_KB = 64 * 1024; // 64MB
    constexpr uint32_t HM_MAX_BUFFER_SIZE_KB = 512 * 1024; // 512MB
    constexpr uint32_t MIN_BUFFER_SIZE_KB = 1024;      // 1 MB
    constexpr uint32_t DEFAULT_TRACE_PERIOD_MS = 250;  // 250 ms
    constexpr uint32_t MAX_BLOCK_SIZE_PAGES = 4096;    // 16 MB
    constexpr uint32_t MIN_BLOCK_SIZE_PAGES = 256;     // 1  MB
    constexpr uint32_t PARSE_CMDLINE_COUNT = 1000;
    const std::set<std::string> g_availableClocks = { "boot", "global", "local", "mono" };
    constexpr uint32_t SAVED_CMDLINE_SIZE_SMALL = 1024; // save cmdline sizes for cpu num less than 8
    constexpr uint32_t SAVED_CMDLINE_SIZE_LARGE = 4096; // save cmdline sizes for cpu num no less than 8
    constexpr int OCTA_CORE_CPU = 8; // 8 core
    constexpr unsigned int RMQ_ENTRY_ALIGN_MASK = (1U << 2) - 1;
    const std::string TRACE_PROPERTY = "debug.hitrace.tags.enableflags";
    const std::string BGSRV_PROPERTY = "5456538433239656448";
} // namespace

FTRACE_NS_BEGIN
FlowController::FlowController()
{
    ftraceParser_ = std::make_unique<FtraceParser>();
    ksymsParser_ = std::make_unique<KernelSymbolsParser>();
    ftraceSupported_ = FtraceFsOps::GetInstance().GetFtraceRoot().size() > 0;
    traceCollector_ = OHOS::HiviewDFX::UCollectClient::TraceCollector::Create();
}

FlowController::~FlowController(void)
{
    PROFILER_LOG_INFO(LOG_CORE, "FlowController destroy!");
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
    return 0;
}

bool FlowController::CreateRawDataReaders()
{
    if (FtraceFsOps::GetInstance().IsHmKernel()) {
        auto reader = std::make_unique<FtraceDataReader>(FtraceFsOps::GetInstance().GetHmRawTracePath());
        CHECK_NOTNULL(reader, false, "create hm raw trace reader FAILED!");
        ftraceReaders_.emplace_back(std::move(reader));
        return true;
    }

    for (int i = 0; i < platformCpuNum_; i++) {
        auto rawPath = FtraceFsOps::GetInstance().GetRawTracePath(i);
        if (fakePath_ != "") {
            rawPath = fakePath_ + "test_raw_" + std::to_string(i);
            CHECK_NOTNULL(ftraceParser_, false, "create FtraceParser FAILED!");
            ftraceParser_->ParseSavedCmdlines(FileUtils::ReadFile(fakePath_ + "test_comm"));
            ftraceParser_->ParseSavedTgid(FileUtils::ReadFile(fakePath_ + "test_tgid"));
        }
        auto reader = std::make_unique<FtraceDataReader>(rawPath);
        CHECK_NOTNULL(reader, false, "create reader %d FAILED!", i);
        ftraceReaders_.emplace_back(std::move(reader));
    }
    return true;
}

bool FlowController::CreatePagedMemoryPool()
{
    PROFILER_LOG_INFO(LOG_CORE, "create memory pool, buffer_size_kb = %u", bufferSizeKb_);
    if (KB_PER_PAGE == 0 || platformCpuNum_ == 0) {
        return false;
    }
    size_t bufferSizePages = bufferSizeKb_ / KB_PER_PAGE;
    size_t pagesPerBlock = bufferSizePages / static_cast<size_t>(platformCpuNum_);
    if (pagesPerBlock < MIN_BLOCK_SIZE_PAGES) {
        pagesPerBlock = MIN_BLOCK_SIZE_PAGES;
    }
    if (pagesPerBlock > MAX_BLOCK_SIZE_PAGES) {
        pagesPerBlock = MAX_BLOCK_SIZE_PAGES;
    }

    if (FtraceFsOps::GetInstance().IsHmKernel()) {
        memPool_ = std::make_unique<PagedMemPool>(bufferSizePages, 1);
    } else {
        memPool_ = std::make_unique<PagedMemPool>(pagesPerBlock, platformCpuNum_);
    }
    CHECK_NOTNULL(memPool_, false, "create PagedMemPool FAILED!");
    return true;
}

bool FlowController::CreateRawDataBuffers()
{
    int num = platformCpuNum_;
    if (FtraceFsOps::GetInstance().IsHmKernel()) {
        num = 1;
    }
    for (int i = 0; i < num; i++) {
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
    CHECK_NOTNULL(resultWriter_, false, "%s: resultWriter_ nullptr", __func__);
    // get clock times
    if (getClockTimes_) {
        if (resultWriter_->isProtobufSerialize) {
            auto traceResult = std::make_unique<TracePluginResult>();
            CHECK_TRUE(ReportClockTimes(traceResult), false, "parse clock times FAILED!");
            CHECK_TRUE(tansporter_->Submit(std::move(traceResult)), false, "report clock times FAILED!");
        } else {
            auto ctx = resultWriter_->startReport(resultWriter_);
            CHECK_NOTNULL(ctx, false, "%s: get RandomWriteCtx FAILED!", __func__);
            auto traceResult = std::make_unique<ProtoEncoder::TracePluginResult>(ctx);
            CHECK_TRUE(ReportClockTimes(traceResult), false, "parse clock times FAILED!");
            int32_t msgSize = traceResult->Finish();
            resultWriter_->finishReport(resultWriter_, msgSize);
            tansporter_->Report(static_cast<size_t>(msgSize));
        }
    }

    // parse kernel symbols
    if (parseKsyms_) {
        if (resultWriter_->isProtobufSerialize) {
            auto traceResult = std::make_unique<TracePluginResult>();
            CHECK_TRUE(ParseKernelSymbols(traceResult), false, "parse kernel symbols FAILED!");
            CHECK_TRUE(tansporter_->Submit(std::move(traceResult)), false, "report kernel symbols FAILED!");
        } else {
            auto ctx = resultWriter_->startReport(resultWriter_);
            CHECK_NOTNULL(ctx, false, "%s: get RandomWriteCtx FAILED!", __func__);
            auto traceResult = std::make_unique<ProtoEncoder::TracePluginResult>(ctx);
            CHECK_TRUE(ParseKernelSymbols(traceResult), false, "parse kernel symbols FAILED!");
            int32_t msgSize = traceResult->Finish();
            resultWriter_->finishReport(resultWriter_, msgSize);
            tansporter_->Report(static_cast<size_t>(msgSize));
        }
    }

    // parse per cpu stats
    if (resultWriter_->isProtobufSerialize) {
        auto traceResult = std::make_unique<TracePluginResult>();
        CHECK_TRUE(ParsePerCpuStatus(traceResult, TRACE_START), false, "parse TRACE_START stats failed!");
        CHECK_TRUE(tansporter_->Submit(std::move(traceResult)), false, "report TRACE_START stats failed!");
    } else {
        auto ctx = resultWriter_->startReport(resultWriter_);
        CHECK_NOTNULL(ctx, false, "%s: get RandomWriteCtx FAILED!", __func__);
        auto traceResult = std::make_unique<ProtoEncoder::TracePluginResult>(ctx);
        CHECK_TRUE(ParsePerCpuStatus(traceResult, TRACE_START), false, "parse TRACE_START stats failed!");
        int32_t msgSize = traceResult->Finish();
        resultWriter_->finishReport(resultWriter_, msgSize);
        tansporter_->Report(static_cast<size_t>(msgSize));
    }
    return 0;
}

std::string FlowController::ReloadTraceArgs()
{
    std::string args;
    for (size_t i = 0; i < traceCategories_.size(); i++) {
        if (i == 0) {
            args += ("tags:" + traceCategories_[i]);
        } else {
            args += ("," + traceCategories_[i]);
        }
    }

    if (traceClock_.size() > 0) {
        args += (" clockType:" + traceClock_);
    }

    if (bufferSizeKb_ > 0) {
        args += (" bufferSize:" + std::to_string(bufferSizeKb_));
    }
    PROFILER_LOG_INFO(LOG_CORE, "trace args: %s", args.c_str());
    return args;
}

int FlowController::StartCapture(void)
{
    CHECK_TRUE(ftraceSupported_, -1, "current kernel not support ftrace!");
    CHECK_NOTNULL(ftraceParser_, -1, "create FtraceParser FAILED!");
    CHECK_NOTNULL(ksymsParser_, -1, "create KernelSymbolsParser FAILED!");
    CHECK_NOTNULL(tansporter_, -1, "create ResultTransporter FAILED!");
    CHECK_NOTNULL(traceCollector_, -1, "create TraceCollector FAILED!");
    CHECK_NOTNULL(resultWriter_, -1, "%s: resultWriter_ nullptr", __func__);

    CHECK_TRUE(ParseBasicData() == 0, -1, "parse basic data failed!");

    // create memory pool, and raw data readers, buffers, caches.
    CHECK_TRUE(CreatePagedMemoryPool(), -1, "create paged memory pool failed!");
    CHECK_TRUE(CreateRawDataReaders(), -1, "create raw data readers failed!");
    CHECK_TRUE(CreateRawDataBuffers(), -1, "create raw data buffers failed!");

    // clear old trace
    FtraceFsOps::GetInstance().ClearTraceBuffer();
    // recover the hitrace
    std::string param = OHOS::system::GetParameter(TRACE_PROPERTY, "");
    if (param != "0" && param != BGSRV_PROPERTY) {
        traceCollector_->Recover();
    }

    uint32_t savedCmdlinesSize = platformCpuNum_ < OCTA_CORE_CPU ? SAVED_CMDLINE_SIZE_SMALL : SAVED_CMDLINE_SIZE_LARGE;
    if (!FtraceFsOps::GetInstance().SetSavedCmdLinesSize(savedCmdlinesSize)) {
        PROFILER_LOG_ERROR(LOG_CORE, "SetSavedCmdLinesSize %u fail.", savedCmdlinesSize);
    }

    // enable additional record options
    FtraceFsOps::GetInstance().SetRecordCmdOption(true);
    FtraceFsOps::GetInstance().SetRecordTgidOption(true);

    // start ftrace event data polling thread
    keepRunning_ = true;

    if (parseMode_ == TracePluginConfig_ParseMode_NORMAL) {
        pollThread_ = std::thread([this] { this->CaptureWorkOnNomalMode(); });
    } else if (parseMode_ == TracePluginConfig_ParseMode_DELAY_PARSE) {
        CHECK_TRUE(CreateRawDataCaches(), -1, "create raw data caches failed!");
        pollThread_ = std::thread([this] { this->CaptureWorkOnDelayMode(); });
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "ParseMode is Illegal parameter!");
        return -1;
    }

    // set trace_clock and enable all tag categories with hiview::TraceCollector
    auto openRet = traceCollector_->OpenRecording(ReloadTraceArgs());
    if (openRet.retCode != OHOS::HiviewDFX::UCollect::UcError::SUCCESS) {
        PROFILER_LOG_ERROR(LOG_CORE, "Enable tag categories failed, trace error code is %d!", openRet.retCode);
        return -1;
    }
    EnableTraceEvents();
    return 0;
}

void FlowController::CaptureWorkOnNomalModeInner()
{
    pthread_setname_np(pthread_self(), "TraceReader");
    PROFILER_LOG_INFO(LOG_CORE, "FlowController::CaptureWorkOnNomalMode start!");
    auto tracePeriod = std::chrono::milliseconds(tracePeriodMs_);
    std::vector<long> rawDataBytes(platformCpuNum_, 0);
    while (keepRunning_) {
        std::this_thread::sleep_for(tracePeriod);
        // read data from percpu trace_pipe_raw, consume kernel ring buffers
        for (size_t i = 0; i < rawDataBytes.size(); i++) {
            if (flushCacheData_ && !keepRunning_) {
                PROFILER_LOG_INFO(LOG_CORE, "flushCacheData_ is true, return");
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
                PROFILER_LOG_INFO(LOG_CORE, "flushCacheData_ is true, return");
                return;
            }
            if (rawDataBytes[i] == 0) {
                PROFILER_LOG_INFO(LOG_CORE, "Get raw data from CPU%zu is 0 bytes.", i);
                continue;
            }
            if (!ParseEventDataOnNomalMode(i, rawDataBytes[i])) {
                PROFILER_LOG_ERROR(LOG_CORE, "%s:ParseEventData failed!", __func__);
            }
        }
        if (isReportBasicData_.load()) {
            ParseBasicData();
            isReportBasicData_ = false;
        }
    }
    tansporter_->Flush();
    PROFILER_LOG_DEBUG(LOG_CORE, "FlowController::CaptureWorkOnNomalMode done!");
}

long FlowController::HmReadEventData()
{
    auto buffer = ftraceBuffers_[0].get();
    auto reader = ftraceReaders_[0].get();
    auto bufferSize = static_cast<long>(memPool_->GetBlockSize());

    long nbytes = 0;
    long used = 0;
    long rest = bufferSize;
    while ((nbytes = reader->Read(&buffer[used], rest)) > 0 && used < bufferSize) {
        used += nbytes;
        rest -= nbytes;
    }
    if (used == bufferSize) {
        PROFILER_LOG_WARN(LOG_CORE, "hm trace raw data may overwrite. current buffer size = %u.",
                          (unsigned int)bufferSize);
    }
    return used;
}

void FlowController::HmCaptureWorkOnNomalModeInner()
{
    pthread_setname_np(pthread_self(), "HmTraceReader");
    PROFILER_LOG_INFO(LOG_CORE, "FlowController::HmCaptureWorkOnNomalMode start!");
    auto tracePeriod = std::chrono::milliseconds(tracePeriodMs_);
    while (keepRunning_) {
        std::this_thread::sleep_for(tracePeriod);
        if (flushCacheData_ && !keepRunning_) {
            PROFILER_LOG_INFO(LOG_CORE, "flushCacheData_ is true, return");
            return;
        }
        long rawDataBytes = HmReadEventData();
        ftraceParser_->ParseSavedCmdlines(FtraceFsOps::GetInstance().GetSavedCmdLines());
        if (flushCacheData_ && !keepRunning_) {
            PROFILER_LOG_INFO(LOG_CORE, "flushCacheData_ is true, return");
            return;
        }
        if (rawDataBytes == 0) {
            PROFILER_LOG_INFO(LOG_CORE, "Get hm raw data is 0 bytes.");
            continue;
        }
        if (!HmParseEventDataOnNomalMode(rawDataBytes)) {
            PROFILER_LOG_ERROR(LOG_CORE, "HmParseEventData failed!");
        }
    }
    tansporter_->Flush();
    PROFILER_LOG_INFO(LOG_CORE, "FlowController::HmCaptureWorkOnNomalMode done!");
}

void FlowController::CaptureWorkOnNomalMode()
{
    if (FtraceFsOps::GetInstance().IsHmKernel()) {
        HmCaptureWorkOnNomalModeInner();
    } else {
        CaptureWorkOnNomalModeInner();
    }
}

void FlowController::CaptureWorkOnDelayMode()
{
    pthread_setname_np(pthread_self(), "TraceReader");
    PROFILER_LOG_INFO(LOG_CORE, "FlowController::CaptureWorkOnDelayMode start!");

    auto tracePeriod = std::chrono::milliseconds(tracePeriodMs_);
    int writeDataCount = 0;
    while (keepRunning_) {
        std::this_thread::sleep_for(tracePeriod);

        // read data from percpu trace_pipe_raw, consume kernel ring buffers
        for (int cpuIdx = 0; cpuIdx < platformCpuNum_; cpuIdx++) {
            if (flushCacheData_ && !keepRunning_) {
                PROFILER_LOG_INFO(LOG_CORE, "flushCacheData_ is true, return");
                return;
            }
            long nbytes = ReadEventData(cpuIdx);
            if (nbytes == 0) {
                PROFILER_LOG_INFO(LOG_CORE, "Get raw data from CPU%d is 0 bytes.", cpuIdx);
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
    PROFILER_LOG_DEBUG(LOG_CORE, "FlowController::CaptureWorkOnDelayMode done!");
}

static inline int RmqEntryTotalSize(unsigned int size)
{
    return sizeof(struct RmqEntry) + ((size + RMQ_ENTRY_ALIGN_MASK) & (~RMQ_ENTRY_ALIGN_MASK));
}

template <typename T, typename E>
bool FlowController::HmParseEventData(T* traceResult, uint8_t*& data, E* ftraceEvent)
{
    struct RmqConsumerData* rmqData = reinterpret_cast<struct RmqConsumerData*>(data);
    uint64_t timeStampBase = rmqData->timeStamp;
    auto cpuDetailMsg = traceResult->add_ftrace_cpu_detail();
    struct RmqEntry* event;
    cpuDetailMsg->set_cpu(rmqData->coreId);
    cpuDetailMsg->set_overwrite(0);
    auto curPtr = rmqData->data;
    auto endPtr = rmqData->data + rmqData->length;
    while (curPtr < endPtr) {
        event = reinterpret_cast<struct RmqEntry*>(curPtr);
        unsigned int evtSize = event->size;
        if (evtSize == 0U) {
            break;
        }
        struct HmTraceHeader* header = reinterpret_cast<struct HmTraceHeader*>(event->data);
        auto parseEventCtx = SubEventParser<E>::GetInstance().GetParseEventCtx(header->commonType);
        if (parseEventCtx == NULL) {
            curPtr += RmqEntryTotalSize(evtSize);
            continue;
        }
        ftraceEvent = cpuDetailMsg->add_event();
        ftraceEvent->set_timestamp(event->timeStampOffset + timeStampBase);
        if (!ftraceParser_->HmParseFtraceEvent(*ftraceEvent, reinterpret_cast<uint8_t*>(header), evtSize,
                                               parseEventCtx)) {
            PROFILER_LOG_ERROR(LOG_CORE, "hm parse event failed!");
        }
        curPtr += RmqEntryTotalSize(evtSize);
    }
    data += PAGE_SIZE;
    return true;
}

bool FlowController::HmParseEventDataOnNomalMode(long dataSize)
{
    CHECK_NOTNULL(resultWriter_, false, "%s: resultWriter_ nullptr", __func__);
    auto buffer = ftraceBuffers_[0].get();
    auto endPtr = buffer + dataSize;

    for (auto data = buffer; data < endPtr;) {
        if (resultWriter_->isProtobufSerialize) {
            auto traceResult = std::make_unique<TracePluginResult>();
            FtraceEvent* event = nullptr;
            CHECK_TRUE(HmParseEventData(traceResult.get(), data, event), false, "hm parse raw data failed!");
            CHECK_TRUE(tansporter_->Submit(std::move(traceResult)), false, "report hm raw event failed!");
        } else {
            auto ctx = resultWriter_->startReport(resultWriter_);
            CHECK_NOTNULL(ctx, false, "%s: get RandomWriteCtx FAILED!", __func__);
            static ProtoEncoder::MessagePool msgPool;
            static ProtoEncoder::TracePluginResult traceResult;
            msgPool.Reset();
            traceResult.Reset(ctx, &msgPool);
            ProtoEncoder::FtraceEvent* event = nullptr;
            CHECK_TRUE(HmParseEventData(&traceResult, data, event), false, "hm parse raw data failed!");
            int32_t msgSize = traceResult.Finish();
            resultWriter_->finishReport(resultWriter_, msgSize);
            tansporter_->Report(static_cast<size_t>(msgSize));
        }
    }

    return true;
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

    if (used == bufferSize) {
        PROFILER_LOG_INFO(LOG_CORE,
            "used(%ld) equals bufferSize(%ld), please expand buffer_size_kb, otherwise the kernel may lose data\n",
            used, bufferSize);
    }
    return used;
}

bool FlowController::ParseEventData(int cpuid, uint8_t* page)
{
    if (resultWriter_->isProtobufSerialize) {
        auto traceResult = std::make_unique<TracePluginResult>();
        FtraceEvent* event = nullptr;  // Used to distinguish between SubEventParser instance types.
        CHECK_TRUE(ParseFtraceEvent(traceResult.get(), cpuid, page, event), false, "parse raw event for cpu-%d failed!",
                   cpuid);
        CHECK_TRUE(tansporter_->Submit(std::move(traceResult)), false, "report raw event for cpu-%d failed!", cpuid);
    } else {
        auto ctx = resultWriter_->startReport(resultWriter_);
        CHECK_NOTNULL(ctx, false, "%s: get RandomWriteCtx FAILED!", __func__);
        static ProtoEncoder::MessagePool msgPool;
        static ProtoEncoder::TracePluginResult traceResult;
        msgPool.Reset();
        traceResult.Reset(ctx, &msgPool);
        ProtoEncoder::FtraceEvent* event = nullptr;  // Used to distinguish between SubEventParser instance types.
        CHECK_TRUE(ParseFtraceEvent(&traceResult, cpuid, page, event), false, "parse raw event for cpu-%d failed!",
                   cpuid);
        int32_t msgSize = traceResult.Finish();
        resultWriter_->finishReport(resultWriter_, msgSize);
        tansporter_->Report(static_cast<size_t>(msgSize));
    }
    return true;
}

bool FlowController::ParseEventDataOnNomalMode(int cpuid, long dataSize)
{
    CHECK_NOTNULL(resultWriter_, false, "%s: resultWriter_ nullptr", __func__);
    auto buffer = ftraceBuffers_[cpuid].get();
    auto endPtr = buffer + dataSize;
    for (auto page = buffer; page < endPtr; page += PAGE_SIZE) {
        if (!ParseEventData(cpuid, page)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s:ParseEventData for cpu-%d failed!", __func__, cpuid);
        }
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
            if (!ParseEventData(cpuId, page)) {
                PROFILER_LOG_ERROR(LOG_CORE, "%s:ParseEventData for cpu-%d failed!", __func__, cpuId);
            }
        }
    }
    return true;
}

int FlowController::StopCapture(void)
{
    CHECK_TRUE(ftraceSupported_, -1, "current kernel not support ftrace!");
    CHECK_NOTNULL(tansporter_, -1, "crate ResultTransporter FAILED!");

    CHECK_TRUE(requestEvents_.size() != 0 || traceApps_.size() != 0 || traceCategories_.size() != 0, -1,
               "StopCapture: ftrace event is not set, return false");

    // disable ftrace event switches
    DisableTraceEvents();

    // stop ftrace event data polling thread
    keepRunning_ = false;
    if (pollThread_.joinable()) {
        PROFILER_LOG_INFO(LOG_CORE, "join thread start!\n");
        pollThread_.join();
        PROFILER_LOG_INFO(LOG_CORE, "join thread  done!\n");
    }

    // parse per cpu stats
    if (resultWriter_->isProtobufSerialize) {
        auto traceResult = std::make_unique<TracePluginResult>();
        CHECK_TRUE(ParsePerCpuStatus(traceResult, TRACE_END), -1, "parse TRACE_END stats FAILED!");
        CHECK_TRUE(tansporter_->Submit(std::move(traceResult)), -1, "report TRACE_END stats FAILED!");
    } else {
        auto ctx = resultWriter_->startReport(resultWriter_);
        CHECK_NOTNULL(ctx, -1, "%s: get RandomWriteCtx FAILED!", __func__);
        auto traceResult = std::make_unique<ProtoEncoder::TracePluginResult>(ctx);
        CHECK_TRUE(ParsePerCpuStatus(traceResult, TRACE_END), -1, "parse TRACE_END stats FAILED!");
        int32_t msgSize = traceResult->Finish();
        resultWriter_->finishReport(resultWriter_, msgSize);
        tansporter_->Report(static_cast<size_t>(msgSize));
    }

    // disable userspace trace triggers
    // because trace cmd will read trace buffer,
    // so we to this action after polling thread exit.
    traceCollector_->Recover();
    tansporter_->Flush();

    // release resources
    ftraceReaders_.clear();   // release ftrace data readers
    ftraceBuffers_.clear();   // release ftrace event read buffers
    memPool_.reset();         // release memory pool
    return 0;
}

template <typename T> bool FlowController::ParsePerCpuStatus(T& tracePluginResult, int stage)
{
    CHECK_NOTNULL(tracePluginResult, false, "create TracePluginResult FAILED!");

    auto cpuStatsMsg = tracePluginResult->add_ftrace_cpu_stats();
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
        PROFILER_LOG_INFO(LOG_CORE, "[%d] ParsePerCpuStatus %d!", i, stage);
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

    return true;
}

template <typename T> bool FlowController::ReportClockTimes(T& tracePluginResult)
{
    CHECK_NOTNULL(tracePluginResult, false, "create TracePluginResult FAILED!");

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
        auto clockMsg = tracePluginResult->add_clocks_detail();
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
    return true;
}

template <typename T> bool FlowController::ParseKernelSymbols(T& tracePluginResult)
{
    CHECK_NOTNULL(tracePluginResult, false, "create TracePluginResult FAILED!");

    ksymsParser_->Accept([&tracePluginResult](const KernelSymbol& symbol) {
        auto symbolDetail = tracePluginResult->add_symbols_detail();
        symbolDetail->set_symbol_addr(symbol.addr);
        symbolDetail->set_symbol_name(symbol.name);
    });
    PROFILER_LOG_INFO(LOG_CORE, "parse kernel symbol message done!");
    return true;
}

template <typename T, typename E>
bool FlowController::ParseFtraceEvent(T* tracePluginResult, int cpuid, uint8_t page[], E* ftraceEvent)
{
    CHECK_NOTNULL(tracePluginResult, false, "create TracePluginResult FAILED!");

    auto cpudetail = tracePluginResult->add_ftrace_cpu_detail();
    cpudetail->set_cpu(static_cast<uint32_t>(cpuid));

    CHECK_TRUE(ftraceParser_->ParsePage(*cpudetail, page, PAGE_SIZE, ftraceEvent), false, "parse page failed!");
    return true;
}

bool FlowController::AddPlatformEventsToParser(void)
{
    CHECK_TRUE(ftraceSupported_, false, "current kernel not support ftrace!");

    PROFILER_LOG_INFO(LOG_CORE, "Add platform events to parser start!");
    for (auto& typeName : FtraceFsOps::GetInstance().GetPlatformEvents()) {
        std::string type = typeName.first;
        std::string name = typeName.second;
        if (ftraceParser_->SetupEvent(type, name)) {
            supportedEvents_.push_back(typeName);
        }
    }
    PROFILER_LOG_INFO(LOG_CORE, "Add platform events to parser done, events: %zu!", supportedEvents_.size());
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
    uint32_t maxBufferSizeKb = MAX_BUFFER_SIZE_KB;
    if (FtraceFsOps::GetInstance().IsHmKernel()) {
        maxBufferSizeKb = HM_MAX_BUFFER_SIZE_KB;
    }
    if (sizeKb < MIN_BUFFER_SIZE_KB) {
        bufferSizeKb_ = MIN_BUFFER_SIZE_KB;
    } else if (sizeKb > maxBufferSizeKb) {
        bufferSizeKb_ = maxBufferSizeKb;
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
        std::string fmtType = type;
        if (type == "power_kernel") {
            fmtType = "power";
        }
        if (userEventSet.count(fmtType + "/" + name)) { // user config format
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

void FlowController::SetReportBasicData(bool isReportBasicData)
{
    isReportBasicData_ = isReportBasicData;
}
FTRACE_NS_END
