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
#include "musl_malloc_dispatch.h"

#define EXPORT_API __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_API bool ohos_malloc_hook_initialize(const MallocDispatchType*, bool*, const char*);
EXPORT_API bool ohos_malloc_hook_get_hook_flag(void);
EXPORT_API bool ohos_malloc_hook_set_hook_flag(bool);
EXPORT_API void ohos_malloc_hook_finalize(void);
EXPORT_API bool ohos_malloc_hook_on_start(void);
EXPORT_API bool ohos_malloc_hook_on_end(void);
EXPORT_API void* ohos_malloc_hook_malloc(size_t);
EXPORT_API void* ohos_malloc_hook_realloc(void*, size_t);
EXPORT_API void* ohos_malloc_hook_calloc(size_t, size_t);
EXPORT_API void* ohos_malloc_hook_valloc(size_t);
EXPORT_API void ohos_malloc_hook_free(void*);
EXPORT_API void* ohos_malloc_hook_memalign(size_t, size_t);
EXPORT_API size_t ohos_malloc_hook_malloc_usable_size(void*);
EXPORT_API void* ohos_malloc_hook_mmap(void*, size_t, int, int, int, off_t);
EXPORT_API int ohos_malloc_hook_munmap(void*, size_t);
EXPORT_API void ohos_malloc_hook_memtrace(void*, size_t, const char*, bool);
EXPORT_API int ohos_malloc_hook_prctl(int option, unsigned long, unsigned long, unsigned long, unsigned long);
EXPORT_API bool ohos_set_filter_size(size_t size, void* ret);

#ifdef __cplusplus
}
#endif


#endif /* __OHOS_MALLOC_HOOK_CLIENT_H__ */