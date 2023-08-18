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
#ifndef FTRACE_NAMESPACE_H
#define FTRACE_NAMESPACE_H
#include "nocopyable.h"

#if __cplusplus >= 201703L
#define FTRACE_NS_BEGIN namespace OHOS::Profiler::Plugins {
#else
#define FTRACE_NS_BEGIN  \
    namespace OHOS {     \
    namespace Profiler { \
    namespace Plugins {
#endif

#if __cplusplus >= 201703L
#define FTRACE_NS_END } /* OHOS::Profiler::Plugins */
#else
#define FTRACE_NS_END \
    } /* Plugins */   \
    } /* Profiler */  \
    } /* OHOS */
#endif

#define FTRACE_NS OHOS::Profiler::Plugins

#endif // FTRACE_NAMESPACE_H