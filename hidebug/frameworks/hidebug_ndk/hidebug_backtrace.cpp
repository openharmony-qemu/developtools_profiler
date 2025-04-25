/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "hidebug/hidebug.h"

#include "dfx_map.h"
#include "fp_backtrace.h"

int OH_HiDebug_BacktraceFromFp(HiDebug_Backtrace_Object object, void* startFp, void** pcArray, int size)
{
    if (object == nullptr || startFp == nullptr || pcArray == nullptr || size <= 0) {
        return 0;
    }
    const auto fpBacktrace = reinterpret_cast<OHOS::HiviewDFX::FpBacktrace*>(object);
    return static_cast<int>(fpBacktrace->BacktraceFromFp(startFp, pcArray, static_cast<uint32_t>(size)));
}

HiDebug_ErrorCode OH_HiDebug_SymbolicAddress(HiDebug_Backtrace_Object object, void* pc, void* arg,
    OH_HiDebug_SymbolicAddressCallback callback)
{
    if (object == nullptr || callback == nullptr || pc == nullptr) {
        return HIDEBUG_INVALID_ARGUMENT;
    }
    auto fpBacktrace = reinterpret_cast<OHOS::HiviewDFX::FpBacktrace*>(object);
    const auto* frame = fpBacktrace->SymbolicAddress(pc);
    if (frame == nullptr) {
        return HIDEBUG_INVALID_SYMBOLIC_PC_ADDRESS;
    }
    HiDebug_StackFrame stackFrame;
    if (frame->isJsFrame) {
        stackFrame.type = HiDebug_StackFrameType::HIDEBUG_STACK_FRAME_TYPE_JS;
        stackFrame.frame.js.column = frame->column;
        stackFrame.frame.js.functionName = frame->funcName.c_str();
        stackFrame.frame.js.mapName = frame->map->name.c_str();
        stackFrame.frame.js.packageName = frame->packageName.c_str();
        stackFrame.frame.js.url = frame->mapName.c_str();
        stackFrame.frame.js.relativePc = frame->relPc;
        stackFrame.frame.js.line = frame->line;
    } else {
        stackFrame.type = HiDebug_StackFrameType::HIDEBUG_STACK_FRAME_TYPE_NATIVE;
        stackFrame.frame.native.buildId = frame->buildId.c_str();
        stackFrame.frame.native.funcOffset = frame->funcOffset;
        stackFrame.frame.native.functionName = frame->funcName.c_str();
        stackFrame.frame.native.mapName = frame->mapName.c_str();
        stackFrame.frame.native.relativePc = frame->relPc;
        stackFrame.frame.native.reserved = nullptr;
    }
    callback(pc, arg, &stackFrame);
    return HIDEBUG_SUCCESS;
}

HiDebug_Backtrace_Object OH_HiDebug_CreateBacktraceObject()
{
    return reinterpret_cast<HiDebug_Backtrace_Object>(OHOS::HiviewDFX::FpBacktrace::CreateInstance());
}

void OH_HiDebug_DestroyBacktraceObject(HiDebug_Backtrace_Object object)
{
    delete reinterpret_cast<OHOS::HiviewDFX::FpBacktrace*>(object);
}