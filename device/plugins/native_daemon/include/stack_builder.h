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
#ifndef STACK_BUILDER_H
#define STACK_BUILDER_H

#include "hook_common.h"
#include "hook_record.h"
#include "virtual_runtime.h"
#include "native_hook_config.pb.h"
namespace OHOS::Developtools::NativeDaemon {

class StackBuilder {
public:
    StackBuilder() = delete;
    StackBuilder(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime)
        : hookConfig_(config), runtimeInstance_(runtime) {};
    virtual ~StackBuilder() {};
    virtual void PrepareUnwind(HookRecordPtr hookRecord) {};
    virtual void FillIps(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) = 0;
    virtual void FillJsSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) = 0;
    virtual void FillNativeSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) = 0;
    void FillStatsVirtualFrames(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord);
    virtual bool IsRecordInfoValid(HookRecordPtr hookRecord) = 0;
    virtual void ReplaceErrStack(std::vector<CallFrame>& callFrames) {};

protected:
    NativeHookConfig* hookConfig_;
    std::shared_ptr<VirtualRuntime> runtimeInstance_{nullptr};
};

class FpStackBuilder : public StackBuilder {
public:
    FpStackBuilder() = delete;
    FpStackBuilder(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime);
    void PrepareUnwind(HookRecordPtr hookRecord);
    void FillIps(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord);
    void FillJsSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord);
    void FillNativeSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord);
    bool IsRecordInfoValid(HookRecordPtr hookRecord);
    void FillJsFrame(CallFrame& jsCallFrame);

private:
    std::mutex mtx_;
};

class DwarfStackBuilder : public StackBuilder {
public:
    DwarfStackBuilder() = delete;
    DwarfStackBuilder(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime);
    void FillIps(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord);
    void FillJsSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord);
    void FillNativeSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord);
    bool IsRecordInfoValid(HookRecordPtr hookRecord);
    void ReplaceErrStack(std::vector<CallFrame>& callFrames);

private:
    size_t stackDepth_ = 0;
    size_t minStackDepth_ = 0;
};

class BuildStackDirector {
public:
    BuildStackDirector() = delete;
    BuildStackDirector(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime);
    std::vector<CallFrame>& ConstructCallFrames(HookRecordPtr hookRecord);
    void SetBuilder(std::shared_ptr<StackBuilder> builder)
    {
        builder_ = builder;
    }
    bool IsRecordUnwindable(HookRecordPtr);

private:
    std::shared_ptr<StackBuilder> builder_{nullptr};
    NativeHookConfig* hookConfig_;
    std::vector<CallFrame> callFrames_;
};
}

#endif