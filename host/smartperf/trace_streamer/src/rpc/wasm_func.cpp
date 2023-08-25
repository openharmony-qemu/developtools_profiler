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

#include "wasm_func.h"

#ifndef NAME_MAX
#define NAME_MAX 255
#endif
namespace SysTuning {
namespace TraceStreamer {
RpcServer g_wasmTraceStreamer;
extern "C" {
using ReplyFunction = void (*)(const char* data, uint32_t len, int32_t finish);
ReplyFunction g_reply;
uint8_t* g_reqBuf;
uint32_t g_reqBufferSize;

uint8_t* g_parserConfigBuf;
uint32_t g_parserConfigSize;

using SendDataCallBack = void (*)(const char* data, int32_t len, int32_t componentId);
SendDataCallBack g_sendData = nullptr;
uint8_t* g_sendDataBuf;
uint32_t g_sendDataBufSize;

using ExportDBCallback = void (*)(const char* data, uint32_t len, int32_t finish);
ExportDBCallback g_dbCallback;

using ParseELFFunction = void (*)(const char* data, uint32_t len, int32_t finish);
ParseELFFunction g_parseELFCallback;
uint8_t* g_FileNameBuf;
uint32_t g_FileNameSize;

void ResultCallback(const std::string& jsonResult, int32_t finish)
{
    g_reply(jsonResult.data(), jsonResult.size(), finish);
}

void ParseELFCallback(const std::string& SODataResult, int32_t finish)
{
    g_parseELFCallback(SODataResult.data(), SODataResult.size(), finish);
}
EMSCRIPTEN_KEEPALIVE uint8_t* Initialize(ReplyFunction replyFunction, uint32_t reqBufferSize)
{
    g_reply = replyFunction;
    g_reqBuf = new uint8_t[reqBufferSize];
    g_reqBufferSize = reqBufferSize;
    return g_reqBuf;
}

EMSCRIPTEN_KEEPALIVE uint8_t* InitializeParseConfig(uint32_t reqBufferSize)
{
    g_parserConfigBuf = new uint8_t[reqBufferSize];
    g_parserConfigSize = reqBufferSize;
    return g_parserConfigBuf;
}

EMSCRIPTEN_KEEPALIVE int TraceStreamerParserConfigEx(int dataLen)
{
    std::string parserConfig(reinterpret_cast<const char*>(g_parserConfigBuf), dataLen);
    if (g_wasmTraceStreamer.ParserConfig(parserConfig)) {
        return 0;
    }
    return -1;
}

EMSCRIPTEN_KEEPALIVE uint8_t* InitFileName(ParseELFFunction parseELFCallback, uint32_t reqBufferSize)
{
    g_parseELFCallback = parseELFCallback;
    if (reqBufferSize > NAME_MAX) {
        return nullptr;
    }
    g_FileNameBuf = new uint8_t[reqBufferSize];
    if (!g_FileNameBuf) {
        return nullptr;
    }
    g_FileNameSize = reqBufferSize;
    return g_FileNameBuf;
}

EMSCRIPTEN_KEEPALIVE int32_t UpdateTraceTime(int32_t len)
{
    return g_wasmTraceStreamer.UpdateTraceTime(g_reqBuf, len);
}

void ThirdPary_SendDataCallback(const char* pluginData, int32_t len, int32_t componentId)
{
    if (g_sendData) {
        g_sendData(pluginData, len, componentId);
    }
}

EMSCRIPTEN_KEEPALIVE uint8_t* TraceStreamer_Set_ThirdParty_DataDealer(SendDataCallBack sendDataCallBack,
                                                                      uint32_t reqBufferSize)
{
    g_sendData = sendDataCallBack;
    g_sendDataBuf = new uint8_t[reqBufferSize];
    g_sendDataBufSize = reqBufferSize;
    return g_sendDataBuf;
}

int32_t TraceStreamer_Plugin_Out_Filter(const char* pluginData, int32_t len, const std::string& componentName)
{
    std::map<int32_t, std::string>::iterator itor = g_wasmTraceStreamer.g_thirdPartyConfig.begin();
    int32_t componentId = 0;
    for (; itor != g_wasmTraceStreamer.g_thirdPartyConfig.end(); ++itor) {
        if (itor->second == componentName) {
            componentId = itor->first;
            return TraceStreamer_Plugin_Out_SendData(pluginData, len, componentId);
        }
    }
    return -1;
}

// Tell js to call the corresponding third-party parser interface according to the compositeId
int32_t TraceStreamer_Plugin_Out_SendData(const char* pluginData, int32_t len, int32_t componentId)
{
    ThirdPary_SendDataCallback(pluginData, len, componentId);
    return 0;
}

EMSCRIPTEN_KEEPALIVE int32_t TraceStreamer_Init_ThirdParty_Config(int32_t dataLen)
{
    return g_wasmTraceStreamer.TraceStreamer_Init_ThirdParty_Config(g_reqBuf, dataLen);
}

// return 0 while ok, -1 while failed
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerParseData(const uint8_t* data, int32_t dataLen)
{
    if (g_wasmTraceStreamer.ParseData(data, dataLen, nullptr)) {
        return 0;
    }
    return -1;
}
// return 0 while ok, -1 while failed
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerParseDataEx(int32_t dataLen)
{
    if (g_wasmTraceStreamer.ParseData(g_reqBuf, dataLen, nullptr)) {
        return 0;
    }
    return -1;
}
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerDownloadELFEx(int32_t totalLen,
                                                        int32_t fileNameLen,
                                                        int32_t dataLen,
                                                        int32_t finish)
{
    std::string fileName(reinterpret_cast<const char*>(g_FileNameBuf), fileNameLen);
#if IS_WASM
    if (g_wasmTraceStreamer.DownloadELFCallback(fileName, totalLen, g_reqBuf, dataLen, finish, &ParseELFCallback)) {
        return 0;
    }
#endif
    return -1;
}
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerParseDataOver()
{
    if (g_wasmTraceStreamer.ParseDataOver(nullptr, 0, nullptr)) {
        return 0;
    }
    return -1;
}
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerSqlOperate(const uint8_t* sql, int32_t sqlLen)
{
    if (g_wasmTraceStreamer.SqlOperate(sql, sqlLen, nullptr)) {
        return 0;
    }
    return -1;
}
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerSqlOperateEx(int32_t sqlLen)
{
    if (g_wasmTraceStreamer.SqlOperate(g_reqBuf, sqlLen, nullptr)) {
        return 0;
    }
    return -1;
}
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerReset()
{
    g_wasmTraceStreamer.Reset(nullptr, 0, nullptr);
    return 0;
}
// return the length of result, -1 while failed
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerSqlQuery(const uint8_t* sql, int32_t sqlLen, uint8_t* out, int32_t outLen)
{
    return g_wasmTraceStreamer.WasmSqlQuery(sql, sqlLen, out, outLen);
}
// return the length of result, -1 while failed
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerSqlQueryEx(int32_t sqlLen)
{
    return g_wasmTraceStreamer.WasmSqlQueryWithCallback(g_reqBuf, sqlLen, &ResultCallback);
}
EMSCRIPTEN_KEEPALIVE int32_t TraceStreamerCancel()
{
    g_wasmTraceStreamer.CancelSqlQuery();
    return 0;
}

void ExportDatabaseCallback(const std::string& jsonResult, int32_t finish)
{
    g_dbCallback(jsonResult.data(), jsonResult.size(), finish);
}

EMSCRIPTEN_KEEPALIVE int32_t WasmExportDatabase(ExportDBCallback fun)
{
    g_dbCallback = fun;
    return g_wasmTraceStreamer.WasmExportDatabase(&ExportDatabaseCallback);
}
} // extern "C"
} // namespace TraceStreamer
} // namespace SysTuning
