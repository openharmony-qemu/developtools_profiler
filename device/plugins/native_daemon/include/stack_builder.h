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
    StackBuilder(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime, std::mutex& mtx)
        : hookConfig_(config), runtimeInstance_(runtime), mtx_(mtx) {};
    virtual ~StackBuilder() = default;
    virtual void PrepareUnwind(HookRecordPtr hookRecord) {};
    virtual void FillIps(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) = 0;
    virtual void FillJsSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) = 0;
    virtual void FillNativeSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) = 0;
    void FillStatsVirtualFrames(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord);
    virtual bool IsRecordInfoValid(HookRecordPtr hookRecord) = 0;
    virtual void ReplaceErrStack(std::vector<CallFrame>& callFrames) {};

protected:
    NativeHookConfig* hookConfig_{nullptr};
    std::shared_ptr<VirtualRuntime> runtimeInstance_{nullptr};
    std::mutex& mtx_;
};

class FpStackBuilder : public StackBuilder {
public:
    FpStackBuilder() = delete;
    FpStackBuilder(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime, std::mutex& mtx);
    void PrepareUnwind(HookRecordPtr hookRecord) override;
    void FillIps(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) override;
    void FillJsSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) override;
    void FillNativeSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) override;
    bool IsRecordInfoValid(HookRecordPtr hookRecord) override;
    void FillJsFrame(CallFrame& jsCallFrame);
};

class DwarfStackBuilder : public StackBuilder {
public:
    DwarfStackBuilder() = delete;
    DwarfStackBuilder(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime, std::mutex& mtx);
    void FillIps(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) override;
    void FillJsSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) override;
    void FillNativeSymbols(std::vector<CallFrame>& callFrames, HookRecordPtr hookRecord) override;
    bool IsRecordInfoValid(HookRecordPtr hookRecord) override;
    void ReplaceErrStack(std::vector<CallFrame>& callFrames) override;

private:
    size_t stackDepth_ = 0;
    size_t minStackDepth_ = 0;
};

class BuildStackDirector {
public:
    BuildStackDirector() = delete;
    BuildStackDirector(NativeHookConfig* config, std::shared_ptr<VirtualRuntime> runtime, std::mutex& mtx);
    std::vector<CallFrame>& ConstructCallFrames(HookRecordPtr hookRecord);
    void SetBuilder(std::shared_ptr<StackBuilder> builder)
    {
        builder_ = builder;
    }
    bool IsRecordUnwindable(HookRecordPtr);

private:
    std::shared_ptr<StackBuilder> builder_{nullptr};
    NativeHookConfig* hookConfig_{nullptr};
    std::vector<CallFrame> callFrames_;
};
}

#endif