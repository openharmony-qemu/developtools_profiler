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

#include "rpc_server.h"

#include <cstdint>
#include <cstring>
#include <functional>
#if IS_WASM
#include <filesystem>
#endif
#include "common_types.h"
#include "bytrace_parser.h"
#include "htrace_parser.h"
#include "json.hpp"
#include "log.h"
#include "string_help.h"
#include "trace_streamer_selector.h"
#include "version.h"

#define UNUSED(expr)             \
    do {                         \
        static_cast<void>(expr); \
    } while (0)
namespace SysTuning {
namespace TraceStreamer {
uint32_t g_fileLen = 0;
FILE* g_importFileFd = nullptr;
const size_t PACKET_HEADER_LENGTH = 1024;
const std::string VALUE = "{\"value\":[";
const std::string OFFSET = "{\"offset\":";
const std::string SIZE = ",\"size\":";

using json = nlohmann::json;
namespace jsonns {
struct ParserConfig {
    int32_t taskConfigValue;
    int32_t appConfigValue;
    int32_t aniConfigValue;
    int32_t binderConfigValue;
};
void from_json(const json& j, ParserConfig& v)
{
    j.at("TaskPool").get_to(v.taskConfigValue);
    j.at("AppStartup").get_to(v.appConfigValue);
    j.at("AnimationAnalysis").get_to(v.aniConfigValue);
    j.at("BinderRunnable").get_to(v.binderConfigValue);
}
} // namespace jsonns
bool RpcServer::ParseData(const uint8_t* data, size_t len, ResultCallBack resultCallBack)
{
    g_loadSize += len;
    size_t blockSize = 1024 * 1024;
    do {
        size_t parseSize = std::min(len, blockSize);
        std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(parseSize);
        std::copy(data, data + parseSize, buf.get());
        if (!ts_->ParseTraceDataSegment(std::move(buf), parseSize, false, false)) {
            if (resultCallBack) {
                resultCallBack("formaterror\r\n", SEND_FINISH);
            }
            return false;
        }
        data += parseSize;
        len -= parseSize;
        lenParseData_ += parseSize;
    } while (len > 0);
    if (resultCallBack) {
        resultCallBack("ok\r\n", SEND_FINISH);
    }
    return true;
}
bool RpcServer::ParseDataWithoutCallback(const uint8_t* data, size_t len, int32_t isFinish, bool isSplitFile)
{
    g_loadSize += len;
    size_t blockSize = 1024 * 1024;
    do {
        size_t parseSize = std::min(len, blockSize);
        std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(parseSize);
        std::copy(data, data + parseSize, buf.get());
        if (!ts_->ParseTraceDataSegment(std::move(buf), parseSize, isSplitFile, isFinish && (len == parseSize))) {
            return false;
        }
        data += parseSize;
        len -= parseSize;
        lenParseData_ += parseSize;
    } while (len > 0);
    return true;
}

bool RpcServer::GetLongTraceTimeSnap(std::string dataString)
{
    int count = dataString.size() / PACKET_HEADER_LENGTH;
    for (int i = 0; i < count; i++) {
        if (!GetTimeSnap(dataString)) {
            TS_LOGE("GetLongTraceTimeSnap error");
            return false;
        }
        dataString = dataString.substr(PACKET_HEADER_LENGTH);
    }
    return true;
}
bool RpcServer::GetTimeSnap(std::string dataString)
{
    if (dataString.size() < PACKET_HEADER_LENGTH) {
        TS_LOGE("buffer size less than profiler trace file header");
        return false;
    }
    uint8_t buffer[PACKET_HEADER_LENGTH];
    (void)memset_s(buffer, PACKET_HEADER_LENGTH, 0, PACKET_HEADER_LENGTH);
    int32_t i = 0;
    for (auto it = dataString.begin(); it != dataString.begin() + PACKET_HEADER_LENGTH; ++it, ++i) {
        buffer[i] = *it;
    }
    ProfilerTraceFileHeader* pHeader = reinterpret_cast<ProfilerTraceFileHeader*>(buffer);
    if (pHeader->data.length <= PACKET_HEADER_LENGTH || pHeader->data.magic != ProfilerTraceFileHeader::HEADER_MAGIC) {
        TS_LOGE("Profiler Trace data is truncated or invalid magic! len = %" PRIu64 ", maigc = %" PRIx64 "",
                pHeader->data.length, pHeader->data.magic);
        return false;
    }
    std::unique_ptr<TraceTimeSnap> longTraceTimeSnap = std::make_unique<TraceTimeSnap>();
    longTraceTimeSnap->startTime = pHeader->data.boottime;
    longTraceTimeSnap->endTime = pHeader->data.boottime + pHeader->data.durationNs;
    vTraceTimeSnap_.emplace_back(std::move(longTraceTimeSnap));
    return true;
}
bool RpcServer::LongTraceSplitFile(const uint8_t* data,
                                   size_t len,
                                   int32_t isFinish,
                                   uint32_t pageNum,
                                   SplitFileCallBack splitFileCallBack)
{
    if (vTraceTimeSnap_.size() <= pageNum) {
        return false;
    }
    ts_->minTs_ = vTraceTimeSnap_[pageNum]->startTime;
    ts_->maxTs_ = vTraceTimeSnap_[pageNum]->endTime;
    ParseSplitFileData(data, len, isFinish, splitFileCallBack, true);
    return true;
}

bool RpcServer::ParseSplitFileData(const uint8_t* data,
                                   size_t len,
                                   int32_t isFinish,
                                   SplitFileCallBack splitFileCallBack,
                                   bool isSplitFile)
{
    if (!ParseDataWithoutCallback(data, len, isFinish, isSplitFile)) {
        TS_LOGE("ParserData failed!");
        return false;
    }
    if (isSplitFile && ts_->GetFileType() == TRACE_FILETYPE_BY_TRACE) {
        splitFileCallBack(ts_->GetBytraceData()->GetTraceDataBytrace(), (int32_t)SplitDataDataType::SPLIT_FILE_DATA,
                          isFinish);
        ts_->GetBytraceData()->ClearByTraceData();
        return true;
    }
    if (isSplitFile && isFinish && ts_->GetFileType() == TRACE_FILETYPE_H_TRACE) {
        uint64_t dataSize = 0;
        std::string result = VALUE;
        ts_->GetHtraceData()->ClearNativehookData();
        for (const auto& itemHtrace : ts_->GetHtraceData()->GetTraceDataHtrace()) {
            dataSize += itemHtrace.second;
            result += OFFSET + std::to_string(itemHtrace.first);
            result += SIZE + std::to_string(itemHtrace.second);
            result += "},";
        }
        auto dataSourceType = ts_->GetHtraceData()->GetDataSourceType();
        auto profilerHeader = ts_->GetHtraceData()->GetProfilerHeader();
        if (dataSourceType == DATA_SOURCE_TYPE_JSMEMORY) {
            dataSize += ts_->GetHtraceData()->GetArkTsConfigData().size() +
                        ts_->GetHtraceData()->GetJsMemoryData()->GetArkTsSize();
        }
        for (auto& commProto : ts_->GetTraceDataCache()->HookCommProtos()) {
            dataSize += (sizeof(uint32_t) + commProto->size());
        }
        // Send Header
        profilerHeader.data.length = PACKET_HEADER_LENGTH + dataSize;
        std::string buffer(reinterpret_cast<char*>(&profilerHeader), sizeof(profilerHeader));
        splitFileCallBack(buffer, (int32_t)SplitDataDataType::SPLIT_FILE_DATA, 0);
        // Send Datas
        ProcHookCommSplitResult(splitFileCallBack);
        if (result != VALUE && !ts_->GetHtraceData()->GetTraceDataHtrace().empty()) {
            result.pop_back();
            result += "]}\r\n";
            splitFileCallBack(result, (int32_t)SplitDataDataType::SPLIT_FILE_JSON, 0);
        }
        if (dataSourceType == DATA_SOURCE_TYPE_JSMEMORY) {
            splitFileCallBack(ts_->GetHtraceData()->GetArkTsConfigData() +
                                  ts_->GetHtraceData()->GetJsMemoryData()->GetArkTsSplitFileData(),
                              (int32_t)SplitDataDataType::SPLIT_FILE_DATA, 0);
        }
        ProcPerfSplitResult(splitFileCallBack, true);
        ProcEbpfSplitResult(splitFileCallBack, true);
    }
    if (isSplitFile && isFinish && ts_->GetFileType() == TRACE_FILETYPE_PERF) {
        ProcPerfSplitResult(splitFileCallBack, true);
    }
    if (isSplitFile && isFinish) {
        std::string resultEnd = "{\"value\":[]}";
        splitFileCallBack(resultEnd, (int32_t)SplitDataDataType::SPLIT_FILE_JSON, 1);
        ts_->GetHtraceData()->ClearTraceDataHtrace();
        ts_->GetHtraceData()->GetJsMemoryData()->ClearArkTsSplitFileData();
        ts_->GetTraceDataCache()->isSplitFile_ = false;
    }
    return true;
}
void RpcServer::ProcHookCommSplitResult(SplitFileCallBack splitFileCallBack)
{
    std::string lenBuffer(sizeof(uint32_t), 0);
    for (auto& commProto : ts_->GetTraceDataCache()->HookCommProtos()) {
        uint32_t len = commProto->size();
        memcpy_s(const_cast<char*>(lenBuffer.data()), sizeof(uint32_t), &len, sizeof(uint32_t));
        splitFileCallBack(lenBuffer, (int32_t)SplitDataDataType::SPLIT_FILE_DATA, 0);
        splitFileCallBack(*commProto, (int32_t)SplitDataDataType::SPLIT_FILE_DATA, 0);
    }
    ts_->GetTraceDataCache()->ClearHookCommProtos();
}
void RpcServer::ProcEbpfSplitResult(SplitFileCallBack splitFileCallBack, bool isLast)
{
    auto splitResult = ts_->GetHtraceData()->GetEbpfDataParser()->GetEbpfSplitResult();
    std::string result = VALUE;
    for (auto it = splitResult.begin(); it != splitResult.end(); ++it) {
        if (it->type == (int32_t)SplitDataDataType::SPLIT_FILE_JSON) {
            result += OFFSET + std::to_string(it->json.offset);
            result += SIZE + std::to_string(it->json.size);
            result += "},";
        } else {
            if (result != VALUE) {
                result.pop_back();
                result += "]}\r\n";
                splitFileCallBack(result, (int32_t)SplitDataDataType::SPLIT_FILE_JSON, 0);
                result = VALUE;
            }
            std::string buffer(reinterpret_cast<char*>(it->buffer.address), it->buffer.size);
            splitFileCallBack(buffer, (int32_t)SplitDataDataType::SPLIT_FILE_DATA, 0);
        }
    }
    if (result != VALUE) {
        result.pop_back();
        result += "]}\r\n";
        splitFileCallBack(result, (int32_t)SplitDataDataType::SPLIT_FILE_JSON, 0);
    }
}
void RpcServer::ProcPerfSplitResult(SplitFileCallBack splitFileCallBack, bool isLast)
{
    auto splitResult = ts_->GetHtraceData()->GetPerfSplitResult();
    std::string result = VALUE;
    for (auto it = splitResult.begin(); it != splitResult.end(); ++it) {
        if (it->type == (int32_t)SplitDataDataType::SPLIT_FILE_JSON) {
            result += OFFSET + std::to_string(it->json.offset);
            result += SIZE + std::to_string(it->json.size);
            result += "},";
        } else {
            if (result != VALUE) {
                result.pop_back();
                result += "]}\r\n";
                splitFileCallBack(result, (int32_t)SplitDataDataType::SPLIT_FILE_JSON, 0);
                result = VALUE;
            }
            std::string buffer(reinterpret_cast<char*>(it->buffer.address), it->buffer.size);
            splitFileCallBack(buffer, (int32_t)SplitDataDataType::SPLIT_FILE_DATA, 0);
        }
    }

    if (result != VALUE) {
        result.pop_back();
        result += "]}\r\n";
        splitFileCallBack(result, (int32_t)SplitDataDataType::SPLIT_FILE_JSON, 0);
    }
}

int32_t RpcServer::UpdateTraceTime(const uint8_t* data, int32_t len)
{
    std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(len);
    std::copy(data, data + len, buf.get());
    ts_->UpdateTraceRangeTime(buf.get(), len);
    return 0;
}

int32_t RpcServer::TraceStreamer_Init_ThirdParty_Config(const uint8_t* data, int32_t len)
{
    TS_LOGE("TraceStreamer_Init_ThirdParty_Config is comming!");
    std::string thirdPartyConfig = reinterpret_cast<const char*>(data);
    TS_LOGE("thirdPartyConfig = %s", thirdPartyConfig.c_str());
    std::vector<std::string> comPonentStr = SplitStringToVec(thirdPartyConfig, ";");
    const int32_t EVENT_COUNT_PAIR = 2;
    if (comPonentStr.size() % EVENT_COUNT_PAIR != 0) {
        TS_LOGE("thirdPartyConfig is wrong!");
        return -1;
    }
    for (int32_t m = 0; m < comPonentStr.size(); m += EVENT_COUNT_PAIR) {
        int32_t componentId = std::stoi(comPonentStr.at(m));
        std::string componentName = comPonentStr.at(m + 1);
        TS_LOGE("comPonentStr[m] = %d, comPonentStr[m + 1] = %s", componentId, componentName.c_str());
        g_thirdPartyConfig.insert((std::map<int32_t, std::string>::value_type(componentId, componentName)));
    }
    return 0;
}

bool RpcServer::ParseDataOver(const uint8_t* data, size_t len, ResultCallBack resultCallBack)
{
    UNUSED(data);
    UNUSED(len);
    MetaData* metaData = ts_->GetMetaData();
    metaData->SetSourceFileName("input stream mode");
    metaData->SetOutputFileName("wasm mode");
    metaData->SetParserToolVersion(g_traceStreamerVersion);
    metaData->SetParserToolPublishDateTime(g_traceStreamerPublishVersion);
    metaData->SetTraceDataSize(g_loadSize);
    metaData->SetTraceType((ts_->DataType() == TRACE_FILETYPE_H_TRACE) ? "proto-based-trace" : "txt-based-trace");
    TS_LOGI("RPC ParseDataOver, has parsed len %zu", lenParseData_);
    ts_->WaitForParserEnd();
#ifndef USE_VTABLE
    ts_->Clear();
#endif
    if (resultCallBack) {
        resultCallBack("ok\r\n", SEND_FINISH);
    }
    lenParseData_ = 0;
    g_loadSize = 0;
    return true;
}

bool RpcServer::SqlOperate(const uint8_t* data, size_t len, ResultCallBack resultCallBack)
{
    ts_->SetCancel(false);
    std::string sql(reinterpret_cast<const char*>(data), len);
    TS_LOGI("RPC SqlOperate(%s, %zu)", sql.c_str(), len);

    int32_t ret = ts_->OperateDatabase(sql);
    if (resultCallBack) {
        std::string response = "ok\r\n";
        if (ret != 0) {
            response = "dberror\r\n";
        }
        resultCallBack(response, SEND_FINISH);
    }
    return (ret == 0);
}

bool RpcServer::SqlQuery(const uint8_t* data, size_t len, ResultCallBack resultCallBack)
{
    ts_->SetCancel(false);
    std::string sql(reinterpret_cast<const char*>(data), len);
    TS_LOGI("RPC SqlQuery %zu:%s", len, sql.c_str());

    int32_t ret = ts_->SearchDatabase(sql, resultCallBack);
    if (resultCallBack && ret != 0) {
        resultCallBack("dberror\r\n", SEND_FINISH);
    }
    ts_->SetCancel(false);
    return (ret == 0);
}

void RpcServer::CancelSqlQuery()
{
    ts_->SetCancel(true);
}

bool RpcServer::Reset(const uint8_t* data, size_t len, ResultCallBack resultCallBack)
{
    UNUSED(data);
    UNUSED(len);
    TS_LOGI("RPC reset trace_streamer");

    ts_->WaitForParserEnd();
    ts_ = std::make_unique<TraceStreamerSelector>();
    if (resultCallBack) {
        resultCallBack("ok\r\n", SEND_FINISH);
    }
    return true;
}

int32_t RpcServer::WasmSqlQuery(const uint8_t* data, size_t len, uint8_t* out, int32_t outLen)
{
    ts_->SetCancel(false);
    std::string sql(reinterpret_cast<const char*>(data), len);
    TS_LOGI("WASM RPC SqlQuery outlen(%d) sql(%zu:%s)", outLen, len, sql.c_str());

    int32_t ret = ts_->SearchDatabase(sql, out, outLen);
    return ret;
}
bool RpcServer::SqlMetricsQueryWithCallback(const uint8_t* data, size_t len, ResultCallBack callback) const
{
    ts_->SetCancel(false);
    std::string metricsName(reinterpret_cast<const char*>(data), len);
    std::string result = ts_->SearchDatabase(ts_->MetricsSqlQuery(metricsName));
    if (result == "") {
        return false;
    }
    Metrics metricsOperator;
    metricsOperator.ParserJson(metricsName, result);
    for (auto item : metricsOperator.GetMetricsMap()) {
        if (item.second == metricsName) {
            metricsOperator.PrintMetricsResult(item.first, callback);
            return true;
        }
    }
}
int32_t RpcServer::WasmSqlQueryWithCallback(const uint8_t* data, size_t len, ResultCallBack callback) const
{
    ts_->SetCancel(false);
    std::string sql(reinterpret_cast<const char*>(data), len);
    TS_LOGI("WASM RPC SqlQuery sql(%zu:%s)", len, sql.c_str());

    int32_t ret = ts_->SearchDatabase(sql, callback);
    return ret;
}

int32_t RpcServer::WasmExportDatabase(ResultCallBack resultCallBack)
{
    return ts_->ExportDatabase("default.db", resultCallBack);
}

#if IS_WASM
int32_t RpcServer::DownloadELFCallback(const std::string& fileName,
                                       size_t totalLen,
                                       const uint8_t* data,
                                       size_t len,
                                       int32_t finish,
                                       ParseELFFileCallBack parseELFFile)
{
    g_fileLen += len;
    std::string filePath = "";
    TS_LOGI("fileName = %s", fileName.c_str());
    std::string symbolsPath = fileName.substr(0, fileName.find("/"));
    TS_LOGI("symbolsPath = %s", symbolsPath.c_str());
    filePath = fileName.substr(0, fileName.find_last_of("/"));
    if (std::filesystem::exists(filePath)) {
        TS_LOGE("%s exist", filePath.c_str());
    } else {
        if (std::filesystem::create_directories(filePath)) {
            TS_LOGI("create_directories success");
        } else {
            TS_LOGI("create_directories failed!");
        }
    }
    TS_LOGI("filePath = %s", filePath.c_str());
    if (g_fileLen < totalLen) {
        if (g_importFileFd == nullptr) {
            g_importFileFd = fopen(fileName.c_str(), "a+");
            if (g_importFileFd == nullptr) {
                TS_LOGE("wasm file create failed");
                return false;
            }
        }
        int32_t writeLength = fwrite(data, len, 1, g_importFileFd);
        if (!writeLength) {
            fclose(g_importFileFd);
            TS_LOGE("wasm write file failed");
            return false;
        }
        return false;
    }
    g_fileLen = 0;
    if (g_importFileFd == nullptr) {
        g_importFileFd = fopen(fileName.c_str(), "a+");
        if (g_importFileFd == nullptr) {
            TS_LOGE("wasm file create failed");
            return false;
        }
    }
    int32_t writeLength = fwrite(data, len, 1, g_importFileFd);
    (void)fclose(g_importFileFd);
    g_importFileFd = nullptr;
    if (!writeLength) {
        TS_LOGE("wasm write file failed");
        return false;
    }
    TS_LOGI("symbolsPath = %s, fileName = %s", symbolsPath.c_str(), fileName.c_str());
    symbolsPathFiles_.emplace_back(fileName);
    parseELFFile("file send over\r\n", SEND_FINISH);

    if (finish) {
        if (!ts_->ReloadSymbolFiles(symbolsPath, symbolsPathFiles_)) {
            symbolsPathFiles_.clear();
            if (parseELFFile) {
                parseELFFile("formaterror\r\n", SEND_FINISH);
            }
            return false;
        }
        symbolsPathFiles_.clear();
        if (parseELFFile) {
            parseELFFile("ok\r\n", SEND_FINISH);
        }
        std::filesystem::remove_all(symbolsPath);
    }
    return true;
}
#endif

bool RpcServer::SplitFile(std::string timeSnaps)
{
    std::vector<std::string> vTimesnaps = SplitStringToVec(timeSnaps, ";");
    if (vTimesnaps.size() <= 1) {
        return false;
    }
    ts_->minTs_ = std::stoull(vTimesnaps.at(0));
    ts_->maxTs_ = std::stoull(vTimesnaps.at(1));
    return true;
}

bool RpcServer::ParserConfig(std::string parserConfigJson)
{
    json jMessage = json::parse(parserConfigJson);
    jsonns::ParserConfig parserConfig = jMessage.at("config");
    ts_->UpdateAppStartTraceStatus(parserConfig.appConfigValue);
    ts_->UpdateAnimationTraceStatus(parserConfig.aniConfigValue);
    ts_->UpdateTaskPoolTraceStatus(parserConfig.taskConfigValue);
    ts_->UpdateBinderRunnableTraceStatus(parserConfig.binderConfigValue);
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning
