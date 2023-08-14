/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#ifndef HIVIEWDFX_HIDEBUG_BASE_H
#define HIVIEWDFX_HIDEBUG_BASE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize environment variable parameters.
 *
 * When temporarily starting the debugging function,developers can set a set of variables,\
 * including the target process and variable key value pairs,through the command line.\
 * restarting the service or app allows variables to take effect in the specified process.
 *
 * @return When param is successfully settded,it returns true,else return false.
 */
bool InitEnvironmentParam(const char *serviceName);

#ifdef __cplusplus
}
#endif

#endif  // HIVIEWDFX_HIDEBUG_BASE_H
