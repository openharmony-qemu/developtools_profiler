/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "async_stack.h"
#include "hilog/log.h"
#include "hidebug_util.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD002D0A
#define LOG_TAG "HidebugAsyncStack"

#if defined(__aarch64__)
namespace {
static bool g_asyncSwitchEnable = false;
}

void OH_HiDebug_SetSwitchCallback(bool enable)
{
    g_asyncSwitchEnable = enable;
}
#endif

extern "C" {
#if defined(__aarch64__)
__attribute__((constructor)) void OH_HiDebug_SetAsyncStackCallback()
{
    DfxSetHiDebugAsyncStackCallback(OH_HiDebug_SetSwitchCallback);
}
#endif

uint64_t OH_HiDebug_AcquireAsyncContext()
{
#if defined(__aarch64__)
    if (!OHOS::HiviewDFX::IsDebuggableHap()) {
        HILOG_ERROR(LOG_CORE, "OH_Hidebug_CollectAsyncStack failed, not debuggable hap");
        return 0;
    }

    if (!g_asyncSwitchEnable) {
        HILOG_ERROR(LOG_CORE, "Hidebug async stack not enabled or callback not set");
        return 0;
    }

    uint64_t ctx = DfxCollectAsyncStack(ASYNC_TYPE_CUSTOMIZE);
    if (ctx == 0) {
        HILOG_ERROR(LOG_CORE, "Collect async stack failed, stackId is 0");
        return 0;
    }

    return ctx;
#else
    return 0;
#endif
}

void OH_HiDebug_ReleaseAsyncContext(uint64_t ctx)
{
#if defined(__aarch64__)
    if (!OHOS::HiviewDFX::IsDebuggableHap()) {
        HILOG_ERROR(LOG_CORE, "OH_Hidebug_ReleaseAsyncContext failed, not debuggable hap");
        return;
    }

    if (!g_asyncSwitchEnable) {
        HILOG_ERROR(LOG_CORE, "Hidebug async stack not enabled");
        return;
    }

    ReleaseAsyncContext(ctx);
#endif
}

void OH_HiDebug_PushAsyncContext(uint64_t ctx)
{
#if defined(__aarch64__)
    if (!OHOS::HiviewDFX::IsDebuggableHap()) {
        HILOG_ERROR(LOG_CORE, "OH_Hidebug_PushAsyncStackId failed, not debuggable hap");
        return;
    }

    if (ctx == 0) {
        HILOG_ERROR(LOG_CORE, "PushAsyncStackId failed, stackId is 0");
        return;
    }

    if (!g_asyncSwitchEnable) {
        HILOG_ERROR(LOG_CORE, "Hidebug async stack not enabled");
        return;
    }

    DfxSetSubmitterStackId(ctx);
#endif
}

void OH_HiDebug_PopAsyncContext(uint64_t ctx)
{
#if defined(__aarch64__)
    if (!OHOS::HiviewDFX::IsDebuggableHap()) {
        HILOG_ERROR(LOG_CORE, "OH_Hidebug_PopAsyncStackId failed, not debuggable hap");
        return;
    }

    if (ctx == 0) {
        HILOG_ERROR(LOG_CORE, "PopAsyncStackId failed, stackId is 0");
        return;
    }

    if (!g_asyncSwitchEnable) {
        HILOG_ERROR(LOG_CORE, "Hidebug async stack not enabled");
        return;
    }

    DfxPopSubmitterStackId(ctx);
#endif
}
}
