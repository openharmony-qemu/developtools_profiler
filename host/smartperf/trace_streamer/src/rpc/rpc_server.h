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

#ifndef RPC_RPC_H
#define RPC_RPC_H

#include <functional>
#include <mutex>
#include "trace_streamer_selector.h"
namespace SysTuning {
namespace TraceStreamer {
class RpcServer {
public:
    using ResultCallBack = std::function<void(const std::string /* result */, int32_t)>;
    using ParseELFFileCallBack = std::function<void(const std::string, int32_t)>;
    using SendDataCallBack = std::function<void(const char*, int32_t, int32_t)>;
    using SplitFileCallBack = std::function<void(const std::string /* result */, int32_t, int32_t)>;
    // In order to bind HTTP, maintain a unified interface, even if some parameters are useless
    bool ParseData(const uint8_t* data, size_t len, ResultCallBack resultCallBack);
    bool ParseSplitFileData(const uint8_t* data,
                            size_t len,
                            int32_t isFinish,
                            SplitFileCallBack splitFileCallBack,
                            bool isSplitFile);
    bool ParserFileTimeSnap(const uint8_t* data, size_t len, ResultCallBack resultCallBack);
    bool ParseDataOver(const uint8_t* data, size_t len, ResultCallBack resultCallBack);
    bool SqlOperate(const uint8_t* data, size_t len, ResultCallBack resultCallBack);
    bool SqlQuery(const uint8_t* data, size_t len, ResultCallBack resultCallBack);
    bool Reset(const uint8_t* data, size_t len, ResultCallBack resultCallBack);
    void CancelSqlQuery();
    bool ParseDataWithoutCallback(const uint8_t* data, size_t len, int32_t isFinish, bool isSplitFile);

    // only for wasm, no callback
    int32_t WasmSqlQuery(const uint8_t* data, size_t len, uint8_t* out, int32_t outLen);
    bool SqlMetricsQueryWithCallback(const uint8_t* data, size_t len, ResultCallBack callback) const;
    int32_t WasmSqlQueryWithCallback(const uint8_t* data, size_t len, ResultCallBack callback) const;
    int32_t UpdateTraceTime(const uint8_t* data, int32_t len);
    int32_t TraceStreamer_Init_ThirdParty_Config(const uint8_t* data, int32_t len);
    int32_t WasmExportDatabase(ResultCallBack resultCallBack);
    bool ParserConfig(std::string parserConfigJson);
    bool SplitFile(std::string timeSnaps);
    void ProcHookCommSplitResult(SplitFileCallBack splitFileCallBack);
    void ProcEbpfSplitResult(SplitFileCallBack splitFileCallBack, bool isLast);
    bool GetLongTraceTimeSnap(std::string dataString);
    bool LongTraceSplitFile(const uint8_t* data,
                            size_t len,
                            int32_t isFinish,
                            uint32_t pageNum,
                            SplitFileCallBack splitFileCallBack);
    bool GetTimeSnap(std::string dataString);
#ifdef IS_WASM
    int32_t DownloadELFCallback(const std::string& fileName,
                                size_t totalLen,
                                const uint8_t* data,
                                size_t len,
                                int32_t finish,
                                ParseELFFileCallBack parseELFFile);
#endif
    std::map<int32_t, std::string> g_thirdPartyConfig;

private:
    void ProcPerfSplitResult(SplitFileCallBack splitFileCallBack, bool isLast);

    std::unique_ptr<TraceStreamerSelector> ts_ = std::make_unique<TraceStreamerSelector>();
    size_t lenParseData_ = 0;
    std::vector<std::string> symbolsPathFiles_;
    std::vector<std::unique_ptr<TraceTimeSnap>> vTraceTimeSnap_;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // RPC_RPC_H
