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

#ifndef __OHOS_MALLOC_HOOK_CLIENT_H__
#define __OHOS_MALLOC_HOOK_CLIENT_H__

#include <cstdlib>
#include <pthread.h>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include "musl_malloc_dispatch.h"
#include "hook_common.h"
#include "hook_socket_client.h"

struct ClientConfig;
class Sampling;

#define EXPORT_API __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

extern pthread_key_t g_updateThreadNameCount;
extern std::atomic<pid_t> g_hookPid;
extern std::atomic<bool> g_hookReady;
extern std::atomic<bool> g_isPidChanged;
extern ClientConfig g_clientConfig;
extern std::mutex g_usableSizeMapMutex;
extern std::unordered_map<size_t, size_t> g_mallocUsableSizeMap;
extern std::atomic<Range> g_targetedRange;
extern std::vector<std::pair<uint64_t, uint64_t>> g_filterStaLibRange;
extern std::recursive_timed_mutex g_FilterMapMutex;
extern std::shared_ptr<HookSocketClient> g_hookClient;
extern std::atomic<int> g_sharedMemCount;

pid_t GetCurThreadId();

EXPORT_API bool ohos_malloc_hook_initialize(const MallocDispatchType*, bool*, const char*);
EXPORT_API bool ohos_malloc_hook_get_hook_flag(void);
EXPORT_API bool ohos_malloc_hook_set_hook_flag(bool);
EXPORT_API void ohos_malloc_hook_finalize(void);
EXPORT_API bool ohos_malloc_hook_on_start(void (*disableHookCallback)());
EXPORT_API bool ohos_malloc_hook_on_end(void);
EXPORT_API void* ohos_malloc_hook_malloc(size_t);
EXPORT_API void* ohos_malloc_hook_realloc(void*, size_t);
EXPORT_API void* ohos_malloc_hook_calloc(size_t, size_t);
EXPORT_API void* ohos_malloc_hook_valloc(size_t);
EXPORT_API void ohos_malloc_hook_free(void*);
EXPORT_API void* ohos_malloc_hook_aligned_alloc(size_t, size_t);
EXPORT_API size_t ohos_malloc_hook_malloc_usable_size(void*);
EXPORT_API void* ohos_malloc_hook_mmap(void*, size_t, int, int, int, off_t);
EXPORT_API int ohos_malloc_hook_munmap(void*, size_t);
EXPORT_API void ohos_malloc_hook_memtrace(void*, size_t, const char*, bool);
EXPORT_API void ohos_malloc_hook_restrace(unsigned long long, void*, size_t, const char*, bool);
EXPORT_API void ohos_malloc_hook_resTraceMove(unsigned long long mask, void* oldAddr, void* newAddr, size_t newSize);
EXPORT_API void ohos_malloc_hook_resTraceFreeRegion(unsigned long long mask, void* addr, size_t size);
EXPORT_API int ohos_malloc_hook_prctl(int option, unsigned long, unsigned long, unsigned long, unsigned long);
EXPORT_API bool ohos_set_filter_size(size_t size, void* ret);
EXPORT_API bool ohos_malloc_hook_send_hook_misc_data(uint64_t, const char*, size_t, uint32_t);
EXPORT_API void* ohos_malloc_hook_get_hook_config();

#ifdef __cplusplus
}
#endif


#endif /* __OHOS_MALLOC_HOOK_CLIENT_H__ */