/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include <dlfcn.h>
#include <gtest/gtest.h>
#include "address_handler.h"
#include "init_param.h"

using namespace testing::ext;

namespace {

constexpr uint64_t TEST_ADDR = 64;
class AddressHandlerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: LowAddrHandlerTest
 * @tc.desc: Test LowAddrHandler.
 * @tc.type: FUNC
 */
HWTEST_F(AddressHandlerTest, LowAddrHandlerTest, TestSize.Level1)
{
    std::unique_ptr<LowAddrHandler> handler = std::make_unique<LowAddrHandler>();
    uint64_t noExistaddr = TEST_ADDR - 1;
    handler->AddAllocAddr(TEST_ADDR);
    ASSERT_TRUE(handler->CheckAddr(TEST_ADDR));
    ASSERT_FALSE(handler->CheckAddr(noExistaddr));
    handler = nullptr;
}

/**
 * @tc.name: MidAddrHandlerTest
 * @tc.desc: Test MidAddrHandler.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(AddressHandlerTest, MidAddrHandlerTest, TestSize.Level1)
{
    std::unique_ptr<MidAddrHandler> handler = std::make_unique<MidAddrHandler>();
    uint64_t addr = 0xFF00000;
    uint64_t noExistaddr = addr - 1;
    handler->AddAllocAddr(addr);
    ASSERT_TRUE(handler->CheckAddr(addr));
    ASSERT_FALSE(handler->CheckAddr(noExistaddr));
    handler = nullptr;
}
#endif

/**
 * @tc.name: WholeAddrHandlerTest
 * @tc.desc: Test WholeAddrHandler.
 * @tc.type: FUNC
 */
HWTEST_F(AddressHandlerTest, WholeAddrHandlerTest, TestSize.Level1)
{
    std::unique_ptr<WholeAddrHandler> handler = std::make_unique<WholeAddrHandler>();
    uint64_t noExistaddr = TEST_ADDR - 1;
    handler->AddAllocAddr(TEST_ADDR);
    ASSERT_TRUE(handler->CheckAddr(TEST_ADDR));
    ASSERT_FALSE(handler->CheckAddr(noExistaddr));
    handler = nullptr;
}

/**
 * @tc.name: LowMidAddrHandlerTest
 * @tc.desc: Test LowAddrHandler and MidAddrHandler.
 * @tc.type: FUNC
 */
HWTEST_F(AddressHandlerTest, LowMidAddrHandlerTest, TestSize.Level1)
{
    std::unique_ptr<LowAddrHandler> handlerLow = std::make_unique<LowAddrHandler>();
    std::unique_ptr<MidAddrHandler> handlerMid = std::make_unique<MidAddrHandler>();
    uint64_t addr = 0xFF000000000;
    uint64_t noExistaddr = 0xFE000000000;
    handlerLow->AddAllocAddr(addr);
    handlerMid->AddAllocAddr(addr);
    handlerLow->SetSuccessor(std::move(handlerMid));
    ASSERT_TRUE(handlerLow->CheckAddr(addr));
    ASSERT_FALSE(handlerLow->CheckAddr(noExistaddr));
    handlerLow = nullptr;
    handlerMid = nullptr;
}

/**
 * @tc.name: LowWholeAddrHandlerTest
 * @tc.desc: Test LowAddrHandler and WholeAddrHandler.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(AddressHandlerTest, LowWholeAddrHandlerTest, TestSize.Level1)
{
    std::unique_ptr<LowAddrHandler> handlerLow = std::make_unique<LowAddrHandler>();
    std::unique_ptr<WholeAddrHandler> handlerWhole = std::make_unique<WholeAddrHandler>();
    uint64_t addr = 0xFF000000000;
    uint64_t noExistaddr = 0xFE000000000;
    handlerLow->AddAllocAddr(addr);
    handlerWhole->AddAllocAddr(addr);
    handlerLow->SetSuccessor(std::move(handlerWhole));
    ASSERT_TRUE(handlerLow->CheckAddr(addr));
    ASSERT_FALSE(handlerLow->CheckAddr(noExistaddr));
    handlerLow = nullptr;
    handlerWhole = nullptr;
}
#endif

/**
 * @tc.name: MidWholeAddrHandlerTest
 * @tc.desc: Test MidAddrHandler and WholeAddrHandler.
 * @tc.type: FUNC
 */
HWTEST_F(AddressHandlerTest, MidWholeAddrHandlerTest, TestSize.Level1)
{
    std::unique_ptr<MidAddrHandler> handlerMid = std::make_unique<MidAddrHandler>();
    std::unique_ptr<WholeAddrHandler> handlerWhole = std::make_unique<WholeAddrHandler>();
    uint64_t addr = TEST_ADDR;
    uint64_t noExistaddr = addr - 1;
    handlerMid->AddAllocAddr(addr);
    handlerWhole->AddAllocAddr(addr);
    handlerMid->SetSuccessor(std::move(handlerWhole));
    ASSERT_TRUE(handlerMid->CheckAddr(addr));
    ASSERT_FALSE(handlerMid->CheckAddr(noExistaddr));
    handlerMid = nullptr;
    handlerWhole = nullptr;
}

/**
 * @tc.name: AllAddrHandlerTest
 * @tc.desc: Test LowAddrHandler, MidAddrHandler and WholeAddrHandler.
 * @tc.type: FUNC
 */
HWTEST_F(AddressHandlerTest, AllAddrHandlerTest, TestSize.Level1)
{
    std::unique_ptr<LowAddrHandler> handlerLow = std::make_unique<LowAddrHandler>();
    std::unique_ptr<MidAddrHandler> handlerMid = std::make_unique<MidAddrHandler>();
    std::unique_ptr<WholeAddrHandler> handlerWhole = std::make_unique<WholeAddrHandler>();
    uint64_t addr = 0xFF0000000000000;
    uint64_t noExistaddr = 0xEF0000000000000;
    handlerLow->AddAllocAddr(addr);
    handlerWhole->AddAllocAddr(addr);
    handlerMid->AddAllocAddr(addr);
    ASSERT_TRUE(handlerLow->CheckAddr(addr));
    handlerMid->SetSuccessor(std::move(handlerWhole));
    handlerLow->SetSuccessor(std::move(handlerMid));
    ASSERT_TRUE(handlerLow->CheckAddr(addr));
    ASSERT_FALSE(handlerLow->CheckAddr(noExistaddr));
    handlerLow = nullptr;
    handlerMid = nullptr;
    handlerWhole = nullptr;
}
}