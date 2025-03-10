/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef NATIVE_MEMORY_PROFILER_SA_INTERFACE_CODE_H
#define NATIVE_MEMORY_PROFILER_SA_INTERFACE_CODE_H

namespace OHOS::Developtools::NativeDaemon {
enum class NativeMemoryProfilerSaInterfaceCode {
    START = 0,
    STOP_HOOK_PID = 1,
    STOP_HOOK_NAME = 2,
    DUMP_DATA = 3,
    DUMP_SIMP_DATA = 4,
};
} // namespace OHOS::Developtools::NativeDaemon

#endif // NATIVE_MEMORY_PROFILER_SA_INTERFACE_CODE_H