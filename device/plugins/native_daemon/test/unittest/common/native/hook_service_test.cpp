/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "hook_service.h"
#include "hook_socket_client.h"
#include "socket_context.h"
#include "share_memory_allocator.h"
#include "event_notifier.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
const int SLEEP_TIME = 30000;

class HookServiceTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.name: ProtocolProc
 * @tc.desc: test HookService::ProtocolProc with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceTest, ProtocolProc, TestSize.Level1)
{
    auto shareMemoryBlock = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("hooknativesmb", 4096);
    ASSERT_TRUE(shareMemoryBlock != nullptr);
    auto eventNotifier = EventNotifier::Create(0, EventNotifier::NONBLOCK);
    ASSERT_TRUE(eventNotifier != nullptr);
    int smbFd = shareMemoryBlock->GetfileDescriptor();
    int eventFd = eventNotifier->GetFd();
    SocketContext socketContext;
    std::vector<unsigned char> buf(4096);
    struct ProtocolHead* pph = (struct ProtocolHead*)buf.data();
    uint32_t head_size = sizeof(struct ProtocolHead);
    ClientConfig clientConfig;
    auto hookService = std::make_shared<HookService>(smbFd, eventFd, 0, "", clientConfig);
    ASSERT_TRUE(hookService != nullptr);
    ASSERT_TRUE(hookService->ProtocolProc(socketContext, 0, pph->datas, head_size));
}
} // namespace
