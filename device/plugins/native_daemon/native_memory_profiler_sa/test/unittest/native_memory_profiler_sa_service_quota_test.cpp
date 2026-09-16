#include "native_memory_profiler_sa_service_test_common.h"
using namespace OHOS::Security::AccessToken;
using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr const char *QUOTA_FILE_PATH = "/data/log/hiprofiler/profiling_rate_limits_quota";
constexpr long long QUOTA_SECONDS_PER_HOUR = 3600LL;
constexpr long long QUOTA_ONE_DAY_SECONDS = 24LL * QUOTA_SECONDS_PER_HOUR;
constexpr long long QUOTA_OVER_ONE_DAY_SECONDS = 25LL * QUOTA_SECONDS_PER_HOUR;
}

/**
 * @tc.name: QuotaIsSeparateByMoreThan24HoursBoundary
 * @tc.desc: Quota is separate by more than 24 hours boundary.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaIsSeparateByMoreThan24HoursBoundary, TestSize.Level2)
{
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsSeparateByMoreThan24Hours(-1, 0));
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsSeparateByMoreThan24Hours(0, -1));
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsSeparateByMoreThan24Hours(100, 100));
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsSeparateByMoreThan24Hours(
        100, 100 + QUOTA_ONE_DAY_SECONDS));
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsSeparateByMoreThan24Hours(
        100, 100 + QUOTA_ONE_DAY_SECONDS + 1));
    EXPECT_TRUE(NativeMemoryProfilerSaService::IsSeparateByMoreThan24Hours(
        100, 100 + QUOTA_OVER_ONE_DAY_SECONDS));
    EXPECT_TRUE(NativeMemoryProfilerSaService::IsSeparateByMoreThan24Hours(
        100 + QUOTA_OVER_ONE_DAY_SECONDS, 100));
}

/**
 * @tc.name: QuotaGetMemProfilerUseCountTodayBranches
 * @tc.desc: Quota get mem profiler use count today branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaGetMemProfilerUseCountTodayBranches, TestSize.Level2)
{
    long long now = static_cast<long long>(time(nullptr));
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerUseCountToday(records, "missing"), 0);
    records["foo"] = {now - QUOTA_OVER_ONE_DAY_SECONDS, 5};
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerUseCountToday(records, "foo"), 0);
    records["foo"] = {now, 7};
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerUseCountToday(records, "foo"), 7);
    records[""] = {now, 3};
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerUseCountToday(records, ""), 3);
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerUseCountToday(records, "missing"), 0);
}

/**
 * @tc.name: QuotaGetMemProfilerTotalUseCountTodayBranches
 * @tc.desc: Quota get mem profiler total use count today branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaGetMemProfilerTotalUseCountTodayBranches, TestSize.Level2)
{
    long long now = static_cast<long long>(time(nullptr));
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerTotalUseCountToday(records), 0);
    records["a"] = {now - QUOTA_OVER_ONE_DAY_SECONDS, 2};
    records["b"] = {now - QUOTA_OVER_ONE_DAY_SECONDS, 3};
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerTotalUseCountToday(records), 0);
    records["c"] = {now, 4};
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerTotalUseCountToday(records), 4);
    records["d"] = {now, 5};
    EXPECT_EQ(NativeMemoryProfilerSaService::GetMemProfilerTotalUseCountToday(records), 9);
}

/**
 * @tc.name: QuotaUpdateMemProfilerUserCountBranches
 * @tc.desc: Quota update mem profiler user count branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaUpdateMemProfilerUserCountBranches, TestSize.Level2)
{
    long long now = static_cast<long long>(time(nullptr));
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_TRUE(NativeMemoryProfilerSaService::UpdateMemProfilerUserCount(records, "new"));
    EXPECT_EQ(records["new"].count, 1);
    EXPECT_GE(records["new"].timestamp, now);
    records["old"] = {now - QUOTA_OVER_ONE_DAY_SECONDS, 5};
    EXPECT_TRUE(NativeMemoryProfilerSaService::UpdateMemProfilerUserCount(records, "old"));
    EXPECT_EQ(records["old"].count, 1);
    records["today"] = {now, 3};
    EXPECT_TRUE(NativeMemoryProfilerSaService::UpdateMemProfilerUserCount(records, "today"));
    EXPECT_EQ(records["today"].count, 4);
    EXPECT_GE(records["today"].timestamp, now);
}

/**
 * @tc.name: QuotaReadQuotaFileNonExistentReturnsTrueEmpty
 * @tc.desc: Quota read quota file non existent returns true empty.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaReadQuotaFileNonExistentReturnsTrueEmpty, TestSize.Level2)
{
    ASSERT_EQ(system("mkdir -p /data/log/hiprofiler/"), 0);
    unlink(QUOTA_FILE_PATH);
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_TRUE(NativeMemoryProfilerSaService::ReadQuotaFile(records));
    EXPECT_TRUE(records.empty());
    unlink(QUOTA_FILE_PATH);
}

/**
 * @tc.name: QuotaWriteAndReadQuotaFileRoundtrip
 * @tc.desc: Quota write and read quota file roundtrip.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaWriteAndReadQuotaFileRoundtrip, TestSize.Level2)
{
    ASSERT_EQ(system("mkdir -p /data/log/hiprofiler/"), 0);
    unlink(QUOTA_FILE_PATH);
    long long now = static_cast<long long>(time(nullptr));
    std::unordered_map<std::string, QuotaRecord> records;
    records["foo"] = {now, 2};
    records["bar"] = {now, 3};
    EXPECT_TRUE(NativeMemoryProfilerSaService::WriteQuotaFile(records));
    std::unordered_map<std::string, QuotaRecord> readRecords;
    EXPECT_TRUE(NativeMemoryProfilerSaService::ReadQuotaFile(readRecords));
    EXPECT_EQ(readRecords.size(), 2u);
    EXPECT_EQ(readRecords["foo"].count, 2);
    EXPECT_EQ(readRecords["bar"].count, 3);
    unlink(QUOTA_FILE_PATH);
}

/**
 * @tc.name: QuotaReadQuotaFileIgnoresCorruptLines
 * @tc.desc: Quota read quota file ignores corrupt lines.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaReadQuotaFileIgnoresCorruptLines, TestSize.Level2)
{
    ASSERT_EQ(system("mkdir -p /data/log/hiprofiler/"), 0);
    unlink(QUOTA_FILE_PATH);
    std::ofstream corruptOfs(QUOTA_FILE_PATH);
    corruptOfs << "foo 123 2\nbar abc 3\n";
    corruptOfs.close();
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_TRUE(NativeMemoryProfilerSaService::ReadQuotaFile(records));
    EXPECT_EQ(records.size(), 1u);
    EXPECT_EQ(records["foo"].count, 2);
    EXPECT_EQ(records["foo"].timestamp, 123);
    unlink(QUOTA_FILE_PATH);
}

/**
 * @tc.name: QuotaWriteQuotaFileEmptyRecordsWritesEmptyFile
 * @tc.desc: Quota write quota file empty records writes empty file.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaWriteQuotaFileEmptyRecordsWritesEmptyFile, TestSize.Level2)
{
    ASSERT_EQ(system("mkdir -p /data/log/hiprofiler/"), 0);
    unlink(QUOTA_FILE_PATH);
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_TRUE(NativeMemoryProfilerSaService::WriteQuotaFile(records));
    std::unordered_map<std::string, QuotaRecord> readRecords;
    EXPECT_TRUE(NativeMemoryProfilerSaService::ReadQuotaFile(readRecords));
    EXPECT_TRUE(readRecords.empty());
    unlink(QUOTA_FILE_PATH);
}

/**
 * @tc.name: QuotaRollbackMemProfilerUserCountDebugSkipsFile
 * @tc.desc: Quota rollback mem profiler user count debug skips file.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaRollbackMemProfilerUserCountDebugSkipsFile, TestSize.Level2)
{
    ASSERT_EQ(system("mkdir -p /data/log/hiprofiler/"), 0);
    unlink(QUOTA_FILE_PATH);
    std::ofstream debugOfs(QUOTA_FILE_PATH);
    debugOfs << "foo 100 2\n";
    debugOfs.close();
    EXPECT_TRUE(NativeMemoryProfilerSaService::RollbackMemProfilerUserCount(true, "foo"));
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_TRUE(NativeMemoryProfilerSaService::ReadQuotaFile(records));
    EXPECT_EQ(records["foo"].count, 2);
    unlink(QUOTA_FILE_PATH);
}

/**
 * @tc.name: QuotaRollbackMemProfilerUserCountDecrementsAndPersists
 * @tc.desc: Quota rollback mem profiler user count decrements and persists.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaRollbackMemProfilerUserCountDecrementsAndPersists, TestSize.Level2)
{
    ASSERT_EQ(system("mkdir -p /data/log/hiprofiler/"), 0);
    unlink(QUOTA_FILE_PATH);
    long long now = static_cast<long long>(time(nullptr));
    std::ofstream decOfs(QUOTA_FILE_PATH);
    decOfs << "foo " << now << " 3\n";
    decOfs.close();
    EXPECT_TRUE(NativeMemoryProfilerSaService::RollbackMemProfilerUserCount(false, "foo"));
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_TRUE(NativeMemoryProfilerSaService::ReadQuotaFile(records));
    EXPECT_EQ(records["foo"].count, 2);
    unlink(QUOTA_FILE_PATH);
}

/**
 * @tc.name: QuotaRollbackMemProfilerUserCountProtectsZeroCount
 * @tc.desc: Quota rollback mem profiler user count protects zero count.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, QuotaRollbackMemProfilerUserCountProtectsZeroCount, TestSize.Level2)
{
    ASSERT_EQ(system("mkdir -p /data/log/hiprofiler/"), 0);
    unlink(QUOTA_FILE_PATH);
    long long now = static_cast<long long>(time(nullptr));
    std::ofstream zeroOfs(QUOTA_FILE_PATH);
    zeroOfs << "foo " << now << " 0\n";
    zeroOfs.close();
    EXPECT_TRUE(NativeMemoryProfilerSaService::RollbackMemProfilerUserCount(false, "foo"));
    std::unordered_map<std::string, QuotaRecord> records;
    EXPECT_TRUE(NativeMemoryProfilerSaService::ReadQuotaFile(records));
    EXPECT_EQ(records["foo"].count, 0);
    unlink(QUOTA_FILE_PATH);
}

/**
 * @tc.name: CheckStopPermissionVerifiesCallingPid
 * @tc.desc: Check stop permission verifies calling PID.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckStopPermissionVerifiesCallingPid, TestSize.Level2)
{
    int32_t callingPid = OHOS::IPCSkeleton::GetCallingPid();
    auto hook = std::make_shared<HookManager>();
    auto task = std::make_shared<TaskConfig>(hook, 0, "perm_check", callingPid, "", -1, false);
    EXPECT_TRUE(NativeMemoryProfilerSaService::CheckCallingPermission(callingPid, task));
    constexpr int32_t mismatchedPid = 99999;
    EXPECT_FALSE(NativeMemoryProfilerSaService::CheckCallingPermission(mismatchedPid, task));
}

/**
 * @tc.name: IsAsyncRequestReturnsFalseForUnknownPid
 * @tc.desc: Is async request returns false for unknown PID.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, IsAsyncRequestReturnsFalseForUnknownPid, TestSize.Level2)
{
    constexpr uint32_t unknownPid = 50002;
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsAsyncRequest(unknownPid));
}

/**
 * @tc.name: StopSyncReturnsNotStartedWithoutTask
 * @tc.desc: Stop sync returns not started without task.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StopSyncReturnsNotStartedWithoutTask, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->StopSync(), static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED));
}

/**
 * @tc.name: DumpDataAsyncRejectsNullConfigAndCallback
 * @tc.desc: Dump data async rejects null config and callback.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, DumpDataAsyncRejectsNullConfigAndCallback, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    std::shared_ptr<NativeMemoryProfilerSaConfig> nullConfig;
    EXPECT_EQ(service->DumpDataAsync(0, nullConfig, nullptr), RET_ERR);
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(service->DumpDataAsync(0, config, nullptr), RET_ERR);
}

/**
 * @tc.name: StartHookLockFilePathAndOpenBranches
 * @tc.desc: Start hook lock file path and open branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartHookLockFilePathAndOpenBranches, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto hook = std::make_shared<HookManager>();
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->pid_ = getpid();
    config->extendProcesses_ = true;
    config->shareMemorySize_ = 0;
    uint32_t fd = 0;
    std::string args = service->GetCmdArgs(config);
    service->StartHookLock(config, fd, hook, args, nullptr);
    config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->extendProcesses_ = true;
    config->filePath_ = "/nonexistent_dir_xyz/x.htrace";
    fd = 0;
    args = service->GetCmdArgs(config);
    EXPECT_EQ(service->StartHookLock(config, fd, hook, args, nullptr), RET_ERR);
    config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->extendProcesses_ = true;
    config->filePath_ = "/data/local/tmp/test_unlink.htrace";
    config->startType_ = START_TYPE_API;
    config->shareMemorySize_ = 0;
    fd = 0;
    args = service->GetCmdArgs(config);
    service->StartHookLock(config, fd, hook, args, nullptr);
    unlink("/data/local/tmp/test_unlink.htrace");
}

/**
 * @tc.name: StartHookLockHookConfigFdBranches
 * @tc.desc: Start hook lock hook config FD branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartHookLockHookConfigFdBranches, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto hook = std::make_shared<HookManager>();
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->extendProcesses_ = true;
    config->filePath_ = "/data/local/tmp/test_hookcfg.htrace";
    config->gwpAsanHoldTime_ = 1;
    config->printNmdOnly_ = false;
    config->nativeOrphanSnapshotsEnable_ = false;
    config->shareMemorySize_ = 0;
    uint32_t fd = 0;
    std::string args = service->GetCmdArgs(config);
    service->StartHookLock(config, fd, hook, args, nullptr);
    config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->extendProcesses_ = true;
    config->filePath_ = "/data/local/tmp/test_hookcfg2.htrace";
    config->nativeOrphanSnapshotsEnable_ = true;
    config->gwpAsanHoldTime_ = 0;
    config->printNmdOnly_ = false;
    config->shareMemorySize_ = 0;
    fd = 0;
    args = service->GetCmdArgs(config);
    service->StartHookLock(config, fd, hook, args, nullptr);
    unlink("/data/local/tmp/test_hookcfg.htrace");
    unlink("/data/local/tmp/test_hookcfg2.htrace");
}

/**
 * @tc.name: GetTaskNumUnknownStartType
 * @tc.desc: Get task num unknown start type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumUnknownStartType, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->startType_ = 2;
    EXPECT_EQ(service->GetTaskNum(config), 0);
    std::shared_ptr<NativeMemoryProfilerSaConfig> nullConfig;
    EXPECT_EQ(service->GetTaskNum(nullConfig), 0);
}

/**
 * @tc.name: CheckSystemResourceDirectCall
 * @tc.desc: Check system resource direct call.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckSystemResourceDirectCall, TestSize.Level2)
{
    int32_t result = NativeMemoryProfilerSaService::CheckSystemResource();
    EXPECT_TRUE(result == static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS) ||
                result == static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_MEM_PRESSURE_CRITICAL) ||
                result == static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CPU_OVERLOADED) ||
                result == static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_STORAGE_PRESSURE_CRITICAL));
}

/**
 * @tc.name: GetTaskNumApiCountsByUid
 * @tc.desc: Get task num API counts by uid.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumApiCountsByUid, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    constexpr int32_t pid = 50010;
    constexpr int32_t pid2 = 50011;
    constexpr uid_t uid = 1000;
    NativeMemoryProfilerSaService::InjectPidInfoForTest(pid, uid, 0);
    NativeMemoryProfilerSaService::InjectPidInfoForTest(pid2, uid, 0);
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->startType_ = START_TYPE_API;
    config->pid_ = pid;
    EXPECT_EQ(NativeMemoryProfilerSaService::GetTaskNum(config), 2);
    config->pid_ = 99999;
    EXPECT_EQ(NativeMemoryProfilerSaService::GetTaskNum(config), 0);
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: GetTaskNumHiviewCountsCtx
 * @tc.desc: Get task num hiview counts ctx.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumHiviewCountsCtx, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    auto hook = std::make_shared<HookManager>();
    auto task = std::make_shared<TaskConfig>(hook, 50020, "hiview_task", -1, "", -1, false);
    task->startType = TEST_START_TYPE_HIVIEW;
    NativeMemoryProfilerSaService::InjectPidCtxForTest(50020, task);
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->startType_ = TEST_START_TYPE_HIVIEW;
    EXPECT_EQ(NativeMemoryProfilerSaService::GetTaskNum(config), 1);
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: StartRejectsWhenPermissionDenied
 * @tc.desc: Start rejects when permission denied.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartRejectsWhenPermissionDenied, TestSize.Level2)
{
    NativeMemoryProfilerSaService::SetMockPermissionForTest(true, false);
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(service->Start(config), RET_ERR);
    NativeMemoryProfilerSaService::SetMockPermissionForTest(false, true);
}

/**
 * @tc.name: StartRejectsWhenNativeDaemonRunning
 * @tc.desc: Start rejects when native daemon running.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartRejectsWhenNativeDaemonRunning, TestSize.Level2)
{
    NativeMemoryProfilerSaService::SetMockNativeDaemonRunningForTest(true);
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(service->Start(config), RET_ERR);
    NativeMemoryProfilerSaService::SetMockNativeDaemonRunningForTest(false);
}

/**
 * @tc.name: DumpDataRejectsWhenPermissionDenied
 * @tc.desc: Dump data rejects when permission denied.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, DumpDataRejectsWhenPermissionDenied, TestSize.Level2)
{
    NativeMemoryProfilerSaService::SetMockPermissionForTest(true, false);
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(service->DumpData(0, config), RET_ERR);
    NativeMemoryProfilerSaService::SetMockPermissionForTest(false, true);
}

/**
 * @tc.name: DumpDataRejectsWhenNativeDaemonRunning
 * @tc.desc: Dump data rejects when native daemon running.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, DumpDataRejectsWhenNativeDaemonRunning, TestSize.Level2)
{
    NativeMemoryProfilerSaService::SetMockNativeDaemonRunningForTest(true);
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(service->DumpData(0, config), RET_ERR);
    NativeMemoryProfilerSaService::SetMockNativeDaemonRunningForTest(false);
}

/**
 * @tc.name: CheckTryBeginStopHook
 * @tc.desc: Check try begin stop hook.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckTryBeginStopHook, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    uint32_t pid = static_cast<uint32_t>(getpid());
    EXPECT_TRUE(service->TryBeginStopHook(pid));
    EXPECT_FALSE(service->TryBeginStopHook(pid));
    service->EndStopHook(pid);
    EXPECT_TRUE(service->TryBeginStopHook(pid));
    service->EndStopHook(pid);
}

/**
 * @tc.name: StopHookRejectsUnknownPid
 * @tc.desc: Stop hook rejects unknown PID.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StopHookRejectsUnknownPid, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    constexpr uint32_t pid = 50030;
    EXPECT_EQ(NativeMemoryProfilerSaService::StopHook(pid, "unknown", 0, false),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED));
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: StopHookPermissionDeniedAndAlreadyStopping
 * @tc.desc: Stop hook permission denied and already stopping.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StopHookPermissionDeniedAndAlreadyStopping, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    auto hook = std::make_shared<HookManager>();
    auto task1 = std::make_shared<TaskConfig>(hook, 50031, "perm", 99999, "", -1, false);
    NativeMemoryProfilerSaService::InjectPidCtxForTest(50031, task1);
    EXPECT_EQ(NativeMemoryProfilerSaService::StopHook(50031, "", 0, false),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED));
    auto task2 = std::make_shared<TaskConfig>(hook, 50032, "stop2",
        OHOS::IPCSkeleton::GetCallingPid(), "", -1, false);
    NativeMemoryProfilerSaService::InjectPidCtxForTest(50032, task2);
    ASSERT_TRUE(NativeMemoryProfilerSaService::TryBeginStopHook(50032));
    EXPECT_EQ(NativeMemoryProfilerSaService::StopHook(50032, "", 0, false), RET_OK);
    NativeMemoryProfilerSaService::EndStopHook(50032);
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: PidAndNameConflictBranches
 * @tc.desc: PID and name conflict branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, PidAndNameConflictBranches, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    std::shared_ptr<NativeMemoryProfilerSaConfig> nullConfig;
    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(nullConfig), RET_ERR);
    auto hook = std::make_shared<HookManager>();
    auto task = std::make_shared<TaskConfig>(hook, 50040, "conflict", -1, "", -1, false);
    task->startType = START_TYPE_API;
    NativeMemoryProfilerSaService::InjectPidCtxForTest(50040, task);
    auto sameType = MakeSaConfig();
    sameType->pid_ = 50040;
    sameType->startType_ = START_TYPE_API;
    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(sameType),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ALREADY_STARTED));
    auto diffType = MakeSaConfig();
    diffType->pid_ = 50040;
    diffType->startType_ = TEST_START_TYPE_HIVIEW;
    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(diffType),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT));
    auto noConflict = MakeSaConfig();
    noConflict->pid_ = 50041;
    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(noConflict), RET_OK);
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: CheckStartupModeConflictBranches
 * @tc.desc: Check startup mode conflict branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckStartupModeConflictBranches, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    config->startupMode_ = true;
    NativeMemoryProfilerSaService::SetHasStartupModeForTest(true);
    EXPECT_EQ(service->CheckStartupModeConflict(config), RET_ERR);
    NativeMemoryProfilerSaService::SetHasStartupModeForTest(false);
    EXPECT_EQ(service->CheckStartupModeConflict(config), RET_OK);
    NativeMemoryProfilerSaService::SetHasStartupModeForTest(true);
    config->startupMode_ = false;
    EXPECT_EQ(service->CheckStartupModeConflict(config), RET_OK);
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: ApplyNmdConfigBranches
 * @tc.desc: Apply NMD config branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ApplyNmdConfigBranches, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto noNmd = MakeSaConfig();
    noNmd->printNmd_ = false;
    EXPECT_FALSE(service->ApplyNmdConfig(noNmd, 0));
    auto nmd = MakeSaConfig();
    nmd->printNmd_ = true;
    nmd->nmdPid_ = 50050;
    nmd->nmdType_ = 1;
    EXPECT_TRUE(service->ApplyNmdConfig(nmd, 1));
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: StartPidVoidRejectsDailyQuotaExceeded
 * @tc.desc: Start PID void rejects daily quota exceeded.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartPidVoidRejectsDailyQuotaExceeded, TestSize.Level2)
{
    ASSERT_EQ(system("mkdir -p /data/log/hiprofiler/"), 0);
    unlink("/data/log/hiprofiler/profiling_rate_limits_quota");
    long long now = static_cast<long long>(time(nullptr));
    std::ofstream ofs("/data/log/hiprofiler/profiling_rate_limits_quota");
    ofs << "bundle_quota_test " << now << " 2\n";
    ofs.close();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    config->bundleName_ = "bundle_quota_test";
    EXPECT_EQ(service->StartPidVoid(config, nullptr),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_DAILY_QUOTA_EXCEEDED));
    unlink("/data/log/hiprofiler/profiling_rate_limits_quota");
}

/**
 * @tc.name: StartServiceAbilityMockResult
 * @tc.desc: Start service ability mock result.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartServiceAbilityMockResult, TestSize.Level2)
{
    NativeMemoryProfilerSaService::SetMockStartServiceAbilityForTest(true);
    EXPECT_TRUE(NativeMemoryProfilerSaService::StartServiceAbility());
    NativeMemoryProfilerSaService::SetMockStartServiceAbilityForTest(false);
    EXPECT_FALSE(NativeMemoryProfilerSaService::StartServiceAbility());
    NativeMemoryProfilerSaService::SetMockStartServiceAbilityForTest(true);
}

/**
 * @tc.name: CheckSystemResourceRejectsResourcePressure
 * @tc.desc: Check system resource rejects resource pressure.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckSystemResourceRejectsResourcePressure, TestSize.Level2)
{
    NativeMemoryProfilerSaService::SetMockSystemResourceForTest(true, false, false);
    EXPECT_EQ(NativeMemoryProfilerSaService::CheckSystemResource(),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_MEM_PRESSURE_CRITICAL));
    NativeMemoryProfilerSaService::SetMockSystemResourceForTest(false, true, false);
    EXPECT_EQ(NativeMemoryProfilerSaService::CheckSystemResource(),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CPU_OVERLOADED));
    NativeMemoryProfilerSaService::SetMockSystemResourceForTest(false, false, true);
    EXPECT_EQ(NativeMemoryProfilerSaService::CheckSystemResource(),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_STORAGE_PRESSURE_CRITICAL));
    NativeMemoryProfilerSaService::SetMockSystemResourceForTest(false, false, false);
}

/**
 * @tc.name: GetTaskNumConfigNull
 * @tc.desc: GetTaskNum config is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumConfigNull, TestSize.Level3)
{
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = nullptr;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    int32_t result = service->GetTaskNum(config);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GetTaskNumApiTypePidNotFound
 * @tc.desc: Get task num API type PID not found.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumApiTypePidNotFound, TestSize.Level3)
{
    int32_t startTypeApi = 1;
    int32_t notFoundPid = 999;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->startType_ = startTypeApi;
    config->pid_ = notFoundPid;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    int32_t result = service->GetTaskNum(config);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GetTaskNumHiviewTypeEmpty
 * @tc.desc: Get task num hiview type empty.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumHiviewTypeEmpty, TestSize.Level3)
{
    uint32_t startTypeHiview = 0;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->startType_ = startTypeHiview;
    
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();

    int32_t result = service->GetTaskNum(config);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: CheckSystemResourceTest
 * @tc.desc: CheckSystemResourceTest
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckSystemResourceTest, TestSize.Level3)
{
    constexpr uint64_t memThreshold = 2ULL * 1024 * 1024;
    constexpr double cpuOverLimit = 0.71;
    constexpr double cpuNormal = 0.7;
    constexpr uint64_t romTotal = 100ULL * 1024 * 1024 * 1024;
    EXPECT_TRUE(NativeMemoryProfilerSaService::IsMemoryPressureCritical(memThreshold - 1, true));
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsMemoryPressureCritical(memThreshold, true));
    EXPECT_TRUE(NativeMemoryProfilerSaService::IsMemoryPressureCritical(memThreshold, false));
    EXPECT_TRUE(NativeMemoryProfilerSaService::IsCpuOverloaded(cpuOverLimit, true));
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsCpuOverloaded(cpuNormal, true));
    EXPECT_TRUE(NativeMemoryProfilerSaService::IsCpuOverloaded(cpuNormal, false));
    EXPECT_TRUE(NativeMemoryProfilerSaService::IsRomInsufficient(0, romTotal, true));
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsRomInsufficient(romTotal, romTotal, true));
    EXPECT_TRUE(NativeMemoryProfilerSaService::IsRomInsufficient(romTotal, romTotal, false));
}

/**
 * @tc.name: GetSystemMemoryInfoTest001
 * @tc.desc: invalid params
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetSystemMemoryInfoTest001, TestSize.Level3)
{
    MemoryInfo* memInfo = nullptr;
    bool result = NativeMemoryProfilerSaService::GetSystemMemoryInfo(memInfo);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetSystemMemoryInfoTest002
 * @tc.desc: normal case
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetSystemMemoryInfoTest002, TestSize.Level3)
{
    MemoryInfo memInfo;
    bool result = NativeMemoryProfilerSaService::GetSystemMemoryInfo(&memInfo);
    if (result) {
        EXPECT_GT(memInfo.totalMem, 0);
        EXPECT_GT(memInfo.usedMem, 0);
        EXPECT_GT(memInfo.freeMem, 0);
        EXPECT_GT(memInfo.availablePercent, 0.0);
    }
}

/**
 * @tc.name: GetSystemCpuUsageInfoTest001
 * @tc.desc: invalid params
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetSystemCpuUsageInfoTest001, TestSize.Level3)
{
    double* cpuUsage = nullptr;
    bool result = NativeMemoryProfilerSaService::GetSystemCpuUsageInfo(cpuUsage);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetSystemCpuUsageInfoTest002
 * @tc.desc: normal case
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetSystemCpuUsageInfoTest002, TestSize.Level3)
{
    double cpuUsage = 0.0;
    bool result = NativeMemoryProfilerSaService::GetSystemCpuUsageInfo(&cpuUsage);
    if (result) {
        EXPECT_GE(cpuUsage, 0.0);
        EXPECT_LE(cpuUsage, 100.0);
    }
}

/**
 * @tc.name: ParseCpuStatsValidContent
 * @tc.desc: ParseCpuStats parses aggregate CPU line.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ParseCpuStatsValidContent, TestSize.Level2)
{
    unsigned long total = 0;
    unsigned long idle = 0;
    std::string content = "cpu  1 2 3 4 5 6 7 8 9 10\ncpu0 1 1 1 1 1 1 1 1 1 1\n";
    EXPECT_TRUE(NativeMemoryProfilerSaService::ParseCpuStats(content, total, idle));
    EXPECT_EQ(total, 36UL);
    EXPECT_EQ(idle, 9UL);
}

/**
 * @tc.name: ParseCpuStatsRejectsMissingCpuLine
 * @tc.desc: ParseCpuStats rejects content without aggregate CPU line.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ParseCpuStatsRejectsMissingCpuLine, TestSize.Level2)
{
    unsigned long total = 0;
    unsigned long idle = 0;
    EXPECT_FALSE(NativeMemoryProfilerSaService::ParseCpuStats("intr 1 2 3\n", total, idle));
}

/**
 * @tc.name: ParseCpuStatsRejectsEmptyContent
 * @tc.desc: ParseCpuStats returns false when content has no first line.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ParseCpuStatsRejectsEmptyContent, TestSize.Level2)
{
    unsigned long total = 0;
    unsigned long idle = 0;
    EXPECT_FALSE(NativeMemoryProfilerSaService::ParseCpuStats("", total, idle));
}

/**
 * @tc.name: CheckTaskLimitPassesWhenUnderLimit
 * @tc.desc: CheckTaskLimit returns RET_OK when task count is within limits.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckTaskLimitPassesWhenUnderLimit, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->extendProcesses_ = false;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckTaskLimit(config), RET_OK);
}

/**
 * @tc.name: CheckStartupModeConflictReturnsOkWhenNoStartupMode
 * @tc.desc: CheckStartupModeConflict returns RET_OK when no startup mode is active.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckStartupModeConflictReturnsOkWhenNoStartupMode, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->startupMode_ = false;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckStartupModeConflict(config), RET_OK);
}

/**
 * @tc.name: CheckPidNameConflictReturnsOkForUniquePid
 * @tc.desc: CheckPidNameConflict returns RET_OK when PID is not in use.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckPidNameConflictReturnsOkForUniquePid, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = getpid();

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckPidNameConflict(config), RET_OK);
}

/**
 * @tc.name: CheckFilePathSkipsWhenFdProvided
 * @tc.desc: CheckFilePath returns RET_OK when fd > 0 (file-cache mode).
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckFilePathSkipsWhenFdProvided, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filePath_.clear();

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckFilePath(STDOUT_FILENO, config), RET_OK);
}

/**
 * @tc.name: CheckFilePathRejectsEmptyPath
 * @tc.desc: CheckFilePath returns RET_ERR when filePath_ is empty and fd is 0.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckFilePathRejectsEmptyPath, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filePath_.clear();

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckFilePath(0, config), RET_ERR);
}


/**
 * @tc.name: GetCmdArgsIncludesConfiguredFields
 * @tc.desc: GetCmdArgs reports configured boolean and numeric fields.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetCmdArgsIncludesConfiguredFields, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filterSize_ = 32;
    config->mallocDisable_ = true;
    config->mmapDisable_ = true;
    config->asyncStackEnable_ = true;
    config->nmdPid_ = 1234;
    config->nmdType_ = 2;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    std::string args = service->GetCmdArgs(config);
    EXPECT_NE(args.find("filter_size: 32"), std::string::npos);
    EXPECT_NE(args.find("malloc_disable: true"), std::string::npos);
    EXPECT_NE(args.find("asyncStack_enable: true"), std::string::npos);
    EXPECT_NE(args.find("nmd_pid: 1234"), std::string::npos);
}


/**
 * @tc.name: GetCmdArgsReportsEnabledBooleanFields
 * @tc.desc: Get cmd args reports enabled boolean fields.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetCmdArgsReportsEnabledBooleanFields, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->freeStackData_ = true;
    config->munmapStackData_ = true;
    config->stringCompressed_ = true;
    config->fpUnwind_ = true;
    config->blocked_ = true;
    config->recordAccurately_ = true;
    config->startupMode_ = true;
    config->memtraceEnable_ = true;
    config->offlineSymbolization_ = true;
    config->callframeCompress_ = true;
    config->responseLibraryMode_ = true;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    std::string args = service->GetCmdArgs(config);
    EXPECT_NE(args.find("free_stack_report: true"), std::string::npos);
    EXPECT_NE(args.find("munmap_stack_report: true"), std::string::npos);
    EXPECT_NE(args.find("string_compressed: true"), std::string::npos);
    EXPECT_NE(args.find("fp_unwind: true"), std::string::npos);
    EXPECT_NE(args.find("response_library_mode: true"), std::string::npos);
}


/**
 * @tc.name: CheckConfigRejectsMissingFilePath
 * @tc.desc: CheckConfig rejects config without output path when fd is empty.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigRejectsMissingFilePath, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filePath_.clear();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->CheckConfig(config), RET_ERR);
}


/**
 * @tc.name: CheckConfigAcceptsExistingFd
 * @tc.desc: CheckConfig accepts valid process config when caller provides fd.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigAcceptsExistingFd, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filePath_.clear();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->CheckConfig(config, STDOUT_FILENO), RET_OK);
}


/**
 * @tc.name: ApplyNmdConfigUsesPrintMode
 * @tc.desc: ApplyNmdConfig returns whether hook should be skipped by NMD mode.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ApplyNmdConfigUsesPrintMode, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_FALSE(service->ApplyNmdConfig(config));
    config->printNmd_ = true;
    config->printNmdOnly_ = false;
    config->nmdPid_ = 4321;
    EXPECT_TRUE(service->ApplyNmdConfig(config));
    config->printNmdOnly_ = true;
    EXPECT_FALSE(service->ApplyNmdConfig(config));
}


/**
 * @tc.name: ApplyMemtraceConfigUpdatesGpuRange
 * @tc.desc: ApplyMemtraceConfig copies GPU ranges used by client config.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ApplyMemtraceConfigUpdatesGpuRange, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->memtraceEnable_ = true;
    config->gpuRange_[GPU_VK_INDEX][0] = 11;
    config->gpuRange_[GPU_GLES_IMAGE_INDEX][1] = 22;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    service->ApplyMemtraceConfig(config);

    ClientConfig clientConfig;
    service->SetGpuConfig(clientConfig);
    EXPECT_EQ(clientConfig.gpuRange.gpuVk[0], 11u);
    EXPECT_EQ(clientConfig.gpuRange.gpuGlesImage[1], 22u);
}


/**
 * @tc.name: ApplyMemtraceConfigIgnoresDisabledConfig
 * @tc.desc: ApplyMemtraceConfig keeps previous GPU ranges when memtrace is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ApplyMemtraceConfigIgnoresDisabledConfig, TestSize.Level2)
{
    auto enabledConfig = MakeSaConfig();
    auto disabledConfig = MakeSaConfig();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(enabledConfig, nullptr);
    ASSERT_NE(disabledConfig, nullptr);
    ASSERT_NE(service, nullptr);

    enabledConfig->memtraceEnable_ = true;
    enabledConfig->gpuRange_[GPU_CL_BUFFER_INDEX][0] = 31;
    enabledConfig->gpuRange_[GPU_GLES_IMAGE_INDEX][1] = 42;
    service->ApplyMemtraceConfig(enabledConfig);

    disabledConfig->memtraceEnable_ = false;
    disabledConfig->gpuRange_[GPU_CL_BUFFER_INDEX][0] = 99;
    disabledConfig->gpuRange_[GPU_GLES_IMAGE_INDEX][1] = 100;
    service->ApplyMemtraceConfig(disabledConfig);

    ClientConfig clientConfig;
    service->SetGpuConfig(clientConfig);
    EXPECT_EQ(clientConfig.gpuRange.gpuClBuffer[0], 31u);
    EXPECT_EQ(clientConfig.gpuRange.gpuGlesImage[1], 42u);
}


/**
 * @tc.name: RefreshFileCacheModeConfigEnablesFlag
 * @tc.desc: RefreshFileCacheModeConfig enables file cache mode.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, RefreshFileCacheModeConfigEnablesFlag, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_FALSE(config->enableFileCacheMode_);
    service->RefreshFileCacheModeConfig(config);
    EXPECT_TRUE(config->enableFileCacheMode_);
}

