/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 */
#include "perf_data_parser.h"
#include "clock_filter_ex.h"
#include "file.h"
#include "perf_data_filter.h"
#include "stat_filter.h"

namespace SysTuning {
namespace TraceStreamer {
PerfDataParser::PerfDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx),
      configNameIndex_(traceDataCache_->dataDict_.GetStringIndex("config_name")),
      workloaderIndex_(traceDataCache_->dataDict_.GetStringIndex("workload_cmd")),
      cmdlineIndex_(traceDataCache_->dataDict_.GetStringIndex("cmdline")),
      runingStateIndex_(traceDataCache_->dataDict_.GetStringIndex("Running")),
      suspendStatIndex_(traceDataCache_->dataDict_.GetStringIndex("Suspend")),
      unkonwnStateIndex_(traceDataCache_->dataDict_.GetStringIndex("-")),
      frameToCallChainId_(INVALID_UINT32)
{
    SymbolsFile::onRecording_ = false;
}
void PerfDataParser::InitPerfDataAndLoad(const std::deque<uint8_t> dequeBuffer, uint64_t size)
{
    bufferSize_ = size;
    buffer_ = std::make_unique<uint8_t[]>(size);
    std::copy(dequeBuffer.begin(), dequeBuffer.begin() + size, buffer_.get());
    LoadPerfData();
}
PerfDataParser::~PerfDataParser()
{
    (void)remove(tmpPerfData_.c_str());
    TS_LOGI("perf data ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(GetPluginStartTime()),
            static_cast<unsigned long long>(GetPluginEndTime()));
}

bool PerfDataParser::PerfReloadSymbolFiles(std::vector<std::string>& symbolsPaths)
{
    if (access(tmpPerfData_.c_str(), F_OK) != 0) {
        TS_LOGE("perf file:%s not exist", tmpPerfData_.c_str());
        return false;
    }
    recordDataReader_ = PerfFileReader::Instance(tmpPerfData_);
    report_ = std::make_unique<Report>();
    report_->virtualRuntime_.SetSymbolsPaths(symbolsPaths);
    if (recordDataReader_ == nullptr) {
        return false;
    }
    if (Reload()) {
        Finish();
        return true;
    } else {
        return false;
    }
}
bool PerfDataParser::LoadPerfData()
{
    // try load the perf data
    int32_t fd(base::OpenFile(tmpPerfData_, O_CREAT | O_RDWR, TS_PERMISSION_RW));
    if (!fd) {
        fprintf(stdout, "Failed to create file: %s", tmpPerfData_.c_str());
        buffer_.reset();
        return false;
    }
    (void)ftruncate(fd, 0);
    if (bufferSize_ != (size_t)write(fd, buffer_.get(), bufferSize_)) {
        close(fd);
        buffer_.reset();
        return false;
    }
    close(fd);
    recordDataReader_ = PerfFileReader::Instance(tmpPerfData_);
    report_ = std::make_unique<Report>();
    buffer_.reset();
    if (recordDataReader_ == nullptr) {
        return false;
    }
    return Reload();
}
bool PerfDataParser::Reload()
{
    frameToCallChainId_.Clear();
    fileDataDictIdToFileId_.clear();
    tidToPid_.clear();
    streamFilters_->perfDataFilter_->BeforeReload();
    traceDataCache_->GetPerfSampleData()->Clear();
    traceDataCache_->GetPerfThreadData()->Clear();

    if (!recordDataReader_->ReadFeatureSection()) {
        printf("record format error.\n");
        return false;
    }
    // update perf report table
    UpdateEventConfigInfo();
    UpdateReportWorkloadInfo();
    UpdateCmdlineInfo();

    // update perf Files table
    UpdateSymbolAndFilesData();

    TS_LOGD("process record");
    UpdateClockType();
    recordDataReader_->ReadDataSection(std::bind(&PerfDataParser::RecordCallBack, this, std::placeholders::_1));
    TS_LOGD("process record completed");
    TS_LOGI("load perf data done");
    return true;
}

void PerfDataParser::UpdateEventConfigInfo()
{
    auto features = recordDataReader_->GetFeatures();
    cpuOffMode_ = find(features.begin(), features.end(), FEATURE::HIPERF_CPU_OFF) != features.end();
    if (cpuOffMode_) {
        TS_LOGD("this is cpuOffMode ");
    }
    const PerfFileSection* featureSection = recordDataReader_->GetFeatureSection(FEATURE::EVENT_DESC);
    if (featureSection) {
        TS_LOGI("have EVENT_DESC");
        LoadEventDesc();
    } else {
        TS_LOGE("Do not have EVENT_DESC !!!");
    }
}

void PerfDataParser::LoadEventDesc()
{
    const auto featureSection = recordDataReader_->GetFeatureSection(FEATURE::EVENT_DESC);
    const auto& sectionEventdesc = *static_cast<const PerfFileSectionEventDesc*>(featureSection);
    TS_LOGI("Event descriptions: %zu", sectionEventdesc.eventDesces_.size());
    for (size_t i = 0; i < sectionEventdesc.eventDesces_.size(); i++) {
        const auto& fileAttr = sectionEventdesc.eventDesces_[i];
        TS_LOGI("event name[%zu]: %s ids: %s", i, fileAttr.name.c_str(), VectorToString(fileAttr.ids).c_str());
        for (uint64_t id : fileAttr.ids) {
            report_->configIdIndexMaps_[id] = report_->configs_.size(); // setup index
            TS_LOGI("add config id map %" PRIu64 " to %zu", id, report_->configs_.size());
        }
        // when cpuOffMode_ , don't use count mode , use time mode.
        auto& config = report_->configs_.emplace_back(fileAttr.name, fileAttr.attr.type, fileAttr.attr.config,
                                                      cpuOffMode_ ? false : true);
        config.ids_ = fileAttr.ids;
        TS_ASSERT(config.ids_.size() > 0);

        auto perfReportData = traceDataCache_->GetPerfReportData();
        auto configValueIndex = traceDataCache_->dataDict_.GetStringIndex(fileAttr.name.c_str());
        perfReportData->AppendNewPerfReport(configNameIndex_, configValueIndex);
    }
}

void PerfDataParser::UpdateReportWorkloadInfo() const
{
    // workload
    auto featureSection = recordDataReader_->GetFeatureSection(FEATURE::HIPERF_WORKLOAD_CMD);
    std::string workloader = "";
    if (featureSection) {
        TS_LOGI("found HIPERF_META_WORKLOAD_CMD");
        auto sectionString = static_cast<const PerfFileSectionString*>(featureSection);
        workloader = sectionString->toString();
    } else {
        TS_LOGW("NOT found HIPERF_META_WORKLOAD_CMD");
    }
    if (workloader.empty()) {
        TS_LOGW("NOT found HIPERF_META_WORKLOAD_CMD");
        return;
    }
    auto perfReportData = traceDataCache_->GetPerfReportData();
    auto workloaderValueIndex = traceDataCache_->dataDict_.GetStringIndex(workloader.c_str());
    perfReportData->AppendNewPerfReport(workloaderIndex_, workloaderValueIndex);
}

void PerfDataParser::UpdateCmdlineInfo() const
{
    auto cmdline = recordDataReader_->GetFeatureString(FEATURE::CMDLINE);
    auto perfReportData = traceDataCache_->GetPerfReportData();
    auto cmdlineValueIndex = traceDataCache_->dataDict_.GetStringIndex(cmdline.c_str());
    perfReportData->AppendNewPerfReport(cmdlineIndex_, cmdlineValueIndex);
}

void PerfDataParser::UpdateSymbolAndFilesData()
{
    // we need unwind it (for function name match) even not give us path
    report_->virtualRuntime_.SetDisableUnwind(false);

    // found symbols in file
    const auto featureSection = recordDataReader_->GetFeatureSection(FEATURE::HIPERF_FILES_SYMBOL);
    if (featureSection != nullptr) {
        const PerfFileSectionSymbolsFiles* sectionSymbolsFiles =
            static_cast<const PerfFileSectionSymbolsFiles*>(featureSection);
        report_->virtualRuntime_.UpdateFromPerfData(sectionSymbolsFiles->symbolFileStructs_);
    }
    // fileid, symbolIndex, filePathIndex
    uint64_t fileId = 0;
    for (auto& symbolsFile : report_->virtualRuntime_.GetSymbolsFiles()) {
        auto filePathIndex = traceDataCache_->dataDict_.GetStringIndex(symbolsFile->filePath_.c_str());
        uint32_t serial = 0;
        for (auto& symbol : symbolsFile->GetSymbols()) {
            auto symbolIndex = traceDataCache_->dataDict_.GetStringIndex(symbol.Name().data());
            streamFilters_->statFilter_->IncreaseStat(TRACE_PERF, STAT_EVENT_RECEIVED);
            streamFilters_->perfDataFilter_->AppendPerfFiles(fileId, serial++, symbolIndex, filePathIndex);
        }
        fileDataDictIdToFileId_.insert(std::make_pair(filePathIndex, fileId));
        ++fileId;
    }
}
void PerfDataParser::UpdateClockType()
{
    const auto& attrIds_ = recordDataReader_->GetAttrSection();
    if (attrIds_.size() > 0) {
        useClockId_ = attrIds_[0].attr.use_clockid;
        clockId_ = attrIds_[0].attr.clockid;
        TS_LOGE("useClockId_ = %u, clockId_ = %u", useClockId_, clockId_);
    }
}
bool PerfDataParser::RecordCallBack(std::unique_ptr<PerfEventRecord> record)
{
    // tell process tree what happend for rebuild symbols
    report_->virtualRuntime_.UpdateFromRecord(*record);

    if (record->GetType() == PERF_RECORD_SAMPLE) {
        std::unique_ptr<PerfRecordSample> sample(static_cast<PerfRecordSample*>(record.release()));
        auto callChainId = UpdatePerfCallChainData(sample);
        UpdatePerfSampleData(callChainId, sample);
    } else if (record->GetType() == PERF_RECORD_COMM) {
        auto recordComm = static_cast<PerfRecordComm*>(record.get());
        auto range = tidToPid_.equal_range(recordComm->data_.tid);
        for (auto it = range.first; it != range.second; it++) {
            if (it->second == recordComm->data_.pid) {
                return true;
            }
        }
        tidToPid_.insert(std::make_pair(recordComm->data_.tid, recordComm->data_.pid));
        auto perfThreadData = traceDataCache_->GetPerfThreadData();
        auto threadNameIndex = traceDataCache_->dataDict_.GetStringIndex(recordComm->data_.comm);
        perfThreadData->AppendNewPerfThread(recordComm->data_.pid, recordComm->data_.tid, threadNameIndex);
    }
    return true;
}

uint32_t PerfDataParser::UpdatePerfCallChainData(const std::unique_ptr<PerfRecordSample>& sample)
{
    uint64_t depth = 0;
    bool callStackNotExist = false;
    uint32_t callChainId = INVALID_UINT32;
    std::vector<std::unique_ptr<CallStackTemp>> callStackTemp = {};
    // Filter callstack unuse data
    for (auto frame = sample->callFrames_.rbegin(); frame != sample->callFrames_.rend(); ++frame) {
        auto symbolId = frame->symbolIndex_;
        if (symbolId == -1 && frame->vaddrInFile_ == 0) {
            continue;
        }
        auto fileDataIndex = traceDataCache_->dataDict_.GetStringIndex(frame->filePath_);
        auto itor = fileDataDictIdToFileId_.find(fileDataIndex);
        if (itor == fileDataDictIdToFileId_.end()) {
            continue;
        }
        auto fileId = itor->second;
        callStackTemp.emplace_back(std::make_unique<CallStackTemp>(depth, frame->vaddrInFile_, fileId, symbolId));
        depth++;
    }
    // Determine whether to write callstack data to cache
    auto size = callStackTemp.size();
    for (auto itor = callStackTemp.begin(); itor != callStackTemp.end(); itor++) {
        auto callstack = itor->get();
        auto ret = frameToCallChainId_.Find(callstack->fileId_, callstack->symbolId_, callstack->depth_, size);
        if (ret != INVALID_UINT32) { // find it
            if (callChainId == INVALID_UINT32) {
                callChainId = ret;
            } else if (callChainId != ret) {
                callStackNotExist = true;
                break;
            }
        } else { // not find it
            callStackNotExist = true;
            break;
        }
    }
    // write callstack data to cache
    if (callStackNotExist) {
        callChainId = ++callChainId_;
        for (auto itor = callStackTemp.begin(); itor != callStackTemp.end(); itor++) {
            auto callstack = itor->get();
            frameToCallChainId_.Insert(callstack->fileId_, callstack->symbolId_, callstack->depth_,
                                       callStackTemp.size(), callChainId);
            streamFilters_->perfDataFilter_->AppendPerfCallChain(
                callChainId, callstack->depth_, callstack->vaddrInFile_, callstack->fileId_, callstack->symbolId_);
        }
    }
    callStackTemp.clear();
    return callChainId;
}

void PerfDataParser::UpdatePerfSampleData(uint32_t callChainId, std::unique_ptr<PerfRecordSample>& sample)
{
    auto perfSampleData = traceDataCache_->GetPerfSampleData();
    uint64_t newTimeStamp = 0;
    if (useClockId_ == 0) {
        newTimeStamp = sample->data_.time;
    } else {
        newTimeStamp =
            streamFilters_->clockFilter_->ToPrimaryTraceTime(perfToTSClockType_.at(clockId_), sample->data_.time);
    }
    UpdatePluginTimeRange(perfToTSClockType_.at(clockId_), sample->data_.time, newTimeStamp);

    DataIndex threadStatIndex = unkonwnStateIndex_;
    auto threadState = report_->GetConfigName(sample->data_.id);
    if (threadState.compare(wakingEventName_) == 0) {
        threadStatIndex = runingStateIndex_;
    } else if (threadState.compare(cpuOffEventName_) == 0) {
        threadStatIndex = suspendStatIndex_;
    }
    auto configIndex = report_->GetConfigIndex(sample->data_.id);
    perfSampleData->AppendNewPerfSample(callChainId, sample->data_.time, sample->data_.tid, sample->data_.period,
                                        configIndex, newTimeStamp, sample->data_.cpu, threadStatIndex);
}

void PerfDataParser::Finish()
{
    streamFilters_->perfDataFilter_->Finish();
    // Update trace_range when there is only perf data in the trace file
    if (traceDataCache_->traceStartTime_ == INVALID_UINT64 || traceDataCache_->traceEndTime_ == 0) {
        traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
    } else {
        TS_LOGI("perfData time is not updated, maybe this trace file has other data");
    }
    frameToCallChainId_.Clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
