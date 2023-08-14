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
 *
 * Description: AtraceOps define
 */
#ifndef FTRACE_MODULE_H
#define FTRACE_MODULE_H

#include <cstdint>

#include "ftrace_namespace.h"
#include "plugin_module_api.h"

int TracePluginStartSession(const uint8_t configData[], const uint32_t configSize);

int TracePluginRegisterWriter(const WriterStruct* writer);

int TracePluginStopSession(void);

#endif // FTRACE_MODULE_H