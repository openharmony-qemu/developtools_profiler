/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#ifndef MEM_LEAK_DETECTOR
#define MEM_LEAK_DETECTOR
#include <vector>
#include <dlfcn.h>
#include <inttypes.h>

using GetMemLeakStringFunc = bool (*)(int, std::string&, size_t);

namespace {
constexpr const char *const SO_NAME = "";
constexpr size_t LIMIT_SIZE = 10000;
constexpr const char *const GET_MEM_LEAK_STRING_SYMBOL =
    "_ZN4OHOS16GetMemLeakStringEiRNSt3__h12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEEm";
}
class MemoryLeakDetector {
public:
    MemoryLeakDetector()
    {
        handle_ = dlopen(SO_NAME, RTLD_LAZY);
        if (!handle_) {
            PROFILER_LOG_ERROR(LOG_CORE, "Error loading %s, error: %s", SO_NAME, dlerror());
            return;
        }
        func_ = reinterpret_cast<GetMemLeakStringFunc>(dlsym(handle_, GET_MEM_LEAK_STRING_SYMBOL));
        if (!func_) {
            PROFILER_LOG_ERROR(LOG_CORE, "Failed to resolve symbols: %s, error: %s",
                GET_MEM_LEAK_STRING_SYMBOL, dlerror());
            dlclose(handle_);
            handle_ = nullptr;
            return;
        }
        PROFILER_LOG_INFO(LOG_CORE, "Success to dlopen %s", SO_NAME);
    }

    ~MemoryLeakDetector()
    {
        if (handle_) {
            dlclose(handle_);
        }
    }

    bool GetMemLeakInfo(int format, std::string &result)
    {
        PROFILER_LOG_INFO(LOG_CORE, "MemoryLeakDetector::GetMemLeakInfo start, type: %d", format);
        if (func_ == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "MemoryLeakDetector::GetMemLeakInfo func_ is nullptr");
            return false;
        }
        return func_(format, result, LIMIT_SIZE);
    }

private:
    void *handle_ = nullptr;
    GetMemLeakStringFunc func_ = nullptr;
};

#endif // MEM_LEAK_DETECTOR