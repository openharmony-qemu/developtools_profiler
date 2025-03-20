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

#include "napi_hidebug_dump.h"

#include <atomic>
#include <cinttypes>
#include <cstring>
#include <fcntl.h>
#include <memory>

#include <sys/statvfs.h>

#include "application_context.h"
#include "common.h"
#include "error_code.h"
#include "hiappevent_util.h"
#include "hidebug_util.h"
#include "hisysevent.h"
#include "napi_util.h"
#include "parameters.h"

#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HidebugDump"

constexpr auto SLASH_STR = "/";
constexpr auto HEAPSNAPSHOT_FILE = ".heapsnapshot";
constexpr auto RAW_HEAP_RECORD_FILE = "rawheap";
constexpr auto RAW_HEAP_FILE_EXT = ".rawheap";
constexpr auto HIDEBUG_DUMP_QUOTA = "user.hidebugdump.quota";
constexpr auto DUMP_MAX_COUNT = "persist.hiview.hidebugdump.maxcount";
constexpr auto PROCESS_DUMP_MAX_COUNT = "persist.hiview.hidebugdump.process.maxcount";
constexpr auto QUOTA_VALUE_LENGTH = 256;
constexpr int ONE_VALUE_LIMIT = 1;
constexpr int64_t MS_TO_NS = 1000 * 1000;
constexpr int64_t S_TO_NS = MS_TO_NS * 1000;
std::atomic<int> g_dumpingCount = 0;

constexpr std::pair<int, const char*> DumpErrCodeMap[]  = {
    {ErrorCode::PARAMETER_ERROR, "Invalid parameter."},
    {DumpRawHeapErrors::REPEAT_DUMPING, "Repeated data dump."},
    {DumpRawHeapErrors::FAILED_CREATE_FILE, "Failed to create dump file."},
    {DumpRawHeapErrors::LOW_DISK_SPACE, "Disk remaining space too low."},
    {DumpRawHeapErrors::QUOTA_EXCEEDED, "Quota exceeded."},
    {DumpRawHeapErrors::FORK_FAILED, "Fork operation failed."},
    {DumpRawHeapErrors::FAILED_WAIT_CHILD_PROCESS_FINISHED, "Failed to wait for the child process to finish."},
    {DumpRawHeapErrors::TIMEOUT_WAIT_CHILD_PROCESS_FINISHED, "Timeout while waiting for the child process to finish."}
};

bool DumpLimitEnable()
{
    return !IsDeveloperOptionsEnabled() && !IsDebuggableHap() && !IsBetaVersion();
}

bool InitRawHeapFile(std::string& fileName)
{
    std::string filePath = GetProcessDir(DirectoryType::CACHE);
    if (filePath.empty()) {
        return false;
    }
    fileName = filePath + SLASH_STR + RAW_HEAP_RECORD_FILE;
    constexpr unsigned fileMode = 0700;
    return CreateDirectory(fileName, fileMode);
}

std::vector<std::string> LoadDumpRecords()
{
    std::vector<std::string> records;
    std::string rawHeapFile;
    if (!InitRawHeapFile(rawHeapFile)) {
        return records;
    }
    std::string quota;
    if (!GetXAttr(rawHeapFile, HIDEBUG_DUMP_QUOTA, quota, QUOTA_VALUE_LENGTH)) {
        return records;
    }
    const int64_t currentTime = GetRealNanoSecondsTimestamp();
    constexpr int64_t timeout = 24 * 60 * 60 * S_TO_NS;
    const int64_t validTime = currentTime - timeout;
    return SplitStr(quota, ',', [validTime, currentTime](const auto& record) {
        if (record.empty()) {
            return false;
        }
        constexpr int decBase = 10;
        int64_t num = std::strtoll(record.c_str(), nullptr, decBase);
        return num > validTime && num < currentTime;
    });
}

bool AppendMetaData(const std::string& newPath)
{
    auto rawHeapFileSize = static_cast<uint32_t>(GetFileSize(newPath));
    if (rawHeapFileSize == 0) {
        HILOG_ERROR(LOG_CORE, "%{public}s is not existed or empty.",  newPath.c_str());
        return false;
    }
    constexpr auto metaDataPath = "/system/lib64/module/arkcompiler/metadata.json";
    auto metaDataFileSize = static_cast<uint32_t>(GetFileSize(metaDataPath));
    if (metaDataFileSize == 0) {
        HILOG_ERROR(LOG_CORE, "%{public}s is not existed or empty.",  metaDataPath);
        return false;
    }
    auto targetFile = SmartFile::OpenFile(newPath, "ab");
    if (targetFile == nullptr) {
        return false;
    }
    auto metaData = SmartFile::OpenFile(metaDataPath, "rb");
    if (metaData == nullptr) {
        return false;
    }
    constexpr auto buffSize = 1024;
    char buffer[buffSize];
    size_t bytesRead;
    while ((bytesRead = metaData->Read(buffer, 1, buffSize)) > 0) {
        if (!targetFile->Write(buffer, 1, bytesRead)) {
            return false;
        }
    }
    return targetFile->Write(&rawHeapFileSize, sizeof(rawHeapFileSize), 1) &&
        targetFile->Write(&metaDataFileSize, sizeof(metaDataFileSize), 1);
}

bool InsertDumpRecord()
{
    std::vector<std::string> records = LoadDumpRecords();
    records.emplace_back(std::to_string(GetRealNanoSecondsTimestamp()));
    std::string recordStr;
    for (const auto& record : records) {
        recordStr += (record + ",");
    }
    std::string rawHeapFile;
    if (!InitRawHeapFile(rawHeapFile)) {
        return false;
    }
    return SetXAttr(rawHeapFile, HIDEBUG_DUMP_QUOTA, recordStr);
}

bool ReportDumpSuccess()
{
    int32_t ret = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK,
        "ARK_STATS_DUMP",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "TYPE", "hidebugDump");
    if (ret != 0) {
        HILOG_ERROR(LOG_CORE, "failed to report dump success ret %{public}d.", ret);
    }
    return true;
}

bool CheckDeviceQuota()
{
    return OHOS::system::GetIntParameter(DUMP_MAX_COUNT, 0) > 0;
}

bool CheckProcessQuota()
{
    int limit = OHOS::system::GetIntParameter(PROCESS_DUMP_MAX_COUNT, 0);
    if (limit <= 0) {
        return false;
    }
    auto size = LoadDumpRecords().size();
    if (size >= static_cast<size_t>(limit)) {
        return false;
    }
    return true;
}

bool CheckDumpDiskSpace()
{
    struct statvfs stat{};
    std::string appDir = GetProcessDir(DirectoryType::FILE);
    if (statvfs(appDir.c_str(), &stat) != 0) {
        return false;
    }
    constexpr uint64_t dumpNeedSpace = 30ULL * 1024 * 1024 * 1024;
    uint64_t freeSize = stat.f_bsize * stat.f_bfree;
    if (freeSize <= dumpNeedSpace) {
        HILOG_ERROR(LOG_CORE, "disk space is not enough, remains %{public}" PRIu64 "B.", freeSize);
        return false;
    }
    return true;
}

std::string GenerateDumpFile(int64_t currentTime)
{
    std::string filesDir = GetProcessDir(DirectoryType::FILE);
    if (filesDir.empty()) {
        return "";
    }
    return filesDir + SLASH_STR + "hidebug-jsheap-" + std::to_string(getpid()) + "-" + std::to_string(gettid()) +
        "-" + std::to_string(currentTime / MS_TO_NS) + RAW_HEAP_FILE_EXT;
}
}

napi_value DumpHeapSnapshot(const std::string& fileName, napi_env env)
{
    std::string filesDir = GetProcessDir(DirectoryType::FILE);
    if (filesDir.empty()) {
        return CreateErrorMessage(env, "Get App files dir failed.");
    }
    std::string filePath = filesDir + SLASH_STR + fileName + HEAPSNAPSHOT_FILE;
    if (!IsLegalPath(filePath)) {
        return CreateErrorMessage(env, "input fileName is illegal.");
    }
    if (!CreateFile(filePath)) {
        return CreateErrorMessage(env, "file created failed.");
    }
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    engine->DumpHeapSnapshot(filePath, true, DumpFormat::JSON, false, true);
    return CreateUndefined(env);
}

napi_value DumpHeapData(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("dumpHeapData");
    std::string fileName = GetFileNameParam(env, info);
    return DumpHeapSnapshot(fileName, env);
}

napi_value DumpJsHeapData(napi_env env, napi_callback_info info)
{
    ApiInvokeRecorder apiInvokeRecorder("dumpJsHeapData");
    std::string fileName;
    if (!GetTheOnlyStringParam(env, info, fileName)) {
        std::string paramErrorMessage = "Invalid parameter, require a string parameter.";
        napi_throw_error(env, std::to_string(ErrorCode::PARAMETER_ERROR).c_str(), paramErrorMessage.c_str());
        return CreateUndefined(env);
    }
    return DumpHeapSnapshot(fileName, env);
}

int32_t GetDumpJsRawHeapParams(napi_env env, napi_callback_info info, std::string& filePath, bool& isGc, int& fd)
{
    size_t argc = ONE_VALUE_LIMIT;  // expected param length.
    napi_value argv = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, nullptr, nullptr);
    if (g_dumpingCount.fetch_add(1) != 0) {
        return DumpRawHeapErrors::REPEAT_DUMPING;
    }
    if (argc > ONE_VALUE_LIMIT  || (argc > 0 && !GetNapiBoolValue(env, argv, isGc))) {
        return ErrorCode::PARAMETER_ERROR;
    }
    if (!CheckDumpDiskSpace()) {
        return DumpRawHeapErrors::LOW_DISK_SPACE;
    }
    if (DumpLimitEnable() && (!CheckDeviceQuota() || !CheckProcessQuota())) {
        return DumpRawHeapErrors::QUOTA_EXCEEDED;
    }
    filePath = GenerateDumpFile(GetRealNanoSecondsTimestamp());
    if (!IsLegalPath(filePath)) {
        return DumpRawHeapErrors::FAILED_CREATE_FILE;
    }
    fd = open(filePath.c_str(),  O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP); // -rw-r-----
    return fd < 0 ? DumpRawHeapErrors::FAILED_CREATE_FILE : 0;
}

napi_value ResolveErrorCode(napi_env env, const int errCode)
{
    if (errCode == 0) {
        return nullptr;
    }
    HILOG_ERROR(LOG_CORE, "%{public}s for %{public}d.", __func__, errCode);
    for (const auto&[code, msg] : DumpErrCodeMap) {
        if (code == errCode) {
            return CreateErrorMessage(env, std::to_string(errCode), msg);
        }
    }
    return nullptr;
}

int TransferRetCodeToErrorCode(const int retCode)
{
    constexpr int dumpSuccess = 0;
    constexpr int forkFailed = 1;
    constexpr int failedToWait = 2;
    constexpr int waitTimeOut = 3;
    switch (retCode) {
        case dumpSuccess:
            return 0;
        case forkFailed:
            return FORK_FAILED;
        case failedToWait:
            return FAILED_WAIT_CHILD_PROCESS_FINISHED;
        case waitTimeOut:
            return TIMEOUT_WAIT_CHILD_PROCESS_FINISHED;
        default:
            HILOG_ERROR(LOG_CORE, "unknown retCode %{public}d.", retCode);
            return TIMEOUT_WAIT_CHILD_PROCESS_FINISHED;
    }
}

void ResolveDumpJsRawHeapData(napi_env env, napi_deferred deferred, const std::string& fileName)
{
    if (!AppendMetaData(fileName)) {
        HILOG_ERROR(LOG_CORE, "failed to append metadata to dump file.");
    }
    if (DumpLimitEnable()) {
        if (!InsertDumpRecord()) {
            HILOG_ERROR(LOG_CORE, "failed to insert process dump record.");
        }
        if (!ReportDumpSuccess()) {
            HILOG_ERROR(LOG_CORE, "failed to report dump success event.");
        }
    }
    g_dumpingCount.fetch_sub(1);
    napi_value ret;
    napi_create_string_utf8(env, fileName.c_str(), fileName.size(), &ret);
    napi_resolve_deferred(env, deferred, ret);
}

napi_value DumpJsRawHeapData(napi_env env, napi_callback_info info)
{
    auto apiInvokeRecorder = std::make_shared<ApiInvokeRecorder>("dumpJsRawHeapData");
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    bool isGc = true;
    std::string fileName;
    int fd = -1;
    int32_t errCode = GetDumpJsRawHeapParams(env, info, fileName, isGc, fd);
    napi_value err = ResolveErrorCode(env, errCode);
    if (err != nullptr) {
        g_dumpingCount.fetch_sub(1);
        apiInvokeRecorder->SetErrorCode(errCode);
        napi_reject_deferred(env, deferred, err);
        return promise;
    }
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env);
    /* When the interface is available, the fd will be set to -1. */
    engine->DumpHeapSnapshot(fd, isGc, [apiInvokeRecorder, env, deferred, fileName] (uint8_t retCode) {
        apiInvokeRecorder->SetErrorCode(retCode);
        napi_send_event(env, [env, deferred, fileName, retCode]() {
            napi_value err = ResolveErrorCode(env, TransferRetCodeToErrorCode(retCode));
            if (err != nullptr) {
                remove(fileName.c_str());
                g_dumpingCount.fetch_sub(1);
                napi_reject_deferred(env, deferred, err);
            } else {
                ResolveDumpJsRawHeapData(env, deferred, fileName);
            }
        }, napi_eprio_high);
    });
    if (fd >= 0) {
        HILOG_ERROR(LOG_CORE, "DumpJsRawHeapData is not supported in current version.");
        close(fd);
        remove(fileName.c_str());
        g_dumpingCount.fetch_sub(1);
        err = ResolveErrorCode(env, DumpRawHeapErrors::TIMEOUT_WAIT_CHILD_PROCESS_FINISHED);
        napi_reject_deferred(env, deferred, err);
    }
    return promise;
}
}
}