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
      pidAndStackHashToCallChainId_(INVALID_UINT32)
{
    SymbolsFile::onRecording_ = false;
}
uint64_t PerfDataParser::InitPerfDataAndLoad(const std::deque<uint8_t> dequeBuffer,
                                             uint64_t size,
                                             uint64_t offset,
                                             bool isSplitFile,
                                             bool isFinish)
{
    if (isSplitFile) {
        return SplitPerfData(dequeBuffer, size, offset, isFinish);
    }

    bufferSize_ = size;
    buffer_ = std::make_unique<uint8_t[]>(size);
    std::copy(dequeBuffer.begin(), dequeBuffer.begin() + size, buffer_.get());
    LoadPerfData();
    buffer_.reset();
    return size;
}

uint64_t PerfDataParser::SplitPerfData(const std::deque<uint8_t>& dequeBuffer,
                                       uint64_t size,
                                       uint64_t offset,
                                       bool isFinish)
{
    if (processedLength_ == 0) {
        perfDataOffset_ = offset;
        perfSplitError_ = false;
    }

    if (perfSplitError_) {
        return size;
    }

    using PerfSplitFunc = bool (PerfDataParser::*)(const std::deque<uint8_t>&, uint64_t, uint64_t&, bool&);
    std::vector<PerfSplitFunc> splitFunc = {&PerfDataParser::SplitPerfStarting,
                                            &PerfDataParser::SplitPerfParsingHead,
                                            &PerfDataParser::SplitPerfWaitForAttr,
                                            &PerfDataParser::SplitPerfParsingAttr,
                                            &PerfDataParser::SplitPerfWaitForData,
                                            &PerfDataParser::SplitPerfParsingData,
                                            &PerfDataParser::SplitPerfParsingFeatureSection,
                                            &PerfDataParser::SplitPerfWaitForFinish};

    if (static_cast<size_t>(splitState_) >= splitFunc.size()) {
        TS_LOGE("Invalid split state %d", splitState_);
        perfSplitError_ = true;
        SplitDataWithdraw();
        return size;
    }

    uint64_t processedLen = 0;
    bool ret = true;
    bool invalid = false;
    while (ret) {
        if (isFinish && splitState_ == SplitPerfState::WAIT_FOR_FINISH) {
            uint64_t currentDataOffset = perfDataOffset_ + processedLength_ + processedLen;
            HtraceSplitResult offsetData = {.type = (int32_t)SplitDataDataType::SPLIT_FILE_JSON,
                                            .json = {.offset = currentDataOffset, .size = size - processedLen}};
            splitResult_.emplace_back(offsetData);
            processedLength_ += size;
            return size;
        }

        ret = (this->*splitFunc[static_cast<int32_t>(splitState_)])(dequeBuffer, size, processedLen, invalid);
        if (invalid) {
            perfSplitError_ = true;
            SplitDataWithdraw();
            return size;
        }
    }

    if (isFinish) {
        TS_LOGE("split not finish but data end");
        perfSplitError_ = true;
        SplitDataWithdraw();
        return size;
    }

    processedLength_ += processedLen;
    return processedLen;
}

bool PerfDataParser::SplitPerfStarting(const std::deque<uint8_t>& dequeBuffer,
                                       uint64_t size,
                                       uint64_t& processedLen,
                                       bool& invalid)
{
    if (hasProfilerHead_) {
        HtraceSplitResult htraceHead = {.type = (int32_t)SplitDataDataType::SPLIT_FILE_DATA,
                                        .buffer = {.address = reinterpret_cast<uint8_t*>(&profilerHeader_),
                                                   .size = sizeof(ProfilerTraceFileHeader)}};
        splitResult_.emplace_back(htraceHead);
    }

    splitState_ = SplitPerfState::PARSING_HEAD;
    return true;
}

bool PerfDataParser::SplitPerfParsingHead(const std::deque<uint8_t>& dequeBuffer,
                                          uint64_t size,
                                          uint64_t& processedLen,
                                          bool& invalid)
{
    processedLen = 0;
    if (size < sizeof(perf_file_header)) {
        return false;
    }

    uint64_t lengthRemain = size;
    std::copy_n(dequeBuffer.begin(), sizeof(perf_file_header), reinterpret_cast<char*>(&perfHeader_));

    if (memcmp(perfHeader_.magic, PERF_MAGIC, sizeof(perfHeader_.magic))) {
        TS_LOGE("invalid magic id");
        invalid = true;
        return false;
    }

    const int FETURE_MAX = 256;
    const int SIZE_FETURE_COUNT = 8;
    featureCount_ = 0;
    for (auto i = 0; i < FETURE_MAX / SIZE_FETURE_COUNT; i++) {
        std::bitset<SIZE_FETURE_COUNT> features(perfHeader_.features[i]);
        for (auto j = 0; j < SIZE_FETURE_COUNT; j++) {
            if (features.test(j)) {
                featureCount_++;
            }
        }
    }

    HtraceSplitResult perfHead = {
        .type = (int32_t)SplitDataDataType::SPLIT_FILE_DATA,
        .buffer = {.address = reinterpret_cast<uint8_t*>(&perfHeader_), .size = sizeof(perf_file_header)}};
    splitResult_.emplace_back(perfHead);

    lengthRemain -= sizeof(perf_file_header);
    processedLen += sizeof(perf_file_header);
    splitState_ = SplitPerfState::WAIT_FOR_ATTR;
    return true;
}

bool PerfDataParser::SplitPerfWaitForAttr(const std::deque<uint8_t>& dequeBuffer,
                                          uint64_t size,
                                          uint64_t& processedLen,
                                          bool& invalid)
{
    if (processedLength_ + processedLen > perfHeader_.attrs.offset) {
        TS_LOGE("offset of attr is wrong %" PRIu64 "", perfHeader_.attrs.offset);
        invalid = true;
        return false;
    }

    if (processedLength_ + size < perfHeader_.attrs.offset) {
        processedLen = size;
        return false;
    }

    processedLen += perfHeader_.attrs.offset - (processedLength_ + processedLen);
    splitState_ = SplitPerfState::PARSING_ATTR;
    return true;
}

bool PerfDataParser::SplitPerfParsingAttr(const std::deque<uint8_t>& dequeBuffer,
                                          uint64_t size,
                                          uint64_t& processedLen,
                                          bool& invalid)
{
    int attrCount = perfHeader_.attrs.size / perfHeader_.attrSize;
    if (attrCount == 0) {
        TS_LOGE("no attr in file");
        invalid = true;
        return false;
    }

    uint64_t LengthRemain = size - processedLen;
    if (LengthRemain < perfHeader_.attrs.size) {
        return false;
    }

    auto buffer = std::make_unique<uint8_t[]>(perfHeader_.attrs.size);
    std::copy_n(dequeBuffer.begin() + processedLen, perfHeader_.attrs.size, buffer.get());
    std::vector<perf_file_attr> vecAttr;
    for (int i = 0; i < attrCount; ++i) {
        perf_file_attr* attr = reinterpret_cast<perf_file_attr*>(buffer.get() + perfHeader_.attrSize * i);
        vecAttr.push_back(*attr);
    }

    sampleType_ = vecAttr[0].attr.sample_type;
    if (!(sampleType_ & PERF_SAMPLE_TIME)) {
        TS_LOGE("no time in sample data, not support split, sampleType_ = %" PRIx64 "", sampleType_);
        invalid = true;
        return false;
    }
    sampleTimeOffset_ = (((sampleType_ & PERF_SAMPLE_IDENTIFIER) != 0) + ((sampleType_ & PERF_SAMPLE_IP) != 0) +
                         ((sampleType_ & PERF_SAMPLE_TID) != 0)) *
                        sizeof(uint64_t);

    processedLen += perfHeader_.attrs.size;
    splitState_ = SplitPerfState::WAIT_FOR_DATA;
    return true;
}

bool PerfDataParser::SplitPerfWaitForData(const std::deque<uint8_t>& dequeBuffer,
                                          uint64_t size,
                                          uint64_t& processedLen,
                                          bool& invalid)
{
    if (processedLength_ + processedLen > perfHeader_.data.offset) {
        TS_LOGE("offset of data is wrong %" PRIu64 "", perfHeader_.data.offset);
        invalid = true;
        return false;
    }

    if (processedLength_ + size < perfHeader_.data.offset) {
        processedLen = size;
        return false;
    }

    HtraceSplitResult offsetData = {.type = (int32_t)SplitDataDataType::SPLIT_FILE_JSON,
                                    .json = {.offset = perfDataOffset_ + sizeof(perf_file_header),
                                             .size = perfHeader_.data.offset - sizeof(perf_file_header)}};
    splitResult_.emplace_back(offsetData);

    processedLen += perfHeader_.data.offset - (processedLength_ + processedLen);
    splitState_ = SplitPerfState::PARSING_DATA;
    return true;
}

bool PerfDataParser::SplitPerfParsingData(const std::deque<uint8_t>& dequeBuffer,
                                          uint64_t size,
                                          uint64_t& processedLen,
                                          bool& invalid)
{
    uint64_t totalDataRemain = perfHeader_.data.offset + perfHeader_.data.size - processedLength_ - processedLen;
    if (totalDataRemain < sizeof(perf_event_header)) {
        processedLen += totalDataRemain;
        splitDataSize_ += totalDataRemain;
        splitState_ = SplitPerfState::PARSING_FEATURE_SECTION;
        return true;
    }

    uint64_t LengthRemain = size - processedLen;
    if (LengthRemain < sizeof(perf_event_header)) {
        return false;
    }

    perf_event_header dataHeader;
    std::copy_n(dequeBuffer.begin() + processedLen, sizeof(perf_event_header), reinterpret_cast<char*>(&dataHeader));
    if (dataHeader.size < sizeof(perf_event_header)) {
        TS_LOGE("invalid data size %u", dataHeader.size);
        invalid = true;
        return false;
    }
    if (LengthRemain < dataHeader.size) {
        return false;
    }
    if (totalDataRemain < sizeof(perf_event_header)) {
        processedLen += totalDataRemain;
        splitDataSize_ += totalDataRemain;
        splitState_ = SplitPerfState::PARSING_FEATURE_SECTION;
        return true;
    }

    bool needRecord = true;
    if (splitDataEnd_) {
        needRecord = false;
    } else if (dataHeader.type == PERF_RECORD_SAMPLE) {
        auto buffer = std::make_unique<uint8_t[]>(dataHeader.size);
        std::copy_n(dequeBuffer.begin() + processedLen + sizeof(perf_event_header),
                    dataHeader.size - sizeof(perf_event_header), buffer.get());
        uint64_t time = *(reinterpret_cast<uint64_t*>(buffer.get() + sampleTimeOffset_));
        if (time < traceDataCache_->SplitFileMinTime()) {
            needRecord = false;
        } else if (time > traceDataCache_->SplitFileMaxTime()) {
            splitDataEnd_ = true;
            needRecord = false;
        }
    }

    if (needRecord) {
        uint64_t currentDataOffset = perfDataOffset_ + processedLength_ + processedLen;
        auto it = splitResult_.rbegin();
        if (it != splitResult_.rend() && (it->json.offset + it->json.size == currentDataOffset)) {
            it->json.size += dataHeader.size;
        } else {
            HtraceSplitResult offsetData = {.type = (int32_t)SplitDataDataType::SPLIT_FILE_JSON,
                                            .json = {.offset = currentDataOffset, .size = dataHeader.size}};
            splitResult_.emplace_back(offsetData);
        }
        splitDataSize_ += dataHeader.size;
    }

    processedLen += dataHeader.size;
    return true;
}

bool PerfDataParser::SplitPerfParsingFeatureSection(const std::deque<uint8_t>& dequeBuffer,
                                                    uint64_t size,
                                                    uint64_t& processedLen,
                                                    bool& invalid)
{
    featureSectioSize_ = featureCount_ * sizeof(perf_file_section);
    if (featureSectioSize_ == 0) {
        TS_LOGI("no feature section in file");
        splitState_ = SplitPerfState::WAIT_FOR_FINISH;
        return false;
    }

    uint64_t LengthRemain = size - processedLen;
    if (LengthRemain < featureSectioSize_) {
        return false;
    }

    featureSection_ = std::make_unique<uint8_t[]>(featureSectioSize_);
    std::copy_n(dequeBuffer.begin() + processedLen, featureSectioSize_, featureSection_.get());
    uint64_t splitDropSize = perfHeader_.data.size - splitDataSize_;
    for (auto i = 0; i < featureCount_; ++i) {
        perf_file_section* featureSections = reinterpret_cast<perf_file_section*>(featureSection_.get());
        featureSections[i].offset -= splitDropSize;
    }
    HtraceSplitResult featureBuff = {.type = (int32_t)SplitDataDataType::SPLIT_FILE_DATA,
                                     .buffer = {.address = featureSection_.get(), .size = featureSectioSize_}};
    splitResult_.emplace_back(featureBuff);

    processedLen += featureSectioSize_;
    perfHeader_.data.size = splitDataSize_;
    profilerHeader_.data.length -= splitDropSize;
    splitState_ = SplitPerfState::WAIT_FOR_FINISH;
    return true;
}

bool PerfDataParser::SplitPerfWaitForFinish(const std::deque<uint8_t>& dequeBuffer,
                                            uint64_t size,
                                            uint64_t& processedLen,
                                            bool& invalid)
{
    return false;
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
        return false;
    }
    (void)ftruncate(fd, 0);
    if (bufferSize_ != (size_t)write(fd, buffer_.get(), bufferSize_)) {
        close(fd);
        return false;
    }
    close(fd);
    recordDataReader_ = PerfFileReader::Instance(tmpPerfData_);
    if (recordDataReader_ == nullptr) {
        return false;
    }
    report_ = std::make_unique<Report>();
    return Reload();
}
bool PerfDataParser::Reload()
{
    pidAndStackHashToCallChainId_.Clear();
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
    ProcessUniStackTableData();
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

void PerfDataParser::ProcessUniStackTableData()
{
    auto featureSection = recordDataReader_->GetFeatureSection(FEATURE::HIPERF_FILES_UNISTACK_TABLE);
    if (featureSection != nullptr) {
        PerfFileSectionUniStackTable* sectioniStackTable =
            static_cast<PerfFileSectionUniStackTable*>(const_cast<PerfFileSection*>(featureSection));
        report_->virtualRuntime_.ImportUniqueStackNodes(sectioniStackTable->uniStackTableInfos_);
        report_->virtualRuntime_.SetDedupStack();
        stackCompressedMode_ = true;
    }
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
        if (symbolsFile->GetSymbols().size() == 0) {
            streamFilters_->perfDataFilter_->AppendPerfFiles(fileId, INVALID_UINT32, INVALID_DATAINDEX, filePathIndex);
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
        uint32_t callChainId = INVALID_UINT32;
        if (stackCompressedMode_) {
            callChainId = UpdateCallChainCompressed(sample);
        } else {
            callChainId = UpdateCallChainUnCompressed(sample);
        }
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

uint32_t PerfDataParser::UpdateCallChainCompressed(const std::unique_ptr<PerfRecordSample>& sample)
{
    auto callChainId = static_cast<uint32_t>(sample->StackId_.section.id);
    if (callChainId == 0) {
        callChainId = --compressFailedCallChainId_;
    }
    if (savedCompressedCallChainId_.count(callChainId) != 0) {
        return callChainId;
    }
    savedCompressedCallChainId_.insert(callChainId);
    uint32_t depth = 0;
    for (auto itor = sample->callFrames_.rbegin(); itor != sample->callFrames_.rend(); ++itor) {
        auto fileDataIndex = traceDataCache_->dataDict_.GetStringIndex(itor->filePath_);
        auto fileId = INVALID_UINT64;
        if (fileDataDictIdToFileId_.count(fileDataIndex) != 0) {
            fileId = fileDataDictIdToFileId_.at(fileDataIndex);
        }
        streamFilters_->perfDataFilter_->AppendPerfCallChain(callChainId, depth++, itor->ip_, itor->vaddrInFile_,
                                                             fileId, itor->symbolIndex_);
    }

    return callChainId;
}
uint32_t PerfDataParser::UpdateCallChainUnCompressed(const std::unique_ptr<PerfRecordSample>& sample)
{
    std::string stackStr = "";
    for (auto& callFrame : sample->callFrames_) {
        stackStr += "+" + base::number(callFrame.ip_, base::INTEGER_RADIX_TYPE_HEX);
    }
    auto stackHash = hashFun_(stackStr);
    auto pid = sample->data_.pid;
    auto callChainId = pidAndStackHashToCallChainId_.Find(pid, stackHash);
    if (callChainId != INVALID_UINT32) {
        return callChainId;
    }
    callChainId = ++callChainId_;
    pidAndStackHashToCallChainId_.Insert(pid, stackHash, callChainId);
    uint64_t depth = 0;
    for (auto frame = sample->callFrames_.rbegin(); frame != sample->callFrames_.rend(); ++frame) {
        uint64_t fileId = INVALID_UINT64;
        auto fileDataIndex = traceDataCache_->dataDict_.GetStringIndex(frame->filePath_);
        if (fileDataDictIdToFileId_.count(fileDataIndex) != 0) {
            fileId = fileDataDictIdToFileId_.at(fileDataIndex);
        }
        streamFilters_->perfDataFilter_->AppendPerfCallChain(callChainId, depth++, frame->ip_, frame->vaddrInFile_,
                                                             fileId, frame->symbolIndex_);
    }
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
    if (!traceDataCache_->isSplitFile_) {
        streamFilters_->perfDataFilter_->Finish();
    }
    // Update trace_range when there is only perf data in the trace file
    if (traceDataCache_->traceStartTime_ == INVALID_UINT64 || traceDataCache_->traceEndTime_ == 0) {
        traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
    } else {
        TS_LOGI("perfData time is not updated, maybe this trace file has other data");
    }
    pidAndStackHashToCallChainId_.Clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
