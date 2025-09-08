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
#ifndef HOOK_SET_FUZZER
#define HOOK_SET_FUZZER

#define FUZZ_PROJECT_NAME "hookset_fuzzer"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <dlfcn.h>
#include <mutex>
#include <securec.h>
#include <string>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <vector>

#include "hook_client.h"
#include "init_param.h"
#include "musl_preinit_common.h"

#endif