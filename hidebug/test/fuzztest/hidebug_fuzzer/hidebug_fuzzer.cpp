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


#include "hidebug_fuzzer.h"
#include "hidebug/hidebug.h"
#include "hidebug/hidebug_type.h"
#include "hidebug_util.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace HidebugFuzz {
constexpr uint8_t MAX_STR_LENGTH = 128;
void OhHiDebugGetGraphicsMemorySummaryFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    int32_t interval = provider.ConsumeIntegral<uint32_t>();
    HiDebug_GraphicsMemorySummary summary;
    OH_HiDebug_GetGraphicsMemorySummary(interval, &summary);
}

void OhHiDebugGetAppNativeMemInfoWithCacheFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    bool flag = provider.ConsumeBool();
    HiDebug_NativeMemInfo memInfo;
    OH_HiDebug_GetAppNativeMemInfoWithCache(&memInfo, flag);
}

void OhHiDebugStartAppTraceCaptureFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    uint32_t fileLength = provider.ConsumeIntegral<uint32_t>();
    char fileName[256] = {0};
    HiDebug_TraceFlag flag = HIDEBUG_TRACE_FLAG_MAIN_THREAD;
    uint64_t tags = provider.ConsumeIntegral<uint64_t>();
    uint32_t limitSize = provider.ConsumeIntegral<uint32_t>();
    OH_HiDebug_StartAppTraceCapture(flag, tags, limitSize, fileName, fileLength);
    sleep(1);
    OH_HiDebug_StopAppTraceCapture();
}

void SetXAttrFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fileName = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    std::string key = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    std::string value = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    OHOS::HiviewDFX::SetXAttr(fileName, key, value);
}

void GetXAttrFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fileName = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    std::string key = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    std::string value = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    size_t maxLength = provider.ConsumeIntegral<size_t>();
    OHOS::HiviewDFX::GetXAttr(fileName, key, value, maxLength);
}

void CreateFileFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fileName = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    OHOS::HiviewDFX::CreateFile(fileName);
}

void CreateDirectoryFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fileName = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    constexpr unsigned fileMode = 0755;
    OHOS::HiviewDFX::CreateDirectory(fileName, fileMode);
}

void IsLegalPathFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fileName = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    OHOS::HiviewDFX::IsLegalPath(fileName);
}

void GetFileSizeFuzz(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fileName = provider.ConsumeRandomLengthString(MAX_STR_LENGTH);
    OHOS::HiviewDFX::GetFileSize(fileName);
}
} // namespace HidebugFuzz

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    HidebugFuzz::OhHiDebugGetGraphicsMemorySummaryFuzz(data, size);
    HidebugFuzz::OhHiDebugGetAppNativeMemInfoWithCacheFuzz(data, size);
    HidebugFuzz::OhHiDebugStartAppTraceCaptureFuzz(data, size);
    HidebugFuzz::SetXAttrFuzz(data, size);
    HidebugFuzz::GetXAttrFuzz(data, size);
    HidebugFuzz::CreateFileFuzz(data, size);
    HidebugFuzz::CreateDirectoryFuzz(data, size);
    HidebugFuzz::IsLegalPathFuzz(data, size);
    HidebugFuzz::GetFileSizeFuzz(data, size);
    return 0;
}