/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include "hook_service_exception_test.h"
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
void HookServiceExceptionTest::SetUpTestCase(void) {}
void HookServiceExceptionTest::TearDownTestCase(void) {}

void HookServiceExceptionTest::SetUp()
{
    hookManager_ = std::make_shared<HookManager>();
    clientConfig_ = ClientConfig();
}

void HookServiceExceptionTest::TearDown()
{
    hookService_ = nullptr;
    hookManager_ = nullptr;
}

/*
 * @tc.name: ProtocolProcWithInvalidDataSize
 * @tc.desc: test ProtocolProc with invalid data size.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithInvalidDataSize, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    int pid = 1;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, 0));
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, 1));
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, 100));
}

/*
 * @tc.name: ProtocolProcWithNullHookManager
 * @tc.desc: test ProtocolProc when hookMgr_ is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithNullHookManager, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, nullptr);
    SocketContext socketContext;
    int pid = 1;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}

/*
 * @tc.name: ProtocolProcWithInvalidPidNegative
 * @tc.desc: test ProtocolProc with negative PID (-1).
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithInvalidPidNegative, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    int pid = -1;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}

/*
 * @tc.name: RemovePidInfoWithInvalidPid
 * @tc.desc: test RemovePidInfo with invalid PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, RemovePidInfoWithInvalidPid, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    hookService_->RemovePidInfo(999999);
    hookService_->RemovePidInfo(-1);
    hookService_->RemovePidInfo(0);
    EXPECT_TRUE(hookService_->pidInfo_.empty());
}

/*
 * @tc.name: AddAndRemovePidInfo
 * @tc.desc: test AddPidInfo and RemovePidInfo.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, AddAndRemovePidInfo, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    pid_t testPid = 12345;
    uid_t testUid = 1000;
    gid_t testGid = 1000;
    hookService_->AddPidInfo(testPid, testUid, testGid);
    EXPECT_EQ(hookService_->pidInfo_.size(), 1);
    EXPECT_EQ(hookService_->pidInfo_[testPid].first, testUid);
    EXPECT_EQ(hookService_->pidInfo_[testPid].second, testGid);
    hookService_->RemovePidInfo(testPid);
    EXPECT_TRUE(hookService_->pidInfo_.empty());
}

/*
 * @tc.name: ProtocolProcWithZeroPid
 * @tc.desc: test ProtocolProc with zero PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithZeroPid, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    int pid = 0;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}

/*
 * @tc.name: ProtocolProcWithMaxPid
 * @tc.desc: test ProtocolProc with INT_MAX PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithMaxPid, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    int pid = INT_MAX;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}

/*
 * @tc.name: AddDuplicatePidInfo
 * @tc.desc: test AddPidInfo with duplicate PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, AddDuplicatePidInfo, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    pid_t testPid = 12345;
    hookService_->AddPidInfo(testPid, 1000, 1000);
    hookService_->AddPidInfo(testPid, 2000, 2000);
    EXPECT_EQ(hookService_->pidInfo_.size(), 1);
    EXPECT_EQ(hookService_->pidInfo_[testPid].first, 2000);
}
} // namespace OHOS::Developtools::NativeDaemon
