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

#ifndef SHARE_MEMORY_EXCEPTION_TEST_H
#define SHARE_MEMORY_EXCEPTION_TEST_H

#include <gtest/gtest.h>
#include "share_memory_block.h"

namespace OHOS::Developtools::NativeDaemon {
class ShareMemoryExceptionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<ShareMemoryBlock> shareMemoryBlock_ = nullptr;
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // SHARE_MEMORY_EXCEPTION_TEST_H
