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

#ifndef HOOK_BOUNDARY_TEST_H
#define HOOK_BOUNDARY_TEST_H

#include <gtest/gtest.h>
#include "hook_manager.h"
#include "native_hook_config.pb.h"

namespace OHOS::Developtools::NativeDaemon {
class HookBoundaryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<HookManager> hookManager_ = nullptr;
    NativeHookConfig hookConfig_;
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // HOOK_BOUNDARY_TEST_H
