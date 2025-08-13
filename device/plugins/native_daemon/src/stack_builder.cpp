/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "stack_builder.h"

namespace OHOS::Developtools::NativeDaemon {
static thread_local std::vector<std::string> g_fpJsCallStacks;
static thread_local std::vector<u64> g_u64regs;
const std::string JS_CALL_STACK_DEPTH_SEP = ",";   // ',' is js call stack depth separator
const std::string JS_SYMBOL_FILEPATH_SEP = "|";    // '|' is js symbol and filepath separator

void StackBuilder::FillStatsVirtualFrames(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord)
{
    auto rawStack = hookRecord->GetRawStack();
    callFrames.emplace_back(rawStack->stackContext->mallocSize | SIZE_MASK);
}

FpStackBuilder::FpStackBuilder(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime,
                               std::mutex& mtx) : StackBuilder(config, runtime, mtx)
{
    if (hookConfig_->fp_unwind() && hookConfig_->js_stack_report() > 0) {
        g_fpJsCallStacks.reserve(hookConfig_->max_js_stack_depth());
    }
}

void FpStackBuilder::PrepareUnwind(HookRecordPtr hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    if (runtimeInstance_ != nullptr) {
        std::lock_guard<std::mutex> guard(mtx_);
        runtimeInstance_->UpdateThread(rawData->stackContext->pid, rawData->stackContext->tid);
    }
}

void FpStackBuilder::FillIps(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    uint64_t* fpIp = reinterpret_cast<uint64_t *>(rawData->data);
    uint8_t depth = rawData->fpDepth;
    for (uint8_t idx = 0; idx < depth; ++idx) {
        if (fpIp[idx] == 0) {
            break;
        }
        callFrames.emplace_back(StripPac(fpIp[idx], 0));
    }
}

void FpStackBuilder::FillJsFrame(CallFrame& jsCallFrame)
{
    DfxSymbol symbol;
    if (!runtimeInstance_->ArktsGetSymbolCache(jsCallFrame, symbol)) {
        symbol.filePathId_ = runtimeInstance_->FillArkTsFilePath(jsCallFrame.filePath_);
        symbol.symbolName_ = jsCallFrame.symbolName_;
        symbol.module_ = jsCallFrame.filePath_;
        symbol.symbolId_ = runtimeInstance_->GetJsSymbolCacheSize();
        runtimeInstance_->FillSymbolNameId(jsCallFrame, symbol);
        runtimeInstance_->FillFileSet(jsCallFrame, symbol);
        jsCallFrame.needReport_ |= CALL_FRAME_REPORT;
        runtimeInstance_->FillJsSymbolCache(jsCallFrame, symbol);
    }
    jsCallFrame.callFrameId_ = symbol.symbolId_;
    jsCallFrame.symbolNameId_ = symbol.symbolNameId_;
    jsCallFrame.filePathId_ = symbol.filePathId_;
    jsCallFrame.filePath_ = symbol.module_;
    jsCallFrame.symbolName_ = symbol.symbolName_;
}

void FpStackBuilder::FillJsSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    if (!(rawData->stackContext->jsChainId > 0 && rawData->jsStackData)) {
        return;
    }
    if (hookConfig_->statistics_interval() > 0) {
        uint16_t type = hookRecord->GetType();
        if (type == FREE_MSG || type == MUNMAP_MSG || type == MEMORY_UNUSING_MSG) {
            return;
        }
    }
    g_fpJsCallStacks.clear();
    if (g_fpJsCallStacks.capacity() == 0) {
        g_fpJsCallStacks.reserve(hookConfig_->max_js_stack_depth());
    }
    AdvancedSplitString(rawData->jsStackData, JS_CALL_STACK_DEPTH_SEP, g_fpJsCallStacks);
    std::lock_guard<std::mutex> guard(mtx_);
    for (std::string& jsCallStack: g_fpJsCallStacks) {
        std::string::size_type jsSymbolFilePathSepPos = jsCallStack.find_first_of(JS_SYMBOL_FILEPATH_SEP);
        if (jsSymbolFilePathSepPos == std::string::npos) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: jsCallStack find FAILED!", __func__);
            continue;
        }
        std::string::size_type jsFilePathPos = jsSymbolFilePathSepPos + 1;
        jsCallStack[jsSymbolFilePathSepPos] = '\0'; // "ts_malloc1'\0'entry/src/main/ets/pages/Index.ets:5:5"
        CallFrame& jsCallFrame = callFrames.emplace_back(0, 0, true);
        jsCallFrame.symbolName_ = StringViewMemoryHold::GetInstance().HoldStringView(jsCallStack.c_str());
        jsCallFrame.filePath_ = StringViewMemoryHold::GetInstance().HoldStringView(jsCallStack.c_str() + jsFilePathPos);
        if (hookConfig_->offline_symbolization()) {
            FillJsFrame(jsCallFrame);
        } else {
            runtimeInstance_->GetSymbolName(rawData->stackContext->pid, rawData->stackContext->tid,
                                            callFrames, 0, true, SymbolType::JS_SYMBOL);
        }
    }
}

void FpStackBuilder::FillNativeSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    std::lock_guard<std::mutex> guard(mtx_);
    runtimeInstance_->GetSymbolName(rawData->stackContext->pid, rawData->stackContext->tid,
                                    callFrames, 0, true, SymbolType::NATIVE_SYMBOL);
}

bool FpStackBuilder::IsRecordInfoValid(HookRecordPtr hookRecord)
{
    return hookRecord->IsValid();
}

DwarfStackBuilder::DwarfStackBuilder(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime,
                                     std::mutex& mtx) : StackBuilder(config, runtime, mtx)
{
    minStackDepth_ = std::min(hookConfig_->max_stack_depth(), MIN_STACK_DEPTH);
    if (hookConfig_->blocked()) {
        minStackDepth_ = static_cast<size_t>(hookConfig_->max_stack_depth());
    }
    minStackDepth_ += FILTER_STACK_DEPTH;
    stackDepth_ = (static_cast<size_t>(hookConfig_->max_stack_depth()) > MAX_CALL_FRAME_UNWIND_SIZE)
            ? MAX_CALL_FRAME_UNWIND_SIZE
            : static_cast<size_t>(hookConfig_->max_stack_depth()) + FILTER_STACK_DEPTH;
}

void DwarfStackBuilder::FillIps(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
#if defined(__arm__)
    if (g_u64regs.size() == 0) {
        g_u64regs.resize(PERF_REG_ARM_MAX);
    }
    uint32_t *regAddrArm = reinterpret_cast<uint32_t *>(rawData->data);
    g_u64regs.assign(regAddrArm, regAddrArm + PERF_REG_ARM_MAX);
#else
    if (g_u64regs.size() == 0) {
        g_u64regs.resize(PERF_REG_ARM64_MAX);
    }
    if (memcpy_s(g_u64regs.data(), sizeof(uint64_t) * PERF_REG_ARM64_MAX, rawData->data,
        sizeof(uint64_t) * PERF_REG_ARM64_MAX) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s regs failed");
        return;
    }
#endif
    const size_t unwindDepth = rawData->reduceStackFlag ? minStackDepth_ : stackDepth_;
    std::lock_guard<std::mutex> guard(mtx_);
    runtimeInstance_->UnwindStack(g_u64regs, rawData->stackData, rawData->stackSize,
                                  rawData->stackContext->pid, rawData->stackContext->tid, callFrames, unwindDepth);
}

void DwarfStackBuilder::FillJsSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord)
{
    if (callFrames.size() <= FILTER_STACK_DEPTH) {
        return;
    }
    auto rawData = hookRecord->GetRawStack();
    std::lock_guard<std::mutex> guard(mtx_);
    runtimeInstance_->GetSymbolName(rawData->stackContext->pid, rawData->stackContext->tid, callFrames,
                                    FILTER_STACK_DEPTH, true, SymbolType::JS_SYMBOL);
}

void DwarfStackBuilder::FillNativeSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    if (callFrames.size() <= FILTER_STACK_DEPTH) {
        return;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    runtimeInstance_->GetSymbolName(rawData->stackContext->pid, rawData->stackContext->tid, callFrames,
                                    FILTER_STACK_DEPTH, true, SymbolType::NATIVE_SYMBOL);
}

bool DwarfStackBuilder::IsRecordInfoValid(HookRecordPtr hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    return ((hookRecord->IsValid()) && (rawData->stackSize != 0));
}

void DwarfStackBuilder::ReplaceErrStack(std::vector<CallFrame>& callFrames)
{
    CallFrame& jsCallFrame = callFrames.emplace_back(0);
    jsCallFrame.symbolName_ = "UnwindErrorDwarf";
    jsCallFrame.isJsFrame_ = true;
    jsCallFrame.needReport_ |= (CALL_FRAME_REPORT | SYMBOL_NAME_ID_REPORT | FILE_PATH_ID_REPORT);
    jsCallFrame.callFrameId_ = DWARF_ERROR_ID;
    jsCallFrame.symbolNameId_ = DWARF_ERROR_ID;
    jsCallFrame.filePathId_ = DWARF_ERROR_ID;
    jsCallFrame.filePath_ = "no-file-path";
}

BuildStackDirector::BuildStackDirector(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime,
                                       std::mutex& mtx) : hookConfig_(config)
{
    if (hookConfig_->fp_unwind()) {
        builder_ = std::make_shared<FpStackBuilder>(hookConfig_, runtime, mtx);
    } else {
        builder_ = std::make_shared<DwarfStackBuilder>(hookConfig_, runtime, mtx);
    }
}

std::vector<CallFrame>& BuildStackDirector::ConstructCallFrames(HookRecordPtr hookRecord)
{
    callFrames_.clear();
    if (builder_ == nullptr || hookConfig_ == nullptr) {
        return callFrames_;
    }
    if (callFrames_.capacity() == 0) {
        callFrames_.reserve(hookConfig_->max_stack_depth() + hookConfig_->max_js_stack_depth());
    }
    if (!IsRecordUnwindable(hookRecord)) {
        return callFrames_;
    }
    if (!builder_->IsRecordInfoValid(hookRecord)) {
        builder_->ReplaceErrStack(callFrames_);
        return callFrames_;
    }
    builder_->PrepareUnwind(hookRecord);
    builder_->FillIps(callFrames_, hookRecord);
    if (hookConfig_->js_stack_report()) {
        builder_->FillJsSymbols(callFrames_, hookRecord);
    }
    if (!hookConfig_->offline_symbolization()) {
        builder_->FillNativeSymbols(callFrames_, hookRecord);
    }
    return callFrames_;
}

bool BuildStackDirector::IsRecordUnwindable(HookRecordPtr hookRecord)
{
    return (hookRecord->GetType() != PR_SET_VMA_MSG);
}
}