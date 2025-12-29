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

#ifndef HOOK_RECORD_FULL_TEST_H
#define HOOK_RECORD_FULL_TEST_H

#include <gtest/gtest.h>
#include "native_hook_result_standard.pb.h"
#include "native_hook_config_standard.pb.h"
#include "google/protobuf/text_format.h"

/** First construct the string, then parse the string into an object,
and verify that the values of each field in the object are consistent with those defined in the string.
 */
namespace {
class HookRecordFullTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};
}
#endif