/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef __RUNTIME_STACK_RANGE_H__
#define __RUNTIME_STACK_RANGE_H__

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

constexpr int INIT_LINE_SIZE = 1000;
constexpr int INC_LINE_SIZE = 100;

void GetRuntimeStackEnd(const char* stackptr, const char** end, pid_t pid, pid_t tid);
void GetMainThreadRuntimeStackRange(void);

#ifdef __cplusplus
}
#endif

#endif  /* __RUNTIME_STACK_RANGE_H__ */
