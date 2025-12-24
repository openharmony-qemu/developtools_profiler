/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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

#ifndef HOOK_SOCKET_CLIENT_MOCK_H
#define HOOK_SOCKET_CLIENT_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include "musl_preinit_common.h"
#include "hook_client.h"
#include "hook_socket_client.h"
#include "init_param.h"
#include <memory_trace.h>

class MockHookSocketClient : public HookSocketClient {
public:
    explicit MockHookSocketClient(void (*disableHookCallback)() = nullptr) : HookSocketClient(
        g_hookPid.load(), &g_clientConfig, &g_targetedRange, &g_sharedMemCount, disableHookCallback) {};
    MOCK_METHOD5(SendStackWithPayload, bool(const void* data, size_t size, const void* payload,
        size_t payloadSize, int smbIndex));
};

#endif // HOOK_SOCKET_CLIENT_MOCK_H
